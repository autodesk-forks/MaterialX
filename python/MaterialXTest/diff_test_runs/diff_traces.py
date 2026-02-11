#!/usr/bin/env python
'''
Compare performance traces between baseline and optimized MaterialX test runs.

Reads Perfetto .perfetto-trace files and compares slice durations, generating
tables, charts, and an HTML report.

Two modes:
  --gpu           Compare GPU render durations per material (from GPU async track)
  --slice NAME    Compare CPU slice durations per material (child slices under
                  material parent slices). Multiple names produce multiple charts.

For image comparison, see diff_images.py in the same directory.

Usage:
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu
    python diff_traces.py <baseline_dir> <optimized_dir> --slice GenerateShader
    python diff_traces.py <baseline_dir> <optimized_dir> --slice GenerateShader CompileShader
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu --slice GenerateShader
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu -o custom_name.html
'''

import argparse
import logging
import sys
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_traces')


# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

try:
    from perfetto.trace_processor import TraceProcessor
except ImportError:
    sys.exit('ERROR: perfetto is required. Install with: pip install perfetto')

try:
    import pandas as pd
except ImportError:
    sys.exit('ERROR: pandas is required. Install with: pip install pandas')

# Optional: matplotlib (for chart generation)
_have_matplotlib = False
try:
    import matplotlib.pyplot as plt
    from matplotlib.patches import Patch
    _have_matplotlib = True
except ImportError:
    logger.debug('matplotlib not found. Chart generation disabled.')


# =============================================================================
# TRACE LOADING
# =============================================================================

def findTraceFile(path):
    '''
    Find a Perfetto trace file from a path.

    If path is a file, return it directly.
    If path is a directory, search for *.perfetto-trace files.
    Returns the first trace file found, or raises FileNotFoundError.
    '''
    path = Path(path)

    if path.is_file():
        return path

    if path.is_dir():
        traces = list(path.glob('*.perfetto-trace'))
        if not traces:
            traces = list(path.glob('**/*.perfetto-trace'))
        if traces:
            if len(traces) > 1:
                print(f'Warning: Multiple traces found in {path}, using: {traces[0].name}')
            return traces[0]
        raise FileNotFoundError(f'No .perfetto-trace files found in: {path}')

    raise FileNotFoundError(f'Path not found: {path}')


def loadSliceDurations(traceProcessor, trackName=None):
    '''
    Load slice durations from a Perfetto trace, optionally filtered by track.

    Args:
        traceProcessor: TraceProcessor instance
        trackName: Optional track name filter (e.g., "GPU")

    Returns:
        DataFrame with columns [name, dur_ms].
    '''

    if trackName:
        query = f'''
        SELECT slice.name, slice.dur / 1000000.0 as dur_ms
        FROM slice
        JOIN track ON slice.track_id = track.id
        WHERE track.name = '{trackName}'
        ORDER BY slice.name
        '''
    else:
        query = '''
        SELECT slice.name, slice.dur / 1000000.0 as dur_ms
        FROM slice
        JOIN track ON slice.track_id = track.id
        ORDER BY slice.name
        '''

    df = traceProcessor.query(query).as_pandas_dataframe()
    if df.empty:
        trackMsg = f' on track "{trackName}"' if trackName else ''
        logger.warning(f'No slices found{trackMsg}')
        return pd.DataFrame(columns=['name', 'dur_ms'])
    return df


def loadChildSliceDurations(traceProcessor, sliceName):
    '''
    Load durations of a named child slice, keyed by parent (material) name.

    Queries the trace for slices matching sliceName that are direct children
    of a parent slice (typically the material name).

    Args:
        traceProcessor: TraceProcessor instance
        sliceName: Name of the child slice (e.g., "GenerateShader", "CompileShader")

    Returns:
        DataFrame with columns [name, dur_ms]. 'name' is the parent (material) name.
    '''

    query = f'''
    SELECT parent.name as name, child.dur / 1000000.0 as dur_ms
    FROM slice child
    JOIN slice parent ON child.parent_id = parent.id
    WHERE child.name = '{sliceName}'
    ORDER BY parent.name
    '''

    df = traceProcessor.query(query).as_pandas_dataframe()
    if df.empty:
        logger.warning(f'No "{sliceName}" slices found')
        return pd.DataFrame(columns=['name', 'dur_ms'])
    return df


def loadOptimizationEvents(traceProcessor, optimizationName=None):
    '''
    Load optimization events from a Perfetto trace.

    Optimization events are nested inside ShaderGen slices with hierarchy:
    MaterialName -> GenerateShader -> OptimizationPass

    Args:
        traceProcessor: TraceProcessor instance
        optimizationName: Filter by optimization pass name

    Returns:
        Set of material names that had the optimization applied.
    '''

    if optimizationName:
        query = f'''
        SELECT DISTINCT grandparent.name as material_name
        FROM slice opt
        JOIN slice parent ON opt.parent_id = parent.id
        JOIN slice grandparent ON parent.parent_id = grandparent.id
        WHERE opt.name = "{optimizationName}"
        '''
    else:
        query = '''
        SELECT DISTINCT opt.name as opt_name, grandparent.name as material_name
        FROM slice opt
        JOIN slice parent ON opt.parent_id = parent.id
        JOIN slice grandparent ON parent.parent_id = grandparent.id
        '''

    result = traceProcessor.query(query)

    optimizedMaterials = set()
    for row in result:
        if row.material_name:
            optimizedMaterials.add(row.material_name)

    return optimizedMaterials


# =============================================================================
# COMPARISON
# =============================================================================

def _aggregateByName(df):
    '''Aggregate durations by name (averaging across multiple samples).'''
    if df.empty:
        return pd.DataFrame(columns=['name', 'mean_ms'])
    agg = df.groupby('name')['dur_ms'].mean().reset_index()
    agg.columns = ['name', 'mean_ms']
    return agg


def _mergeAndCompare(baselineAgg, optimizedAgg, minDeltaMs=0.0):
    '''Merge baseline and optimized aggregates, compute delta and percentage.'''
    merged = pd.merge(
        baselineAgg[['name', 'mean_ms']],
        optimizedAgg[['name', 'mean_ms']],
        on='name',
        suffixes=('_baseline', '_optimized'),
        how='outer'
    )
    merged['delta_ms'] = merged['mean_ms_optimized'] - merged['mean_ms_baseline']
    merged['change_pct'] = (merged['delta_ms'] / merged['mean_ms_baseline']) * 100
    if minDeltaMs > 0:
        merged = merged[merged['delta_ms'].abs() >= minDeltaMs]
    merged = merged.sort_values('delta_ms', ascending=True)
    return merged


def compareGpuTraces(baselineTraceProcessor, optimizedTraceProcessor, minDeltaMs=0.0):
    '''
    Compare GPU render durations between baseline and optimized traces.

    Reads the GPU async track and averages per material across frames.

    Returns:
        (merged_df, samplesPerMaterial) -- the comparison DataFrame and the
        typical number of samples per material (for display in titles).
    '''
    logger.info('Comparing GPU traces...')
    baselineData = loadSliceDurations(baselineTraceProcessor, trackName='GPU')
    baselineAgg = _aggregateByName(baselineData)

    optimizedData = loadSliceDurations(optimizedTraceProcessor, trackName='GPU')
    optimizedAgg = _aggregateByName(optimizedData)

    # Determine typical samples per material (frames rendered)
    if not baselineData.empty:
        samplesPerMaterial = int(baselineData.groupby('name').size().median())
    else:
        samplesPerMaterial = 0

    return _mergeAndCompare(baselineAgg, optimizedAgg, minDeltaMs), samplesPerMaterial


def compareChildSlices(baselineTraceProcessor, optimizedTraceProcessor, sliceName, minDeltaMs=0.0):
    '''
    Compare durations of a named child slice (e.g., GenerateShader) per material.

    Queries child slices under material parent slices in both traces,
    then merges by material name and computes delta/percentage.
    '''
    logger.info(f'Comparing "{sliceName}" slices...')
    baselineData = loadChildSliceDurations(baselineTraceProcessor, sliceName)
    baselineAgg = _aggregateByName(baselineData)

    optimizedData = loadChildSliceDurations(optimizedTraceProcessor, sliceName)
    optimizedAgg = _aggregateByName(optimizedData)

    return _mergeAndCompare(baselineAgg, optimizedAgg, minDeltaMs)


# =============================================================================
# OUTPUT: TABLE
# =============================================================================

def _isna(val):
    '''Check if value is None or NaN.'''
    return val is None or pd.isna(val)


def printTraceTable(df, title, baselineName='Baseline', optimizedName='Optimized',
                    optimizedMaterials=None):
    '''Print a formatted trace comparison table to stdout.

    Args:
        df: Comparison DataFrame with columns [name, mean_ms_baseline,
            mean_ms_optimized, delta_ms, change_pct]
        title: Section title printed above the table
        baselineName: Display name for the baseline column
        optimizedName: Display name for the optimized column
        optimizedMaterials: Optional set of material names affected by optimization
    '''
    if df is None or df.empty:
        return

    if optimizedMaterials is None:
        optimizedMaterials = set()

    # Truncate column headers to fit
    bCol = baselineName[:10]
    oCol = optimizedName[:10]

    print(f'\n{"=" * 85}')
    print(f'  {title}')
    print(f'{"=" * 85}')
    marker = ' *' if optimizedMaterials else ''
    print(f"{'Name':<40} {bCol:>10} {oCol:>10} {'Delta':>10} {'Change':>8}{marker}")
    print('-' * 85)

    for _, row in df.iterrows():
        fullName = str(row['name'])
        name = fullName[:38]
        baseMs = row['mean_ms_baseline']
        optMs = row['mean_ms_optimized']
        deltaMs = row['delta_ms']
        changePct = row['change_pct']

        affected = fullName in optimizedMaterials
        marker = ' *' if affected else '  '

        baseline = f'{baseMs:.2f}ms' if not _isna(baseMs) else 'N/A'
        optimized = f'{optMs:.2f}ms' if not _isna(optMs) else 'N/A'
        delta = f'{deltaMs:+.2f}ms' if not _isna(deltaMs) else 'N/A'
        change = f'{changePct:+.1f}%' if not _isna(changePct) else 'N/A'
        print(f'{name:<40} {baseline:>10} {optimized:>10} {delta:>10} {change:>8}{marker}')

    print('-' * 85)

    improved = df[df['change_pct'] < 0]
    regressed = df[df['change_pct'] > 0]
    unchanged = df[df['change_pct'] == 0]
    validChanges = df.dropna(subset=['change_pct'])['change_pct']

    print(f'\nSummary: {len(improved)} improved, {len(regressed)} regressed, '
          f'{len(unchanged)} unchanged, {len(df)} total')

    if len(improved) > 0:
        best = improved.iloc[0]
        print(f"Best improvement: {best['name']} ({best['change_pct']:.1f}%)")

    if len(regressed) > 0:
        worst = regressed.iloc[-1]
        print(f"Worst regression: {worst['name']} ({worst['change_pct']:+.1f}%)")

    if len(validChanges) > 0:
        print(f'Overall: mean {validChanges.mean():+.1f}%, median {validChanges.median():+.1f}%')

    if optimizedMaterials:
        print(f'\n* = affected by optimization ({len(optimizedMaterials)} materials)')


# =============================================================================
# OUTPUT: CHART
# =============================================================================

def createTraceChart(data, outputPath, title,
                     baselineName='Baseline', optimizedName='Optimized',
                     optimizedMaterials=None, optimizationName=None,
                     subtitle=None):
    '''
    Create a paired before/after horizontal bar chart sorted by time saved.

    Args:
        data: DataFrame with comparison results
        outputPath: Path to save the chart image
        title: Chart title (required)
        baselineName: Display name for the baseline series
        optimizedName: Display name for the optimized series
        optimizedMaterials: Set of material names affected by optimization
        optimizationName: Name of the optimization pass (for legend)
        subtitle: Optional subtitle line (e.g., filter parameters)
    '''
    if data is None:
        return

    if optimizedMaterials is None:
        optimizedMaterials = set()
    if not _have_matplotlib:
        logger.error('Cannot create chart: matplotlib not installed.')
        return

    chartDf = data.dropna(subset=['mean_ms_baseline', 'mean_ms_optimized']).copy()
    if chartDf.empty:
        logger.warning('No data to chart')
        return

    # Reverse so largest improvements at TOP
    chartDf = chartDf.iloc[::-1].reset_index(drop=True)
    chartDf['is_optimized'] = chartDf['name'].isin(optimizedMaterials)

    def make_label(row):
        name = row['name'][:28] + '...' if len(row['name']) > 28 else row['name']
        delta = row['delta_ms']
        pct = row['change_pct']
        prefix = '* ' if row['is_optimized'] else ''
        if pd.notna(delta) and pd.notna(pct):
            return f"{prefix}{name} ({delta:+.1f}ms, {pct:+.1f}%)"
        return f"{prefix}{name}"

    chartDf['display_name'] = chartDf.apply(make_label, axis=1)

    figHeight = max(10, len(chartDf) * 0.5)
    fig, ax = plt.subplots(figsize=(14, figHeight))

    y_pos = range(len(chartDf))
    bar_height = 0.35

    ax.barh([y + bar_height/2 for y in y_pos], chartDf['mean_ms_baseline'],
            bar_height, label=baselineName, color='#3498db', alpha=0.8)

    colors = ['#2ecc71' if d < 0 else '#e74c3c' for d in chartDf['delta_ms']]
    ax.barh([y - bar_height/2 for y in y_pos], chartDf['mean_ms_optimized'],
            bar_height, label=optimizedName, color=colors, alpha=0.8)

    for i, (b, o, delta) in enumerate(zip(chartDf['mean_ms_baseline'],
                                           chartDf['mean_ms_optimized'],
                                           chartDf['delta_ms'])):
        ax.text(b + 1, i + bar_height/2, f'{b:.1f}ms', va='center', fontsize=7,
                color='#2980b9')
        ax.text(o + 1, i - bar_height/2, f'{o:.1f}ms', va='center', fontsize=7,
                color='#27ae60' if delta < 0 else '#c0392b')

    ax.set_yticks(y_pos)
    ax.set_yticklabels(chartDf['display_name'])
    ax.set_xlabel('Time (ms)')

    if optimizedMaterials:
        for i, (label, isOpt) in enumerate(zip(ax.get_yticklabels(), chartDf['is_optimized'])):
            if isOpt:
                label.set_fontweight('bold')
                label.set_color('#8e44ad')

    titleLines = [title]
    if optimizationName and optimizedMaterials:
        titleLines.append(f'* = affected by {optimizationName}')
    if subtitle:
        titleLines.append(subtitle)
    ax.set_title('\n'.join(titleLines), fontsize=11)

    legendElements = [
        Patch(facecolor='#3498db', label=baselineName),
        Patch(facecolor='#2ecc71', label=f'{optimizedName} (faster)'),
        Patch(facecolor='#e74c3c', label=f'{optimizedName} (slower)')
    ]
    ax.legend(handles=legendElements, loc='lower right')

    plt.tight_layout()
    plt.savefig(outputPath, dpi=150, bbox_inches='tight')
    plt.close(fig)
    logger.info(f'Chart saved to: {outputPath}')


# =============================================================================
# OUTPUT: HTML REPORT
# =============================================================================

def generateHtmlReport(reportPath, sections, pageTitle='MaterialX Trace Comparison Report',
                       subtitle=None):
    '''
    Generate an HTML report with multiple chart sections.

    Args:
        reportPath: Path to output HTML file
        sections: List of (title, chartPath) tuples
        pageTitle: Title for the HTML page header
        subtitle: Optional subtitle shown under the page title (e.g., filter params)
    '''
    reportPath = Path(reportPath)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)

    def relPath(absPath):
        if absPath is None:
            return None
        try:
            return str(Path(absPath).relative_to(reportDir))
        except ValueError:
            return 'file:///' + str(Path(absPath)).replace('\\', '/')

    html = []
    html.append(f'''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{pageTitle}</title>
    <style>
        * {{ box-sizing: border-box; }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 0; padding: 20px; background: #f5f5f5;
        }}
        .container {{ max-width: 1800px; margin: 0 auto; }}
        h1, h2 {{ color: #333; }}
        h1 {{ border-bottom: 2px solid #3498db; padding-bottom: 10px; }}
        .subtitle {{ color: #666; font-size: 14px; margin-top: -8px; margin-bottom: 16px; }}
        .chart-section {{ background: white; border-radius: 8px; padding: 20px; margin-bottom: 30px;
                         box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .chart-section img {{ max-width: 100%; height: auto; }}
    </style>
</head>
<body>
<div class="container">
    <h1>{pageTitle}</h1>
''')

    if subtitle:
        html.append(f'    <p class="subtitle">{subtitle}</p>\n')

    for title, chartPath in sections:
        if chartPath and Path(chartPath).exists():
            chartRel = relPath(str(chartPath))
            html.append(f'''
    <div class="chart-section">
        <h2>{title}</h2>
        <img src="{chartRel}" alt="{title}">
    </div>
''')

    html.append('''
    <footer style="margin-top: 40px; padding-top: 20px; border-top: 1px solid #ddd; color: #666; font-size: 12px;">
        Generated by diff_traces.py
    </footer>
</div>
</body>
</html>
''')

    with open(reportPath, 'w', encoding='utf-8') as f:
        f.write(''.join(html))

    logger.info(f'HTML report saved to: {reportPath}')


# =============================================================================
# CHART PATH HELPERS
# =============================================================================

def _chartPath(basePath, suffix):
    '''Derive a chart output path by inserting a suffix before the extension.'''
    basePath = Path(basePath)
    return basePath.parent / f'{basePath.stem}_{suffix}{basePath.suffix}'


# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Compare performance traces between baseline and optimized MaterialX test runs.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s ./baseline/ ./optimized/ --gpu
  %(prog)s ./baseline/ ./optimized/ --slice GenerateShader
  %(prog)s ./baseline/ ./optimized/ --slice GenerateShader CompileShader RenderMaterial
  %(prog)s ./baseline/ ./optimized/ --gpu --slice GenerateShader
  %(prog)s ./baseline/ ./optimized/ --gpu -o custom_name.html

For image comparison, see diff_images.py in the same directory.
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline directory containing Perfetto traces')
    parser.add_argument('optimized', type=Path,
                        help='Optimized directory containing Perfetto traces')

    modeGroup = parser.add_argument_group('comparison modes (at least one required)')
    modeGroup.add_argument('--gpu', action='store_true',
                           help='Compare GPU render durations per material')
    modeGroup.add_argument('--slice', nargs='+', metavar='NAME',
                           help='Compare named child-slice durations per material '
                                '(e.g., GenerateShader, CompileShader, RenderMaterial)')

    optGroup = parser.add_argument_group('options')
    optGroup.add_argument('--min-delta-ms', type=float, default=0.0,
                          help='Minimum absolute time difference in ms to include')
    optGroup.add_argument('-o', '--outputfile', dest='outputfile', type=str,
                          default=None,
                          help='Output HTML report file name (default: <baseline>_vs_<optimized>.html)')
    optGroup.add_argument('--show-opt', type=str, metavar='OPT_NAME',
                          help='Highlight materials affected by optimization pass')

    args = parser.parse_args()

    if not args.gpu and not args.slice:
        parser.print_help()
        sys.exit(0)

    # Load trace files once
    try:
        baselineTracePath = findTraceFile(args.baseline)
        optimizedTracePath = findTraceFile(args.optimized)
    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)

    logger.info(f'Loading baseline trace: {baselineTracePath}')
    baselineTraceProcessor = TraceProcessor(trace=str(baselineTracePath))

    logger.info(f'Loading optimized trace: {optimizedTracePath}')
    optimizedTraceProcessor = TraceProcessor(trace=str(optimizedTracePath))

    # Load optimization events if requested
    optimizedMaterials = set()
    if args.show_opt:
        baselineMaterials = loadOptimizationEvents(baselineTraceProcessor, args.show_opt)
        if baselineMaterials:
            logger.error(f'ERROR: Baseline has {len(baselineMaterials)} materials '
                        f'with {args.show_opt}!')
            sys.exit(1)

        optimizedMaterials = loadOptimizationEvents(optimizedTraceProcessor, args.show_opt)
        logger.info(f'Found {len(optimizedMaterials)} materials affected by {args.show_opt}')

    # Directory leaf names for display
    baselineName = Path(args.baseline).name
    optimizedName = Path(args.optimized).name

    # Derive default report name from directory names
    if args.outputfile is None:
        args.outputfile = f'{baselineName}_vs_{optimizedName}.html'

    # Build the list of comparisons to run
    comparisons = []

    if args.gpu:
        comparisons.append('GPU')

    if args.slice:
        for sliceName in args.slice:
            comparisons.append(sliceName)

    # Build filter subtitle from active options
    filterParts = []
    if args.min_delta_ms > 0:
        filterParts.append(f'min delta: {args.min_delta_ms:.1f} ms')
    if args.show_opt:
        filterParts.append(f'highlighting: {args.show_opt}')
    subtitle = 'Filters: ' + ', '.join(filterParts) if filterParts else None

    # Derive chart paths from the report file name
    reportPath = Path(args.outputfile)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)
    chartBase = reportDir / (reportPath.stem + '.png')

    reportSections = []

    try:
        for label in comparisons:
            # Run comparison and build title
            if label == 'GPU':
                traceData, samplesPerMaterial = compareGpuTraces(
                    baselineTraceProcessor, optimizedTraceProcessor, args.min_delta_ms)
                avgNote = f' (averaged over {samplesPerMaterial} frames)' if samplesPerMaterial > 1 else ''
                title = f'GPU Render Duration per Material{avgNote}: {baselineName} vs {optimizedName}'
            else:
                traceData = compareChildSlices(
                    baselineTraceProcessor, optimizedTraceProcessor, label, args.min_delta_ms)
                title = f'{label} Duration per Material: {baselineName} vs {optimizedName}'

            # Print table
            printTraceTable(traceData, title,
                            baselineName=baselineName, optimizedName=optimizedName,
                            optimizedMaterials=optimizedMaterials)

            # Generate chart
            if traceData is not None and not traceData.empty:
                chartPath = _chartPath(chartBase, label)
                createTraceChart(traceData, chartPath, title=title,
                                 baselineName=baselineName, optimizedName=optimizedName,
                                 optimizedMaterials=optimizedMaterials,
                                 optimizationName=args.show_opt,
                                 subtitle=subtitle)
                reportSections.append((title, chartPath))

        # HTML Report (always generated)
        pageTitle = f'Trace Comparison: {baselineName} vs {optimizedName}'
        if reportSections:
            generateHtmlReport(reportPath, reportSections, pageTitle=pageTitle,
                               subtitle=subtitle)
            absReportPath = reportPath.resolve()
            print(f'\n{"=" * 85}')
            print(f'  Report: {absReportPath}')
            print(f'{"=" * 85}')
            # Auto-open in the default browser
            import webbrowser
            webbrowser.open(absReportPath.as_uri())
        else:
            print(f'\n{"=" * 85}')
            print('  No data to report.')
            print(f'{"=" * 85}')

        sys.exit(0)

    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)
    except Exception as e:
        logger.error(f'Error: {e}')
        raise


if __name__ == '__main__':
    main()

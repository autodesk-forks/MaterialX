#!/usr/bin/env python
'''
Compare performance traces between baseline and optimized MaterialX test runs.

Reads Perfetto .perfetto-trace files and compares slice durations, generating
tables, charts, and optional CSV/HTML output.

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
    python diff_traces.py <baseline_dir> <optimized_dir> --gpu -o my_report.html
'''

import argparse
import logging
import sys
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_traces')


# -----------------------------------------------------------------------------
# Dependency Checking
# -----------------------------------------------------------------------------

# Optional: perfetto (for reading traces)
_have_perfetto = False
try:
    from perfetto.trace_processor import TraceProcessor
    _have_perfetto = True
except ImportError:
    logger.debug('perfetto not found. Trace comparison disabled.')

# Optional: pandas (for CSV export and data aggregation)
_have_pandas = False
try:
    import pandas as pd
    _have_pandas = True
except ImportError:
    logger.debug('pandas not found. CSV export disabled.')

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


def loadSliceDurations(tracePath, trackName=None):
    '''
    Load slice durations from a Perfetto trace, optionally filtered by track.

    Args:
        tracePath: Path to the .perfetto-trace file
        trackName: Optional track name filter (e.g., "GPU")

    Returns:
        DataFrame with columns [name, dur_ms] if pandas available,
        otherwise list of dicts.
    '''
    tp = TraceProcessor(trace=str(tracePath))

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

    result = tp.query(query)

    if _have_pandas:
        df = result.as_pandas_dataframe()
        if df.empty:
            trackMsg = f' on track "{trackName}"' if trackName else ''
            logger.warning(f'No slices found{trackMsg} in {tracePath}')
            return pd.DataFrame(columns=['name', 'dur_ms'])
        return df
    else:
        rows = []
        for row in result:
            rows.append({'name': row.name, 'dur_ms': row.dur_ms})
        if not rows:
            trackMsg = f' on track "{trackName}"' if trackName else ''
            logger.warning(f'No slices found{trackMsg} in {tracePath}')
        return rows


def loadChildSliceDurations(tracePath, sliceName):
    '''
    Load durations of a named child slice, keyed by parent (material) name.

    Queries the trace for slices matching sliceName that are direct children
    of a parent slice (typically the material name).

    Args:
        tracePath: Path to the .perfetto-trace file
        sliceName: Name of the child slice (e.g., "GenerateShader", "CompileShader")

    Returns:
        DataFrame with columns [name, dur_ms] if pandas available,
        otherwise list of dicts. 'name' is the parent (material) name.
    '''
    tp = TraceProcessor(trace=str(tracePath))

    query = f'''
    SELECT parent.name as name, child.dur / 1000000.0 as dur_ms
    FROM slice child
    JOIN slice parent ON child.parent_id = parent.id
    WHERE child.name = '{sliceName}'
    ORDER BY parent.name
    '''

    result = tp.query(query)

    if _have_pandas:
        df = result.as_pandas_dataframe()
        if df.empty:
            logger.warning(f'No "{sliceName}" slices found in {tracePath}')
            return pd.DataFrame(columns=['name', 'dur_ms'])
        return df
    else:
        rows = []
        for row in result:
            rows.append({'name': row.name, 'dur_ms': row.dur_ms})
        if not rows:
            logger.warning(f'No "{sliceName}" slices found in {tracePath}')
        return rows


def loadOptimizationEvents(tracePath, optimizationName=None):
    '''
    Load optimization events from a Perfetto trace.

    Optimization events are nested inside ShaderGen slices with hierarchy:
    MaterialName -> GenerateShader -> OptimizationPass

    Args:
        tracePath: Path to the .perfetto-trace file
        optimizationName: Filter by optimization pass name

    Returns:
        Set of material names that had the optimization applied.
    '''
    tp = TraceProcessor(trace=str(tracePath))

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

    result = tp.query(query)

    optimizedMaterials = set()
    for row in result:
        if row.material_name:
            optimizedMaterials.add(row.material_name)

    return optimizedMaterials


# =============================================================================
# COMPARISON
# =============================================================================

def _aggregateByName(data):
    '''Aggregate durations by name (averaging across multiple samples).'''
    if _have_pandas:
        df = data
        if df.empty:
            return pd.DataFrame(columns=['name', 'mean_ms'])
        agg = df.groupby('name')['dur_ms'].mean().reset_index()
        agg.columns = ['name', 'mean_ms']
        return agg
    else:
        from collections import defaultdict
        groups = defaultdict(list)
        for row in data:
            groups[row['name']].append(row['dur_ms'])
        return {name: {'mean_ms': sum(durs) / len(durs)} for name, durs in groups.items()}


def _mergeAndCompare(baselineAgg, optimizedAgg, minDeltaMs=0.0):
    '''Merge baseline and optimized aggregates, compute delta and percentage.'''
    if _have_pandas:
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
    else:
        allNames = set(baselineAgg.keys()) | set(optimizedAgg.keys())
        results = []
        for name in allNames:
            baseMs = baselineAgg.get(name, {}).get('mean_ms')
            optMs = optimizedAgg.get(name, {}).get('mean_ms')
            deltaMs = None
            changePct = None
            if baseMs is not None and optMs is not None:
                deltaMs = optMs - baseMs
                changePct = (deltaMs / baseMs) * 100 if baseMs != 0 else None
            if minDeltaMs > 0 and (deltaMs is None or abs(deltaMs) < minDeltaMs):
                continue
            results.append({
                'name': name,
                'mean_ms_baseline': baseMs,
                'mean_ms_optimized': optMs,
                'delta_ms': deltaMs,
                'change_pct': changePct
            })
        results.sort(key=lambda x: (x['delta_ms'] is None, x['delta_ms'] or 0))
        return results


def compareGpuTraces(baselinePath, optimizedPath, minDeltaMs=0.0):
    '''
    Compare GPU render durations between baseline and optimized traces.

    Reads the GPU async track and averages per material across frames.
    '''
    baselineTrace = findTraceFile(baselinePath)
    optimizedTrace = findTraceFile(optimizedPath)

    logger.info(f'Loading baseline GPU trace: {baselineTrace}')
    baselineData = loadSliceDurations(baselineTrace, trackName='GPU')
    baselineAgg = _aggregateByName(baselineData)

    logger.info(f'Loading optimized GPU trace: {optimizedTrace}')
    optimizedData = loadSliceDurations(optimizedTrace, trackName='GPU')
    optimizedAgg = _aggregateByName(optimizedData)

    return _mergeAndCompare(baselineAgg, optimizedAgg, minDeltaMs)


def compareChildSlices(baselinePath, optimizedPath, sliceName, minDeltaMs=0.0):
    '''
    Compare durations of a named child slice (e.g., GenerateShader) per material.

    Queries child slices under material parent slices in both traces,
    then merges by material name and computes delta/percentage.
    '''
    baselineTrace = findTraceFile(baselinePath)
    optimizedTrace = findTraceFile(optimizedPath)

    logger.info(f'Loading baseline "{sliceName}" slices: {baselineTrace}')
    baselineData = loadChildSliceDurations(baselineTrace, sliceName)
    baselineAgg = _aggregateByName(baselineData)

    logger.info(f'Loading optimized "{sliceName}" slices: {optimizedTrace}')
    optimizedData = loadChildSliceDurations(optimizedTrace, sliceName)
    optimizedAgg = _aggregateByName(optimizedData)

    return _mergeAndCompare(baselineAgg, optimizedAgg, minDeltaMs)


# =============================================================================
# OUTPUT: TABLE
# =============================================================================

def _isna(val):
    '''Check if value is None or NaN.'''
    if val is None:
        return True
    if _have_pandas:
        return pd.isna(val)
    return False


def printTraceTable(data, title, optimizedMaterials=None):
    '''Print a formatted trace comparison table to stdout.

    Args:
        data: Comparison data (DataFrame or list of dicts)
        title: Section title printed above the table
        optimizedMaterials: Optional set of material names affected by optimization
    '''
    if data is None:
        return

    if optimizedMaterials is None:
        optimizedMaterials = set()

    print(f'\n{"=" * 85}')
    print(f'  {title}')
    print(f'{"=" * 85}')
    marker = ' *' if optimizedMaterials else ''
    print(f"{'Name':<40} {'Baseline':>10} {'Optimized':>10} {'Delta':>10} {'Change':>8}{marker}")
    print('-' * 85)

    if _have_pandas and hasattr(data, 'iterrows'):
        rows = [row for _, row in data.iterrows()]
        totalLen = len(data)
    else:
        rows = data
        totalLen = len(data)

    for row in rows:
        if hasattr(row, '__getitem__'):
            fullName = str(row['name'])
            name = fullName[:38]
            baseMs = row['mean_ms_baseline']
            optMs = row['mean_ms_optimized']
            changePct = row['change_pct']
            deltaMs = row.get('delta_ms')
        else:
            fullName = str(row.name)
            name = fullName[:38]
            baseMs = row.mean_ms_baseline
            optMs = row.mean_ms_optimized
            changePct = row.change_pct
            deltaMs = getattr(row, 'delta_ms', None)

        affected = fullName in optimizedMaterials
        marker = ' *' if affected else '  '

        baseline = f'{baseMs:.2f}ms' if not _isna(baseMs) else 'N/A'
        optimized = f'{optMs:.2f}ms' if not _isna(optMs) else 'N/A'
        delta = f'{deltaMs:+.2f}ms' if not _isna(deltaMs) else 'N/A'
        change = f'{changePct:+.1f}%' if not _isna(changePct) else 'N/A'
        print(f'{name:<40} {baseline:>10} {optimized:>10} {delta:>10} {change:>8}{marker}')

    print('-' * 85)

    if _have_pandas and hasattr(data, 'iterrows'):
        improved = data[data['change_pct'] < 0]
        regressed = data[data['change_pct'] > 0]
        unchanged = data[data['change_pct'] == 0]
        nImproved, nRegressed, nUnchanged = len(improved), len(regressed), len(unchanged)
        validChanges = list(data.dropna(subset=['change_pct'])['change_pct'])
    else:
        nImproved = sum(1 for r in data if r['change_pct'] is not None and r['change_pct'] < 0)
        nRegressed = sum(1 for r in data if r['change_pct'] is not None and r['change_pct'] > 0)
        nUnchanged = sum(1 for r in data if r['change_pct'] == 0)
        validChanges = [r['change_pct'] for r in data if r['change_pct'] is not None]

    print(f'\nSummary: {nImproved} improved, {nRegressed} regressed, '
          f'{nUnchanged} unchanged, {totalLen} total')

    if nImproved > 0:
        if _have_pandas and hasattr(data, 'iterrows'):
            best = improved.iloc[0]
            print(f"Best improvement: {best['name']} ({best['change_pct']:.1f}%)")
        else:
            best = next(r for r in data if r['change_pct'] is not None and r['change_pct'] < 0)
            print(f"Best improvement: {best['name']} ({best['change_pct']:.1f}%)")

    if nRegressed > 0:
        if _have_pandas and hasattr(data, 'iterrows'):
            worst = regressed.iloc[-1]
            print(f"Worst regression: {worst['name']} ({worst['change_pct']:+.1f}%)")
        else:
            worst = [r for r in data if r['change_pct'] is not None and r['change_pct'] > 0][-1]
            print(f"Worst regression: {worst['name']} ({worst['change_pct']:+.1f}%)")

    if validChanges:
        avgChange = sum(validChanges) / len(validChanges)
        sortedChanges = sorted(validChanges)
        medianChange = sortedChanges[len(sortedChanges) // 2]
        print(f'Overall: mean {avgChange:+.1f}%, median {medianChange:+.1f}%')

    if optimizedMaterials:
        print(f'\n* = affected by optimization ({len(optimizedMaterials)} materials)')


# =============================================================================
# OUTPUT: CHART
# =============================================================================

def createTraceChart(data, outputPath, title,
                     optimizedMaterials=None, optimizationName=None):
    '''
    Create a paired before/after horizontal bar chart sorted by time saved.

    Args:
        data: DataFrame or list of dicts with comparison results
        outputPath: Path to save the chart image
        title: Chart title (required)
        optimizedMaterials: Set of material names affected by optimization
        optimizationName: Name of the optimization pass (for legend)
    '''
    if data is None:
        return

    if optimizedMaterials is None:
        optimizedMaterials = set()
    if not _have_matplotlib:
        logger.error('Cannot create chart: matplotlib not installed.')
        return
    if not _have_pandas:
        logger.error('Cannot create chart: pandas not installed.')
        return

    df = data if hasattr(data, 'iterrows') else pd.DataFrame(data)

    chartDf = df.dropna(subset=['mean_ms_baseline', 'mean_ms_optimized']).copy()
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
            bar_height, label='Baseline', color='#3498db', alpha=0.8)

    colors = ['#2ecc71' if d < 0 else '#e74c3c' for d in chartDf['delta_ms']]
    ax.barh([y - bar_height/2 for y in y_pos], chartDf['mean_ms_optimized'],
            bar_height, label='Optimized', color=colors, alpha=0.8)

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

    optNote = f'\n* = affected by {optimizationName}' if optimizationName and optimizedMaterials else ''
    ax.set_title(f'{title}{optNote}')

    legendElements = [
        Patch(facecolor='#3498db', label='Baseline'),
        Patch(facecolor='#2ecc71', label='Optimized (faster)'),
        Patch(facecolor='#e74c3c', label='Optimized (slower)')
    ]
    ax.legend(handles=legendElements, loc='lower right')

    plt.tight_layout()
    plt.savefig(outputPath, dpi=150, bbox_inches='tight')
    plt.close(fig)
    logger.info(f'Chart saved to: {outputPath}')


# =============================================================================
# OUTPUT: HTML REPORT
# =============================================================================

def generateHtmlReport(reportPath, sections):
    '''
    Generate an HTML report with multiple chart sections.

    Args:
        reportPath: Path to output HTML file
        sections: List of (title, chartPath) tuples
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
    html.append('''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MaterialX Trace Comparison Report</title>
    <style>
        * { box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 0; padding: 20px; background: #f5f5f5;
        }
        .container { max-width: 1800px; margin: 0 auto; }
        h1, h2 { color: #333; }
        h1 { border-bottom: 2px solid #3498db; padding-bottom: 10px; }
        .chart-section { background: white; border-radius: 8px; padding: 20px; margin-bottom: 30px;
                         box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .chart-section img { max-width: 100%; height: auto; }
    </style>
</head>
<body>
<div class="container">
    <h1>MaterialX Trace Comparison Report</h1>
''')

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
  %(prog)s ./baseline/ ./optimized/ --gpu -o my_report.html

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
                          default='trace_diff.html',
                          help='Output HTML report file name (default: trace_diff.html)')
    optGroup.add_argument('--show-opt', type=str, metavar='OPT_NAME',
                          help='Highlight materials affected by optimization pass')
    optGroup.add_argument('--csv', type=Path,
                          help='Export trace results to CSV file (last comparison only)')

    args = parser.parse_args()

    if not args.gpu and not args.slice:
        parser.print_help()
        sys.exit(0)

    if not _have_perfetto:
        logger.error('perfetto is required. Install with: pip install perfetto')
        sys.exit(1)

    # Load optimization events if requested
    optimizedMaterials = set()
    if args.show_opt:
        try:
            baselineTracePath = findTraceFile(args.baseline)
            baselineMaterials = loadOptimizationEvents(baselineTracePath, args.show_opt)
            if baselineMaterials:
                logger.error(f'ERROR: Baseline has {len(baselineMaterials)} materials '
                            f'with {args.show_opt}!')
                sys.exit(1)

            optimizedTracePath = findTraceFile(args.optimized)
            optimizedMaterials = loadOptimizationEvents(optimizedTracePath, args.show_opt)
            logger.info(f'Found {len(optimizedMaterials)} materials affected by {args.show_opt}')
        except FileNotFoundError as e:
            logger.error(f'{e}')
            sys.exit(1)

    # Build the list of comparisons to run: [(label, title), ...]
    comparisons = []

    if args.gpu:
        comparisons.append(('GPU', 'GPU Render Duration per Material: Baseline vs Optimized'))

    if args.slice:
        for sliceName in args.slice:
            comparisons.append((sliceName, f'{sliceName} Duration per Material: Baseline vs Optimized'))

    # Derive chart paths from the report file name
    reportPath = Path(args.outputfile)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)
    chartBase = reportDir / (reportPath.stem + '.png')

    reportSections = []
    lastTraceData = None

    try:
        for label, title in comparisons:
            # Run comparison
            if label == 'GPU':
                traceData = compareGpuTraces(args.baseline, args.optimized, args.min_delta_ms)
            else:
                traceData = compareChildSlices(args.baseline, args.optimized, label, args.min_delta_ms)

            lastTraceData = traceData

            # Print table
            printTraceTable(traceData, title, optimizedMaterials)

            # Generate chart
            if traceData is not None:
                if len(comparisons) > 1:
                    chartPath = _chartPath(chartBase, label)
                else:
                    chartPath = chartBase
                createTraceChart(traceData, chartPath, title=title,
                                 optimizedMaterials=optimizedMaterials,
                                 optimizationName=args.show_opt)
                reportSections.append((title, chartPath))

        # CSV export (last comparison)
        if args.csv and lastTraceData is not None:
            if _have_pandas:
                df = lastTraceData if hasattr(lastTraceData, 'to_csv') else pd.DataFrame(lastTraceData)
                df.to_csv(args.csv, index=False)
                logger.info(f'CSV exported to: {args.csv}')
            else:
                logger.error('Cannot export CSV: pandas not installed.')

        # HTML Report (always generated)
        if reportSections:
            generateHtmlReport(reportPath, reportSections)

        print(f'\n{"=" * 85}')
        print('RESULT: Trace comparison complete')
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

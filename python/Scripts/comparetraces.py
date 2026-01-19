#!/usr/bin/env python
'''
Compare performance data between two Perfetto trace files.

This script analyzes trace data from MaterialX performance tests, comparing
baseline and optimized runs to identify improvements and regressions.

Usage:
    python comparetraces.py <baseline> <optimized> [options]

Arguments can be either:
  - Direct paths to .perfetto-trace files
  - Directories (will search for *.perfetto-trace files)

Examples:
    # Compare two trace files directly
    python comparetraces.py baseline.perfetto-trace optimized.perfetto-trace

    # Compare traces in directories (finds *.perfetto-trace files)
    python comparetraces.py ./baseline_output/ ./optimized_output/

    # Specify track name and output chart
    python comparetraces.py base.perfetto-trace opt.perfetto-trace --track GPU --output chart.png

    # Export results to CSV
    python comparetraces.py base.perfetto-trace opt.perfetto-trace --csv results.csv
'''

import argparse
import sys
from pathlib import Path


# -----------------------------------------------------------------------------
# Dependency Checking
# -----------------------------------------------------------------------------

def checkDependencies():
    '''Check that required dependencies are available.'''
    missing = []

    try:
        from perfetto.trace_processor import TraceProcessor
    except ImportError:
        missing.append('perfetto')

    try:
        import pandas
    except ImportError:
        missing.append('pandas')

    try:
        import matplotlib
    except ImportError:
        missing.append('matplotlib')

    if missing:
        print('Error: Missing required packages. Install with:')
        print(f'  pip install {" ".join(missing)}')
        sys.exit(1)


# -----------------------------------------------------------------------------
# Trace Loading
# -----------------------------------------------------------------------------

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
            # Try recursive search
            traces = list(path.glob('**/*.perfetto-trace'))
        if traces:
            if len(traces) > 1:
                print(f'Warning: Multiple traces found in {path}, using: {traces[0].name}')
            return traces[0]
        raise FileNotFoundError(f'No .perfetto-trace files found in: {path}')

    raise FileNotFoundError(f'Path not found: {path}')


def loadSliceDurations(tracePath, trackName=None):
    '''
    Load slice durations from a Perfetto trace.

    Args:
        tracePath: Path to the .perfetto-trace file
        trackName: Optional track name filter (e.g., "GPU", "ShaderGen")

    Returns:
        DataFrame with columns: [name, dur_ns, dur_ms, track]
    '''
    from perfetto.trace_processor import TraceProcessor
    import pandas as pd

    tp = TraceProcessor(trace=str(tracePath))

    # Build query with optional track filter
    if trackName:
        query = f'''
        SELECT 
            slice.name,
            slice.dur as dur_ns,
            track.name as track
        FROM slice
        JOIN track ON slice.track_id = track.id
        WHERE track.name = '{trackName}'
        ORDER BY slice.name
        '''
    else:
        query = '''
        SELECT 
            slice.name,
            slice.dur as dur_ns,
            track.name as track
        FROM slice
        JOIN track ON slice.track_id = track.id
        ORDER BY slice.name
        '''

    result = tp.query(query)
    df = result.as_pandas_dataframe()

    if df.empty:
        trackMsg = f' on track "{trackName}"' if trackName else ''
        print(f'Warning: No slices found{trackMsg} in {tracePath}')
        return pd.DataFrame(columns=['name', 'dur_ns', 'dur_ms', 'track'])

    df['dur_ms'] = df['dur_ns'] / 1_000_000
    return df


def aggregateByName(df):
    '''
    Aggregate durations by slice name (averaging across multiple samples).

    Returns:
        DataFrame with columns: [name, mean_ms, std_ms, count]
    '''
    import pandas as pd

    if df.empty:
        return pd.DataFrame(columns=['name', 'mean_ms', 'std_ms', 'count'])

    agg = df.groupby('name')['dur_ms'].agg(['mean', 'std', 'count']).reset_index()
    agg.columns = ['name', 'mean_ms', 'std_ms', 'count']
    return agg


# -----------------------------------------------------------------------------
# Comparison Logic
# -----------------------------------------------------------------------------

def compareTraces(baselinePath, optimizedPath, trackName=None):
    '''
    Compare durations between baseline and optimized traces.

    Args:
        baselinePath: Path to baseline trace file or directory
        optimizedPath: Path to optimized trace file or directory
        trackName: Optional track name filter

    Returns:
        DataFrame sorted by improvement percentage (most improved first)
    '''
    import pandas as pd

    baselineTrace = findTraceFile(baselinePath)
    optimizedTrace = findTraceFile(optimizedPath)

    print(f'Loading baseline: {baselineTrace}')
    baselineDf = loadSliceDurations(baselineTrace, trackName)
    baselineAgg = aggregateByName(baselineDf)

    print(f'Loading optimized: {optimizedTrace}')
    optimizedDf = loadSliceDurations(optimizedTrace, trackName)
    optimizedAgg = aggregateByName(optimizedDf)

    # Merge on slice name
    merged = pd.merge(
        baselineAgg[['name', 'mean_ms']],
        optimizedAgg[['name', 'mean_ms']],
        on='name',
        suffixes=('_baseline', '_optimized'),
        how='outer'
    )

    # Calculate change
    merged['change_ms'] = merged['mean_ms_optimized'] - merged['mean_ms_baseline']
    merged['change_pct'] = (merged['change_ms'] / merged['mean_ms_baseline']) * 100

    # Sort by improvement (most negative = biggest improvement)
    merged = merged.sort_values('change_pct', ascending=True)

    return merged


# -----------------------------------------------------------------------------
# Output Formatting
# -----------------------------------------------------------------------------

def printTable(df, topN=30):
    '''Print a formatted comparison table to stdout.'''
    import pandas as pd

    print('\n' + '=' * 80)
    print(f"{'Name':<45} {'Baseline':>10} {'Optimized':>10} {'Change':>10}")
    print('=' * 80)

    for _, row in df.head(topN).iterrows():
        name = row['name'][:44]  # Truncate long names
        baseline = f"{row['mean_ms_baseline']:.2f}ms" if pd.notna(row['mean_ms_baseline']) else 'N/A'
        optimized = f"{row['mean_ms_optimized']:.2f}ms" if pd.notna(row['mean_ms_optimized']) else 'N/A'
        change = f"{row['change_pct']:+.1f}%" if pd.notna(row['change_pct']) else 'N/A'
        print(f'{name:<45} {baseline:>10} {optimized:>10} {change:>10}')

    if len(df) > topN:
        print(f'... and {len(df) - topN} more entries')

    print('=' * 80)

    # Summary statistics
    improved = df[df['change_pct'] < 0]
    regressed = df[df['change_pct'] > 0]
    unchanged = df[df['change_pct'] == 0]

    print(f'\nSummary: {len(improved)} improved, {len(regressed)} regressed, '
          f'{len(unchanged)} unchanged, {len(df)} total')

    if not improved.empty:
        best = improved.iloc[0]
        print(f"Best improvement: {best['name']} ({best['change_pct']:.1f}%)")

    if not regressed.empty:
        worst = regressed.iloc[-1]
        print(f"Worst regression: {worst['name']} ({worst['change_pct']:+.1f}%)")

    # Overall statistics
    validRows = df.dropna(subset=['change_pct'])
    if not validRows.empty:
        avgChange = validRows['change_pct'].mean()
        medianChange = validRows['change_pct'].median()
        print(f'\nOverall: mean {avgChange:+.1f}%, median {medianChange:+.1f}%')


def createChart(df, outputPath, topN=25, title=None):
    '''
    Create a horizontal bar chart of performance changes.

    Args:
        df: DataFrame with comparison results
        outputPath: Path to save the chart image
        topN: Number of entries to show (top improvements + top regressions)
        title: Optional chart title
    '''
    import pandas as pd
    import matplotlib.pyplot as plt
    from matplotlib.patches import Patch

    # Filter to top improvements and regressions
    topImproved = df.head(topN // 2)
    topRegressed = df.tail(topN // 2)
    chartDf = pd.concat([topImproved, topRegressed]).drop_duplicates()
    chartDf = chartDf.sort_values('change_pct', ascending=True)

    if chartDf.empty:
        print('Warning: No data to chart')
        return

    # Truncate names for display
    chartDf = chartDf.copy()
    chartDf['display_name'] = chartDf['name'].apply(
        lambda x: x[:35] + '...' if len(x) > 35 else x)

    # Create figure
    figHeight = max(8, len(chartDf) * 0.35)
    fig, ax = plt.subplots(figsize=(12, figHeight))

    # Color by improvement/regression
    colors = ['#2ecc71' if x < 0 else '#e74c3c' for x in chartDf['change_pct']]

    bars = ax.barh(chartDf['display_name'], chartDf['change_pct'], color=colors)

    # Add value labels
    for bar, pct in zip(bars, chartDf['change_pct']):
        width = bar.get_width()
        labelX = width + (1 if width >= 0 else -1)
        ax.text(labelX, bar.get_y() + bar.get_height() / 2,
                f'{pct:+.1f}%', va='center', ha='left' if width >= 0 else 'right',
                fontsize=9)

    ax.axvline(x=0, color='black', linewidth=0.5)
    ax.set_xlabel('Time Change (%)')

    if title:
        ax.set_title(title)
    else:
        ax.set_title('Performance Comparison: Baseline vs Optimized\n(negative = faster)')

    # Legend
    legendElements = [
        Patch(facecolor='#2ecc71', label='Improved'),
        Patch(facecolor='#e74c3c', label='Regressed')
    ]
    ax.legend(handles=legendElements, loc='lower right')

    plt.tight_layout()
    plt.savefig(outputPath, dpi=150, bbox_inches='tight')
    print(f'\nChart saved to: {outputPath}')


# -----------------------------------------------------------------------------
# Main Entry Point
# -----------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description='Compare performance data between two Perfetto trace files.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s baseline.perfetto-trace optimized.perfetto-trace
  %(prog)s ./baseline_dir/ ./optimized_dir/ --track GPU
  %(prog)s base.perfetto-trace opt.perfetto-trace --output chart.png --csv results.csv
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline trace file or directory')
    parser.add_argument('optimized', type=Path,
                        help='Optimized trace file or directory')
    parser.add_argument('--track', '-t', type=str, default=None,
                        help='Filter by track name (e.g., GPU, ShaderGen)')
    parser.add_argument('--output', '-o', type=Path,
                        help='Output path for chart image (e.g., chart.png)')
    parser.add_argument('--top', '-n', type=int, default=30,
                        help='Number of entries to show in table (default: 30)')
    parser.add_argument('--csv', type=Path,
                        help='Export full results to CSV file')
    parser.add_argument('--title', type=str,
                        help='Custom title for the chart')
    parser.add_argument('--show', action='store_true',
                        help='Display chart interactively (requires display)')

    args = parser.parse_args()

    # Check dependencies before proceeding
    checkDependencies()

    try:
        df = compareTraces(args.baseline, args.optimized, args.track)
        printTable(df, args.top)

        if args.output:
            createChart(df, args.output, title=args.title)
            if args.show:
                import matplotlib.pyplot as plt
                plt.show()

        if args.csv:
            df.to_csv(args.csv, index=False)
            print(f'CSV exported to: {args.csv}')

    except FileNotFoundError as e:
        print(f'Error: {e}')
        sys.exit(1)
    except Exception as e:
        print(f'Error processing traces: {e}')
        raise


if __name__ == '__main__':
    main()


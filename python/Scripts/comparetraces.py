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
    python comparetraces.py base.perfetto-trace opt.perfetto-trace --track GPU --chart chart.png

    # Export results to CSV
    python comparetraces.py base.perfetto-trace opt.perfetto-trace --csv results.csv
'''

import argparse
import logging
import sys
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('comparetraces')


# -----------------------------------------------------------------------------
# Dependency Checking
# -----------------------------------------------------------------------------

# Required: perfetto (for reading traces)
try:
    from perfetto.trace_processor import TraceProcessor
except ImportError:
    logger.error('Required module not found. Install with: pip install perfetto')
    raise

# Optional: pandas (for CSV export and data aggregation)
_have_pandas = False
try:
    import pandas as pd
    _have_pandas = True
except ImportError:
    logger.warning('pandas not found. CSV export and detailed statistics disabled.')

# Optional: matplotlib (for chart generation)
_have_matplotlib = False
try:
    import matplotlib.pyplot as plt
    from matplotlib.patches import Patch
    _have_matplotlib = True
except ImportError:
    logger.warning('matplotlib not found. Chart generation disabled.')


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
        List of dicts with keys: [name, dur_ns, dur_ms, track]
        Or DataFrame if pandas is available.
    '''
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

    if _have_pandas:
        df = result.as_pandas_dataframe()
        if df.empty:
            trackMsg = f' on track "{trackName}"' if trackName else ''
            logger.warning(f'No slices found{trackMsg} in {tracePath}')
            return pd.DataFrame(columns=['name', 'dur_ns', 'dur_ms', 'track'])
        df['dur_ms'] = df['dur_ns'] / 1_000_000
        return df
    else:
        # Return as list of dicts when pandas not available
        rows = []
        for row in result:
            rows.append({
                'name': row.name,
                'dur_ns': row.dur_ns,
                'dur_ms': row.dur_ns / 1_000_000,
                'track': row.track
            })
        if not rows:
            trackMsg = f' on track "{trackName}"' if trackName else ''
            logger.warning(f'No slices found{trackMsg} in {tracePath}')
        return rows


def aggregateByName(data):
    '''
    Aggregate durations by slice name (averaging across multiple samples).

    Returns:
        DataFrame with columns: [name, mean_ms, std_ms, count] if pandas available,
        otherwise dict of {name: {'mean_ms': float, 'count': int}}.
    '''
    if _have_pandas:
        df = data
        if df.empty:
            return pd.DataFrame(columns=['name', 'mean_ms', 'std_ms', 'count'])
        agg = df.groupby('name')['dur_ms'].agg(['mean', 'std', 'count']).reset_index()
        agg.columns = ['name', 'mean_ms', 'std_ms', 'count']
        return agg
    else:
        # Manual aggregation without pandas
        from collections import defaultdict
        groups = defaultdict(list)
        for row in data:
            groups[row['name']].append(row['dur_ms'])
        result = {}
        for name, durations in groups.items():
            result[name] = {
                'mean_ms': sum(durations) / len(durations),
                'count': len(durations)
            }
        return result


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
        List of comparison dicts sorted by improvement percentage,
        or DataFrame if pandas is available.
    '''
    baselineTrace = findTraceFile(baselinePath)
    optimizedTrace = findTraceFile(optimizedPath)

    logger.info(f'Loading baseline: {baselineTrace}')
    baselineData = loadSliceDurations(baselineTrace, trackName)
    baselineAgg = aggregateByName(baselineData)

    logger.info(f'Loading optimized: {optimizedTrace}')
    optimizedData = loadSliceDurations(optimizedTrace, trackName)
    optimizedAgg = aggregateByName(optimizedData)

    if _have_pandas:
        # Merge on slice name using pandas
        merged = pd.merge(
            baselineAgg[['name', 'mean_ms']],
            optimizedAgg[['name', 'mean_ms']],
            on='name',
            suffixes=('_baseline', '_optimized'),
            how='outer'
        )
        merged['change_ms'] = merged['mean_ms_optimized'] - merged['mean_ms_baseline']
        merged['change_pct'] = (merged['change_ms'] / merged['mean_ms_baseline']) * 100
        merged = merged.sort_values('change_pct', ascending=True)
        return merged
    else:
        # Manual merge without pandas
        allNames = set(baselineAgg.keys()) | set(optimizedAgg.keys())
        results = []
        for name in allNames:
            baseMs = baselineAgg.get(name, {}).get('mean_ms')
            optMs = optimizedAgg.get(name, {}).get('mean_ms')
            changeMs = None
            changePct = None
            if baseMs is not None and optMs is not None:
                changeMs = optMs - baseMs
                changePct = (changeMs / baseMs) * 100 if baseMs != 0 else None
            results.append({
                'name': name,
                'mean_ms_baseline': baseMs,
                'mean_ms_optimized': optMs,
                'change_ms': changeMs,
                'change_pct': changePct
            })
        # Sort by change_pct (None values at end)
        results.sort(key=lambda x: (x['change_pct'] is None, x['change_pct'] or 0))
        return results


# -----------------------------------------------------------------------------
# Output Formatting
# -----------------------------------------------------------------------------

def _isna(val):
    '''Check if value is None or NaN.'''
    if val is None:
        return True
    if _have_pandas:
        return pd.isna(val)
    return False


def printTable(data, topN=30):
    '''Print a formatted comparison table to stdout.'''
    print('\n' + '=' * 80)
    print(f"{'Name':<45} {'Baseline':>10} {'Optimized':>10} {'Change':>10}")
    print('=' * 80)

    # Handle both DataFrame and list of dicts
    if _have_pandas and hasattr(data, 'iterrows'):
        rows = [row for _, row in data.head(topN).iterrows()]
        totalLen = len(data)
    else:
        rows = data[:topN]
        totalLen = len(data)

    for row in rows:
        if hasattr(row, '__getitem__'):
            name = str(row['name'])[:44]
            baseMs = row['mean_ms_baseline']
            optMs = row['mean_ms_optimized']
            changePct = row['change_pct']
        else:
            name = str(row.name)[:44]
            baseMs = row.mean_ms_baseline
            optMs = row.mean_ms_optimized
            changePct = row.change_pct

        baseline = f'{baseMs:.2f}ms' if not _isna(baseMs) else 'N/A'
        optimized = f'{optMs:.2f}ms' if not _isna(optMs) else 'N/A'
        change = f'{changePct:+.1f}%' if not _isna(changePct) else 'N/A'
        print(f'{name:<45} {baseline:>10} {optimized:>10} {change:>10}')

    if totalLen > topN:
        print(f'... and {totalLen - topN} more entries')

    print('=' * 80)

    # Summary statistics
    if _have_pandas and hasattr(data, 'iterrows'):
        improved = data[data['change_pct'] < 0]
        regressed = data[data['change_pct'] > 0]
        unchanged = data[data['change_pct'] == 0]
        nImproved, nRegressed, nUnchanged = len(improved), len(regressed), len(unchanged)
        validRows = data.dropna(subset=['change_pct'])
        validChanges = list(validRows['change_pct'])
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

    # Overall statistics
    if validChanges:
        avgChange = sum(validChanges) / len(validChanges)
        sortedChanges = sorted(validChanges)
        medianChange = sortedChanges[len(sortedChanges) // 2]
        print(f'\nOverall: mean {avgChange:+.1f}%, median {medianChange:+.1f}%')


def createChart(data, outputPath, topN=25, title=None):
    '''
    Create a horizontal bar chart of performance changes.

    Args:
        data: DataFrame or list of dicts with comparison results
        outputPath: Path to save the chart image
        topN: Number of entries to show (top improvements + top regressions)
        title: Optional chart title

    Requires: matplotlib, pandas
    '''
    if not _have_matplotlib:
        logger.error('Cannot create chart: matplotlib not installed.')
        logger.error('Install with: pip install matplotlib')
        return

    if not _have_pandas:
        logger.error('Cannot create chart: pandas not installed.')
        logger.error('Install with: pip install pandas')
        return

    df = data if hasattr(data, 'iterrows') else pd.DataFrame(data)

    # Filter to top improvements and regressions
    topImproved = df.head(topN // 2)
    topRegressed = df.tail(topN // 2)
    chartDf = pd.concat([topImproved, topRegressed]).drop_duplicates()
    chartDf = chartDf.sort_values('change_pct', ascending=True)

    if chartDf.empty:
        logger.warning('No data to chart')
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
    logger.info(f'Chart saved to: {outputPath}')


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
  %(prog)s base.perfetto-trace opt.perfetto-trace --chart chart.png --csv results.csv
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline trace file or directory')
    parser.add_argument('optimized', type=Path,
                        help='Optimized trace file or directory')
    parser.add_argument('--track', '-t', type=str, default=None,
                        help='Filter by track name (e.g., GPU, ShaderGen)')
    parser.add_argument('--chart', '-c', type=Path,
                        help='Output path for chart image (e.g., chart.png). Requires matplotlib.')
    parser.add_argument('--top', '-n', type=int, default=30,
                        help='Number of entries to show in table (default: 30)')
    parser.add_argument('--csv', type=Path,
                        help='Export full results to CSV file. Requires pandas.')
    parser.add_argument('--title', type=str,
                        help='Custom title for the chart')
    parser.add_argument('--show', action='store_true',
                        help='Display chart interactively (requires display)')

    args = parser.parse_args()

    try:
        data = compareTraces(args.baseline, args.optimized, args.track)
        printTable(data, args.top)

        if args.chart:
            createChart(data, args.chart, title=args.title)
            if args.show and _have_matplotlib:
                plt.show()

        if args.csv:
            if _have_pandas:
                df = data if hasattr(data, 'to_csv') else pd.DataFrame(data)
                df.to_csv(args.csv, index=False)
                logger.info(f'CSV exported to: {args.csv}')
            else:
                logger.error('Cannot export CSV: pandas not installed.')
                logger.error('Install with: pip install pandas')

    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)
    except Exception as e:
        logger.error(f'Error processing traces: {e}')
        raise


if __name__ == '__main__':
    main()


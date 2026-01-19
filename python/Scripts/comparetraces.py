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

def compareTraces(baselinePath, optimizedPath, trackName=None, minDeltaMs=0.0):
    '''
    Compare durations between baseline and optimized traces.

    Args:
        baselinePath: Path to baseline trace file or directory
        optimizedPath: Path to optimized trace file or directory
        trackName: Optional track name filter
        minDeltaMs: Minimum absolute delta in ms to include (filters out noise)

    Returns:
        List of comparison dicts sorted by absolute time saved (largest first),
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
        # delta_ms: negative = faster (good), positive = slower (bad)
        merged['delta_ms'] = merged['mean_ms_optimized'] - merged['mean_ms_baseline']
        merged['change_pct'] = (merged['delta_ms'] / merged['mean_ms_baseline']) * 100
        # Filter by minimum absolute delta threshold
        if minDeltaMs > 0:
            merged = merged[merged['delta_ms'].abs() >= minDeltaMs]
        # Sort by delta (most negative = biggest improvement first)
        merged = merged.sort_values('delta_ms', ascending=True)
        return merged
    else:
        # Manual merge without pandas
        allNames = set(baselineAgg.keys()) | set(optimizedAgg.keys())
        results = []
        for name in allNames:
            baseMs = baselineAgg.get(name, {}).get('mean_ms')
            optMs = optimizedAgg.get(name, {}).get('mean_ms')
            deltaMs = None
            changePct = None
            if baseMs is not None and optMs is not None:
                # delta_ms: negative = faster (good), positive = slower (bad)
                deltaMs = optMs - baseMs
                changePct = (deltaMs / baseMs) * 100 if baseMs != 0 else None
            # Filter by minimum absolute delta threshold
            if minDeltaMs > 0 and (deltaMs is None or abs(deltaMs) < minDeltaMs):
                continue
            results.append({
                'name': name,
                'mean_ms_baseline': baseMs,
                'mean_ms_optimized': optMs,
                'delta_ms': deltaMs,
                'change_pct': changePct
            })
        # Sort by delta (most negative = biggest improvement first)
        results.sort(key=lambda x: (x['delta_ms'] is None, x['delta_ms'] or 0))
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


def printTable(data):
    '''Print a formatted comparison table to stdout.'''
    print('\n' + '=' * 80)
    print(f"{'Name':<40} {'Baseline':>10} {'Optimized':>10} {'Delta':>10} {'Change':>8}")
    print('=' * 80)

    # Handle both DataFrame and list of dicts
    if _have_pandas and hasattr(data, 'iterrows'):
        rows = [row for _, row in data.iterrows()]
        totalLen = len(data)
    else:
        rows = data
        totalLen = len(data)

    for row in rows:
        if hasattr(row, '__getitem__'):
            name = str(row['name'])[:39]
            baseMs = row['mean_ms_baseline']
            optMs = row['mean_ms_optimized']
            changePct = row['change_pct']
            deltaMs = row.get('delta_ms')
        else:
            name = str(row.name)[:39]
            baseMs = row.mean_ms_baseline
            optMs = row.mean_ms_optimized
            changePct = row.change_pct
            deltaMs = getattr(row, 'delta_ms', None)

        baseline = f'{baseMs:.2f}ms' if not _isna(baseMs) else 'N/A'
        optimized = f'{optMs:.2f}ms' if not _isna(optMs) else 'N/A'
        # negative delta = faster (optimization), positive = slower (regression)
        delta = f'{deltaMs:+.2f}ms' if not _isna(deltaMs) else 'N/A'
        change = f'{changePct:+.1f}%' if not _isna(changePct) else 'N/A'
        print(f'{name:<40} {baseline:>10} {optimized:>10} {delta:>10} {change:>8}')

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


def createChart(data, outputPath, title=None, minDeltaMs=0.0):
    '''
    Create a paired before/after horizontal bar chart sorted by absolute time saved.

    Args:
        data: DataFrame or list of dicts with comparison results
        minDeltaMs: Minimum delta threshold (shown in title)
        outputPath: Path to save the chart image
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
    
    # Filter to rows with both values
    chartDf = df.dropna(subset=['mean_ms_baseline', 'mean_ms_optimized']).copy()

    if chartDf.empty:
        logger.warning('No data to chart')
        return

    # Reverse order so largest improvements appear at TOP of chart
    # (matplotlib draws bars from bottom to top)
    chartDf = chartDf.iloc[::-1].reset_index(drop=True)

    # Create display names with absolute delta and percentage
    def make_label(row):
        name = row['name'][:30] + '...' if len(row['name']) > 30 else row['name']
        delta = row['delta_ms']
        pct = row['change_pct']
        if pd.notna(delta) and pd.notna(pct):
            return f"{name} ({delta:+.1f}ms, {pct:+.1f}%)"
        return name

    chartDf['display_name'] = chartDf.apply(make_label, axis=1)

    # Create figure
    figHeight = max(10, len(chartDf) * 0.5)
    fig, ax = plt.subplots(figsize=(14, figHeight))

    y_pos = range(len(chartDf))
    bar_height = 0.35

    # Baseline bars (blue) - ABOVE (higher y position)
    bars1 = ax.barh([y + bar_height/2 for y in y_pos], chartDf['mean_ms_baseline'],
                    bar_height, label='Baseline', color='#3498db', alpha=0.8)

    # Optimized bars (green/red based on improvement) - BELOW (lower y position)
    # delta < 0 = faster (green), delta > 0 = slower (red)
    colors = ['#2ecc71' if d < 0 else '#e74c3c' for d in chartDf['delta_ms']]
    bars2 = ax.barh([y - bar_height/2 for y in y_pos], chartDf['mean_ms_optimized'],
                    bar_height, label='Optimized', color=colors, alpha=0.8)

    # Add duration labels to the right of each bar
    for i, (b, o, delta) in enumerate(zip(chartDf['mean_ms_baseline'],
                                           chartDf['mean_ms_optimized'],
                                           chartDf['delta_ms'])):
        # Baseline duration at end of baseline bar
        ax.text(b + 1, i + bar_height/2, f'{b:.1f}ms', va='center', fontsize=7,
                color='#2980b9')
        # Optimized duration at end of optimized bar
        ax.text(o + 1, i - bar_height/2, f'{o:.1f}ms', va='center', fontsize=7,
                color='#27ae60' if delta < 0 else '#c0392b')

    ax.set_yticks(y_pos)
    ax.set_yticklabels(chartDf['display_name'])
    ax.set_xlabel('Time (ms)')

    if title:
        ax.set_title(title)
    else:
        filterNote = f', filtered |Δ| ≥ {minDeltaMs:.1f}ms' if minDeltaMs > 0 else ''
        ax.set_title(f'Absolute Duration Comparison: Baseline vs Optimized\n(sorted by time saved{filterNote})')

    # Legend
    legendElements = [
        Patch(facecolor='#3498db', label='Baseline'),
        Patch(facecolor='#2ecc71', label='Optimized (faster)'),
        Patch(facecolor='#e74c3c', label='Optimized (slower)')
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
    parser.add_argument('--min-delta-ms', type=float, default=0.0,
                        help='Minimum absolute time difference in ms to include (filters out noise)')
    parser.add_argument('--chart', '-c', type=Path,
                        help='Output path for chart image (e.g., chart.png). Requires matplotlib.')
    parser.add_argument('--csv', type=Path,
                        help='Export full results to CSV file. Requires pandas.')
    parser.add_argument('--title', type=str,
                        help='Custom title for the chart')
    parser.add_argument('--open-matplotlib', action='store_true',
                        help='Open interactive matplotlib window after saving chart')

    args = parser.parse_args()

    try:
        data = compareTraces(args.baseline, args.optimized, args.track, args.min_delta_ms)
        printTable(data)

        if args.chart:
            createChart(data, args.chart, title=args.title, minDeltaMs=args.min_delta_ms)
            if args.open_matplotlib and _have_matplotlib:
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

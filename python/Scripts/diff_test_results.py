#!/usr/bin/env python
'''
Compare test results between baseline and optimized MaterialX test runs.

This script supports comparing:
  - Performance traces (Perfetto .perfetto-trace files)
  - Rendered images using NVIDIA FLIP perceptual comparison

FLIP (A Difference Evaluator for Alternating Images) approximates human
perception of differences when flipping between images. A FLIP score of 0
means identical, 1 means maximally different.

Usage:
    python diff_test_results.py <baseline_dir> <optimized_dir> [options]

Examples:
    # Compare traces only
    python diff_test_results.py ./baseline/ ./optimized/ --traces

    # Compare images only  
    python diff_test_results.py ./baseline/ ./optimized/ --images

    # Compare both (default)
    python diff_test_results.py ./baseline/ ./optimized/

    # With options
    python diff_test_results.py ./baseline/ ./optimized/ --min-delta-ms 1.0 --image-threshold 0.05
'''

import argparse
import logging
import sys
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_test_results')


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

# Optional: FLIP (for perceptual image comparison)
_have_flip = False
try:
    import flip_evaluator as flip
    _have_flip = True
except ImportError:
    logger.debug('flip-evaluator not found. Install with: pip install flip-evaluator')

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
# TRACE COMPARISON
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


def loadOptimizationEvents(tracePath, optimizationName=None):
    '''
    Load optimization events from a Perfetto trace.

    Optimization events are nested inside ShaderGen slices with hierarchy:
    MaterialName → GenerateShader → OptimizationPass

    Args:
        tracePath: Path to the .perfetto-trace file
        optimizationName: Filter by optimization pass name (e.g., optReplaceBsdfMixWithLinearCombination)

    Returns:
        Set of material names that had the optimization applied.
    '''
    tp = TraceProcessor(trace=str(tracePath))

    # Query for optimization events and find grandparent slices (material names)
    # Hierarchy: opt → GenerateShader → MaterialName
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
    if not _have_perfetto:
        logger.error('Cannot compare traces: perfetto not installed.')
        logger.error('Install with: pip install perfetto')
        return None

    baselineTrace = findTraceFile(baselinePath)
    optimizedTrace = findTraceFile(optimizedPath)

    logger.info(f'Loading baseline trace: {baselineTrace}')
    baselineData = loadSliceDurations(baselineTrace, trackName)
    baselineAgg = aggregateByName(baselineData)

    logger.info(f'Loading optimized trace: {optimizedTrace}')
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


def _isna(val):
    '''Check if value is None or NaN.'''
    if val is None:
        return True
    if _have_pandas:
        return pd.isna(val)
    return False


def printTraceTable(data, optimizedMaterials=None):
    '''Print a formatted trace comparison table to stdout.
    
    Args:
        data: Comparison data (DataFrame or list of dicts)
        optimizedMaterials: Optional set of material names affected by optimization
    '''
    if data is None:
        return
        
    if optimizedMaterials is None:
        optimizedMaterials = set()
    
    print('\n' + '=' * 85)
    marker = ' *' if optimizedMaterials else ''
    print(f"{'Name':<40} {'Baseline':>10} {'Optimized':>10} {'Delta':>10} {'Change':>8}{marker}")
    print('=' * 85)

    # Handle both DataFrame and list of dicts
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

        # Mark materials affected by the optimization
        affected = fullName in optimizedMaterials
        marker = ' *' if affected else '  '
        
        baseline = f'{baseMs:.2f}ms' if not _isna(baseMs) else 'N/A'
        optimized = f'{optMs:.2f}ms' if not _isna(optMs) else 'N/A'
        # negative delta = faster (optimization), positive = slower (regression)
        delta = f'{deltaMs:+.2f}ms' if not _isna(deltaMs) else 'N/A'
        change = f'{changePct:+.1f}%' if not _isna(changePct) else 'N/A'
        print(f'{name:<40} {baseline:>10} {optimized:>10} {delta:>10} {change:>8}{marker}')

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

    print(f'\nTrace Summary: {nImproved} improved, {nRegressed} regressed, '
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
    
    # Optimization tracking legend
    if optimizedMaterials:
        print(f'\n* = affected by optimization ({len(optimizedMaterials)} materials)')


def createTraceChart(data, outputPath, title=None, minDeltaMs=0.0,
                     optimizedMaterials=None, optimizationName=None):
    '''
    Create a paired before/after horizontal bar chart sorted by absolute time saved.

    Args:
        data: DataFrame or list of dicts with comparison results
        minDeltaMs: Minimum delta threshold (shown in title)
        outputPath: Path to save the chart image
        title: Optional chart title
        optimizedMaterials: Set of material names affected by optimization (highlighted)
        optimizationName: Name of the optimization pass (for title/legend)

    Requires: matplotlib, pandas
    '''
    if data is None:
        return
        
    if optimizedMaterials is None:
        optimizedMaterials = set()
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

    # Track which materials are affected by the optimization
    chartDf['is_optimized'] = chartDf['name'].isin(optimizedMaterials)
    
    # Create display names with absolute delta and percentage
    # Prefix with "★ " for materials affected by the optimization
    def make_label(row):
        name = row['name'][:28] + '...' if len(row['name']) > 28 else row['name']
        delta = row['delta_ms']
        pct = row['change_pct']
        prefix = '★ ' if row['is_optimized'] else ''
        if pd.notna(delta) and pd.notna(pct):
            return f"{prefix}{name} ({delta:+.1f}ms, {pct:+.1f}%)"
        return f"{prefix}{name}"

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
    
    # Highlight Y-axis labels for materials affected by the optimization
    if optimizedMaterials:
        for i, (label, isOpt) in enumerate(zip(ax.get_yticklabels(), chartDf['is_optimized'])):
            if isOpt:
                label.set_fontweight('bold')
                label.set_color('#8e44ad')  # Purple for highlighted items

    if title:
        ax.set_title(title)
    else:
        filterNote = f', filtered |Δ| ≥ {minDeltaMs:.1f}ms' if minDeltaMs > 0 else ''
        optNote = f'\n★ = affected by {optimizationName}' if optimizationName and optimizedMaterials else ''
        ax.set_title(f'Absolute Duration Comparison: Baseline vs Optimized\n(sorted by time saved{filterNote}){optNote}')

    # Legend
    legendElements = [
        Patch(facecolor='#3498db', label='Baseline'),
        Patch(facecolor='#2ecc71', label='Optimized (faster)'),
        Patch(facecolor='#e74c3c', label='Optimized (slower)')
    ]
    ax.legend(handles=legendElements, loc='lower right')

    plt.tight_layout()
    plt.savefig(outputPath, dpi=150, bbox_inches='tight')
    logger.info(f'Trace chart saved to: {outputPath}')


# =============================================================================
# IMAGE COMPARISON
# =============================================================================

def findImages(directory, pattern='**/*.png'):
    '''Find all PNG images in a directory recursively.'''
    directory = Path(directory)
    if not directory.exists():
        raise FileNotFoundError(f'Directory not found: {directory}')
    return list(directory.glob(pattern))


def computeImageDiff(img1Path, img2Path, ppd=70.0, heatmapPath=None):
    '''
    Compute FLIP perceptual difference metrics between two images.
    
    FLIP (A Difference Evaluator for Alternating Images) is a perceptual
    image comparison metric from NVIDIA that approximates human perception
    of differences when flipping between images.
    
    Args:
        img1Path: Path to reference (baseline) image
        img2Path: Path to test (optimized) image
        ppd: Pixels per degree (viewing distance). Default 70 assumes
             a 0.7m viewing distance for a 1080p 24" monitor.
        heatmapPath: Optional path to save FLIP heatmap image (magma colormap)
    
    Returns:
        dict with keys: mean_flip, max_flip, pct_diff_pixels, identical, heatmap_path
    '''
    import numpy as np
    
    # FLIP evaluate() accepts file paths directly as strings
    # Returns: (error_map, mean_error, params)
    # error_map is HxWx3 if applyMagma=True (colored heatmap), HxW if False
    try:
        flipMap, meanFlip, _ = flip.evaluate(
            str(img1Path),
            str(img2Path),
            "LDR",  # Low dynamic range (PNG images)
            inputsRGB=True,
            applyMagma=False,  # Get raw FLIP values for metrics
            computeMeanError=True,
            parameters={"ppd": ppd}
        )
    except Exception as e:
        return {
            'error': str(e),
            'identical': False
        }
    
    # flipMap is a numpy array with per-pixel FLIP values in [0, 1]
    flipMap = np.array(flipMap)
    
    # Maximum FLIP value
    maxFlip = float(flipMap.max())
    
    # Percentage of pixels with perceptible difference (FLIP > 0.01)
    # 0.01 is a reasonable threshold for "just noticeable difference"
    diffPixels = flipMap > 0.01
    pctDiffPixels = 100.0 * diffPixels.sum() / diffPixels.size
    
    result = {
        'mean_flip': float(meanFlip),
        'max_flip': maxFlip,
        'pct_diff_pixels': pctDiffPixels,
        'identical': meanFlip < 1e-6,
        'heatmap_path': None
    }
    
    # Save heatmap if requested
    if heatmapPath:
        try:
            # Re-run with magma colormap for visualization
            heatmapImg, _, _ = flip.evaluate(
                str(img1Path),
                str(img2Path),
                "LDR",
                inputsRGB=True,
                applyMagma=True,  # Get colored heatmap
                computeMeanError=False,
                parameters={"ppd": ppd}
            )
            # Save as PNG
            from PIL import Image
            heatmapArr = np.array(heatmapImg)
            if heatmapArr.max() <= 1.0:
                heatmapArr = (heatmapArr * 255).astype(np.uint8)
            Image.fromarray(heatmapArr).save(heatmapPath)
            result['heatmap_path'] = str(heatmapPath)
        except Exception as e:
            logger.warning(f'Failed to save heatmap: {e}')
    
    return result


def compareImages(baselineDir, optimizedDir, threshold=0.05, ppd=70.0, reportDir=None):
    '''
    Compare all matching images between two directories using FLIP.
    
    Args:
        baselineDir: Path to baseline images
        optimizedDir: Path to optimized images
        threshold: FLIP threshold above which to report differences (default: 0.05)
        ppd: Pixels per degree for FLIP calculation (default: 70)
        reportDir: Optional directory to save FLIP heatmaps for HTML report
        
    Returns:
        List of comparison results with paths for report generation
    '''
    if not _have_flip:
        logger.error('Cannot compare images: flip-evaluator not installed.')
        logger.error('Install with: pip install flip-evaluator')
        return None

    baselineDir = Path(baselineDir)
    optimizedDir = Path(optimizedDir)
    
    # Create heatmap directory if generating report
    heatmapDir = None
    if reportDir:
        heatmapDir = Path(reportDir) / 'heatmaps'
        heatmapDir.mkdir(parents=True, exist_ok=True)
    
    baselineImages = findImages(baselineDir)
    logger.info(f'Found {len(baselineImages)} images in baseline')
    
    results = []
    matched = 0
    missing = 0
    
    for baselineImg in baselineImages:
        # Get relative path from baseline directory
        relPath = baselineImg.relative_to(baselineDir)
        optimizedImg = optimizedDir / relPath
        
        if not optimizedImg.exists():
            logger.warning(f'Missing in optimized: {relPath}')
            missing += 1
            continue
        
        matched += 1
        
        # Determine heatmap path if saving for report
        heatmapPath = None
        if heatmapDir:
            heatmapPath = heatmapDir / f'{relPath.stem}_flip.png'
        
        metrics = computeImageDiff(baselineImg, optimizedImg, ppd, heatmapPath)
        metrics['name'] = relPath.stem
        metrics['path'] = str(relPath)
        # Store absolute paths for HTML report
        metrics['baseline_path'] = str(baselineImg.absolute())
        metrics['optimized_path'] = str(optimizedImg.absolute())
        results.append(metrics)
    
    logger.info(f'Compared {matched} image pairs, {missing} missing')
    return results


def printImageTable(results, threshold=0.05):
    '''Print a formatted FLIP image comparison table.'''
    if results is None:
        return False
        
    print('\n' + '=' * 85)
    print(f"{'Image':<40} {'Mean FLIP':>10} {'Max FLIP':>10} {'% Diff':>10} {'Status':>8}")
    print('=' * 85)
    
    identical = 0
    different = 0
    errors = 0
    
    # Sort by mean FLIP (highest first)
    sortedResults = sorted(results, key=lambda x: x.get('mean_flip', 0), reverse=True)
    
    for r in sortedResults:
        name = r['name'][:38]
        
        if 'error' in r:
            print(f"{name:<40} {'ERROR':>10} {r['error']}")
            errors += 1
            continue
            
        meanFlip = r['mean_flip']
        maxFlip = r['max_flip']
        pctDiff = r['pct_diff_pixels']
        
        if r['identical']:
            status = 'OK'
            identical += 1
        elif meanFlip < threshold:
            status = 'OK'
            identical += 1
        else:
            status = 'DIFF'
            different += 1
        
        print(f"{name:<40} {meanFlip:>10.6f} {maxFlip:>10.4f} {pctDiff:>9.2f}% {status:>8}")
    
    print('=' * 85)
    print(f'\nImage Summary (FLIP): {identical} identical, {different} different, {errors} errors')
    print(f'Threshold: mean FLIP < {threshold}')
    
    if different > 0:
        print(f'\n*** WARNING: {different} images differ above threshold! ***')
        return False
    else:
        print('\nAll images match within threshold.')
        return True


# =============================================================================
# HTML REPORT GENERATION
# =============================================================================

def generateHtmlReport(reportPath, traceData=None, imageResults=None, 
                       chartPath=None, threshold=0.05):
    '''
    Generate an HTML report with performance chart and image comparisons.
    
    Args:
        reportPath: Path to output HTML file
        traceData: Trace comparison results (DataFrame or list of dicts)
        imageResults: Image comparison results from compareImages()
        chartPath: Path to performance chart image (optional)
        threshold: FLIP threshold used for pass/fail
    '''
    reportPath = Path(reportPath)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)
    
    # Convert paths to be relative to report directory for portability
    def relPath(absPath):
        if absPath is None:
            return None
        try:
            return str(Path(absPath).relative_to(reportDir))
        except ValueError:
            # Path is not relative to report dir, use absolute with file:// prefix
            return 'file:///' + str(Path(absPath)).replace('\\', '/')
    
    html = []
    html.append('''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MaterialX Test Comparison Report</title>
    <style>
        * { box-sizing: border-box; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            margin: 0; padding: 20px; background: #f5f5f5;
        }
        .container { max-width: 1800px; margin: 0 auto; }
        h1, h2 { color: #333; }
        h1 { border-bottom: 2px solid #3498db; padding-bottom: 10px; }
        
        /* Summary cards */
        .summary { display: flex; gap: 20px; margin-bottom: 30px; flex-wrap: wrap; }
        .card { 
            background: white; border-radius: 8px; padding: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1); flex: 1; min-width: 200px;
        }
        .card h3 { margin: 0 0 10px 0; color: #666; font-size: 14px; text-transform: uppercase; }
        .card .value { font-size: 32px; font-weight: bold; }
        .card .value.good { color: #27ae60; }
        .card .value.bad { color: #e74c3c; }
        .card .value.neutral { color: #3498db; }
        
        /* Chart */
        .chart-container { background: white; border-radius: 8px; padding: 20px; margin-bottom: 30px; }
        .chart-container img { max-width: 100%; height: auto; }
        
        /* Trace table */
        table { width: 100%; border-collapse: collapse; background: white; border-radius: 8px; overflow: hidden; }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #eee; }
        th { background: #3498db; color: white; }
        tr:hover { background: #f8f9fa; }
        .improved { color: #27ae60; }
        .regressed { color: #e74c3c; }
        
        /* Image comparison grid */
        .image-grid { display: grid; gap: 20px; }
        .image-row {
            display: grid; grid-template-columns: 1fr 1fr 1fr;
            gap: 10px; background: white; border-radius: 8px; padding: 15px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .image-row.failed { border-left: 4px solid #e74c3c; }
        .image-row.passed { border-left: 4px solid #27ae60; }
        .image-cell { text-align: center; }
        .image-cell img { max-width: 100%; height: auto; border: 1px solid #ddd; border-radius: 4px; }
        .image-cell .label { font-size: 12px; color: #666; margin-top: 5px; }
        .image-header { 
            grid-column: 1 / -1; display: flex; justify-content: space-between;
            align-items: center; padding-bottom: 10px; border-bottom: 1px solid #eee;
        }
        .image-header h3 { margin: 0; }
        .image-header .metrics { font-size: 14px; color: #666; }
        .status-badge {
            display: inline-block; padding: 4px 12px; border-radius: 20px;
            font-size: 12px; font-weight: bold;
        }
        .status-badge.pass { background: #d4edda; color: #155724; }
        .status-badge.fail { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
<div class="container">
    <h1>MaterialX Test Comparison Report</h1>
''')
    
    # Summary cards
    traceImproved = 0
    traceRegressed = 0
    imgPassed = 0
    imgFailed = 0
    
    if traceData is not None:
        if _have_pandas and hasattr(traceData, 'iterrows'):
            traceImproved = len(traceData[traceData['change_pct'] < 0])
            traceRegressed = len(traceData[traceData['change_pct'] > 0])
        else:
            traceImproved = sum(1 for r in traceData if r.get('change_pct', 0) and r['change_pct'] < 0)
            traceRegressed = sum(1 for r in traceData if r.get('change_pct', 0) and r['change_pct'] > 0)
    
    if imageResults:
        for r in imageResults:
            if 'error' not in r:
                if r['identical'] or r['mean_flip'] < threshold:
                    imgPassed += 1
                else:
                    imgFailed += 1
    
    html.append(f'''
    <div class="summary">
        <div class="card">
            <h3>Trace Improvements</h3>
            <div class="value good">{traceImproved}</div>
        </div>
        <div class="card">
            <h3>Trace Regressions</h3>
            <div class="value {'bad' if traceRegressed > 0 else 'neutral'}">{traceRegressed}</div>
        </div>
        <div class="card">
            <h3>Images Passed</h3>
            <div class="value good">{imgPassed}</div>
        </div>
        <div class="card">
            <h3>Images Failed</h3>
            <div class="value {'bad' if imgFailed > 0 else 'neutral'}">{imgFailed}</div>
        </div>
    </div>
''')
    
    # Performance chart
    if chartPath and Path(chartPath).exists():
        chartRel = relPath(chartPath)
        html.append(f'''
    <h2>Performance Comparison</h2>
    <div class="chart-container">
        <img src="{chartRel}" alt="Performance Chart">
    </div>
''')
    
    # Image comparisons
    if imageResults:
        html.append('''
    <h2>Image Comparisons (FLIP)</h2>
    <p>FLIP score: 0 = identical, 1 = maximally different. Threshold: ''' + f'{threshold}' + '''</p>
    <div class="image-grid">
''')
        # Sort by mean_flip descending (worst first)
        sortedImages = sorted(imageResults, key=lambda x: x.get('mean_flip', 0), reverse=True)
        
        for r in sortedImages:
            if 'error' in r:
                continue
            
            passed = r['identical'] or r['mean_flip'] < threshold
            statusClass = 'passed' if passed else 'failed'
            statusBadge = 'pass' if passed else 'fail'
            statusText = 'PASS' if passed else 'FAIL'
            
            baselineRel = relPath(r.get('baseline_path'))
            optimizedRel = relPath(r.get('optimized_path'))
            heatmapRel = relPath(r.get('heatmap_path'))
            
            html.append(f'''
        <div class="image-row {statusClass}">
            <div class="image-header">
                <h3>{r['name']}</h3>
                <div class="metrics">
                    Mean FLIP: {r['mean_flip']:.4f} | Max: {r['max_flip']:.4f} | {r['pct_diff_pixels']:.1f}% pixels differ
                    <span class="status-badge {statusBadge}">{statusText}</span>
                </div>
            </div>
            <div class="image-cell">
                <img src="{baselineRel}" alt="Baseline">
                <div class="label">Baseline</div>
            </div>
            <div class="image-cell">
                <img src="{optimizedRel}" alt="Optimized">
                <div class="label">Optimized</div>
            </div>
            <div class="image-cell">
''')
            if heatmapRel:
                html.append(f'''                <img src="{heatmapRel}" alt="FLIP Heatmap">
                <div class="label">FLIP Heatmap</div>
''')
            else:
                html.append('''                <div style="padding: 50px; color: #999;">No heatmap</div>
''')
            html.append('''            </div>
        </div>
''')
        
        html.append('    </div>\n')
    
    # Footer
    html.append('''
    <footer style="margin-top: 40px; padding-top: 20px; border-top: 1px solid #ddd; color: #666; font-size: 12px;">
        Generated by diff_test_results.py | NVIDIA FLIP for perceptual image comparison
    </footer>
</div>
</body>
</html>
''')
    
    # Write HTML file
    with open(reportPath, 'w', encoding='utf-8') as f:
        f.write(''.join(html))
    
    logger.info(f'HTML report saved to: {reportPath}')


# =============================================================================
# MAIN ENTRY POINT
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Compare test results between baseline and optimized MaterialX runs.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s ./baseline/ ./optimized/                    # Compare both traces and images
  %(prog)s ./baseline/ ./optimized/ --traces           # Compare traces only
  %(prog)s ./baseline/ ./optimized/ --images           # Compare images only
  %(prog)s ./baseline/ ./optimized/ --min-delta-ms 1.0 # Filter trace noise
  %(prog)s ./baseline/ ./optimized/ --chart chart.png  # Generate performance chart
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline directory (with traces and/or images)')
    parser.add_argument('optimized', type=Path,
                        help='Optimized directory (with traces and/or images)')
    
    # Mode selection
    modeGroup = parser.add_argument_group('comparison mode')
    modeGroup.add_argument('--traces', action='store_true',
                           help='Compare performance traces only')
    modeGroup.add_argument('--images', action='store_true',
                           help='Compare rendered images only')
    
    # Trace options
    traceGroup = parser.add_argument_group('trace options')
    traceGroup.add_argument('--track', '-t', type=str, default=None,
                            help='Filter traces by track name (e.g., GPU, ShaderGen)')
    traceGroup.add_argument('--min-delta-ms', type=float, default=0.0,
                            help='Minimum absolute time difference in ms to include')
    traceGroup.add_argument('--chart', '-c', type=Path,
                            help='Output path for trace chart image (e.g., chart.png)')
    traceGroup.add_argument('--show-opt', type=str, metavar='OPT_NAME',
                            help='Highlight materials affected by optimization pass')
    
    # Image options (FLIP perceptual comparison)
    imageGroup = parser.add_argument_group('image options')
    imageGroup.add_argument('--image-threshold', type=float, default=0.05,
                            help='FLIP threshold for image differences (default: 0.05)')
    imageGroup.add_argument('--ppd', type=float, default=70.0,
                            help='Pixels per degree for FLIP (default: 70, ~0.7m from 24" 1080p)')
    
    # Output options
    outputGroup = parser.add_argument_group('output options')
    outputGroup.add_argument('--csv', type=Path,
                             help='Export trace results to CSV file')
    outputGroup.add_argument('--title', type=str,
                             help='Custom title for the chart')
    outputGroup.add_argument('--report', type=Path,
                             help='Generate HTML report with side-by-side image comparison')

    args = parser.parse_args()

    # Default: compare both if neither specified
    doTraces = args.traces or (not args.traces and not args.images)
    doImages = args.images or (not args.traces and not args.images)
    
    allPassed = True
    traceData = None
    imageResults = None
    chartPath = None
    
    # Determine report directory for saving heatmaps
    reportDir = None
    if args.report:
        reportDir = args.report.parent
        # If generating report, auto-generate chart if not specified
        if not args.chart:
            chartPath = reportDir / 'chart.png'
        else:
            chartPath = args.chart

    try:
        # -------------------------
        # Trace Comparison
        # -------------------------
        if doTraces:
            print('\n' + '=' * 85)
            print('PERFORMANCE TRACE COMPARISON')
            print('=' * 85)
            
            if not _have_perfetto:
                logger.warning('Skipping traces: perfetto not installed (pip install perfetto)')
            else:
                traceData = compareTraces(args.baseline, args.optimized,
                                          args.track, args.min_delta_ms)
                
                # Load optimization events if requested
                optimizedMaterials = set()
                if args.show_opt and traceData is not None:
                    baselineTracePath = findTraceFile(args.baseline)
                    baselineMaterials = loadOptimizationEvents(baselineTracePath, args.show_opt)
                    if baselineMaterials:
                        logger.error(f'ERROR: Baseline has {len(baselineMaterials)} materials '
                                    f'with {args.show_opt}!')
                        sys.exit(1)
                    
                    optimizedTracePath = findTraceFile(args.optimized)
                    optimizedMaterials = loadOptimizationEvents(optimizedTracePath, args.show_opt)
                    logger.info(f'Found {len(optimizedMaterials)} materials affected by {args.show_opt}')
                
                printTraceTable(traceData, optimizedMaterials)

                # Generate chart (for report or if explicitly requested)
                actualChartPath = chartPath or args.chart
                if actualChartPath and traceData is not None:
                    createTraceChart(traceData, actualChartPath, title=args.title,
                                     minDeltaMs=args.min_delta_ms,
                                     optimizedMaterials=optimizedMaterials,
                                     optimizationName=args.show_opt)

                if args.csv and traceData is not None:
                    if _have_pandas:
                        df = traceData if hasattr(traceData, 'to_csv') else pd.DataFrame(traceData)
                        df.to_csv(args.csv, index=False)
                        logger.info(f'CSV exported to: {args.csv}')
                    else:
                        logger.error('Cannot export CSV: pandas not installed.')

        # -------------------------
        # Image Comparison (FLIP)
        # -------------------------
        if doImages:
            print('\n' + '=' * 85)
            print('RENDERED IMAGE COMPARISON (FLIP)')
            print('=' * 85)
            
            if not _have_flip:
                logger.warning('Skipping images: flip-evaluator not installed (pip install flip-evaluator)')
            else:
                imageResults = compareImages(args.baseline, args.optimized, 
                                             args.image_threshold, args.ppd,
                                             reportDir=reportDir)
                imagesMatch = printImageTable(imageResults, args.image_threshold)
                if not imagesMatch:
                    allPassed = False

        # -------------------------
        # HTML Report Generation
        # -------------------------
        if args.report:
            generateHtmlReport(
                args.report,
                traceData=traceData,
                imageResults=imageResults,
                chartPath=chartPath,
                threshold=args.image_threshold
            )

        # -------------------------
        # Final Summary
        # -------------------------
        print('\n' + '=' * 85)
        if allPassed:
            print('RESULT: All comparisons passed')
        else:
            print('RESULT: Some comparisons FAILED - review above for details')
        print('=' * 85)
        
        sys.exit(0 if allPassed else 1)

    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)
    except Exception as e:
        logger.error(f'Error: {e}')
        raise


if __name__ == '__main__':
    main()

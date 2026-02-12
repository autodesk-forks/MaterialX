#!/usr/bin/env python
'''
Compare dumped shader files between baseline and optimized MaterialX test runs.

Computes per-material metrics from generated GLSL shader source files and,
optionally, from external shader analysis tools found in PATH.

Built-in metrics (always available):
  LOC   Lines of code (non-blank lines in the pixel shader)

Tool-based metrics (when the tool is in PATH):
  glslangValidator  SPIR-V size (bytes) after GLSL -> SPIR-V compilation
  spirv-opt         SPIR-V size after optimisation passes
  rga               VGPR count from AMD Radeon GPU Analyzer

Usage:
    python diff_shaders.py <baseline_dir> <optimized_dir>
    python diff_shaders.py <baseline_dir> <optimized_dir> -o shader_diff.html
'''

import argparse
import logging
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger('diff_shaders')

# Import shared reporting utilities (works both as package and standalone script)
try:
    from ._report import (mergeComparison, printComparisonTable,
                           createComparisonChart, generateHtmlReport,
                           chartPath, openReport)
except ImportError:
    from _report import (mergeComparison, printComparisonTable,
                          createComparisonChart, generateHtmlReport,
                          chartPath, openReport)


# =============================================================================
# SHADER FILE DISCOVERY
# =============================================================================

def findShaderPairs(baselineDir, optimizedDir, pattern='**/*_ps.glsl'):
    '''
    Find matching pixel shader files between baseline and optimized directories.

    Args:
        baselineDir: Path to baseline shader directory
        optimizedDir: Path to optimized shader directory
        pattern: Glob pattern for shader files (default: pixel shaders)

    Returns:
        List of (materialName, baselinePath, optimizedPath) tuples,
        sorted by material name.
    '''
    baselineDir = Path(baselineDir)
    optimizedDir = Path(optimizedDir)

    pairs = []
    for baselineFile in sorted(baselineDir.glob(pattern)):
        relPath = baselineFile.relative_to(baselineDir)
        optimizedFile = optimizedDir / relPath

        if not optimizedFile.exists():
            logger.warning(f'Missing in optimized: {relPath}')
            continue

        # Derive material name from filename (strip _ps.glsl suffix)
        stem = baselineFile.stem
        if stem.endswith('_ps'):
            materialName = stem[:-3]
        elif stem.endswith('_vs'):
            materialName = stem[:-3]
        else:
            materialName = stem

        pairs.append((materialName, baselineFile, optimizedFile))

    logger.info(f'Found {len(pairs)} matching shader pairs')
    return pairs


# =============================================================================
# METRICS: LOC
# =============================================================================

def countLoc(shaderPath):
    '''Count non-blank lines in a shader file.'''
    text = Path(shaderPath).read_text(encoding='utf-8', errors='replace')
    return sum(1 for line in text.splitlines() if line.strip())


def computeLocMetrics(pairs):
    '''
    Compute LOC (lines of code) for all shader pairs.

    Returns:
        (baselineDict, optimizedDict) -- {materialName: loc} for each side.
    '''
    baseline = {}
    optimized = {}
    for materialName, baselinePath, optimizedPath in pairs:
        baseline[materialName] = countLoc(baselinePath)
        optimized[materialName] = countLoc(optimizedPath)
    return baseline, optimized


# =============================================================================
# METRICS: SPIR-V SIZE (via glslangValidator)
# =============================================================================

def _compileToSpirv(glslPath, outputPath):
    '''Compile a GLSL shader to SPIR-V using glslangValidator. Returns True on success.'''
    try:
        result = subprocess.run(
            ['glslangValidator', '-V', '-S', 'frag', '-o', str(outputPath), str(glslPath)],
            capture_output=True, text=True, timeout=30
        )
        return result.returncode == 0
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return False


def computeSpirvSizeMetrics(pairs):
    '''
    Compute SPIR-V binary size for all shader pairs via glslangValidator.

    Returns:
        (baselineDict, optimizedDict) or (None, None) if tool not available.
    '''
    if not shutil.which('glslangValidator'):
        return None, None

    logger.info('Computing SPIR-V sizes via glslangValidator...')
    baseline = {}
    optimized = {}

    with tempfile.TemporaryDirectory(prefix='mtlx_spirv_') as tmpDir:
        tmpPath = Path(tmpDir)
        for materialName, baselinePath, optimizedPath in pairs:
            bOut = tmpPath / f'{materialName}_baseline.spv'
            oOut = tmpPath / f'{materialName}_optimized.spv'

            if _compileToSpirv(baselinePath, bOut) and _compileToSpirv(optimizedPath, oOut):
                baseline[materialName] = bOut.stat().st_size
                optimized[materialName] = oOut.stat().st_size
            else:
                logger.warning(f'SPIR-V compilation failed for {materialName}')

    logger.info(f'Compiled {len(baseline)} shader pairs to SPIR-V')
    return baseline, optimized


# =============================================================================
# METRICS: OPTIMISED SPIR-V SIZE (via spirv-opt)
# =============================================================================

def _optimizeSpirv(inputPath, outputPath):
    '''Run spirv-opt on a SPIR-V binary. Returns True on success.'''
    try:
        result = subprocess.run(
            ['spirv-opt', '-O', '-o', str(outputPath), str(inputPath)],
            capture_output=True, text=True, timeout=60
        )
        return result.returncode == 0
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return False


def computeOptSpirvSizeMetrics(pairs):
    '''
    Compute optimised SPIR-V binary size (glslangValidator + spirv-opt).

    Returns:
        (baselineDict, optimizedDict) or (None, None) if tools not available.
    '''
    if not shutil.which('glslangValidator') or not shutil.which('spirv-opt'):
        return None, None

    logger.info('Computing optimised SPIR-V sizes via spirv-opt...')
    baseline = {}
    optimized = {}

    with tempfile.TemporaryDirectory(prefix='mtlx_spirvopt_') as tmpDir:
        tmpPath = Path(tmpDir)
        for materialName, baselinePath, optimizedPath in pairs:
            bSpv = tmpPath / f'{materialName}_baseline.spv'
            oSpv = tmpPath / f'{materialName}_optimized.spv'
            bOpt = tmpPath / f'{materialName}_baseline_opt.spv'
            oOpt = tmpPath / f'{materialName}_optimized_opt.spv'

            bOk = _compileToSpirv(baselinePath, bSpv) and _optimizeSpirv(bSpv, bOpt)
            oOk = _compileToSpirv(optimizedPath, oSpv) and _optimizeSpirv(oSpv, oOpt)

            if bOk and oOk:
                baseline[materialName] = bOpt.stat().st_size
                optimized[materialName] = oOpt.stat().st_size
            else:
                logger.warning(f'spirv-opt pipeline failed for {materialName}')

    logger.info(f'Optimised {len(baseline)} shader pairs')
    return baseline, optimized


# =============================================================================
# MAIN
# =============================================================================

def main():
    parser = argparse.ArgumentParser(
        description='Compare dumped shader files between baseline and optimized MaterialX test runs.',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s ./baseline/ ./optimized/
  %(prog)s ./baseline/ ./optimized/ -o shader_diff.html
  %(prog)s ./baseline/ ./optimized/ --pattern "**/*_vs.glsl"

Available metrics depend on tools found in PATH:
  LOC               Always available (non-blank line count)
  SPIR-V size       Requires glslangValidator
  Optimised SPIR-V  Requires glslangValidator + spirv-opt
''')

    parser.add_argument('baseline', type=Path,
                        help='Baseline directory containing dumped shaders')
    parser.add_argument('optimized', type=Path,
                        help='Optimized directory containing dumped shaders')
    parser.add_argument('-o', '--outputfile', type=str, default=None,
                        help='Output HTML report file name '
                             '(default: <baseline>_vs_<optimized>_shaders.html)')
    parser.add_argument('--pattern', type=str, default='**/*_ps.glsl',
                        help='Glob pattern for shader files (default: **/*_ps.glsl)')

    args = parser.parse_args()

    # Discover shader pairs
    try:
        pairs = findShaderPairs(args.baseline, args.optimized, args.pattern)
    except FileNotFoundError as e:
        logger.error(f'{e}')
        sys.exit(1)

    if not pairs:
        logger.error('No matching shader pairs found.')
        sys.exit(1)

    # Directory leaf names for display
    baselineName = Path(args.baseline).name
    optimizedName = Path(args.optimized).name

    # Derive default report name
    if args.outputfile is None:
        args.outputfile = f'{baselineName}_vs_{optimizedName}_shaders.html'

    reportPath = Path(args.outputfile)
    reportDir = reportPath.parent
    reportDir.mkdir(parents=True, exist_ok=True)
    chartBase = reportDir / (reportPath.stem + '.svg')

    reportSections = []

    # Discover available tools
    tools = {
        'glslangValidator': shutil.which('glslangValidator'),
        'spirv-opt': shutil.which('spirv-opt'),
        'rga': shutil.which('rga'),
    }
    foundTools = [name for name, path in tools.items() if path]
    if foundTools:
        logger.info(f'Found tools in PATH: {", ".join(foundTools)}')
    else:
        logger.info('No optional shader tools found in PATH; only LOC will be computed')

    # ---- Metric: LOC ----
    logger.info('Computing LOC metrics...')
    bLoc, oLoc = computeLocMetrics(pairs)
    locData = mergeComparison(bLoc, oLoc)

    title = f'Lines of Code (non-blank): {baselineName} vs {optimizedName}'
    printComparisonTable(locData, title,
                         baselineLabel=baselineName, optimizedLabel=optimizedName,
                         unit='', valueFormat='.0f')

    if not locData.empty:
        svgPath = chartPath(chartBase, 'LOC')
        createComparisonChart(locData, svgPath, title=title,
                              baselineLabel=baselineName, optimizedLabel=optimizedName,
                              unit=' lines')
        reportSections.append((title, svgPath))

    # ---- Metric: SPIR-V size ----
    bSpirv, oSpirv = computeSpirvSizeMetrics(pairs)
    if bSpirv is not None:
        spirvData = mergeComparison(bSpirv, oSpirv)
        title = f'SPIR-V Size (bytes): {baselineName} vs {optimizedName}'
        printComparisonTable(spirvData, title,
                             baselineLabel=baselineName, optimizedLabel=optimizedName,
                             unit=' B', valueFormat='.0f')

        if not spirvData.empty:
            svgPath = chartPath(chartBase, 'SPIRV')
            createComparisonChart(spirvData, svgPath, title=title,
                                  baselineLabel=baselineName, optimizedLabel=optimizedName,
                                  unit=' B')
            reportSections.append((title, svgPath))

    # ---- Metric: Optimised SPIR-V size ----
    bOptSpirv, oOptSpirv = computeOptSpirvSizeMetrics(pairs)
    if bOptSpirv is not None:
        optSpirvData = mergeComparison(bOptSpirv, oOptSpirv)
        title = f'Optimised SPIR-V Size (bytes): {baselineName} vs {optimizedName}'
        printComparisonTable(optSpirvData, title,
                             baselineLabel=baselineName, optimizedLabel=optimizedName,
                             unit=' B', valueFormat='.0f')

        if not optSpirvData.empty:
            svgPath = chartPath(chartBase, 'SPIRV_opt')
            createComparisonChart(optSpirvData, svgPath, title=title,
                                  baselineLabel=baselineName, optimizedLabel=optimizedName,
                                  unit=' B')
            reportSections.append((title, svgPath))

    # ---- HTML Report ----
    pageTitle = f'Shader Comparison: {baselineName} vs {optimizedName}'
    if reportSections:
        generateHtmlReport(reportPath, reportSections, pageTitle=pageTitle,
                           footerText='Generated by diff_shaders.py')
        openReport(reportPath)
    else:
        print(f'\n{"=" * 85}')
        print('  No data to report.')
        print(f'{"=" * 85}')


if __name__ == '__main__':
    main()

'''
Utility to render all MaterialX renderable elements found in a given folder or file.
By default the output image(s) is written to the current working directory. 
'''
import MaterialX as mx
from mtlxutils import mxrenderer
from mtlxutils import mxbase
import os
import argparse
import logging

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

def loadLibraries(searchPath, libraryFolders):
    status = ''
    lib = mx.createDocument()
    try:
        libFiles = mx.loadLibraries(libraryFolders, searchPath, lib)
        status = '- Loaded %d library definitions from %d files (%s)' % (len(lib.getNodeDefs()), len(libFiles), searchPath)
    except mx.Exception as err:
        status = '- Failed to load library definitions: "%s"' % err

    return lib, status

def createWorkingDocument(libraries):
    # Create a working document and import any libraries
    doc = mx.createDocument()
    for lib in libraries:
        doc.importLibrary(lib)

    return doc

def haveVersion(major, minor, patch):
    """
    Check if the current version matches a given version
    """ 
    return mxbase.haveVersion(major, minor, patch)

def getFiles(rootPath):
    filelist = []
    exts = ('mtlx', 'MTLX')
    for subdir, dirs, files in os.walk(rootPath):
        for file in files:
            if file.lower().endswith(exts):
                filelist.append(os.path.join(subdir, file)) 
    return filelist

def main():
    parser = argparse.ArgumentParser(description="Extract out source implementation information.")
    parser.add_argument('--outputPath', dest='outputPath', default="", help="Output path. Default is empty.")
    parser.add_argument('--path', dest='paths', action='append', nargs='+', help='An additional absolute search path location (e.g. "/projects/MaterialX")')
    parser.add_argument('--library', dest='libraries', action='append', nargs='+', help='An additional relative path to a custom data library folder (e.g. "libraries/custom")')
    parser.add_argument('--geometryPath', dest='geometryPath', default="", help="Path to geometry shape. Default is empty")
    parser.add_argument('--radiancePath', dest='radiancePath', default="", help="Path to radiance IBL file. Default is empty.")
    parser.add_argument('--irradiancePath', dest='irradiancePath', default="", help="Path to irradiance IBL file. Default is empty.")
    parser.add_argument('--size', dest='size', default=-1, type=int, help="Size of the render. Default is 512.")
    parser.add_argument(dest="inputFileName", help="Filename of the input document.")
    opts = parser.parse_args()

    if not haveVersion(1, 38, 7):
        logger.error('Minimum MaterialX version is 1.38.7. Exiting')
        exit(-1)

    # Get absolute path of opts.outputPath
    if opts.outputPath:    
        opts.outputPath = os.path.abspath(opts.outputPath)
    outputPath = mx.FilePath(opts.outputPath)
    # Check that output path exists
    if outputPath.size() > 0 and not os.path.isdir(outputPath.asString()):
        logger.error('Output path "%s" does not exist.', outputPath.asString())
        exit(-1)

    fileList = []
    if os.path.isdir(opts.inputFileName): 
        fileList = getFiles(opts.inputFileName)
    else:
        fileList.append(opts.inputFileName)

    # Load standard libraries
    libraries = []
    searchPath = mx.getDefaultDataSearchPath()
    libraryFolders = mx.getDefaultDataLibraryFolders()
    print(f'searchPath: {searchPath}. libraryFolders: {libraryFolders}')
    stdlib, status = loadLibraries(searchPath, libraryFolders)
    if not stdlib:
        logger.error('Error loading standard libraries: "%s"', status)
        exit(-1)
    else:
        logger.info(status)
    libraries.append(stdlib)

    # Check for additional use libraries
    userPath = mx.FileSearchPath()
    userLibraryFolders = []
    if opts.paths:
        for pathList in opts.paths:
            for path in pathList:
                searchPath.append(path)
                userPath.append(path)
    if opts.libraries:
        for libraryList in opts.libraries:
            for library in libraryList:
                userLibraryFolders.append(library)

    userlib, status = loadLibraries(userPath, userLibraryFolders)
    if not userlib:
        logger.error('Error loading user libraries: "%s"', status)
        exit(-1)
    else:
        logger.info('Loaded user libraries successfully: "%s"', status)
    libraries.append(userlib)

    renderer = None
    if fileList:
        w = h = opts.size
        # Load in lighting.
        radianceFilePath = opts.radiancePath
        irradianceFilePath = opts.irradiancePath
        if not os.path.exists(radianceFilePath) or not os.path.exists(irradianceFilePath):
            logger.error('-- Radiance or Irradiance file does not exist. Exiting')
            exit(-1)

        # Load in geometry.
        geometryShape = './notebooks/data/sphere.obj'
        if len(opts.geometryPath) > 0:
            geometryShape = opts.geometryPath
        if not os.path.exists(geometryShape):
            logger.error('-- Geometry shape "%s" does not exist. Exiting', geometryShape)
            exit(-1)
        renderer = mxrenderer.initializeRenderer(stdlib, searchPath, radianceFilePath, irradianceFilePath, w, h, 
                                                 geometryShape)
        renderer.addToRenderLog('--------------------------')

    if not renderer:
        logger.error('Error initializing renderer')
        exit(-1)

    for fileName in fileList:
        fullSearchPath = searchPath

        # Create a new working document for each file
        doc = createWorkingDocument(libraries)
        if not doc:
            logger.error('Error creating working document')
            continue

        try:
            mx.readFromXmlFile(doc, fileName)        
            valid, msg = doc.validate()
            if not valid:
                raise mx.Exception(msg)

            # Add the absolute path directory of the file to the search path
            dirname = os.path.dirname(fileName)
            abspath = os.path.abspath(dirname)
            fullSearchPath.append(abspath)   

        except mx.ExceptionFileMissing as err:
            logger.error('File %s could not be loaded: "%s"', fileName, err)
            continue
        except mx.Exception as err:
            logger.error('File %s failed to load properly: "%s"', fileName, err)
            continue

        logger.info('Render file: %s', fileName)
        renderer.addToRenderLog('Render file:' + fileName + '. SearchPath: ' + fullSearchPath.asString())
        rendered, errors = mxrenderer.performRender(renderer, doc, fileName, outputPath, fullSearchPath)
        if not rendered:
            logger.error('Error rendering file: "%s"', fileName)
            logger.error('Errors: "%s"', errors)
        renderer.addToRenderLog('--- Finished rendering file "%s"\n' % fileName)

    # Open text file
    logger.info('Wrote render log to: render_log.txt')
    with open('render_log.txt', 'w') as f:
        f.write('\n'.join(renderer.getRenderLog()))        

if __name__ == '__main__':
    main()

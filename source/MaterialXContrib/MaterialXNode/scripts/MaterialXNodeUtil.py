import os
import maya.cmds as cmds

from subprocess import Popen
from distutils.spawn import find_executable

def editMaterialXDocument(documentPath, element, editor):
    editor_path = find_executable(editor)
    if editor_path == None:
        print ("Unable to find editor: " + editor)
    else:
        print "Launching editor..."
        editor_dir = os.path.dirname(editor_path)
        Popen([editor, documentPath, element], cwd=editor_dir)

def editMaterialXNode(nodeName):
    documentFilePath = cmds.getAttr(nodeName + "documentFilePath")
    elementPath = cmds.getAttr(nodeName + "elementPath")
    editor = "LookdevX.exe"
    editMaterialXDocument(documentFilePath, elementPath, editor)

def getMaterialXNodesForDocument(documentPath):
    nodes = cmds.ls(type="MaterialXNode")
    results = []
    for node in nodes:
        nodeDocumentPath = cmds.getAttr(node + ".documentFilePath")
        if os.path.realpath(nodeDocumentPath) == os.path.realpath(documentPath):
            results.append(node)
    return results

def reloadMaterialXNodesForDocument(documentPath):
    nodes = getMaterialXNodesForDocument(documentPath)
    for node in nodes:
        cmds.reloadMaterialXNode(node)

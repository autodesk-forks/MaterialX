# MaterialXView Module

This module contains the source for a sample **MaterialX Viewer**, which leverages shader generation to generate GLSL shaders from MaterialX graphs, rendering the results using the NanoGUI framework.  Both the standard set of MaterialX nodes and the PBR node set are supported.

### Viewer Options

1. **Load Mesh**: Load in geometry for viewing. There is currently support for OBJ file loading.
2. **Load Material**: Load in a MaterialX document containing elements to render.
3. **Property Editor**: View or edit the properties for the current  element selected.
4. **Advanced Settings** : Load and fidelity options.

### Example Images

**Standard Surface Shader with procedural and uniform materials**
<img src="/documents/Images/MaterialXView_StandardSurface_01.png" width="1024">

**Standard Surface Shader with textured, color-space-managed materials**
<img src="/documents/Images/MaterialXView_StandardSurface_02.png" width="480">

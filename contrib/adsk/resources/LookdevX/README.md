# LookdevX

Some graphs found in the adsk contrib libraries are authored and generated from LookdevX within Maya.
This is a work in progress, and LookdevX is changing constantly, but it's so far the most reliable way to author graphs.

How to organize the "MaterialX stack" inside a Maya scene, is also something not well defined yet. Some Maya files might store a single graph, some might store multiple graphs.
Often graphs are numbered, and the highest number is the most current.
Some graphs might just be development tests, or usage test. The name is usually self explanatory.

The publishing process is still incomplete, so there is manual work to do, especially to adjust or match node, graphs, and definition names.

Coordinates are not removed when publishing, which might make diffs hard to read sometime. But having coordinates allows us to re-import those graphs back into LookdevX or Graph editor, and keeping the layout is important.
If options to remove coordinates when publishing will become available, we will consider it, as long as LookdevX becomes our authoring tool of choice.


All paths here (in double-quotes) are relative to the root of the 'Antiquated' repository. On the author's PC that is D:\GameDev\Antiquated - this may be different on the current setup.

**Overview**

The Antiquated "Interface" is a 2d graphics layer built on top of DirectX 9. It provides wrapper functionality for various aspects 
2d graphics (fonts, untextured and textured sprites/rectangles and other primitive shapes, etc). Also handles the creation and management
of the DirectX device.

**Folder Structure**

Interface project : "Pub\Projects\Interface.vcxproj"

Most code files for the interface are kept in "Pub\LibCode\Interface"
External headers are in "Pub\Include"

**Interface Dependencies**

- The Interface project relies on the presence of the accompanying CodeUtil library:  "Pub\Projects\CodeUtil.vcxproj"
- "Pub\Include\CodeUtil.h"
- "Pub\Include\StandardDef.h"

There is the notion of support for different renderers (OpenGL, DX11) but these are not fully supported or implemented, so for now 
we should assume a dependency on DirectX 9.

**Interface Headers**
Read 
- "Pub\Include\Interface.h" (this externs most of the Engine functionality)
- "Pub\Include\EngineMaths.h" 

Functionality specific to rendering, animation and manipulation of 3d models is separated off into "Pub\Include\Rendering.h"

**Engine Sample**
A basic example of using the engine is contained in Samples\02_BasicEngine.vcxproj



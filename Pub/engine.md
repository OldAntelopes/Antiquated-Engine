
All paths here (in double-quotes) are relative to the root of the 'Antiquated' repository. On the author's PC that is D:\GameDev\Antiquated - this may be different on the current setup.

**Overview**

The Antiquated "Engine" is a 3d graphics layer built on top of DirectX 9. It provides wrapper functionality for various aspects 
3d graphics (Vertex buffers, texture handling, mesh creation, render target handling etc) and maths. (Vector, matrix, quaternion classes etc)

**Folder Structure**

Engine project : "Pub\Projects\Engine.vcxproj"

Most code files for the engine are kept in "Pub\LibCode\Engine"
External headers are in "Pub\Include"

The project generates as output "Pub\Libs\EngineD-DX9.lib" (DX9 debug version) or "Pubs\Libs\Engine-DX9.lib" (DX9 Release version)


**Engine Dependencies**

- The Engine project relies on the presence of the accompanying CodeUtil library:  "Pub\Projects\CodeUtil.vcxproj"
- "Pub\Include\CodeUtil.h"
- "Pub\Include\StandardDef.h"

**Engine Headers**
Read 
- "Pub\Include\Engine.h" (this externs most of the Engine functionality)
- "Pub\Include\EngineMaths.h" 

Functionality specific to rendering, animation and manipulation of 3d models is separated off into "Pub\Include\Rendering.h"

**Engine Sample**
A basic example of using the engine is contained in Samples\02_BasicEngine.vcxproj



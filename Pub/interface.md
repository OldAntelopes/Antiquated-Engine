All paths here (in double-quotes) are relative to the root of the 'Antiquated' repository (One layer up from this file). 
On the author's PC that is D:\GameDev\Antiquated - this may be different on the current setup.

**Overview**

The Antiquated "Interface" is a 2d graphics layer built on top of DirectX 9. It provides wrapper functionality for various aspects 
2d graphics (fonts, untextured and textured sprites/rectangles and other primitive shapes, etc). Also handles the creation and management
of the DirectX device.

**Folder Structure**

Interface project : "Pub\Projects\Interface.vcxproj"

Most code files for the interface are kept in "Pub\LibCode\Interface"
External headers are in "Pub\Include"

The project generates as output "Pub\Libs\InterfaceD-DX9.lib" (DX9 debug version) or "Pubs\Libs\Interface-DX9.lib" (DX9 Release version)

**Interface Dependencies**

- The Interface project relies on the presence of the accompanying CodeUtil library:  "Pub\Projects\CodeUtil.vcxproj"
- "Pub\Include\CodeUtil.h"
- "Pub\Include\StandardDef.h"

There is the notion of support for different renderers (OpenGL, DX11) but these are not fully supported or implemented, so for now 
we should assume a dependency on DirectX 9.

**Interface Headers**
Read 
- "Pub\Include\Interface.h" (this externs most of the base Interface functionality)
- "Pub\Include\InterfaceEx.h"  (this externs core Interface functionality through a class interface, which is used by applications that require multiple interface instances (e.g. multiple windows on different graphics display devices)

**Interface Tests**
There are no tests currently available for the Interface library.

**Interface Sample**
A basic example of using the interface is contained in Samples\01_BasicInterface.vcxproj



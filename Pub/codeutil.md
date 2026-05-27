All paths here (in double-quotes) are relative to the root of the 'Antiquated' repository (One layer up from this file). 
On the author's PC that is D:\GameDev\Antiquated - this may be different on the current setup.

**Overview**

CodeUtil is a library providing common base framework code defining common platform-agnostic functions and type defines etc

**Folder Structure**

CodeUtil project : "Pub\Projects\CodeUtil.vcxproj"

Code files for the library are kept in "Pub\LibCode\CodeUtil"
External headers are in "Pub\Include"

The project generates as output "Pub\Libs\CodeUtilD.lib" (debug version) or "Pubs\Libs\CodeUtil.lib" (Release version)

**CodeUtil Headers**
Read 
- "Pub\Include\StandardDef.h" (this is a common standard defines header. 
StandardDef.h includes:
- "Pub\Include\CodeUtil.h"
- "Pub\Include\System.h"

Relatedly, there is
- "Pub\Include\stdwininclude.h" which is included by any part of the application that requires specific Win32 functionality

**CodeUtil Tests**
There are no tests currently available for the CodeUtil library.




Coding Standards
----------------

House style for first-party code (Pub, GameCommon, Samples, Tools). Derived by measuring the existing code
(~600 files, ~138k lines); where the codebase is inconsistent the majority convention was adopted and the
split is noted. Third-party / vendored code keeps its own style — never reformat it.

**Golden rule: when editing an existing file, match that file.** These standards decide new files and
break ties; they are not a licence to reformat code you are not otherwise changing.


File format
-----------

| Rule | Measured |
|------|----------|
| CRLF line endings | 570 of 598 files. The 28 LF-only files (mostly Pub\LibCode\CodeUtil and parts of Engine) keep LF - don't convert a file's endings as a side effect of an edit. |
| No BOM, plain ASCII | 0 BOMs; 7 files contain stray non-ASCII bytes in comments. Use ASCII only in new code. |
| Indent with **tabs** (tab width 4) | ~95% of indented lines. Tabs are also used to align declarations and columns. |
| `.h` / `.cpp` (`.c` only for genuinely C files) | |
| One public header per library in Pub\Include; internal headers live beside their .cpp | |


Layout
------

Allman braces, always (13,471 own-line braces vs 128 end-of-line):

```cpp
void		EntityManagerUpdate( float fDelta )
{
Entity*		pEntity = mspFirstEntity;
int			nCount = 0;

	while ( pEntity )
	{
		if ( pEntity->IsActive() )
		{
			pEntity->Update( fDelta );
			nCount++;
		}
		else
		{
			pEntity->OnInactive();
		}
		pEntity = pEntity->GetNext();
	}
	return( nCount );
}
```

- **Local variables are declared at the top of the function body, at column 0** (unindented), aligned with
  tabs, followed by a blank line; statements are then indented one tab. (1,587 vs 231 functions.)
  Declaring a loop counter or a variable mid-function is fine when it reads better, but keep the top block.
- **Spaces inside parentheses** for calls, declarations and control statements: `Foo( a, b )`, `if ( bFlag )`.
  (~21.5k vs ~4.3k.) Exceptions that are conventionally tight: `sizeof(x)`, casts `(byte*)p`, empty `()`,
  and nested trivial calls `GetModuleHandle(NULL)`.
- **Space between a control keyword and its paren**: `if (`, `for (`, `while (`, `switch (`.
  (`if (` 6,143 vs 220; `for (` 581 vs 144; `while`/`switch` are roughly split - use the space in new code.)
- **`return( value );`** with parentheses (2,493 vs ~580). Plain `return;` for void.
- `else` on its own line (1,085 vs 29 `} else`).
- `case` labels at the same indent as the `switch` braces; case bodies indented one more tab.
- Function definitions: return type, tab(s), name - `void\t\tEngineEnableVR( BOOL bEnable )`. (2,144 vs 523.)
- `( void )` for empty parameter lists in C-linkage declarations/definitions; `()` is acceptable for C++ methods.
- Pointer star binds to the type: `char* szName`, `VECT* pxPos` (2,921 vs 216).
- Float literals always carry `f`: `1.0f`, `0.5f`. Hex literals upper-case: `0xFFFFFFFF`.
- Colours are `uint32` ARGB written as hex: `0xFF101010`.
- No hard line-length limit, but keep new lines under ~160 columns; long public prototypes are tolerated.


Naming
------

### Functions & types

| Thing | Convention | Examples |
|-------|------------|----------|
| Public C-API function | `ModulePrefix` + PascalCase verb phrase | `InterfaceTextGetWidth`, `EngineCameraSetPos`, `ModelRender`, `SysFileOpen`, `PlatformInit`, `HTTPGet` |
| Class | PascalCase, no `C` prefix | `InterfaceInstance`, `EntityManager`, `UIXButton` |
| Class method | PascalCase; event hooks `OnXxx` | `Init`, `Update`, `Render`, `OnUpdateComponent` |
| C struct | `typedef struct { } UPPER_SNAKE;` | `ENGINE_LIGHT`, `SYS_LOCALTIME`, `FONT_UVCHAR` |
| C++-only struct | PascalCase | `InterfaceRECT` |
| Enum constants | `UPPER_SNAKE` with a shared group prefix | `BLEND_MODE_ALPHABLEND`, `KEY_ESCAPE`, `UIXBUTTON_DISABLED` |
| Named enum type | `eXxx` or `UPPER_SNAKE` (typedef'd) | `eRenderFlags`, `INTF_RENDER_TYPES` |
| Macro | `UPPER_SNAKE` | `SAFE_DELETE`, `REGISTER_ENTITY`, `NOTFOUND` |
| Callback typedef | `fnXxx`, `XxxHandler`, `XxxCallback` | `fnFetchFileCallback`, `HTTPResponseHandler` |
| Handle typedef | `UPPER_SNAKE` over `int` | `TEXTURE_HANDLE`, `VERTEX_BUFFER_HANDLE` |
| File | PascalCase, named after its class/module; back-end suffix `DX`, `GL`, `Win32`, `Marmalade` | `ShadowMapDX.cpp`, `PlatformWin32.cpp` |

### Variables (Hungarian prefixes)

Hungarian notation is used consistently across the codebase and is required for new code.

| Prefix | Type | Examples |
|--------|------|----------|
| `n` | `int` (also counts, indices) | `nLayer`, `nNumVerts` |
| `f` | `float` | `fDelta`, `fScale` |
| `b` | `BOOL` / `bool` (`bo` is an older variant - accept, don't add) | `bFlag`, `bFullscreen` |
| `ul` | `uint32` / `unsigned long` | `ulCol`, `ulRenderFlags` |
| `ull` | `u64` | `ullEventID` |
| `uw` / `w` | `ushort` / `short` | `uwQuantity`, `wDay` |
| `uc` / `c` | `uchar` / `char` | `cChar` |
| `dw` | `DWORD` | `dwWindowStyle` |
| `sz` | zero-terminated string (`const char*` / `char*`) | `szFilename` |
| `pc` | `char*` pointer into a buffer / text being walked | `pcRunner` |
| `ac` | `char[]` array | `acFilename` |
| `a` + type | other arrays | `afAngles`, `anIndices` |
| `p` | pointer to object / void | `pEntity`, `pUserParam` |
| `px` | pointer to struct (VECT, matrix, etc.) | `pxPos`, `pxMatrix` |
| `x` | struct by value | `xPos`, `xLightDir` |
| `pb` | `byte*` / `BYTE*` buffer | `pbMem` |
| `h` | handle (int) | `hModel`, `hTexture` |
| `fn` | function pointer | `fnHandler` |

Scope prefixes go in front of the type prefix:

| Prefix | Scope | Examples |
|--------|-------|----------|
| `m` | class member (no underscore) | `mnWidth`, `mpNext`, `mxPos`, `mfnCallback` (4,004 uses vs 902 for `m_`) |
| `ms` | static class member, or file-scope variable | `mspMainInterfaceSingleton`, `msnNextEntityTypeID` |
| `g` | true global shared across files (`extern` in a header) | `ghwndMain`, `gnHeightmapWidth` |

Older files use `m` / `m_p` for file-scope variables (`mnDesiredWidth`, `m_pEngineGameInterface`); leave them,
but use `ms` for new file-scope variables and declare them `static` unless they genuinely need external linkage.

### Spelling

British English in new identifiers and comments: `Colour`, `Initialise`, `Centre`, `Normalise`
(the codebase is majority British; `color` appears mostly from D3D/third-party names). Existing American
or misspelled public names stay as they are (`InterfaceTextCenter` and `InterfaceTextCentre` both exist).


Headers
-------

- Include guards, not `#pragma once` (261 of 298 headers): `#ifndef MODULE_FILE_H` / `#define MODULE_FILE_H` /
  `#endif`. Prefix with the library or module (`GAMECOMMON_UI_H`, `INTERFACE_INSTANCE_H`, `ENGINE_INSTANCE_H`).
  Old public headers use `TRACTOR_`/`UNIVERSAL_` prefixes - don't rename them.
- Public C-API headers wrap declarations in `#ifdef __cplusplus extern "C" { #endif ... }` and declare
  functions with `extern`, aligning return types and names in tab columns.
- Forward-declare classes (`class InterfaceInstance;`) rather than including heavy headers.
- Don't use STL types in Pub\Include public headers. Internal / GameCommon C++ may use `<vector>`, `<map>`,
  `<string>` sparingly (UIX does), but the codebase is overwhelmingly raw arrays and intrusive linked lists
  (`mpNext`), and new code in existing modules should match its neighbours.
- Gate platform-specific declarations the existing way (`#ifdef DIRECT3D_VERSION`, `#ifdef WINUSERAPI`).

Include order in a .cpp:

```cpp
#include <StdWinInclude.h>		// only if this file needs real windows.h types - must come first
#include <StandardDef.h>
#include <Interface.h>			// then other Pub public headers
#include <Engine.h>

#include "LocalModule.h"		// then local/internal headers with relative quoted paths
#include "../Common/Other.h"
```

Pub public headers are included as `<StandardDef.h>` or `"StandardDef.h"` (both common - 121 vs 161; match the
file). Use forward slashes in `#include` paths (498 vs 25 backslash).


Language & idioms
-----------------

The code is "C with classes" written for MSVC. Keep to that dialect so it reads like the rest:

- `BOOL` / `TRUE` / `FALSE` (531 vs 46 for `bool`). Use `bool` only where an external API needs it.
- `NULL`, not `nullptr` (1,468 vs 3).
- C-style casts are the norm (`static_cast` appears 4 times).
- `NOTFOUND` (-1) for "no handle / not found" returns and invalid handle sentinels.
- Ownership: `new`/`delete` for objects; `malloc`/`free` or `SystemMalloc`/`SystemFree` for raw buffers.
  Clean up with `SAFE_DELETE`, `SAFE_DELETE_ARRAY`, `SAFE_FREE`, `SAFE_RELEASE` (COM), `SAFE_RELEASE_TEXTURE`.
- No exceptions, no RTTI-dependent code, no templates in first-party code (0 occurrences), essentially no
  namespaces (only inside `REGISTER_*` macros). Don't introduce them into existing modules.
- `virtual` with empty default bodies for overridable hooks (`virtual void OnUpdate( float fDelta ) {}`).
- Debug output: `SysDebugPrint( "fmt", ... )`. Fatal errors: `SysPanicIf( cond, "fmt", ... )` / `PANIC_IF`.
  User-facing message box: `SysMessageBox`. Not `printf`/`OutputDebugString` directly.
- File I/O through `SysFileOpen/Read/Write/Close` and `SysFileAllocateAndLoad` (they handle archives/paths),
  not raw `fopen`, in Pub and GameCommon.
- Timing via `SysGetTick()` (ms), `SysGetMicrosecondTick()`, `SysGetFrameDelta()`.
- Win32 API calls use the ANSI (`A`) variants implicitly - projects are MultiByte.
- Fixed-size `char` buffers for strings (`char acBuffer[256]`). For **new** code use bounded functions
  (`snprintf`, `strncpy` + explicit terminator, or the `_s` variants) rather than adding more `sprintf`/`strcpy`.


Comments
--------

- `//` line comments (7,852 vs 746 block). Short, informal tone is fine - it's the house voice.
- Section separators: a dashed line `//-------------------------------------------------------------` with an
  optional title, between groups of functions and within headers (802 uses).
- Public headers group functions under banner comments (`/*****  Interface - Main Update Functions  *****/`).
- Optional function header used in Samples/older files:

  ```cpp
  //-------------------------------------------------------------------------
  // Function    : MainUpdate
  // Description : Called each frame to do all we need to do and draw all we need to draw
  //-------------------------------------------------------------------------
  ```

- Mark known shortcuts with `// TODO - ...` (existing convention, ~300 occurrences).
- Don't strip the author's existing comments or jokes; they're part of the history of the code.


Templates for new files
-----------------------

`GameCommon/Example/ExampleThing.h`

```cpp
#ifndef GAMECOMMON_EXAMPLE_THING_H
#define GAMECOMMON_EXAMPLE_THING_H

class InterfaceInstance;

//----------------------------------------------------------------------
// ExampleThing
//   One-line description of what this is for
//----------------------------------------------------------------------
class ExampleThing
{
public:
	ExampleThing();
	virtual ~ExampleThing();

	void		Initialise( InterfaceInstance* pInterface );
	void		Update( float fDelta );
	void		Render( void );
	void		Shutdown( void );

	int			GetNumItems( void ) { return( mnNumItems ); }

protected:
	virtual void	OnItemAdded( int nItemNum ) {}

private:
	InterfaceInstance*		mpInterface = NULL;
	int						mnNumItems = 0;
	float					mfTimer = 0.0f;
};

#endif
```

`GameCommon/Example/ExampleThing.cpp`

```cpp
#include <StandardDef.h>
#include <Interface.h>

#include "ExampleThing.h"

static int		msnExampleThingInstances = 0;

//----------------------------------------------------------------------

ExampleThing::ExampleThing()
{
	msnExampleThingInstances++;
}

ExampleThing::~ExampleThing()
{
	msnExampleThingInstances--;
}

void	ExampleThing::Update( float fDelta )
{
int		nLoop;

	mfTimer += fDelta;
	for ( nLoop = 0; nLoop < mnNumItems; nLoop++ )
	{
		// ...
	}
}
```

A public C-API addition (e.g. in Pub\Include\Engine.h + Pub\LibCode\Engine\...):

```cpp
// Engine.h, inside the extern "C" block, next to related functions
extern void		EngineSetFogDensity( float fDensity );

// Engine.cpp / EngineDX.cpp
void		EngineSetFogDensity( float fDensity )
{
	EngineInstanceMain()->mpEngineInternals->SetFogDensity( fDensity );
}
```


Modernisation guidelines
------------------------

The long-term aim is a more modern, better-structured codebase **without breaking the interfaces existing
projects use**. Concretely:

Do freely (internal, non-breaking):
- Fix bugs, leaks, buffer overruns; replace unbounded string ops with bounded ones in code you touch.
- Move file-global state into the `...Instance` classes behind the existing C functions.
- Add `const` to *new* parameters, add new functions/overloads alongside old ones.
- Add documentation (`*.md`) and comments.

Do only when asked / with care:
- Reformatting whole files, renaming internal symbols across files, moving files between folders
  (vcxproj files list every source file and must be updated too).
- Touching non-DX9 back-ends beyond keeping signatures in sync.
- Syncing the forked copies in Tools\TileLandscapeGen etc. with GameCommon.

Don't (breaking):
- Change, rename or remove public functions/types/enums in Pub\Include or GameCommon module headers, or change
  their linkage, calling convention, argument order or meaning. Add a new function instead and leave the old
  one (optionally marked `// Deprecated - use Xxx`).
- Renumber existing enum values or reorder public struct members (binary data and old code depend on them).
- Change the ATM file format in a way that stops existing .atm files loading (see ATMFormat.md); new data goes in
  new chunk codes.
- Switch the API to `bool`, `nullptr`, STL types, exceptions, namespaces, or Unicode.
- Change CRT linkage (/MT), character set, or library output names.

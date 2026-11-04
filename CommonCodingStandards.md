Common Coding Standards
----------------

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
| Enum constants | `UPPER_SNAKE` with a shared group prefix | `BLEND_MODE_ALPHABLEND`, `KEY_ESCAPE`, `UIXBUTTON_DISABLED` |
| Named enum type | `eXxx` or `UPPER_SNAKE` (typedef'd) | `eRenderFlags`, `INTF_RENDER_TYPES` |
| Macro | `UPPER_SNAKE` | `SAFE_DELETE`, `REGISTER_ENTITY`, `NOTFOUND` |
| Callback typedef | `fnXxx`, `XxxHandler`, `XxxCallback` | `fnFetchFileCallback`, `HTTPResponseHandler` |
| Handle typedef | `UPPER_SNAKE` over `int` | `TEXTURE_HANDLE`, `VERTEX_BUFFER_HANDLE` |

### Variables (Hungarian prefixes)

Hungarian notation should generally be used.

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
or misspelled public names stay as they are


Headers
-------
- Public C-API headers wrap declarations in `#ifdef __cplusplus extern "C" { #endif ... }` and declare
  functions with `extern`, aligning return types and names in tab columns.
- Forward-declare classes (`class InterfaceInstance;`) rather than including heavy headers.


Language & idioms
-----------------

The developer generally prefers:

- `NULL`, not `nullptr`
- `NOTFOUND` (-1) for "no handle / not found" returns and invalid handle sentinels.

Comments
--------

- `//` line comments (7,852 vs 746 block). Short, informal tone is fine - it's the house voice.
- Section separators: a dashed line `//-------------------------------------------------------------` with an
  optional title, between groups of functions and within headers (802 uses).
- Mark known shortcuts with `// TODO - ...` 
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
- Add `const` to *new* parameters, add new functions/overloads alongside old ones.
- Add documentation (`*.md`) and comments.

Do only when asked / with care:
- Reformatting whole files, renaming internal symbols across files, moving files between folders
  (vcxproj files list every source file and must be updated too).

Architecture & Codebase Map
---------------------------

Working notes on how the Antiquated codebase fits together. Covers Pub, GameCommon, Samples and Tools.
3rdParty, Exe and Milkdrop are out of scope here (treat them as vendored / external code).

All paths are relative to the repo root. (The docs in Pub\ say `D:\GameDev\Antiquated`, the author's checkout;
yours may be somewhere else.)


Layering
--------

    Application (game / Sample / Tool)                 -> builds an .exe into Exe\ (or Tools\Exe, Samples\..\Exe)
        |
        +-- GameCommon\<Module>\*.cpp                   -> compiled directly INTO the app project (no .lib)
        |
        +-- Pub libs (static .lib, linked)              -> Pub\Libs\*.lib
              Engine     (3D, ATM models, DX9)          depends on CodeUtil, Interface
              Interface  (2D, window + D3D device)      depends on CodeUtil
              CodeUtil   (platform layer, strings, files, archives, profiling)
              Browser    (tiny HTML renderer "NoddyBrowser" -> UniversalBrowser.lib; rarely used)
              Lua, Bullet (LinearMath/BulletCollision/BulletDynamics) - vendored, built from Pub\Libraries.sln

Rules of thumb:
- Lower layers never include higher ones. CodeUtil knows nothing of Interface; Interface knows nothing of Engine
  (exception: Interface.h forward-declares a few things and Engine.h forward-declares `InterfaceInstance`).
- GameCommon modules depend on Pub, and sometimes on each other (e.g. EntitySystem -> Physics, UIX -> Interface).
- Apps tie everything together and own `WinMain` (directly, or via GameCommon\Platform).


Public headers (Pub\Include)
----------------------------

Applications add `Pub\Include` to their include path and include headers with angle brackets or quotes
(both are used; see CodingStandards.md).

| Header           | Purpose |
|------------------|---------|
| StdWinInclude.h  | Pulls in `<windows.h>` (WIN32_LEAN_AND_MEAN) + mmsystem. **Must be included before StandardDef.h** in any file that needs real Win32 types. |
| StandardDef.h    | The "everything" base header. Re-declares a minimal subset of windows.h types (BOOL, DWORD, BYTE, RECT, HWND...) when windows.h was *not* included, plus project types (`uint32`, `u64`, `ushort`, `uchar`, `byte`, `VECT`, `ENGINEQUATERNION`, `ENGINEMATRIX`), constants (`NOTFOUND`=-1, `PI`, `A90` etc), `SAFE_DELETE/SAFE_RELEASE/SAFE_FREE`, `DEGTORAD`. Includes CodeUtil.h and System.h. |
| CodeUtil.h       | String helpers (`tinstricmp`, `SplitStringSep`, `ScriptGetNextLine`...), colour helpers (`COLOUR`, `GetBlendedCol`), `FRand/RandInt/FClamp`, archive mount/extract. |
| System.h         | Platform abstraction: `SysDebugPrint`, `SysPanicIf`, time/tick (`SysGetTick`, `SysGetFrameDelta`), keyboard (`KEY_*`, `SysCheckKeyState`), files (`SysFileOpen`, `SysFileAllocateAndLoad`...), dialogs, threads, profiling (`SysProfileStartEvent`). One implementation per platform in Pub\LibCode\CodeUtil\Sys*.c/.cpp. |
| Interface.h      | 2D API: window/device init, frame loop, rects/lines/text, textures, textured overlays, images, fonts. C linkage. |
| InterfaceEx.h    | Interface.h + `InterfaceInstance` class (multi-window / multi-device). |
| Engine.h         | 3D API: render state, shaders (.fx), shadows, textures, render targets, camera, lights, collision, vertex/index buffers, materials. C linkage, plus C++ callback interfaces `EngineGameInterface`, `SceneMapGameInterface`. |
| EngineEx.h       | Engine.h + `EngineInstance`. |
| EngineMaths.h    | Vector/matrix/quaternion C functions on `VECT` / `ENGINEMATRIX`. |
| Rendering.h      | ATM model API: `ModelLoad`, `ModelRender`, `ModelFree`, animation, sub-models, materials (handles are `int`). |
| UnivSocket.h, UnivObfuscation.h | Small legacy networking / obfuscation helpers from The Universal. |

Everything else in Pub\Include (Bullet*, LinearMath, DirectX, GL, curl, fbxsdk, jpg, png, steam, zlib, cJSON) is
third-party and should not be edited.


API design patterns (preserve these)
------------------------------------

The public interfaces are what existing games (The Universal, Elemental) compile against. Keep them stable.

1. **C-linkage free functions with a module prefix.** `InterfaceXxx`, `EngineXxx`, `ModelXxx`, `SysXxx`,
   `PlatformXxx`, `HTTPXxx`, `EntityManagerXxx`. Declared `extern` in the public header inside
   `#ifdef __cplusplus extern "C" { #endif`. Default arguments are used in a few places (C++-only callers).
2. **Integer handles, not pointers.** Textures (`TEXTURE_HANDLE` = int), models, vertex/index buffers, overlays,
   images, fonts, archives, UI textboxes/dropdowns are all `int` handles. `NOTFOUND` (-1) means invalid/failed.
   Release with the matching `...Release...`/`...Free...` call (`SAFE_RELEASE_TEXTURE(h)` for Interface textures).
3. **Callbacks are C function pointers** typedef'd as `fnXxx` / `XxxHandler` / `XxxCallback`, usually with a
   `void* pUserParam` or `uint32 ulParam`.
4. **Init / Update / Render / Free lifecycle.** Each module has `XxxInit*`, `XxxUpdate( float fDelta )` (or `uint32` ms),
   optional `XxxRender`, and `XxxFree` / `XxxShutdown`.
5. **Instance classes behind the C API (newer refactor, in progress).** `InterfaceInstance`, `EngineInstance`,
   `UIInstance` hold state that used to be file-globals. The C functions forward to the main singleton
   (`InterfaceInstanceMain()`, `EngineInstanceMain()`). New functionality should go into the instance class
   *and* get a C wrapper if it belongs in the public API. Many members are still public with
   `// TODO - Make these private` notes.
6. **Modules inside a library** derive from small base classes (`InterfaceModule`, `EngineModule`) which receive the
   instance + device pointer and get `ReleasePreGraphicsDeviceReset()` / `RestorePostGraphicsDeviceReset()` hooks.
   Anything owning D3D resources in DEFAULT pool must handle device reset this way.
7. **Factory registration macros** for extensible types: `REGISTER_ENTITY( Class, "Name" )` (EntitySystem),
   `REGISTER_REQUEST_HANDLER( Class, "path" )` (WebServer). Each derived .cpp uses the macro at file scope.
8. **Renderer back-ends are selected by preprocessor + per-backend subfolders**: `DirectX\`, `OpenGL\`,
   `Marmalade\`, `Win32\`, with file suffixes `...DX.cpp`, `...GL.cpp`. Only DirectX 9 is real (see below).


Typical application lifecycle
-----------------------------

Hand-rolled (Samples\00..02):

    WinMain
      InterfaceInitWindow( title, &WNDCLASSEX, bAllowResize );  ghwndMain = InterfaceGetWindow();
      InterfaceInitDisplayDevice( 0 );      // creates D3D9 device
      InterfaceInit( TRUE );                // loads default fonts / core textures
      EngineInitFromInterface();            // only if using Engine
      loop (PeekMessage / else MainUpdate):
          EngineUpdate( TRUE ); ModelRenderingUpdate( ms );   // 3D apps
          InterfaceNewFrame( clearCol );
          InterfaceBeginRender();   ...3D render... draw 2D...   InterfaceDraw();   InterfaceEndRender();
          InterfacePresent();
      InterfaceFree();                      // (+ EngineFree( TRUE ) when the Engine was used)

Via GameCommon\Platform (PlatformWin32.cpp owns `WinMain`): the app must provide
`MainInitialise()`, `MainUpdate()`, `MainHighFreqUpdate()`, `MainShutdown()`, `MainOnPress(x,y)`,
`MainOnRelease(x,y)`, `MainOnReleaseRightButton(x,y)`, and calls `PlatformInit( name, bAllowResize, bAllowSleep )`.

2D drawing is layer-based and batched: `InterfaceRect/Text/...( nLayer, ... )` queue primitives, and
`InterfaceDraw()` flushes them. Textured drawing requires creating an overlay per texture per frame
(`InterfaceCreateNewTexturedOverlay( layer, hTex )`) then drawing into it (`InterfaceTexturedRect( overlay, ... )`).
Colours are packed `uint32` 0xAARRGGBB.

The working directory at runtime must be `Exe\` (data files are loaded relative to it, e.g. `Data\Samples\...`).


Pub\LibCode layout
------------------

    Pub\LibCode\CodeUtil   Sys<Platform>.c/.cpp (Windows, Unix, GTK, Marmalade, consoles), Archive, AsyncFile,
                           StringFunctions, SysProfiling
    Pub\LibCode\Interface  Common\ (InterfaceInstance, Canvas, Overlays, Fonts, texture lists, options)
                           DirectX\ (DX9Device, DX11Device stub, FontDX, Image, Jpeg), OpenGL\, Marmalade\, Win32\
    Pub\LibCode\Engine     Engine.cpp, EngineCamera, EngineInstance, EngineMesh, ModelFiles/ModelArchive (ATM
                           load/save), ModelRendering, ModelMaterialData, SkinnedModel, Loader, Maths
                           DirectX\ (EngineDX, TextureManagerDX, ShaderEffectsDX, ShadowMapDX, CascadeShadowMapDX,
                           PostProcessDX, SkinMesh*, OculusDX ...), OpenGL\, FileFormats\ (3ds/obj/fbx),
                           Scene\ (SceneMap), RenderScene\, BSPRender\, CollisionMaths\ (3rd-party-ish maths)
                           Skeleton\ (experimental skeletal animation: BVH clips, retargeting, CPU skinning,
                           .atm chunks 0x1C-0x1E - see Pub\skeletalanimation.md)
    Pub\Projects           *.vcxproj for each lib; Pub\Libraries.sln builds them all
    Pub\Libs               Output .libs + prebuilt third-party .libs (curl, ssl, fbxsdk, bullet...)
    Pub\Objs               Intermediate files

Internal (non-public) headers live next to their .cpp in LibCode and are included with relative quotes.
EngineEx.h / InterfaceEx.h reach into LibCode on purpose to expose the instance classes.


GameCommon
----------

Source-level component library: an app adds the .cpp files of the modules it wants to its own project
(e.g. ModelConverter compiles `..\GameCommon\RenderUtil\RenderObject.cpp`). There is no GameCommon .lib/.vcxproj.
See GameCommon\Readme.md for the module list. Additional notes:

- **UIX** (not in the readme) is the newer retained-mode UI (`UIXPage`, `UIXButton`, `UIXListBox`, ...,
  reserved button IDs from 0x550000). **UI** is the older immediate-mode UI (`UIInstance::ButtonDraw` each frame).
  Prefer UIX for new screens; keep UI working for existing ones.
- **Platform** is C-linkage like the Pub APIs; Win32, Marmalade and Steam-commerce implementations.
- **Physics** wraps Bullet (`btBulletWorldImporter` etc are vendored Bullet extras). BulletFileLoader is vendored.
- **WebServer\Tcp4u** and **tcp4u** are vendored copies of the tcp4u library.
- **Util\cJSON.c**, **Util\Blowfish** are vendored. (`Pub\LibCode\Interface\Common\cJSON.c` is another copy.)


Samples & Tools
---------------

- Samples\Samples.sln: 00_HelloWorld (Interface only), 01_BasicInterface, 02_BasicEngine (ATM model + camera),
  04_MilkdropPlayer (links Milkdrop source). 03_ArcadeGameSample.vcxproj exists but is not in the .sln, its file
  paths are broken (`..\..\Samples\...`), and the game itself is a placeholder.
- Tools\Tools.sln: **ModelConverter** (main ATM authoring tool: FBX import/export, materials, sub-models, UV unwrap,
  primitives) and **SceneEditor**. Both link the `Tool Debug/Tool Release` Engine build (`EngineTool[D].lib`, `TOOL` define).
- Tools\TileLandscapeGen, TreeSpriteGen, InteriorMapEditor: source only, **no project files**. They contain
  their own *forked, diverged* copies of GameCommon modules (Platform, UI, Landscape...). Do not assume they match
  GameCommon and do not "sync" them without being asked.


Build system
------------

- Visual Studio 2022, toolset **v143**, MSBuild `.vcxproj` / `.sln`. No CMake for first-party code
  (the CMakeLists.txt files are vendored Bullet ones). Git repo, published on GitHub; working copies use
  `core.autocrlf=true`, so files are CRLF on disk.
- Character set **MultiByte** (not Unicode) — Win32 calls use the `A` variants via `char*`. Keep it so.
- Static CRT: `/MT` (Release) and `/MTd` (Debug). Every lib and app must agree.
- Warning level 3. No explicit C++ language standard set (compiler default, C++14 on v143).
- **Win32 (x86) is the primary platform.** x64 configurations exist for CodeUtil/Engine/Interface/MilkdropPlayer
  but are not what the samples/tools use.
- Build order: CodeUtil -> Interface -> Engine (Pub\Libraries.sln) -> apps. Apps link libs from `Pub\Libs` and
  `Pub\Libs\DirectX`.

Solution configurations and the key defines they set:

| Configuration | Key defines | Output |
|---------------|-------------|--------|
| Debug / Debug DX9  | `WIN32 _DEBUG ENGINEDX TUD9` | `EngineD-DX9.lib`, `InterfaceD-DX9.lib`, `CodeUtilD.lib` |
| Release / Release DX9 | `WIN32 NDEBUG ENGINEDX TUD9` | `Engine-DX9.lib`, `Interface-DX9.lib`, `CodeUtil.lib` |
| Tool Debug / Tool Release | as above + `TOOL` (+ `STANDALONE` in tools) | `EngineToolD.lib` / `EngineTool.lib` |
| Debug DX11 | `TUD11` | experimental, incomplete |
| Debug GL / Release GL | `USING_OPENGL` | experimental, incomplete (`...-OGL.lib`) |

All configs also define `_CRT_SECURE_NO_WARNINGS` and `_CRT_NONSTDC_NO_DEPRECATE`. Library naming:
`<Name>[D][-DX9|-DX11|-OGL].lib` (`D` = debug).

Other preprocessor switches seen in code: `UNIX`, `USE_GTK_INTERFACE`, `IW_SDK` (Marmalade), `SERVER`, `MEMLEAK_DEBUG`,
`USE_D3DEX_INTERFACE` (always forced on in Engine.h), `DIRECT3D_VERSION` (used to gate DX-specific declarations
in public headers), `WINUSERAPI` (gates HWND-using declarations), `OLD_FUNCTION_COMPAT`.


Live vs dead code paths
-----------------------

| Path | Status |
|------|--------|
| DirectX 9 (D3D9Ex) on Win32 | **Live** — the only fully working renderer |
| OpenGL (`USING_OPENGL`) | Partial; compiles in places, not maintained |
| DirectX 11 (`TUD11`) | Experimental; many `#ifdef TUD11` branches in DX code but not a working build |
| Marmalade (mobile SDK) | Dead (SDK discontinued); keep files, don't extend |
| Unix / GTK CodeUtil | Historic server/tool builds; not built from these solutions (`CodeUtilLinux.vcxproj` exists) |
| Oculus VR (`OculusDX`, `EngineEnableVR`) | Legacy, old LibOVR |
| Skeletal animation (`ModelSkeleton*`, `Skeleton\`) | New, experimental (2026). DX9 path only, CPU skinned |
| D3DX .x skinned meshes (`SkinnedModel`, `SkinMesh*DX`) | Older, separate; compiled out under `TUD9` |

When changing a public function, update the live DX9 implementation; update other back-ends only if they
contain that function and the change is mechanical (signature), so they don't silently rot further.


Known quirks / gotchas
----------------------

- `StandardDef.h` fakes windows.h types. Including `<windows.h>` *after* it causes redefinition errors —
  always include `StdWinInclude.h` first in files that need Win32.
- `StandardDef.h` defines `max`/`min` macros (unless `NOMINMAX`), which break `std::min/max` and `<algorithm>`.
- `VECT`, `D3DMATRIX`, `uint32`, `BOOL` are defined in several headers behind guard macros
  (`MIT_TYPES`, `BASETYPES`, `UINT32_DEFINED`, `D3DMATRIX_DEFINED`, `TRACTOR_ENGINE_H`). Keep those guards
  intact when touching type definitions; include order matters.
- `StandardDef.h` contains game-specific globals (`gnHeightmapWidth`, `AN_ITEM`, `MAX_PLAYERS_ON_CLIENT`) from
  The Universal — existing games rely on them; don't remove.
- Header guards are named after the old project: `TRACTOR_*_H`, `UNIVERSAL_*_H`.
- `uint32` is `unsigned __int32` on Win32 but Interface.h's fallback typedef is `unsigned long` — same size on
  Win32, differs on x64 Unix. Don't add more fallbacks.
- Typos baked into identifiers (`eCompronentBehaviourTypes`, `DXHeirarchyContainer`, `EngineCameraSetOthorgonalView`)
  are part of the API. Don't rename public ones; new code can use correct spelling.
- Paths in source use backslashes in string literals (`"Data\\Samples\\caveman1.atm"`).
- No automated tests exist anywhere. Verification = build the affected solution(s) and run a sample/tool.

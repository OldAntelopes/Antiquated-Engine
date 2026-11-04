Tools Overview
---------------

Windows tools built on the Pub libraries. `Tools\Tools.sln` builds ModelConverter and SceneEditor along with
the CodeUtil, Interface and Engine library projects. The tools link the **Tool** build of the Engine
(`EngineTool[D].lib`, `TOOL` define) and output to `..\Exe\`.

- **ModelConverter** : The main authoring tool for the .atm model format (see ATMFormat.md). Loads and exports
  ATM, imports/exports FBX (Autodesk FBX SDK), edits materials, sub-models and attach points, builds
  animations, UV-unwraps and adds primitives. Win32 dialog app (`ModelConverter.rc`).
- ModelConverter also has **Animations & Effects > Skeletal (Experimental)**: auto-rig as humanoid, link .bvh animations,
  and FBX to .bvh animation import/export. Command line: `-fbxanim2bvh in.fbx out.bvh [fps]` and `-bvh2fbx in.bvh out.fbx`.
  See Pub\skeletalanimation.md.
- **SceneEditor** : Editor for scene maps (collections of placed models) used by the Engine's SceneMap system.

Source only, no project files in this repo:

- **TileLandscapeGen** : Tile-based landscape generator (has its own copy of the Landscape code).
- **TreeSpriteGen** : Tree sprite generator.
- **InteriorMapEditor** : Interior map editor, structured like SceneEditor.

These contain their own forked copies of GameCommon modules (Platform, UI, Landscape etc.) which have
diverged from the versions in GameCommon. Some includes still point at old folder names
(e.g. `../../../UniversalPub/LibCode/...`), so they need path fixes and project files before they will build.

ModelConverter: Next Version Plan
---------------------------------

Status: **reviewed; decisions recorded in section 5** (19 Nov 2026). Pre-work committed (a7b14ae). **Phase 1 mostly done**
(19 Nov 2026, see "Phase 1 status" in section 4).
Paths are relative to the repo root. Line numbers are from the working tree on 19 Nov 2026 (including uncommitted work).

Goal: an updated ModelConverter that shows every current .atm correctly (skinned and playing, relief, material channels,
night-only materials, sign lamps), then grows into a skeletal toolset: visualise, rig, apply/export mocap, and later
author animations and skeletons.

Contents: 1 Baseline - 2 Current-state audit - 3 Design - 4 Roadmap (phases 0-6) - 5 Decisions - 6 Pre-work checklist


1 Baseline (what was run on 19 Nov 2026)
----------------------------------------

- `msbuild Tools\Tools.sln /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v143`:
  CodeUtilD.lib, InterfaceD-DX9.lib, EngineToolD.lib and SceneEditorD.exe build. **ModelConverter fails to link**:
  `LNK2019 FbxAnimationConvertToBVH / FbxAnimationCreateFromBVH` (from `Skeletal\SkeletalTools.cpp`).
  Cause: `Pub\LibCode\Engine\FileFormats\fbxAnimation.cpp` is untracked and commit 96a740a took it out of
  `Pub\Projects\Engine.vcxproj`.
- The failed link deleted the untracked `Exe\ModelConverterD.exe` (Oct 31 build). It was rebuilt from the current
  source without touching any repo file: a props file in the scratchpad, passed with `/p:ForceImportBeforeCppTargets`, added
  `fbxAnimation.cpp` to ModelConverter for that one build. It compiles and links cleanly (the file is all `#ifdef TOOL`),
  so restoring the Engine.vcxproj entry fixes the build (pre-work item 2, now done).
- The converter UI and the 05 sample were **not run interactively**. This session had no screen access. Behaviour notes
  below come from reading the code. Phase 1 starts by confirming them in the running tool.


2 Current-state audit
---------------------

### 2.1 Chunk by chunk

"Engine" = `Pub\LibCode\Engine\ModelFiles.cpp` (the converter loads with `ModelLoad`, ModelConverter.cpp:696, and saves with
`ModelExportATM`, ModelConverter.cpp:4140, so the engine decides load/save). "Display/Edit" = what the converter does with it.

| ID | Chunk | Load | Display in converter | Edit in converter | Save (round-trip) |
|----|-------|------|----------------------|-------------------|-------------------|
| 0x1, 0x7, 0x8, 0xD, 0x10, 0x11 | Unused / TBD | ignored | - | - | not written |
| 0x2/0xC | Verts / compressed verts | yes (pass 2) | yes | many tools | yes; converter default saves 16-bit verts (`mulLastExportFlags`, ModelConverter.cpp:119) |
| 0x3 | Normals | yes | yes (show normals) | Recalc normals | yes |
| 0x4 | Colours | pass 3 | yes | Vertex colours | only with the Colours export flag |
| 0x5/0xB | UVs / byte UVs | yes | yes | UV unwrap/generate | yes (flag picks which) |
| 0x6 | Indices | yes | yes | plane cut, delete faces... | yes |
| 0x9/0xE/0xF | Vertex/normal keyframes | yes | plays (toolbar combo) | Animation Builder, Remove all | yes |
| 0xA | Normal keyframes (float) | not in the load passes | - | - | not written |
| 0x12 | Keyframe data | yes | yes | Animation Builder | yes |
| 0x13/0x14/0x17 | Turrets, attach, LOD | yes | yes (ball at attach point, LOD combo) | Attach dialog, sub-model browser | yes |
| 0x15/0x16/0x1B | Global props, creator, basic contents | yes | - | Materials dialog (global props) | yes |
| 0x18 | Effect data | yes | types 1-13 via `RenderEffectsAddParticle` | type, vertex, offset (AttachDialog.cpp:152-199) | yes, all 28 bytes |
| 0x18 type 14 | Sign lamp | yes | **no**: `AddLight` is a stub (ModelConverter.cpp:4557), and with no `IsDaytime` override it is always "night" | **listed as "Unknown effect (14)"** (AttachDialog.cpp:195); **Param 1/2 (range, colour) can't be edited** | yes |
| 0x19 | Texture material | yes | yes | Material Browser | yes |
| 0x19 flag 0x4 | Night-only colours | yes (ModelFiles.cpp:2734) | **always on**: `EngineGameInterface::IsDaytime` defaults to false (Engine.h:645) and the converter doesn't override it | **no control** in MaterialBrowser.cpp | yes (ModelFiles.cpp:794) |
| 0x1A | Attributes | yes | yes | Combine materials etc. | yes |
| 0x1C/0x1D | Skeleton, skin weights | yes (ModelSkeletonFiles.cpp:133-183) | bind pose, and skinned **only if an animation loaded** (ModelSkeleton.cpp:1082) | Auto-rig only; no bone view or joint editing | yes; bind pose restored first, no welding (ModelFiles.cpp:2357-2368) |
| 0x1E | Anim links | yes | plays the first anim, **at 1/10 speed** (see 2.3), and **toolbar anim controls are disabled** | Link (filename guess for use) | **partly**: a link whose .bvh is missing at load is dropped, so Save loses it; speed is always written as 1.0 |
| 0x1F | Material relief | yes, keeps the source bytes | **no**: `EngineSetModelRelief` is off (ModelMaterialData.cpp:665), and `Exe\Data\Shaders\ATMMaterial.psh` has no relief path (no `gLandRelief`; only The Universal's shader has it) | **no** | yes, if loaded from file (ModelFiles.cpp:1002) |
| 0x20 | Material channels | yes (ModelFiles.cpp:2864) | **never lit**: the converter never calls `EngineSetModelMaterialChannel` | **no** | yes (ModelFiles.cpp:1070) |

Summary: the **engine round-trips every chunk** apart from the missing-link case. The gaps are in the converter: it
can't show or edit the newer data, and its edit tools aren't safe on skinned models.

### 2.2 Existing skeletal features

- Menu **Animations & Effects > Skeletal (Experimental)** (ModelConverter.rc:929-935, handlers ModelConverter.cpp:5297-5310):
  Auto-rig as Humanoid, Link Animation (.bvh), Import FBX Animation as .bvh, Export .bvh as FBX.
- Command line `-fbxanim2bvh` / `-bvh2fbx` (SkeletalTools.cpp:62, ModelConverter.cpp:5888).
- Auto-rig = `ModelRemoveVertexAnimation` + `ModelSkeletonEstimateHumanoidJoints` (bounds-based, caveman proportions,
  ModelSkeleton.cpp:1215) + `ModelSkeletonAutoGenerateWeights( 2 )`. No way to adjust joints afterwards.
- Link guesses the ANIM_ use from the filename (walk/idle/stand/jump/die/death/wave, else RUN) (SkeletalTools.cpp:196-201).
- Engine side (`Pub\LibCode\Engine\Skeleton\`, committed in b8da7c5): BVH load/save, by-name retargeting, loop finding,
  auto weights, CPU skinning, chunk I/O. `fbxAnimation.*` (FBX SDK 2015.1, Tool build only) is untracked.
- The bone overlay exists only in `Samples\05_SkeletalAnim\Scene.cpp:440` (`SceneDrawBones`: joints projected to screen,
  `InterfaceLine` per bone).

### 2.3 Known bugs and limitations affecting the tool

1. **Build**: ModelConverter doesn't link (section 1).
2. **Animation clock**: `ModelRenderingUpdate( ulTickGap / 10 )` (ModelConverter.cpp:1462) runs the render clock at 1/10
   speed, and the integer division also drops remainders. Skinned clips (ModelSkeleton.cpp:1108, ms-based) play about 10x
   slow. Vertex-keyframe anims share the clock, so this is probably long-standing. Check whether slow keyframe playback
   was deliberate before changing it (decision 1: real time plus a speed combo).
3. **Toolbar animation controls** are enabled only for `pxVertexKeyframes` models (ModelConverter.cpp:1256-1269). The
   combo lists every ANIM_ use, not the ones the model has (ModelConverter.cpp:4667).
4. **Pause** (`ModelAnimationPause`) is ignored by skeletal playback, and there's no way to set or scrub the clip time.
5. **Missing .bvh drops the link** (ModelSkeleton.cpp:735-741, `AddAnimation` returns NOTFOUND), so opening and saving a
   model whose clips aren't beside it silently strips 0x1E.
6. **Links store only the filename** (ModelSkeleton.cpp:1306-1331). Linking a clip from another folder, or Save As to a
   new folder, breaks the link unless the .bvh is copied next to the .atm. The tool doesn't copy it or warn.
7. **Vertex edits on a skinned model are overwritten**. The skeleton caches the bind mesh on first skin
   (`CaptureBindMesh`, ModelSkeleton.cpp:153) and rewrites the vertex buffer from it every frame. So Scale, Move, Rotate,
   Recalc normals and vertex tools appear to do nothing (or are lost), and bones don't follow Scale or Move. Tools that change
   the vertex count stop skinning altogether (weight count mismatch, ModelSkeleton.cpp:1083) without saying so.
8. Two links with the same ANIM_ use: only the first can ever play (`PlayAnimation`, ModelSkeleton.cpp:987). Links can't be
   removed or re-tagged.
9. Effects that call `AddLight` (type 14 and several others, ModelRendering.cpp:2009-2243) light nothing in the tool.
10. `ModelConvMouseControl.cpp` is empty (0 lines). The mouse handling lives in ModelConverter.cpp.
11. Engine limitations listed in Pub\skeletalanimation.md: CPU skinning, clones share a pose, no shadow-volume skinning,
    no blending, shortest-arc retargeting (no twist), identity bind rotations, BVH as the runtime format.


3 Design
--------

### 3.1 UI approach (stay a Win32 dialog app)

No new UI framework: the tool keeps its existing pattern of a main dialog with a D3D viewport, menus, and modeless child
dialogs (Material Browser, Sub-model Browser). New pieces:

- **Menu**: promote **Skeletal (Experimental)** to a top-level **Skeleton** menu: Show Bones / Joint Names / Weights /
  Bind Pose (toggles), Rig..., Animations..., Library..., Import/Export submenus. Add **View > Lighting > Day / Night** and
  **View > Material Channels > Headlights / Brake / Reverse / Indicators** toggles, plus **View > Relief** (on/off and depth scale).
- **Animations panel** (modeless dialog, new `Skeletal\AnimationPanel.cpp`): list of the model's links (file, ANIM_ use,
  frames, loop, speed, "missing" flag). Buttons: Add/Link, Remove, Set Use, Play/Pause, a **timeline trackbar** (scrub),
  speed edit, loop start/end, root motion checkbox. The toolbar anim combo shows the model's links when it's skinned.
- **Rig panel** (Phase 2/3, `Skeletal\RigPanel.cpp`): bone tree (TreeView), joint X/Y/Z edits, Estimate, Mirror L->R,
  Generate Weights (smoothing passes), and a weight brush (add/remove/smooth, radius, strength).
- **Viewport overlays** (new `Skeletal\SkeletonOverlay.cpp`), drawn with the converter's existing `Util\Line_Renderer` and
  `Util\Point_Renderer` (3D, already depth-tested in the scene) plus `InterfaceText` for names: bones as lines, joints as
  points, selected bone highlighted, names at the projected joints, weight heatmap (see 3.3).
- **Picking**: click a joint (screen-space nearest, as `ModelConvUpdated3dPointerCoord` already does for vertices), drag
  in the view plane to move it in rig mode.

### 3.2 Data flow

    ModelConverter (UI, state, overlays, file dialogs, library, BVH name normalising)
        |  C API in Rendering.h / Engine.h  (+ internal headers where the TOOL build already reaches in, e.g. ModelMaterialData.h)
        v
    EngineToolD.lib (TOOL build): ModelSkeleton (bones, weights, baked clips, skinning), ModelFiles (.atm I/O),
                                  SkeletalAnimation (.bvh), fbxAnimation (FBX<->BVH, TOOL only)

The tool owns no copy of skeletal data. It reads and edits through the engine API, so what you see is what the game gets.
Editing the bind pose (joint moves) re-bakes the model's clips through the engine. Weights are edited as a tool-side array
and pushed with `ModelSkeletonSetVertexWeights`.

### 3.3 Engine APIs needed (all additive; nothing existing changes signature or meaning)

All of these go in the Skeletal section of `Pub\Include\Rendering.h`, implemented in `Pub\LibCode\Engine\Skeleton\ModelSkeleton.cpp`.
DX9 only, so there's no other back-end to keep in step.

| Phase | New function / type | Purpose |
|-------|---------------------|---------|
| 1 | `MODEL_SKELETON_ANIM_INFO` + `ModelSkeletonGetAnimationInfo( h, nAnim, pxOut )` | file, use, linked, loaded/missing, frames, frame time, loop range |
| 1 | `ModelSkeletonGetCurrentAnimation( h )` / `ModelSkeletonPlayAnimationSlot( h, nAnim )` | pick by slot (two clips can share a use) |
| 1 | internal: skeletal playback honours `ModelAnimationPause` (agreed behaviour change, decision 2) | pause, in games too |
| 1 | `ModelSkeletonSetAnimationTime( h, fSeconds )` / `ModelSkeletonGetAnimationTime( h )` | scrub and time readout |
| 1 | `ModelSkeletonSetShowBindPose( h, bFlag )` | show the bind pose while clips are loaded |
| 1 | internal: keep unresolved links (pSource NULL, skipped by skinning, still exported) | fixes bug 5; no API change |
| 1 | internal: `ModelSkeletonInvalidateBindMesh( h )` | re-capture the bind mesh after a vertex edit |
| 2 | `ModelSkeletonGetCurrentBoneRotations( h, pxOut, nMax )` | draw bone axes and export poses |
| 2 | `ModelSkeletonTransformBind( h, const ENGINEMATRIX* )` | Scale/Move/Rotate keep bones in step with the mesh |
| 3 | `ModelSkeletonSetBoneBindPos( h, nBone, const VECT* )` + `ModelSkeletonRebakeAnimations( h )` | interactive joint placement |
| 3 | `ModelSkeletonEstimateHumanoidJointsEx( h, nFlags, pxOut )` | better estimation (see Phase 3), old function untouched |
| 4 | `ModelSkeletonRemoveAnimation`, `ModelSkeletonSetAnimationUse`, `ModelSkeletonSetAnimationSpeed`, `ModelSkeletonSetAnimationLoop` | link management |
| 4 | `ModelSkeletonExportAnimationBVH( h, nAnim, szFile )` | write the retargeted clip on the model's own skeleton (uses `SkeletalAnimation::SaveBVH`) |
| 5 | `ModelSkeletonCreateAnimation( h, nFrames, fFrameTime, nAnimUse )` + `ModelSkeletonSetAnimationFrameLocal( h, nAnim, nFrame, pxLocalRots, pxRootOffset )` | authoring (mirrors MitAtm's `MitAtmClipCreate` / `MitAtmClipSetLocal`) |

Display-only items need **no engine change**: day/night (override `IsDaytime` / `GetNightLightStrength` in the converter's
`ModelConverterEngineGameInterface`), sign lamp light (implement its `AddLight` with `EngineActivateSimpleLight( n, POINT_LIGHT, ... )`),
channels (`EngineSetModelMaterialChannel[Colour]` around the draw), relief (`EngineSetModelRelief` + The Universal's shaders, decision 3).
The weight heatmap uses the uncommitted `ModelGetVertexPositions` (Rendering.h, working tree) to read skinned positions. Points
are coloured by the selected bone's weight (blue 0 to red 1); the mesh's own vertex colours are left alone.

### 3.4 Format additions (proposed; **not registered yet**)

Per ATMFormat.md, nothing below is written by code until it's added to the register first, with the developer's agreement.
These entries also need agreement from MitAtm, the other reader. Phase 1 needs **no** format change.

**(a) 0x1E Skeletal Animation Links: define the reserved fields and append (Phase 4).** Element size 72 to 80.

	0x42	[2 bytes]		Flags: 0x1 = keep horizontal root motion, 0x2 = play once (hold the last frame), 0x4 = loop range set
	0x44	[4 bytes]		Playback speed multiplier (float; 0 or missing = 1.0). Multiplies ModelSetAnimationSpeedModifier
	0x48	[4 bytes]		Loop start frame (used when flag 0x4 is set; otherwise the loader finds the loop)
	0x4C	[4 bytes]		Loop end frame, exclusive

Older readers step by the stored size and ignore these, so they keep today's behaviour.

**(b) 0x21 Skeletal Animation Clip (Phase 4, decision 6).** A compact baked clip on *this model's* skeleton (no
retargeting at load). It lives either inside the .atm or in a standalone `.atma` file (ATM file header + 0x21 chunks only,
the same parser and zlib compression).

	Header: 0x0 [2] header size, 0x2 [2] flags (0x1 loops, 0x2 has root track), 0x4 [4] frames, 0x8 [4] frame time (float),
	        0xC [2] bones, 0xE [2] ANIM_ use, 0x10 [32] name
	Then per frame: per bone a local rotation as 3x int16 (smallest-three quaternion), then the root position as 3x float
	if flag 0x2

0x1E links would then accept `.atma` as well as `.bvh`. Exact layout to be settled at the start of Phase 4.


4 Roadmap
---------

Sizes are rough, in focused developer-days.

### Phase 0: Pre-work (0.5 day, mostly the developer)

See section 6. Done on 19 Nov 2026 apart from the commit itself, which is the developer's.

### Phase 1: "Loads, shows, plays and round-trips every current model" (target: about 1 week)

Goals: the skinned caveman opens, plays its linked run at real speed, and can be paused, picked and saved without losing
anything. Night/day, channels, relief and sign lamps display as they do in The Universal (editing them is Phase 2, decision 9).

Tasks:
1. Confirm the audit's suspected bugs in the running tool (clock, controls, link loss, edits). The build is fixed (pre-work).
2. **Clock**: feed `ModelRenderingUpdate` real milliseconds, and add a playback speed combo (0.1x / 0.25x / 0.5x / 1x / 2x)
   so slow keyframe previews are still available (decision 1).
3. **Animation controls for skinned models**: enable the toolbar when `ModelSkeletonGetNumAnimations > 0`, fill the combo
   from the model's links, play by slot. Play/Pause uses `ModelAnimationPause`, which skeletal playback now honours
   (engine, decision 2). A simple time readout.
4. **Show Bind Pose** and **Show Bones** toggles (port `SceneDrawBones` to a 3D line overlay).
5. **Keep missing links**: the engine keeps unresolved links. The tool lists them on load ("2 animations not found: ...") and
   still saves them.
6. **Save As copies linked clips automatically** (decision 7): when the target folder differs, copy each linked .bvh beside
   the new .atm (never overwriting a different file of the same name without asking), and report what was copied and what's
   missing. Same when linking a clip from another folder.
7. **Skinned-model edit guard**: before any vertex-changing tool on a skinned model, restore the bind pose. Afterwards
   invalidate the bind mesh. Block tools that change the vertex count (or warn that weights will be dropped). Scale/Move/Rotate: warn
   in Phase 1, properly supported in Phase 2.
8. **Day/Night**: `View > Lighting > Day/Night` overrides `IsDaytime` and `GetNightLightStrength` in
   `ModelConverterEngineGameInterface`. Night is the default, which matches what the tool shows today.
9. **Sign lamp**: name type 14 "Sign lamp (night)" in the effect combo (a label only). Implement `AddLight` with
   fixed-function point lights so lamps and soft lights light the model. Draw a small marker at the lamp position.
10. **Material channels**: `View > Material Channels` toggles 1-4 (+ "reverse" white for 2) call `EngineSetModelMaterialChannel`
    before the model draw and `EngineClearModelMaterialChannels` after.
11. **Night-only materials**: shown by day/night (task 8). The Material Browser shows the flag read-only.
12. **Relief and Universal shaders** (decision 3): copy The Universal's `ATMMaterial.vsh/.psh` and everything they include
    (`Lighting.psh`, `Shadows.psh`, `Fog.psh`, `Clouds.psh`, `WeatherCover.fxh`; from `Universal\Exe\Clients\Data\Shaders`) into
    `Exe\Data\Shaders`. `View > Relief` turns on `EngineSetModelRelief` with The Universal's settings. Check that samples 02
    and 05 still render, since they share these shaders. If one doesn't, give the converter its own shader folder through an
    additive `EngineSetShaderFolder( szPath )` (today the path `Data\Shaders\` is hard-coded in ShaderLoaderDX.cpp:47/133).
    Also set the constants the Universal shaders expect but the tool doesn't (clouds, weather) to neutral values.
    Long term the two shader sets should be one, with The Universal's as the master.
13. **Model info**: the info text lists bones, weights, linked clips (and missing ones), channels, relief, night-only
    materials and effect type, so the tool tells you everything in the file.
14. Docs: Tools\Readme.md, Pub\skeletalanimation.md (tool section), this plan's status.

Files: `Tools\ModelConverter\ModelConverter.cpp/.rc/resource.h`, `Skeletal\SkeletalTools.*`, `Dialogs\AttachDialog.cpp`,
`MaterialBrowser.cpp`, new `Skeletal\SkeletonOverlay.cpp/.h` (+ `Tools\ModelConverter.vcxproj`/`.filters`);
`Pub\Include\Rendering.h`, `Pub\LibCode\Engine\Skeleton\ModelSkeleton.cpp/.h`, `ModelSkeletonFiles.cpp`.

New APIs: the Phase 1 rows of 3.3 (additive). Format: none.

Risks: The Universal's shaders may expect constants that only the game sets (fallback: own shader folder, task 12). The test
models' textures aren't stored as filenames (embedded, or supplied by the game, like the tractor's livery), so some may need
the existing Override Texture option. Pausing skinned clips changes behaviour in games that call `ModelAnimationPause` on a
skinned model (agreed). Rendering.h already has the developer's uncommitted edits, so commit those first (pre-work 4).

Verification:
- Build `Pub\Libraries.sln` (Debug|Win32, `/p:PlatformToolset=v143`), then `Tools\Tools.sln` Debug|Win32. Also build
  `Samples\Samples.sln` "Debug DX9", since Rendering.h changes, and confirm 05_SkeletalAnim still runs (`-shots`).
- `Exe\ModelConverterD.exe`, working dir `Exe\`:
  - Open `Data\Samples\caveman1_skinned.atm`: bind pose shows, the run plays at the same speed as the 05 sample (compare
    one stride against its video). Pause/scrub works, bones overlay tracks the mesh, Bind Pose toggle works.
  - Save As to `%TEMP%\x\cm.atm`: it offers to copy `cmu_09_01_run.bvh`. Reopen: identical (bones, weight count, link).
    Compare with `MitAtmTests --check` from the MitAtm repo (read-only use).
  - Rename the .bvh away, open, save, restore it, reopen: the link survives.
  - Open `Data\Samples\caveman1.atm` (keyframed): anims still play. Auto-rig, Link the run, save, reopen: plays.
  - Scale x2 on the skinned caveman: warned, nothing corrupt.
  - Test models copied from The Universal into `Exe\Data\Samples\Universal\` (found by scanning its Content for the chunks):
    `atTractor.atm` (0x20 channels: toggle headlights/brake/reverse), `peakSchoolTall_T01.atm` (0x1F relief: View > Relief
    on/off, compare with an in-game screenshot), `smstFarmSignBasicLamp_T01.atm` and `LoveFairyLights_T01.atm` (type 14 and
    night-only materials: day/night toggle, lamp lights the sign), `LoveFloodlight_T01.atm` and `atracStreetlampGlow.atm`
    (night-only materials). Nothing in The Universal is skinned, so the caveman stays the only skinned test.
- Re-save each of those with no changes: compare chunk lists before and after (`-atminfo`, below) to check nothing is lost.

Small extra: a `-atminfo <file>` command line that lists chunks, IDs, sizes and counts (SysDebugPrint + exit code). It
makes the round-trip checks scriptable. About 0.5 day.

Size: about 5-6 days (engine 1.5, tool 3.5, verification 1).

**Phase 1 status (19 Nov 2026)**

Built: tasks 2-10, 12, 13 and 14.
- Engine: the Rendering.h skeletal additions (`ModelSkeletonGetAnimationInfo`, `...GetCurrentAnimation`, `...PlayAnimationSlot`,
  `...Set/GetAnimationTime`, `...SetShowBindPose`, `...RestoreBindPose`, `...RecaptureBindPose`). Skeletal playback honours
  `ModelAnimationPause`, and unresolved links are kept.
- Tool code: `Skeletal\SkeletalView.cpp/.h` (anim controls, bones overlay, load report, edit guard, copying clips) and
  `RenderEffects\ModelDisplayOptions.cpp/.h` (day/night, effect lights, channels, relief, playback speed).
- Menus differ from the plan's wording:
  - A top-level **Skeleton** menu replaces Animations & Effects > Skeletal.
  - Day / Night, Material Channels and Relief are under **Rendering**.
  - Playback Speed is under **Animations & Effects**.
- Edit guard as built: every menu or button command runs with the bind pose shown, and the bind mesh is then re-taken from
  the mesh. If the vertex count changed, it warns once that the weights no longer match. Vertex and poly picking show the
  bind pose.
- Shaders: copied the Universal's ATMMaterial.vsh/.psh, Lighting, Shadows, Fog and Clouds. Nothing includes
  WeatherCover.fxh, so it wasn't copied. The constants the Universal sets for land, clouds and weather are left unset:
  their branches only run when the constants are non-zero.
- Fix: the view window is frameless again. Windowed device init no longer restyles a child window (InterfaceInstance::ApplyWindowStyle).

Checked: all three solutions build. Sample 05 renders with the new shaders. In the converter, File > Open of the skinned caveman
plays the run, with the combo, status text and load report working. The tractor reports its channels.

Not done yet:
- Task 11 (Material Browser night-only flag).
- A scrub slider. The API exists; only play and pause have UI.
- `-atminfo`.
- The interactive checks in the Verification list: Save As round trip, missing-link round trip, relief against an in-game
  screenshot, lamps lighting the sign, Scale on the skinned model.

Released as **2.50** (Exe\ModelConverter250.zip, 19 Nov 2026). That release also fixes the command-line load (a trailing
space after the filename made it fail silently) and the Release link (it now links legacy_stdio_definitions.lib, as Debug did).

### Phase 2: Visualisation and inspection (about 1 week)

Goals: see and understand any rig.
Tasks: bones with joint spheres and optional axes (needs `ModelSkeletonGetCurrentBoneRotations`); joint names; select a bone
(click or tree) to show its weight heatmap; per-vertex weight readout on vertex pick (up to 4 bones and weights); stats
(unweighted verts, verts with >4 influences trimmed, weight sums); "bind vs animated" ghost (draw the bind skeleton in
grey under the animated one); `ModelSkeletonTransformBind` so Scale/Move/Rotate work on rigged models; animated skinned
models in sub-model/LOD views (check whether each LOD carries its own skeleton, since chunks are per sub-model).
Editing moved here from Phase 1 (decision 9): effect Param 1/2 edits (range, colour with a picker) for types 3 and 14; a
night-only checkbox, a channel combo + on-colour, and relief import (alpha PNG, depth; `LoadReliefFromMem` + `SetReliefSource`)
in the Material Browser. About +1.5 days.
Files: `Skeletal\SkeletonOverlay.*`, new `Skeletal\RigPanel.*` (read-only tree in this phase), ModelConverter.cpp, engine
Skeleton files. Risk: point-cloud heatmap readability on dense meshes; fallback is a coloured copy mesh made with
`EngineCreateMesh`. Verify: on caveman1_skinned check each bone's heatmap against the island cleanup described in
skeletalanimation.md (hands don't weight to the loincloth); scale x0.5 then save, reopen and play; Samples + Tools builds.
Size: about 4-5 days.

### Phase 3: Better auto-rig and the mocap library (about 2 weeks)

Goals: rig any reasonable humanoid in minutes; a starter set of clips ready to link.

Auto-rig:
- **Interactive joint placement**: Rig panel joint edits + drag in viewport, symmetric editing (mirror across Y=0,
  character's left = +Y), snap the joint to the centre of the mesh cross-section under the cursor (ray in, ray out, midpoint).
  `ModelSkeletonSetBoneBindPos` + re-bake.
- **Better estimation** (`...EstimateHumanoidJointsEx`, new function in the engine so the sample can use it too):
  slice the mesh along Z into height bands and find the crotch (where the cross-section splits into two legs), the
  armpits/shoulders (the band where the arms leave the torso) and hand ends (extreme points of the arm islands). Also detect
  T-pose vs A-pose vs arms-down from where the arms are. Fall back to today's proportions if detection fails. Optionally
  normalise facing (find the toes, check that +X is forward) and offer "rotate model to face +X".
- **Weight painting**: brush on the mesh surface (add/subtract/smooth/set) for the selected bone, normalised to the top 4.
  Undo (one level per stroke). Re-run auto weights per bone or for a region.
- Templates: allow 2-bone and 3-bone spine variants and optional fingers later; keep the 21-bone humanoid as the standard
  (`ModelSkeletonCreateHumanoid`) everything retargets to.

Mocap library (decision 5: in git, under 10 MB):
- Folder `Exe\Data\Animations\Humanoid\<Source>\<use>_<source>_<id>.bvh`, e.g.
  `Exe\Data\Animations\Humanoid\CMU\run_cmu_09_01.bvh`, plus `Exe\Data\Animations\CREDITS.md` and a short README.
  The tool's **Skeleton > Library...** browses it by use, previews on the current model, and Link copies the file next to the .atm
  (links are filename-only).
- **Name normalising on import**: the retargeter matches Mixamo/CMU names. 100STYLE uses `Chest..Chest4`,
  `RightCollar/RightShoulder/RightElbow/RightWrist`, `RightHip/RightKnee/RightAnkle/RightToe`, and Quaternius' names aren't
  documented. So library clips are converted once, in the tool (`-bvhrename in out` + an alias table in `Skeletal\BoneAliases.cpp`),
  to the standard names. Extra spine joints merge onto Spine/Spine1. Runtime retargeting stays unchanged.
- Starter set (about 12-20 clips): idle, walk, run, jog, jump, die, wave, sit, crouch/crawl, turn, strafe, kick/punch.
  Sources, licence checks done 19 Nov 2026:
  - **CMU Graphics Lab** (mocap.cs.cmu.edu), already in use. Pub\skeletalanimation.md records "no restrictions". The CMU site
    couldn't be read from here (TLS certificate error), so **open its FAQ in a browser and confirm the terms and the requested
    acknowledgement before shipping**. My understanding is that commercial use is allowed but reselling the raw data isn't,
    which a free repo/game doesn't do. Good for walk/run/jump/idle/kick and odd actions; trials to be picked from the CMU index.
  - **Quaternius Universal Animation Library**: **CC0** (quaternius.com / itch.io say free for commercial use). The free
    Standard version has 45 clips (FBX/glTF/OBJ, root-motion and in-place variants): locomotion, death, sitting, etc.
    Convert FBX to .bvh with `-fbxanim2bvh`. Risk: the FBX SDK 2015.1 may not read their FBX version; fallback is Blender to BVH.
  - **100STYLE** (Ian Mason): **CC BY 4.0**, BVH at 60 fps, 100 locomotion styles (walk/run fwd/back, sidestep, idle,
    transitions). Commercial use is fine with the credit "The 100STYLE Dataset - Ian Mason". Good for varied walks/idles. Needs
    name normalising and trimming (the files are long).
  - **Rejected**: Bandai Namco Research Motiondataset (CC BY-NC: non-commercial, and Elemental is sold), Mixamo (free to use
    in projects but its terms don't allow redistributing the raw files, so it can't live in the repo; users can still import
    their own), Ubisoft LAFAN1 (non-commercial; not re-checked).
- Credits: `Exe\Data\Animations\CREDITS.md` (source, licence, URL, original file/trial per clip, any required wording),
  a line in README.md "Shared Code", and Help > About in the converter.

Files: `Skeletal\RigPanel.*`, `Skeletal\WeightPaint.*`, `Skeletal\BoneAliases.*`, `Skeletal\AnimLibrary.*`, engine
`ModelSkeleton.cpp` (estimation, bind edits), `Exe\Data\Animations\...` (needs the developer's OK, Exe is normally out of scope).
Verify: rig 3 different humanoids (caveman1.atm keyframed base pose, one Universal character, one Quaternius/Kenney CC0
character converted via FBX import) and play 5 library clips on each without visible tearing; measure the estimate's joint
error against the hand-measured caveman table (05 sample, Scene.cpp:64); Samples + Tools build. Size: about 8-10 days.

### Phase 4: Animation application and export (about 1-1.5 weeks)

Tasks: Animations panel complete (add/remove/re-tag/speed/loop range/root motion/play once); retarget preview with the source
skeleton drawn beside the model; trim (set frame range) and save the trimmed .bvh; export the retargeted clip as .bvh on
the model's skeleton (`ModelSkeletonExportAnimationBVH`) and as FBX (via the existing `FbxAnimationCreateFromBVH`); batch
command line (`-retarget model.atm clip.bvh out.bvh`). The 0x21 compact clip (decision 6) and `.atma` (register first,
engine reader/writer, tool export, MitAtm informed). Needs format addition (a). Risks: speed/loop fields change game
behaviour only for files that set them; old files keep 1.0 and auto loop. Verify: round-trip a clip through BVH and FBX
and compare (the existing 0.02 degree test); file size of .atma vs .bvh for the run; play in 05 and in Elemental's build.
Size: about 6-8 days (+3 for 0x21).

### Phase 5: Creating animations and skeletons (about 2-3 weeks)

- Keyframe posing: select a bone, rotate with a gizmo (or numeric), set key, timeline with keys, linear/slerp
  interpolation, copy/paste/mirror pose, loop-closing. Saved as BVH (or .atma). FK first; **two-bone IK** for legs/arms
  (foot/hand targets) optional and tool-only (baked into the clip on save).
- Non-humanoid skeletons: add/remove/rename/reparent bones, generic auto weights (nearest bone + smoothing, without the
  humanoid torso/head capsules), clips authored on that skeleton. Retargeting stays humanoid-only (name-based), so a
  non-humanoid plays only its own clips. Say so in the UI.
- Engine (Pub): `ModelSkeletonCreateAnimation` / `SetAnimationFrameLocal`; generic weights path.
Verify: author a 1 s wave on the caveman and a 3-bone tail on a test model, save, reopen, play in 05. Size: about 10-15 days.

### Phase 6: Engine track (Pub, not the tool; schedule with the games)

Blending/crossfades between clips (`ModelSetAnimationNext` crossfade time; MitAtm's `MitAtmPoseBlend` is the reference);
GPU skinning (vertex shader matrices as `MitAtmPoseMatrices`, which also fixes clones sharing a pose and enables per-instance
poses); shadow-map/volume skinning; twist-preserving retargeting and real bind rotations (defines the reserved `xBindRot`,
register first); event triggers. The tool gains a crossfade preview once the engine has it. Size: separate plan.


5 Decisions (developer, 19 Nov 2026)
------------------------------------

1. **Converter clock**: real time, plus a playback speed combo (0.1x to 2x) so slow previews are still there. (Phase 1)
2. **`ModelAnimationPause` pauses skeletal clips too**, in games as well as the tool. An agreed behaviour change in the
   engine; resuming carries on from where it paused. (Phase 1)
3. **Relief and look**: the converter should look like The Universal. Copy The Universal's ATMMaterial shaders and their
   includes into `Exe\Data\Shaders` (Phase 1, task 12). Fallback if the samples suffer: a converter shader folder.
4. **Test models**: taken from The Universal (scan of its Content for the newer chunks). Six models (784 KB) copied into
   `Exe\Data\Samples\Universal\`: atTractor (channels), peakSchoolTall_T01 (relief), smstFarmSignBasicLamp_T01 and
   LoveFairyLights_T01 (sign lamp + night-only), LoveFloodlight_T01 and atracStreetlampGlow (night-only). **Staged in git**
   at the developer's request (they're The Universal's assets; the repo is public).
5. **Mocap library**: yes, in git under `Exe\Data\Animations\Humanoid\`, trimmed clips, under 10 MB, with CREDITS.md.
   CMU's terms still need a look in a browser (the site wouldn't load from here). (Phase 3)
6. **Compact binary clips**: yes, chunk 0x21 / `.atma` in Phase 4. BVH stays supported for import and as a link target.
   Reasons: the 43 s karate clip is 3.8 MB of text and is parsed and retargeted at every load. A clip baked onto the model's
   skeleton with 16-bit rotations is about a tenth of that and loads with no retargeting. And it's the natural output of the
   Phase 5 animation editor. The layout gets agreed and registered in ATMFormat.md (with MitAtm told) before any code writes it.
7. **Save As copies linked .bvh files automatically**, reporting what it copied and what's missing. (Phase 1)
8. **Engine first**: new capability (estimation, bind editing, clip creation, export) goes in the engine behind the C API,
   and the tool is a front-end for it, so games can use it for in-game tooling. The plan already puts it that way. Keep to it.
9. **Phase 1 is display only** for channels, night-only, relief and sign lamps. Their editing UI moves to Phase 2.


6 Pre-work checklist
--------------------

Done on 19 Nov 2026. Everything is **staged, not committed**: review with `git diff --cached`, then commit.

1. [x] `Pub\LibCode\Engine\FileFormats\fbxAnimation.cpp/.h` (staged by the developer).
2. [x] Re-added to `Pub\Projects\Engine.vcxproj` and `.filters` (96a740a reversed).
3. [x] `Tools\ModelConverter.vcxproj` / `.filters`: `Skeletal\SkeletalTools.h` added to the project. Both files are in the
   filters, under a new "Skeletal" filter (they were missing from the filters altogether).
4. [x] Staged `Pub\skeletalanimation.md`, `Samples\05_SkeletalAnim.vcxproj`, `Samples\05_SkeletalAnim\*` (Samples.sln
   already references the project) and the test assets `Exe\Data\Samples\caveman1_skinned.atm`, `cmu_09_01_run.bvh`,
   `cmu_09_02_run.bvh`, `cmu_135_02_karate.bvh` (3.8 MB; the sample's `-karate` option needs it). `SkelOut\` left out (generated).
5. [x] Also found and staged, outside the converter work: `Pub\LibCode\Interface\Common\Font\FontTrueType.cpp` and
   `stb_truetype.h`. Commit bcfbca2 put them in Interface.vcxproj but never added the files, so a clean clone couldn't build
   Interface. Also staged: the docs that the tracked AGENTS.md includes (`CodingStandards.md`, `GameCommon.md`,
   `GameCommon\UIX\UIX.md`) and the UIX test harness it requires (`Tests\UIXTests\`, with its own .gitignore).
6. [ ] Your other uncommitted work that Phase 1 builds on: `Pub\Include\Rendering.h` (`ModelGetVertexPositions` etc.) and
   `Engine.h` with their implementations. Commit when ready.
7. [x] `Exe\Data\Samples\Universal\` staged (decision 4).
8. Not touched, for you to decide: `Pub\LibCode\lua-5.4.6\` (untracked; no project references it), `Pub\Include\Canvas.h`
   (listed in Interface.vcxproj but doesn't exist; harmless for a header), `Samples\03_ArcadeGameSample` (known broken),
   `Tools\ModelConverter\AddPrimitives\xxDisc.*`, `ModelConverter.aps`, `RCa16268` (junk), and a `.gitignore` for build
   outputs (`Pub\Libs` outputs, `Objs`, `.vs`, `*.user`).

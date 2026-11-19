Skeletal Animation (Experimental)
---------------------------------

All paths here are relative to the root of the repository.

**Status:** a quick first pass, built to get the caveman running on mocap data. It works for that, but expect
rough edges. See "Limitations" at the end.

**Overview**

A model can now carry:
- a **skeleton**: bones with names, parents and bind-pose joint positions
- **skin weights**: up to 4 bones per vertex
- **animation links**: filenames of animation files in the same folder as the .atm, each tagged with an `ANIM_xxx` use

All three are saved in the .atm (chunks 0x1C, 0x1D and 0x1E, described in ATMFormat.md). When the model is loaded, the
linked animations are loaded too, retargeted onto the model's skeleton, and the mesh is skinned on the CPU every frame.
Existing game code needs no changes: `ModelSetAnimationImmediate( hModel, ANIM_RUN, ... )` / `ModelSetAnimationNext`
select the linked animation with that use, and `ModelRender` does the rest.

Animation files are **.bvh** (Biovision Hierarchy) for now. It's a simple text format that most mocap libraries
and DCC tools can export. The ModelConverter can convert FBX animations (e.g. from Mixamo) to .bvh.

**Code**

| File | What |
|------|------|
| "Pub\Include\Rendering.h" | Public C API: `ModelSkeleton*`, `ModelRemoveVertexAnimation`, `eHumanoidBones`, `MODEL_SKELETON_BONE`, `MODEL_SKIN_WEIGHT` |
| "Pub\LibCode\Engine\Skeleton\SkeletalAnimation.*" | BVH loading/saving; a source clip in its own skeleton space |
| "Pub\LibCode\Engine\Skeleton\ModelSkeleton.*" | Per-model skeleton, weights, retargeting/baking, loop finding, auto-weights, CPU skinning |
| "Pub\LibCode\Engine\Skeleton\ModelSkeletonFiles.*" | .atm chunk export/load |
| "Pub\LibCode\Engine\Skeleton\SkeletonMaths.h" | Quaternion helpers (Hamilton product, column vectors) |
| "Pub\LibCode\Engine\FileFormats\fbxAnimation.*" | FBX to BVH and BVH to FBX conversion (Tool build only, FBX SDK 2015.1) |
| "Tools\ModelConverter\Skeletal\SkeletalTools.*" | ModelConverter menus and command line |
| "Samples\05_SkeletalAnim" | Rigs caveman1.atm, plays it, records the video |

Hooks into existing code: `MODEL_RENDER_DATA::pSkeleton` (ModelRendering.h), `ModelFree`, `ModelSetAnimationImmediate/Next`
(ModelRendering.cpp), the DX render path (`ModelRenderImplDX`, `ModelUpdateAnimationState`), and ModelFiles.cpp (chunk export
and load, keeping the bind pose on export, vertex remap, and no vertex welding for skinned models).

**The standard humanoid skeleton**

`ModelSkeletonCreateHumanoid` builds 21 bones, named in the common Mixamo/CMU style:

    Hips
      Spine - Spine1 - Neck - Head
                Spine1 - LeftShoulder - LeftArm - LeftForeArm - LeftHand      (and Right...)
      LeftUpLeg - LeftLeg - LeftFoot - LeftToeBase                            (and Right...)

Engine model conventions: **Z up, facing +X, character's left = +Y**. The bind pose is whatever pose the mesh is in
(arms down is fine; it doesn't need to be a T-pose).

**Retargeting (how any humanoid animation drives any humanoid skeleton)**

Done once per animation at load time ("baked"), in `ModelSkeleton::BakeAnimation`:

1. Model bones are matched to source joints **by name**. Names are compared after lower-casing and stripping
   any `prefix:` (e.g. `mixamorig:LeftArm` matches `LeftArm`) and `_`/spaces. Unmatched model bones follow their parent rigidly.
2. The source rest pose (all rotations zero, as in BVH) gives a rest direction for each bone. The model's bind pose gives
   another. `A_B` is the shortest rotation between the two (`QuatFromTo`). This handles A-pose vs T-pose, splayed mocap legs, etc.
3. Every frame: `S_B(t) = Yaw * C * G_J(t) * C^-1 * A_B`, where G_J is the source joint's global rotation, C converts
   source axes (Y up, facing +Z) to engine axes, and Yaw turns the clip's average heading to face +X.
4. Joint positions come from FK using the **model's own bone lengths**. The root height is scaled by the leg-length ratio,
   and the clip is grounded so the lowest foot/toe joint matches the bind pose. Horizontal root motion is removed by default
   (`ModelSkeletonSetRootMotion` keeps it).
5. `FindLoop` picks the most seamless loop in the clip (closest pair of poses at least 0.45s apart). Frame 0 is ignored,
   because mocap BVHs often start with a calibration T-pose. Clips longer than 4 seconds are treated as whole performances
   (a karate kata, say) and play through in full rather than being trimmed to a loop.

Skinning: `v' = sum_k w_k * ( S_k(t) * ( v - bind_k ) + joint_k(t) )`, and normals are rotated by the same weighted rotations.

**Auto-rigging**

- `ModelSkeletonEstimateHumanoidJoints` makes a crude guess from the model's bounds. The proportions were taken from
  caveman1.atm, so expect to adjust them for other models.
- `ModelSkeletonAutoGenerateWeights( hModel, nSmoothingPasses )` works in four steps:
  1. Vertices at the same position are welded first, so UV seams don't tear.
  2. Each vertex is labelled with its nearest bone. Torso bones (Hips, Spine*) and the Head count as thick capsules, with
     radii of 5% and 3% of the model height (`msfAutoWeightTorsoRadius` / `msfAutoWeightHeadRadius` in ModelSkeleton.cpp).
     The radius tapers to zero at the bone ends. Without this, the sides of the chest are nearer the upper-arm line than
     the spine line and get labelled as arm.
  3. **Connectivity cleanup.** Where parts of the mesh touch in the bind pose (caveman1's hands hang against the loincloth,
     and his forearms against his waist), nearest-bone labelling puts loincloth and waist vertices on the hand or forearm.
     That stretched polygons between the hands and the loincloth. Now each bone keeps only one surface-connected island
     of its vertices: the one with the best score of count × (spread along the bone) ÷ mean distance. Vertices in that
     bone's other islands are relabelled from their surface neighbours, but can't take a bone from the same limb chain
     (e.g. a stray forearm patch can't become upper arm instead).
  4. Weights are smoothed across the mesh surface, then the top 4 influences are kept and normalised.

  The sample's `-dump` writes `<model>_weights.txt` next to the .obj, so weights can be checked. On caveman1 there are now
  no triangles joining a hand or forearm to the hips, legs or loincloth, or one arm to the other.

**Pipeline: making a skinned model**

In code (this is what `Samples\05_SkeletalAnim` does with `-rig`):

    int hModel = ModelLoad( "Data\\Samples\\caveman1.atm", 0, 1.0f );
    ModelRemoveVertexAnimation( hModel );                    // keep the current (standing) pose as the bind pose
    ModelSkeletonCreateHumanoid( hModel, axJointPositions ); // or ModelSkeletonEstimateHumanoidJoints first
    ModelSkeletonAutoGenerateWeights( hModel, 2 );
    ModelSkeletonAddAnimation( hModel, "cmu_09_01_run.bvh", ANIM_RUN, TRUE );   // TRUE = save the link in the .atm
    ModelExportATM( hModel, "Data\\Samples\\caveman1_skinned.atm", ATM_EXPORT_FLAGS_MATERIALS | ATM_EXPORT_FLAGS_COMPRESSION, 0 );

In the ModelConverter, under **Animations & Effects > Skeletal (Experimental)**:
- **Auto-rig as Humanoid**: removes vertex animation, adds estimated joints and automatic weights
- **Link Animation (.bvh)...**: the anim use is guessed from the filename (walk/run/idle/jump/die/wave; anything else is run)
- **Import FBX Animation as .bvh...**: bakes the FBX's first take, sampled at 30fps
- **Export .bvh as FBX...**
- Then **Save As** an .atm. Keep the .bvh files in the same folder.

Command line (no UI): `ModelConverter.exe -fbxanim2bvh in.fbx out.bvh [fps]` and `ModelConverter.exe -bvh2fbx in.bvh out.fbx`.

**caveman1.atm**

- The original is a compressed v1 .atm: 774 verts in use and 64 vertex keyframes (walk, run, standing, firing...).
  Its mesh allocates 2130 vertex slots (one per index), and only the first 774 are used.
- `Exe\Data\Samples\caveman1_skinned.atm` (19KB, vs 405KB for the keyframed original) is the rigged version. Its joint
  positions were measured from vertex clusters around each joint in the standing pose (table in `Samples\05_SkeletalAnim\Scene.cpp`).
- It links `cmu_09_01_run.bvh`, a run from the CMU Graphics Lab Motion Capture Database (subject 09, trial 01, via the BVH
  conversion at github.com/Shriinivas/cmubvh). CMU places no restrictions on use of the data. `cmu_09_02_run.bvh` is a second take.
- `cmu_135_02_karate.bvh` is CMU 135_02, the 'Empi' karate kata (43 s). The sample loads it at runtime, with root motion on.
- `Samples\05_SkeletalAnim` options (run with the working directory set to `Exe\`):
  - `-rig` rebuilds the skinned atm
  - `-video` writes `Exe\Data\Samples\SkelOut\caveman_skeletal_run.mp4`
  - `-karate` writes `caveman_karate_empi.mp4` to the same folder
  - `-shots` saves stills
  - `-dump` writes mesh info and weights
  - no option runs the interactive viewer (B = bones, O = show/hide the original)

**Limitations / next steps**

- CPU skinning, DX9 path only. It rewrites the base mesh's vertex buffer every frame, so clones share the skinned pose,
  and the shadow volume path isn't skinned. A vertex-shader skinning path would be the next step.
- One animation plays at a time, with no blending or crossfades, and no event triggers.
- Retargeting is direction-only (shortest arc), so bone twist isn't preserved. Fine for limbs, rough for hands and the spine.
- Bind orientations are always identity. The `xBindRot` field in the chunk is reserved.
- BVH is the runtime animation format. A compact binary animation chunk/file would be better for shipping.
- FBX import was round-trip tested (BVH to FBX to BVH matches to 0.02 degrees), but not yet on a real Mixamo file. Joint
  orients and pre-rotations are handled by working in world-space deltas from the bind pose, but this is unverified.
- `ModelSkeletonEstimateHumanoidJoints` only suits caveman-like proportions and poses.

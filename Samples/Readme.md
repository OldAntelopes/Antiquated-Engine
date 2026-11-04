Samples Overview
------------------

Minimal programs showing the Pub libraries in use. Open `Samples\Samples.sln` (it also pulls in the
Interface and Engine library projects). Executables are written to `..\Exe\`; set the debugger working
directory to `Exe\` so data files (e.g. `Data\Samples\caveman1.atm`) are found.

- **00_HelloWorld** : Smallest possible Interface app. Creates a window, initialises the display device and
  draws "Hello World". Shows the basic frame loop (`InterfaceNewFrame` / `BeginRender` / `Draw` / `EndRender` / `Present`).
- **01_BasicInterface** : More of the 2D Interface API (shapes, text, an on-screen display module `OSD`).
- **02_BasicEngine** : Interface + Engine. Sets up a camera and light and loads/renders an .atm model
  (`Scene.cpp`, `Model.cpp`).
- **03_ArcadeGameSample** : Placeholder ("under construction"). Its .vcxproj is not in Samples.sln and its
  source paths are out of date.
- **05_SkeletalAnim** : Experimental skeletal animation. Rigs caveman1.atm with the standard humanoid skeleton,
  plays a CMU motion-capture run on it next to the original vertex-keyframe version, and can record an .mp4
  (`-rig`, `-video`, `-karate`, `-shots`, `-dump`). See Pub\skeletalanimation.md.
- **04_MilkdropPlayer** : Standalone Milkdrop visualiser built on the Engine, compiling Milkdrop sources
  directly and capturing audio via loopback.

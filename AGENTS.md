@README.md
@GameCommon.md
@ATMFormat.md
@Architecture.md
@CodingStandards.md

This is an old codebase; a legacy gamedev platform used for indie development (mostly many years ago).

The general goal for AI development here will be to update this into a more modern, comprehensive structure without fundamentally changing the interfaces that existing projects use.

Working rules for AI agents
---------------------------

- **Scope.** First-party code is Pub (except vendored headers/libs), GameCommon, Samples and Tools.
  Don't edit 3rdParty, Exe or Milkdrop unless explicitly asked. Treat vendored code inside first-party folders
  as read-only too: Pub\Include\{Bullet*,LinearMath,DirectX,GL,curl,fbxsdk,jpg,png,steam,zlib,cJSON},
  Pub\LibCode\Lua and lua-5.4.6, Pub\LibCode\Interface\Common\Font\stb_truetype.h (v1.26), GameCommon\tcp4u, GameCommon\WebServer\Tcp4u, GameCommon\BulletFileLoader,
  GameCommon\Physics\bt*Importer*, cJSON.c copies, Util\Blowfish, Engine\CollisionMaths\.
- **Interfaces are sacred.** Public functions, types, enum values and struct layouts in Pub\Include and the
  GameCommon module headers are used by shipped games (The Universal, Elemental). Add, don't change. See
  "Modernisation guidelines" in CodingStandards.md.
- **Style.** Follow CodingStandards.md for new code; when editing an existing file, match that file (including its
  line endings) and don't reformat code you aren't changing.
- **Project files.** Adding, removing or renaming a source file in Pub means updating the matching
  `Pub\Projects\*.vcxproj` (and `.vcxproj.filters` where present). GameCommon has no project of its own: files are
  added to each consuming app's .vcxproj.
- **Renderer.** DirectX 9 on Win32 is the only live back-end. Implement there first; keep other back-ends'
  signatures in sync only where mechanical.
- **UIX changes** follow GameCommon\UIX\UIX.md: additive and opt-in, a test in Tests\UIXTests for each, and the
  harness (`Tests\UIXTests\RunUIXTests.ps1`) passing with its references unchanged, plus Elemental building.
- **Verification.** Apart from the UIX harness there are no tests. Verify by building the affected solution (Pub\Libraries.sln, then
  Samples\Samples.sln and/or Tools\Tools.sln) for Debug|Win32, and running a relevant sample with the working
  directory set to Exe\. Projects target toolset v143; if only a newer toolset is installed, override it on the
  command line (`msbuild Pub\Libraries.sln /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v145`)
  rather than retargeting the project files. Say clearly when a change could not be built or run.
  Samples.sln has no plain `Debug` configuration: use `Debug DX9`. `Pub\Projects\LinearMath.vcxproj` still says
  `v120_xp`, so build Libraries.sln with `/p:PlatformToolset=v143`. Incremental builds are reliable: no project
  uses `/Gm` (minimal rebuild) any more, so don't turn `<MinimalRebuild>` back on.
- **Docs.** Keep Architecture.md and CodingStandards.md current when you learn something that contradicts them.

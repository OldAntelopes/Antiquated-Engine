UIX: the test harness and how to change UIX
===========================================

UIX is GameCommon's retained UI: a tree of `UIXObject`s (pages, buttons, dropdowns, sliders, sections...) built once
and drawn each frame by `UIX::Render`, on top of the older immediate-mode `GameCommon\UI`. Elemental is built on it,
and The Universal is moving its screens to it, so a change here can change a shipped game. This page is the rule for
changing it, and the harness that checks a change.


How an app drives UIX
---------------------

Each frame, in this order (Elemental's control panel, and the harness, do exactly this):

    UIUpdate( fDelta );  UIX::Update( fDelta );
    InterfaceNewFrame(...);  InterfaceBeginRender();
    UIX::Render( pInterface );  InterfaceDraw();  UIOnInterfaceDraw();
    InterfaceEndRender();  InterfacePresent();

Input arrives between frames: `UISetCurrentCursorPosition( x, y )` on mouse move, `UIOnPress` / `UIOnRelease` (and the
right-button versions), `UIX::OnMouseWheel( -wheelDelta / 8 )`, `UIX::OnKeyUp( key )` plus
`PlatformKeyboardHandleSpecialKeyUp/Down` and `PlatformKeyboardHandleChar` for text. UIX works out what was pressed
while it renders, so a click is: move, frame, press, frame, release, frame.


The harness: `Tests\UIXTests`
-----------------------------

A standalone exe (`UIXTests.vcxproj`, Win32, v143) that compiles GameCommon UI, UIX, Platform and Console from source
and links the Pub libraries, as an app does. It doesn't use or link anything from Elemental.

    powershell -ExecutionPolicy Bypass -File Tests\UIXTests\RunUIXTests.ps1          # build and run
    ... -UpdateRefs     write new reference files (then run again without it, to show they're stable)
    ... -Only <text>    run the tests whose names contain <text>
    ... -BuildLibs      build Pub\Libraries.sln first
    ... -NoBuild        run what's built (Out\UIXTestsD.exe takes --update-refs, --only, --list, --verbose)

It prints `UIXTests: ALL PASSED (N tests, ...)` or `UIXTests: FAILED (...)` and exits 0 only if everything passed.

**What a test does.** It builds pages in code (`UIX_TEST( Name )` in `Source\UIXTests_*.cpp`), feeds scripted mouse,
wheel and keyboard input (`UIXTestClick`, `UIXTestDrag`, `UIXTestWheel`, `UIXTestKey`, `UIXTestType`), and checks
three ways:

- `UIX_CHECK( cond, "Area: what it shows" )`: an assertion about behaviour we mean to keep.
- **The trace**, compared line by line with `Refs\<test>.txt`: callbacks (with their params), state (expanded,
  selected, scroll position, focus, values) and layout dumps (`UIXTestLogLayout`: every object's type and the rect it
  drew at, or "not drawn"). This records today's behaviour, whatever it is.
- **Snapshots**, compared with `Refs\<test>_<name>.png`: the rendered frame (or a crop of it). A pixel differs when a
  channel is more than 24 out; a snapshot fails when more than 8 pixels differ. A failure leaves the frame and a
  diff image (red = differing pixels) in `Out\Results`.

Private widget state is read through `Source\UIXTestAccess.h/.cpp`, which reach members with the explicit
instantiation idiom, so UIX itself doesn't change for the tests.

**Deterministic by construction:** a hidden 640x480 window; a fixed 1/60 s frame delta; Antiquated's default fonts
from `Exe\Data\Fonts`; an icon sheet the harness draws itself (`Out\UIXTestIcons.png`); and before every test, UIX's
statics reset, the button ID counter and UI's glow timer reset, and one fixed "primer" frame drawn, because Interface
leaves some device state (whether textured overlays blend with alpha) as the last draw set it. Every test passes on
its own and in sequence. The one input it can't control: `UIXScrollbar::OnMouseWheel` reads the real Shift key, so
don't hold Shift while it runs. On a different GPU or driver, text edges may differ by more than the tolerance;
regenerate the references there with `-UpdateRefs` before relying on them.

**Coverage** (weighted by Elemental's use: AddText, AddButton, AddShape and AddDropdown are most of its calls):

| Area | Tests |
|---|---|
| Pages | stacking layout, negative x/w, overlapping pages and click order, delete, mouse wheel handler, clipping, `IsWithinPage` |
| Text | every flag, fonts 0-4, colour 0, formatting, width limit, dynamic text |
| Shapes | all modes, blocking, pressable shape, both animations over time |
| Buttons | every mode, hover, click edges, press-inside-release-outside, right press ID and right-click callback, label edit (accept and reject), image and dynamic image, image path (found and missing), glow |
| Checkboxes | every mode, toggling, callbacks, icons, text and background setters |
| Dropdowns | open, choose, greyed entries, escape, click elsewhere, setters, update handler, only one open, arrow keys when focused, long lists with scrollbar and wheel, drop-up near the bottom, drop list width |
| Text boxes | focus, select-all then type, backspace, Enter, Tab, switching boxes, clicking away, long text |
| Sliders | every mode drawn; drag, clamp, vertical, angle; range handles; typing into value and range boxes with and without range expand; `ModifySliderRange` |
| List boxes, tab bars, dials, horizontal scrollbars | layout, selection and callbacks, update handlers, dragging |
| Sections | collapsable (modes, toggle, nested, not-collapsable, right click, state data), scrollable (overflow, wheel, scrollbar drag, store and restore, fits without a bar), sub-pages |
| Menus | menu bar (open, choose, switch, escape, `CloseAllMenus`), popup menu (header, divider, checkbox item, submenu on hover, right click), modal popup |
| Custom render | callback drawing, `CheckForPress` / `CheckForRightButtonPress` from the callback, post-render |
| Across objects | drag and drop (buttons, section headers, list entries), tooltips, object selection mode and handler, `SetSelectedUIXObjectID`, IDs and user params, `GetCurrentPressObjectID`, escape and `CloseAllDropdowns`, deleting focused objects, deleting a child, `Reset`, icon sheets |
| Pad focus hook (GameCommon UI) | `UISetControllerSelectableHook`: off by default, a UIX button reports itself when on, activating presses it, slider and dropdown steps and their ends (`ControllerHook_ReportsActivatesAndSteps`) |

**Not covered yet:** UIX drawn into a second `InterfaceInstance` (Elemental's control panel window), a device reset,
UI scale other than 1, and the `UIXTextBox` mode and `UIXListBox` mode parameters (stored, never read). Add tests
for these when a change touches them.

**Adding a test:** add a `UIX_TEST` to the right `UIXTests_*.cpp`, run with `-Only <name> -UpdateRefs`, look at the
new trace and images in `Refs` to check they show what you meant, then run without `-UpdateRefs`.


Changing UIX: the process
-------------------------

UIX is shared and Elemental depends on it, so changes are **additive** and checked by the harness. A change that
follows every step below doesn't need the developer's line-by-line review. A change that can't (a changed default,
signature or look) needs the developer's approval first.

1. **Additive only.** New functions, new enum values at the end, new flags and setters, new fields at the end of
   classes. Don't change a public signature, an existing enum value, a callback type or what a default argument does.
2. **New behaviour is opt-in, with the old behaviour as the default.** An existing app that doesn't call the new thing
   must behave and look exactly as before. Fixing a bug changes behaviour too: it's opt-in as well, unless the developer
   approves changing the default.
3. **A harness test for every change.** Add tests for the new behaviour, with the opt-in on.
4. **The existing references don't change.** Run the harness without `-UpdateRefs`: every trace and image must still
   match. If one changes, either the change isn't opt-in, or it changes a default: stop and ask the developer. Only
   update the references of tests you added.
5. **Everything builds:** the harness (`RunUIXTests.ps1`), `Pub\Libraries.sln` (Debug, Win32) when Pub changed, and
   Elemental (`..\Games\Elemental\Elemental.vcxproj`, Debug, Win32). Elemental adds GameCommon files to its own project,
   so a new UIX `.cpp` must be added there too, and to `UIXTests.vcxproj`.
6. **List the changed files for the developer** (Antiquated is their git repo: never commit, branch or push there),
   with the harness's verdict line and what was and wasn't built.

The harness itself can change freely, as long as the existing references still pass.


Known issues the harness found (not fixed)
------------------------------------------

The references record these as today's behaviour. Fixing one means a test's reference changes, so each fix is opt-in
or needs the developer's approval, as above.

**Memory and lifetime**
- `UIX::DeleteObject` on a child doesn't remove it from its parent's child list, so the next render uses freed memory
  (`System_DeleteChildObject`). Deleting a page is fine.
- Deleting a `UIXTextBox` (or `UIXButton` label edit) while it's being edited clears `UIX`'s edit focus but leaves the
  platform keyboard handler pointing at the deleted object; the next Enter or Tab calls into freed memory
  (`System_DeleteFocusedObjects`).
- `UIX::Reset` erases from the page list while looping over it. With two pages it works by reading the stale slot past
  the end; with three or more it reaches a page it has already deleted.
- A user object set with `SetUserObject( p, true )` is deleted through a `void*`, so its destructor never runs.
- A `UIXBUTTON_IMAGE` button releases the texture handle it was given (its icon number) when deleted, so it takes
  ownership of the app's texture; with the default icon number 0 (e.g. a dynamic image button) it releases texture 0.
- `UIXListBox::OnShutdown` deletes only top-level entries (sub-entries would leak), and `UIXObject::Shutdown` runs
  each child's `OnShutdown` twice (once directly, once through `DeleteObject`); today's `OnShutdown`s cope with that.

**Buffers and formats**
- `UIXTextBox::OnRender` copies the text (or the keyboard string, up to 511 characters) into a 256-byte stack buffer:
  a text box with more than 255 characters overflows it.
- `UIX::AddText`, `UIXTextBox::SetText` and `UIXPopupMenu::AddHeaderMenuItem` treat their text as a printf format
  into fixed buffers (1024 / 2048 bytes): a `%` in app text, or text that's too long, misbehaves.
- `UIXHorizScrollbar::HoldHandler` divides by `max - view`, which is 0 when the bar fills the track.

**Behaviour that may surprise**
- The mouse wheel target is never cleared, so after hovering a scrollable object the wheel keeps going to it when the
  mouse has moved away (`Page_MousewheelHandler`).
- A button press fires even when the mouse is released outside the button (`Button_NormalClickAndHover`); dragging a
  draggable button onto empty space also counts as a click.
- An open dropdown doesn't close when you click elsewhere; only its header, an entry, Escape or another dropdown
  close it. Arrow keys only reach a dropdown that a right click has focused.
- `UIXCollapsableSection::SetNotCollapsable` hides the triangle, but a click still collapses the section.
- A `UIXModalPopup` doesn't stop clicks reaching objects drawn before it, underneath (`ModalPopup_BlocksWhatIsBehind`).
- `UIXTabBar` passes the tab's index, not its user param, to the selection callback, and doesn't mark the clicked tab
  selected (the app must call `SetSelected`).
- Choosing a menu bar item, or a popup submenu item, leaves the menu open; the app closes it (`UIX::CloseAllMenus`).
- `UIXText` matches its flags exactly, so `ALIGN_RIGHT | BOLD` (or any combination) draws left-aligned; `BOLD` does
  nothing.
- In layout, a child's y offset moves it but not the siblings after it, so a child at y = 10 overlaps the next one;
  a negative w means "parent width minus x minus (-w - 1)"; a negative x always means "from the right".
- A scrollable section compares its contents with the remaining height of its parent, not its own rect, and only
  knows its contents' height a frame after it's built.
- `UIXListBoxEntry::AddSubElement` is protected, so apps can't make sub-entries.
- A slider's value box cuts long text from the front, so 40.000 shows as "..000".
- Textured overlays (icons, images) draw under plain rects on the same layer, and whether their transparent parts
  blend depends on what Interface drew before (the harness's primer frame exists for this).

# Run setup

Choosing New Game opens Run Setup before the Birch introduction. Rules are
selected for the new run and saved as individual flags. Editing the setup draft
does not change a loaded save.

## Controls

- Up/down selects the fixed preset row or a setting. Four settings are visible;
  the list scrolls as needed, with arrows indicating more rows.
- A or left/right changes the selected setting. On the preset row, A/right cycles
  forward and left cycles backward. Arrows flank the preset name while editing.
- Start opens confirmation. A confirms from any row and begins the introduction;
  B returns to editing with the draft and selection retained. Up/down can scroll
  through the settings summary before confirming.
- B from setup discards the draft and returns to the main menu.

## Presets

| Preset | Full compatibility | Reusable TMs |
|---|---|---|
| Vanilla (default) | Off | Off |
| Nuzlocke | On | On |
| Bishey | On | On |

Applying a preset replaces all draft settings. Vanilla means this fork's defaults
with optional run rules disabled. Nuzlocke and Bishey currently have identical
settings; neither name adds rules beyond the table above.

Individual edits show a matching preset or Custom if none matches. When presets
match identically, the chosen name is retained. Without a matching prior choice,
the first matching preset is shown. Custom is a status, not a selectable preset.
The preset name is draft-only; the confirmed rules are stored in the save.

## Adding settings

- Add draft accessors and new-game application in `src/run_setup.c`, including
  current-build save storage for the rule.
- Add the label, two-line help, and accessors to `sRunSetupSettings` in
  `src/main_menu.c`. Editing and confirmation share this table.
- Update every entry in `sRunSetupPresets` and the preset matcher when adding a
  rule. Keep Vanilla Off; Nuzlocke and Bishey enable all run options until their
  definitions are refined.
- Extend the run setup tests for defaults, preset matching/application,
  confirmation guards, and independent rules. Test save/load behavior for the
  rule using the existing rule tests as examples.

The setup window and its border share graphics memory later used by the Birch
introduction. Keep their tile allocations separate and clear background graphics
after setup fades out, before the introduction changes character base.

## Automated checks

Run `make check -j2 TESTS="Run setup"` for the draft, preset, scrolling, and display
cleanup tests. Run `./build_and_export.sh` for the final `make -j2` build and ROM
export. This menu changes no save layout.

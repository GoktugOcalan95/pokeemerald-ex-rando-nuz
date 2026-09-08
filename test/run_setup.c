#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "run_setup.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"

TEST("Run setup detaches the main menu tilemap and clears its highlight mask")
{
    static EWRAM_DATA u16 mainMenuTilemap[BG_SCREEN_SIZE / sizeof(u16)];
    static const struct BgTemplate bgTemplate =
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    };
    u16 previousDisplayControl = GetGpuReg(REG_OFFSET_DISPCNT);
    u16 displayControl = DISPCNT_BG0_ON | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON;

    ResetBgsAndClearDma3BusyFlags(FALSE);
    InitBgFromTemplate(&bgTemplate);
    SetBgTilemapBuffer(0, mainMenuTilemap);
    SetGpuReg(REG_OFFSET_DISPCNT, displayControl);
    RunSetup_PrepareDisplay();

    EXPECT_EQ(GetBgTilemapBuffer(0), NULL);
    EXPECT_EQ(GetGpuReg(REG_OFFSET_DISPCNT), displayControl & ~DISPCNT_WIN0_ON);
    ResetBgsAndClearDma3BusyFlags(FALSE);
    SetGpuReg(REG_OFFSET_DISPCNT, previousDisplayControl);
}

TEST("Run setup clears its tilemap before the introduction")
{
    static EWRAM_DATA u16 runSetupTilemap[BG_SCREEN_SIZE / sizeof(u16)];
    bool32 tilemapCleared = TRUE;
    static const struct BgTemplate bgTemplate =
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    };

    memset(runSetupTilemap, 0xFF, sizeof(runSetupTilemap));
    ResetBgsAndClearDma3BusyFlags(FALSE);
    InitBgFromTemplate(&bgTemplate);
    SetBgTilemapBuffer(0, runSetupTilemap);
    RunSetup_ClearDisplayTilemap();

    for (u32 y = 0; y < DISPLAY_TILE_HEIGHT; y++)
    {
        for (u32 x = 0; x < DISPLAY_TILE_WIDTH; x++)
        {
            if (runSetupTilemap[y * 32 + x] != 0)
                tilemapCleared = FALSE;
        }
    }
    EXPECT(tilemapCleared);
    ResetBgsAndClearDma3BusyFlags(FALSE);
}

TEST("Run setup drafts start with defaults after discard")
{
    RunSetup_Begin();
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_SetReusableTMs(TRUE);
    RunSetup_SetNoEVGain(TRUE);
    EXPECT(RunSetup_GetFullCompatibility());
    EXPECT(RunSetup_GetReusableTMs());
    EXPECT(RunSetup_GetNoEVGain());

    RunSetup_Discard();
    RunSetup_Begin();
    EXPECT(!RunSetup_GetFullCompatibility());
    EXPECT(!RunSetup_GetReusableTMs());
    EXPECT(!RunSetup_GetNoEVGain());
    RunSetup_Discard();
}

TEST("Run setup confirmation Back retains the draft")
{
    RunSetup_Begin();
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_SetReusableTMs(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ReturnToDraft();

    EXPECT(RunSetup_GetFullCompatibility());
    EXPECT(RunSetup_GetReusableTMs());
    RunSetup_Discard();
}

TEST("Run setup draft does not change the loaded save")
{
    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    RunSetup_Begin();
    EXPECT(!RunSetup_GetFullCompatibility());
    EXPECT(!RunSetup_GetReusableTMs());
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();

    EXPECT(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));
    EXPECT(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS));
    RunSetup_Discard();
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup applies a confirmed draft to the new save")
{
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    RunSetup_Begin();
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_SetReusableTMs(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();

    EXPECT(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));
    EXPECT(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS));

    InitEventData();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));
    EXPECT(!FlagGet(FLAG_RUN_RULE_REUSABLE_TMS));
}

TEST("Run setup applies reusable TMs independently of full compatibility")
{
    bool32 reusableTMs;
    bool32 fullCompatibility;

    PARAMETRIZE { reusableTMs = FALSE; fullCompatibility = FALSE; }
    PARAMETRIZE { reusableTMs = FALSE; fullCompatibility = TRUE; }
    PARAMETRIZE { reusableTMs = TRUE; fullCompatibility = FALSE; }
    PARAMETRIZE { reusableTMs = TRUE; fullCompatibility = TRUE; }

    FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    RunSetup_Begin();
    RunSetup_SetReusableTMs(reusableTMs);
    RunSetup_SetFullCompatibility(fullCompatibility);
    RunSetup_EnterConfirmation();
    RunSetup_SetReusableTMs(!reusableTMs);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();

    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), reusableTMs);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), fullCompatibility);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
}

TEST("Run setup does not apply unconfirmed reusable TMs")
{
    RunSetup_Begin();
    RunSetup_SetReusableTMs(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_REUSABLE_TMS));
}

TEST("Run setup scrolling keeps the selected row visible")
{
    u32 selection;
    u32 top;
    u32 count;
    u32 expected;

    PARAMETRIZE { selection = 1; top = 0; count = 2; expected = 0; }
    PARAMETRIZE { selection = 3; top = 0; count = 4; expected = 0; }
    PARAMETRIZE { selection = 3; top = 0; count = 8; expected = 0; }
    PARAMETRIZE { selection = 4; top = 0; count = 8; expected = 1; }
    PARAMETRIZE { selection = 7; top = 1; count = 8; expected = 4; }
    PARAMETRIZE { selection = 2; top = 3; count = 8; expected = 2; }
    PARAMETRIZE { selection = 0; top = 3; count = 8; expected = 0; }
    PARAMETRIZE { selection = 7; top = 0; count = 8; expected = 4; }
    PARAMETRIZE { selection = 8; top = 0; count = 9; expected = 5; }
    PARAMETRIZE { selection = 6; top = 0; count = 8; expected = 3; }
    PARAMETRIZE { selection = 4; top = 5; count = 8; expected = 4; }

    EXPECT_EQ(RunSetup_GetScrollTop(selection, top, count), expected);
}

TEST("Run setup presets replace the draft without changing the loaded save")
{
    enum RunSetupPreset preset;
    bool32 enabled;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; enabled = FALSE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; enabled = TRUE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; enabled = TRUE; }

    FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
    RunSetup_Begin();
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_VANILLA);
    RunSetup_SetFullCompatibility(!enabled);
    RunSetup_SetReusableTMs(!enabled);
    RunSetup_SetNoEVGain(TRUE);
    RunSetup_SetPreset(preset);
    EXPECT(!RunSetup_GetNoEVGain());
    EXPECT_EQ(RunSetup_GetFullCompatibility(), enabled);
    EXPECT_EQ(RunSetup_GetReusableTMs(), enabled);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    EXPECT(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY));
    EXPECT(!FlagGet(FLAG_RUN_RULE_REUSABLE_TMS));
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), enabled);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), enabled);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup preset names follow edits and retain the chosen identical preset")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_SetReusableTMs(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetReusableTMs(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_SetFullCompatibility(FALSE);
    RunSetup_SetReusableTMs(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_VANILLA);
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_SetReusableTMs(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_Discard();
    RunSetup_Begin();
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_VANILLA);
    RunSetup_SetFullCompatibility(TRUE);
    RunSetup_SetReusableTMs(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_NUZLOCKE);
    RunSetup_Discard();
}

TEST("Run setup presets cannot change a confirmation and survive returning to the draft")
{
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_EnterConfirmation();
    RunSetup_SetPreset(RUN_SETUP_PRESET_VANILLA);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_BISHEY);
    EXPECT(RunSetup_GetFullCompatibility());
    EXPECT(RunSetup_GetReusableTMs());
    RunSetup_ReturnToDraft();
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_BISHEY);
    RunSetup_SetPreset(RUN_SETUP_PRESET_VANILLA);
    EXPECT(!RunSetup_GetFullCompatibility());
    EXPECT(!RunSetup_GetReusableTMs());
    RunSetup_Discard();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_VANILLA);
}

TEST("Run setup rejects Custom and invalid preset choices")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_CUSTOM; }
    PARAMETRIZE { preset = -1; }
    PARAMETRIZE { preset = 255; }

    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_SetPreset(preset);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_BISHEY);
    EXPECT(RunSetup_GetFullCompatibility());
    EXPECT(RunSetup_GetReusableTMs());
    RunSetup_Discard();
}

TEST("Run setup clears background graphics before Birch changes character base")
{
    volatile u16 *background = (volatile u16 *)VRAM;
    volatile u16 *sprites = (volatile u16 *)(VRAM + BG_VRAM_SIZE);
    u16 previousSpritePixel = sprites[0];
    bool32 cleared = TRUE;

    DmaFill16(3, 0x1234, (void *)VRAM, BG_VRAM_SIZE);
    sprites[0] = 0x5678;
    RunSetup_ClearDisplayGraphics();

    EXPECT_EQ(*(volatile u16 *)BG_CHAR_ADDR(3), 0);
    for (u32 i = 0; i < BG_VRAM_SIZE / sizeof(u16); i++)
    {
        if (background[i] != 0)
            cleared = FALSE;
    }
    EXPECT(cleared);
    EXPECT_EQ(sprites[0], 0x5678);
    sprites[0] = previousSpritePixel;
}

TEST("Run setup No EV gain is independent and locked during confirmation")
{
    bool32 enabled;
    bool32 otherRules;

    PARAMETRIZE { enabled = FALSE; otherRules = FALSE; }
    PARAMETRIZE { enabled = FALSE; otherRules = TRUE; }
    PARAMETRIZE { enabled = TRUE; otherRules = FALSE; }
    PARAMETRIZE { enabled = TRUE; otherRules = TRUE; }

    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    RunSetup_Begin();
    RunSetup_SetNoEVGain(enabled);
    RunSetup_SetFullCompatibility(otherRules);
    RunSetup_SetReusableTMs(otherRules);
    EXPECT(FlagGet(FLAG_RUN_RULE_NO_EV_GAIN));
    RunSetup_EnterConfirmation();
    RunSetup_SetNoEVGain(!enabled);
    EXPECT_EQ(RunSetup_GetNoEVGain(), enabled);
    RunSetup_ReturnToDraft();
    EXPECT_EQ(RunSetup_GetNoEVGain(), enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_SetNoEVGain(!enabled);
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_NO_EV_GAIN), enabled);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), otherRules);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), otherRules);
    FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup rejects unconfirmed No EV gain and clears stale rule flags")
{
    FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    RunSetup_Begin();
    RunSetup_SetNoEVGain(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_NO_EV_GAIN));
    RunSetup_SetNoEVGain(TRUE);
    EXPECT(!RunSetup_GetNoEVGain());
}

TEST("Run setup No EV gain participates in preset matching")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_SetNoEVGain(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetNoEVGain(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_Discard();
}

TEST("Run setup opponent HP percentage is independent and locked during confirmation")
{
    bool32 enabled;
    bool32 otherRules;

    PARAMETRIZE { enabled = FALSE; otherRules = FALSE; }
    PARAMETRIZE { enabled = FALSE; otherRules = TRUE; }
    PARAMETRIZE { enabled = TRUE; otherRules = FALSE; }
    PARAMETRIZE { enabled = TRUE; otherRules = TRUE; }

    FlagSet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    RunSetup_Begin();
    RunSetup_SetOpponentHPPercentage(enabled);
    RunSetup_SetFullCompatibility(otherRules);
    RunSetup_SetReusableTMs(otherRules);
    EXPECT(FlagGet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE));
    RunSetup_EnterConfirmation();
    RunSetup_SetOpponentHPPercentage(!enabled);
    EXPECT_EQ(RunSetup_GetOpponentHPPercentage(), enabled);
    RunSetup_ReturnToDraft();
    EXPECT_EQ(RunSetup_GetOpponentHPPercentage(), enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_SetOpponentHPPercentage(!enabled);
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE), enabled);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), otherRules);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), otherRules);
    FlagClear(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup rejects unconfirmed opponent HP percentage and clears stale rule flags")
{
    FlagSet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    RunSetup_Begin();
    RunSetup_SetOpponentHPPercentage(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE));
    RunSetup_SetOpponentHPPercentage(TRUE);
    EXPECT(!RunSetup_GetOpponentHPPercentage());
}

TEST("Run setup opponent HP percentage participates in preset matching")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_SetOpponentHPPercentage(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetOpponentHPPercentage(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_SetOpponentHPPercentage(TRUE);
    RunSetup_SetPreset(preset);
    EXPECT(!RunSetup_GetOpponentHPPercentage());
    RunSetup_Discard();
}

TEST("Run setup level caps is independent and locked during confirmation")
{
    bool32 enabled;
    bool32 otherRules;

    PARAMETRIZE { enabled = FALSE; otherRules = FALSE; }
    PARAMETRIZE { enabled = FALSE; otherRules = TRUE; }
    PARAMETRIZE { enabled = TRUE; otherRules = FALSE; }
    PARAMETRIZE { enabled = TRUE; otherRules = TRUE; }

    FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    RunSetup_Begin();
    RunSetup_SetLevelCaps(enabled);
    RunSetup_SetFullCompatibility(otherRules);
    RunSetup_SetReusableTMs(otherRules);
    EXPECT(FlagGet(FLAG_RUN_RULE_LEVEL_CAPS));
    RunSetup_EnterConfirmation();
    RunSetup_SetLevelCaps(!enabled);
    EXPECT_EQ(RunSetup_GetLevelCaps(), enabled);
    RunSetup_ReturnToDraft();
    EXPECT_EQ(RunSetup_GetLevelCaps(), enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_SetLevelCaps(!enabled);
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_LEVEL_CAPS), enabled);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), otherRules);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), otherRules);
    FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup rejects unconfirmed level caps and clears stale rule flags")
{
    FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    RunSetup_Begin();
    RunSetup_SetLevelCaps(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_LEVEL_CAPS));
    RunSetup_SetLevelCaps(TRUE);
    EXPECT(!RunSetup_GetLevelCaps());
}

TEST("Run setup level caps participates in preset matching")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_SetLevelCaps(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetLevelCaps(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_SetLevelCaps(TRUE);
    RunSetup_SetPreset(preset);
    EXPECT(!RunSetup_GetLevelCaps());
    RunSetup_Discard();
}

TEST("Run setup Frostbite is independent and locked during confirmation")
{
    bool32 enabled;
    bool32 otherRules;

    PARAMETRIZE { enabled = FALSE; otherRules = FALSE; }
    PARAMETRIZE { enabled = FALSE; otherRules = TRUE; }
    PARAMETRIZE { enabled = TRUE; otherRules = FALSE; }
    PARAMETRIZE { enabled = TRUE; otherRules = TRUE; }

    FlagSet(FLAG_RUN_RULE_FROSTBITE);
    RunSetup_Begin();
    RunSetup_SetFrostbite(enabled);
    RunSetup_SetFullCompatibility(otherRules);
    RunSetup_SetReusableTMs(otherRules);
    EXPECT(FlagGet(FLAG_RUN_RULE_FROSTBITE));
    RunSetup_EnterConfirmation();
    RunSetup_SetFrostbite(!enabled);
    EXPECT_EQ(RunSetup_GetFrostbite(), enabled);
    RunSetup_ReturnToDraft();
    EXPECT_EQ(RunSetup_GetFrostbite(), enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_SetFrostbite(!enabled);
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FROSTBITE), enabled);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), otherRules);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), otherRules);
    FlagClear(FLAG_RUN_RULE_FROSTBITE);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup rejects unconfirmed Frostbite and clears stale rule flags")
{
    FlagSet(FLAG_RUN_RULE_FROSTBITE);
    RunSetup_Begin();
    RunSetup_SetFrostbite(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_FROSTBITE));
    RunSetup_SetFrostbite(TRUE);
    EXPECT(!RunSetup_GetFrostbite());
}

TEST("Run setup Frostbite participates in preset matching")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_SetFrostbite(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetFrostbite(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_SetFrostbite(TRUE);
    RunSetup_SetPreset(preset);
    EXPECT(!RunSetup_GetFrostbite());
    RunSetup_Discard();
}

TEST("Run setup Frostbite survives saving and loading")
{
    bool32 enabled;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    RunSetup_Begin();
    RunSetup_SetFrostbite(enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    if (enabled)
        FlagClear(FLAG_RUN_RULE_FROSTBITE);
    else
        FlagSet(FLAG_RUN_RULE_FROSTBITE);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FROSTBITE), enabled);
    FlagClear(FLAG_RUN_RULE_FROSTBITE);
}


TEST("Run setup SetupMovePP is independent and locked during confirmation")
{
    bool32 enabled;
    bool32 otherRules;

    PARAMETRIZE { enabled = FALSE; otherRules = FALSE; }
    PARAMETRIZE { enabled = FALSE; otherRules = TRUE; }
    PARAMETRIZE { enabled = TRUE; otherRules = FALSE; }
    PARAMETRIZE { enabled = TRUE; otherRules = TRUE; }

    FlagSet(FLAG_RUN_RULE_SETUP_MOVE_PP);
    RunSetup_Begin();
    RunSetup_SetSetupMovePP(enabled);
    RunSetup_SetFullCompatibility(otherRules);
    RunSetup_SetReusableTMs(otherRules);
    EXPECT(FlagGet(FLAG_RUN_RULE_SETUP_MOVE_PP));
    RunSetup_EnterConfirmation();
    RunSetup_SetSetupMovePP(!enabled);
    EXPECT_EQ(RunSetup_GetSetupMovePP(), enabled);
    RunSetup_ReturnToDraft();
    EXPECT_EQ(RunSetup_GetSetupMovePP(), enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_SetSetupMovePP(!enabled);
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_SETUP_MOVE_PP), enabled);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), otherRules);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_REUSABLE_TMS), otherRules);
    FlagClear(FLAG_RUN_RULE_SETUP_MOVE_PP);
    FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);
}

TEST("Run setup rejects unconfirmed SetupMovePP and clears stale rule flags")
{
    FlagSet(FLAG_RUN_RULE_SETUP_MOVE_PP);
    RunSetup_Begin();
    RunSetup_SetSetupMovePP(TRUE);
    RunSetup_EnterConfirmation();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_SETUP_MOVE_PP));
    RunSetup_SetSetupMovePP(TRUE);
    EXPECT(!RunSetup_GetSetupMovePP());
}

TEST("Run setup SetupMovePP participates in preset matching")
{
    enum RunSetupPreset preset;

    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    RunSetup_Begin();
    RunSetup_SetPreset(preset);
    RunSetup_SetSetupMovePP(TRUE);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetSetupMovePP(FALSE);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_SetSetupMovePP(TRUE);
    RunSetup_SetPreset(preset);
    EXPECT(!RunSetup_GetSetupMovePP());
    RunSetup_Discard();
}

TEST("Run setup SetupMovePP survives saving and loading")
{
    bool32 enabled;

    PARAMETRIZE { enabled = FALSE; }
    PARAMETRIZE { enabled = TRUE; }

    RunSetup_Begin();
    RunSetup_SetSetupMovePP(enabled);
    RunSetup_EnterConfirmation();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    if (enabled)
        FlagClear(FLAG_RUN_RULE_SETUP_MOVE_PP);
    else
        FlagSet(FLAG_RUN_RULE_SETUP_MOVE_PP);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_SETUP_MOVE_PP), enabled);
    FlagClear(FLAG_RUN_RULE_SETUP_MOVE_PP);
}

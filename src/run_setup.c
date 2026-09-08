#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "run_setup.h"
#include "constants/flags.h"

enum RunSetupState
{
    RUN_SETUP_INACTIVE,
    RUN_SETUP_DRAFT,
    RUN_SETUP_CONFIRMATION,
    RUN_SETUP_CONFIRMED,
};

struct RunSetupDraft
{
    bool8 fullCompatibility;
    bool8 reusableTMs;
    bool8 noEVGain;
    bool8 opponentHPPercentage;
    bool8 levelCaps;
    enum RunSetupState state:8;
    enum RunSetupPreset preferredPreset:8;
};

static EWRAM_DATA struct RunSetupDraft sRunSetupDraft = {0};

static const struct
{
    bool8 fullCompatibility;
    bool8 reusableTMs;
    bool8 noEVGain;
    bool8 opponentHPPercentage;
    bool8 levelCaps;
} sRunSetupPresets[RUN_SETUP_PRESET_COUNT] =
{
    [RUN_SETUP_PRESET_VANILLA] = {FALSE, FALSE, FALSE, FALSE, FALSE},
    [RUN_SETUP_PRESET_NUZLOCKE] = {TRUE, TRUE, FALSE, FALSE, FALSE},
    [RUN_SETUP_PRESET_BISHEY] = {TRUE, TRUE, FALSE, FALSE, FALSE},
};

static bool32 RunSetup_MatchesPreset(enum RunSetupPreset preset)
{
    return sRunSetupDraft.fullCompatibility == sRunSetupPresets[preset].fullCompatibility
        && sRunSetupDraft.reusableTMs == sRunSetupPresets[preset].reusableTMs
        && sRunSetupDraft.noEVGain == sRunSetupPresets[preset].noEVGain
        && sRunSetupDraft.opponentHPPercentage == sRunSetupPresets[preset].opponentHPPercentage
        && sRunSetupDraft.levelCaps == sRunSetupPresets[preset].levelCaps;
}

void RunSetup_SetPreset(enum RunSetupPreset preset)
{
    if (sRunSetupDraft.state != RUN_SETUP_DRAFT || (u32)preset >= RUN_SETUP_PRESET_COUNT)
        return;
    sRunSetupDraft.fullCompatibility = sRunSetupPresets[preset].fullCompatibility;
    sRunSetupDraft.reusableTMs = sRunSetupPresets[preset].reusableTMs;
    sRunSetupDraft.noEVGain = sRunSetupPresets[preset].noEVGain;
    sRunSetupDraft.opponentHPPercentage = sRunSetupPresets[preset].opponentHPPercentage;
    sRunSetupDraft.levelCaps = sRunSetupPresets[preset].levelCaps;
    sRunSetupDraft.preferredPreset = preset;
}

enum RunSetupPreset RunSetup_GetPreset(void)
{
    // Retain the chosen name when presets have identical settings.
    if (RunSetup_MatchesPreset(sRunSetupDraft.preferredPreset))
        return sRunSetupDraft.preferredPreset;
    for (u32 preset = 0; preset < RUN_SETUP_PRESET_COUNT; preset++)
    {
        if (RunSetup_MatchesPreset(preset))
            return preset;
    }
    return RUN_SETUP_PRESET_CUSTOM;
}

void RunSetup_Begin(void)
{
    sRunSetupDraft.fullCompatibility = FALSE;
    sRunSetupDraft.reusableTMs = FALSE;
    sRunSetupDraft.noEVGain = FALSE;
    sRunSetupDraft.opponentHPPercentage = FALSE;
    sRunSetupDraft.levelCaps = FALSE;
    sRunSetupDraft.state = RUN_SETUP_DRAFT;
    sRunSetupDraft.preferredPreset = RUN_SETUP_PRESET_VANILLA;
}

void RunSetup_Discard(void)
{
    memset(&sRunSetupDraft, 0, sizeof(sRunSetupDraft));
}

void RunSetup_EnterConfirmation(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.state = RUN_SETUP_CONFIRMATION;
}

void RunSetup_ReturnToDraft(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMATION)
        sRunSetupDraft.state = RUN_SETUP_DRAFT;
}

void RunSetup_Confirm(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMATION)
        sRunSetupDraft.state = RUN_SETUP_CONFIRMED;
}

void RunSetup_ApplyToNewGame(void)
{
#if IS_FRLG
    RunSetup_Discard();
#else
    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.fullCompatibility)
        FlagSet(FLAG_RUN_RULE_FULL_COMPATIBILITY);
    else
        FlagClear(FLAG_RUN_RULE_FULL_COMPATIBILITY);

    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.reusableTMs)
        FlagSet(FLAG_RUN_RULE_REUSABLE_TMS);
    else
        FlagClear(FLAG_RUN_RULE_REUSABLE_TMS);

    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.noEVGain)
        FlagSet(FLAG_RUN_RULE_NO_EV_GAIN);
    else
        FlagClear(FLAG_RUN_RULE_NO_EV_GAIN);

    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.opponentHPPercentage)
        FlagSet(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);
    else
        FlagClear(FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE);

    if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && sRunSetupDraft.levelCaps)
        FlagSet(FLAG_RUN_RULE_LEVEL_CAPS);
    else
        FlagClear(FLAG_RUN_RULE_LEVEL_CAPS);

    RunSetup_Discard();
#endif
}

void RunSetup_PrepareDisplay(void)
{
    UnsetBgTilemapBuffer(0);
    ClearGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_WIN1_ON | DISPCNT_OBJWIN_ON);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);
}

void RunSetup_ClearDisplayTilemap(void)
{
    FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, DISPLAY_TILE_WIDTH, DISPLAY_TILE_HEIGHT);
}

void RunSetup_ClearDisplayGraphics(void)
{
    // The introduction uses a different character base, including setup's border tiles.
    DmaFill16(3, 0, (void *)VRAM, BG_VRAM_SIZE);
}

bool32 RunSetup_GetFullCompatibility(void)
{
    return sRunSetupDraft.fullCompatibility;
}

void RunSetup_SetFullCompatibility(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.fullCompatibility = enabled;
}

bool32 RunSetup_GetReusableTMs(void)
{
    return sRunSetupDraft.reusableTMs;
}

void RunSetup_SetReusableTMs(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.reusableTMs = enabled;
}

bool32 RunSetup_GetNoEVGain(void)
{
    return sRunSetupDraft.noEVGain;
}

void RunSetup_SetNoEVGain(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.noEVGain = enabled;
}

u32 RunSetup_GetScrollTop(u32 selection, u32 top, u32 count)
{
    if (count <= RUN_SETUP_VISIBLE_ROWS)
        return 0;
    if (top > count - RUN_SETUP_VISIBLE_ROWS)
        top = count - RUN_SETUP_VISIBLE_ROWS;
    if (selection < top)
        return selection;
    if (selection >= top + RUN_SETUP_VISIBLE_ROWS)
        return selection - RUN_SETUP_VISIBLE_ROWS + 1;
    return top;
}

bool32 RunSetup_GetOpponentHPPercentage(void)
{
    return sRunSetupDraft.opponentHPPercentage;
}

void RunSetup_SetOpponentHPPercentage(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.opponentHPPercentage = enabled;
}

bool32 RunSetup_GetLevelCaps(void)
{
    return sRunSetupDraft.levelCaps;
}

void RunSetup_SetLevelCaps(bool32 enabled)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.levelCaps = enabled;
}

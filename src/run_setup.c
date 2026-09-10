#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "run_setup.h"
#include "constants/flags.h"
#include "constants/vars.h"

static const u8 sOff[] = _("Off");
static const u8 sOn[] = _("On");
static const u8 sNo[] = _("No");
static const u8 sYes[] = _("Yes");
static const u8 sNormal[] = _("Normal");
static const u8 sHard[] = _("Hard");
static const u8 sUnfair[] = _("Unfair");
static const u8 *const sBooleanChoices[] = {sOff, sOn};
static const u8 *const sStoryChoices[] = {sNo, sYes};
static const u8 *const sDifficultyChoices[] = {sNormal, sHard, sUnfair};
static const u8 sPercent0[] = _("0%");
static const u8 sPercent1[] = _("10%");
static const u8 sPercent2[] = _("20%");
static const u8 sPercent3[] = _("30%");
static const u8 sPercent4[] = _("40%");
static const u8 sPercent5[] = _("50%");
static const u8 sPercent6[] = _("60%");
static const u8 sPercent7[] = _("70%");
static const u8 sPercent8[] = _("80%");
static const u8 sPercent9[] = _("90%");
static const u8 sPercent10[] = _("100%");
static const u8 *const sChanceChoices[] = {sPercent0, sPercent1, sPercent2, sPercent3, sPercent4, sPercent5, sPercent6, sPercent7, sPercent8, sPercent9, sPercent10};

static const u8 sLabel_ENCOUNTERS[] = _("Randomize encounters");
static const u8 sHelp_ENCOUNTERS[] = _("Randomize wild and gift Pokémon,\nstarters, and received trades.");
static const u8 sLabel_TRAINERS[] = _("Randomize trainers");
static const u8 sHelp_TRAINERS[] = _("Randomize enemy trainer Pokémon.");
static const u8 sLabel_ABILITIES[] = _("Randomize abilities");
static const u8 sHelp_ABILITIES[] = _("Randomize abilities for both\nplayer and enemy Pokémon.");
static const u8 sLabel_ITEMS[] = _("Randomize items");
static const u8 sHelp_ITEMS[] = _("Randomize pickups, gifts, held\nitems, and starting PC items.");
static const u8 sLabel_BAN_SLATEPORT[] = _("Ban Slateport items");
static const u8 sHelp_BAN_SLATEPORT[] = _("Exclude Slateport shop items\nfrom randomized item pools.");
static const u8 sLabel_BAN_GIMMICKS[] = _("Ban gimmick items");
static const u8 sHelp_BAN_GIMMICKS[] = _("Exclude Mega Stones, Z-Crystals,\nand Tera Shards from item pools.");
static const u8 sLabel_BAN_BATTLE_ITEMS[] = _("Ban in-battle items");
static const u8 sHelp_BAN_BATTLE_ITEMS[] = _("Exclude battle boosters and escape\nitems from randomized item pools.");
static const u8 sLabel_LEARNSETS[] = _("Randomize learnsets");
static const u8 sHelp_LEARNSETS[] = _("Randomize learned level-up\nmoves for Pokémon.");
static const u8 sLabel_TMS_TUTORS[] = _("Randomize TMs/tutors");
static const u8 sHelp_TMS_TUTORS[] = _("Randomize TM and tutor moves.\nHM moves stay unchanged.");
static const u8 sLabel_GOOD_MOVE_CHANCE[] = _("Good move chance");
static const u8 sHelp_GOOD_MOVE_CHANCE[] = _("Extra chance to choose a good\nattack when randomizing moves.");
static const u8 sLabel_DIFFICULTY[] = _("Difficulty");
static const u8 sHelp_DIFFICULTY[] = _("Set trainer levels, IVs, and\nrandomized team difficulty.");
static const u8 sLabel_LEVEL_CAPS[] = _("Level caps");
static const u8 sHelp_LEVEL_CAPS[] = _("Limit leveling to the ace\nPokémon of the next gym leader.");
static const u8 sLabel_NO_EV_GAIN[] = _("No EVs");
static const u8 sHelp_NO_EV_GAIN[] = _("Prevent EV gain from battles\nand EV-raising items.");
static const u8 sLabel_FROSTBITE[] = _("Frostbite");
static const u8 sHelp_FROSTBITE[] = _("Replace Freeze: Halves special\ndamage, loses 1/16 HP each turn.");
static const u8 sLabel_SETUP_MOVE_PP[] = _("Setup move PP");
static const u8 sHelp_SETUP_MOVE_PP[] = _("Limit selected setup moves to 1 PP.\nApplies to both sides.");
static const u8 sLabel_OPPONENT_HP_PERCENTAGE[] = _("Opponent HP %");
static const u8 sHelp_OPPONENT_HP_PERCENTAGE[] = _("Show opponent HP percentages.\nDoubles: START swaps bars/numbers.");
static const u8 sLabel_ENEMY_STAB[] = _("Enemy STAB");
static const u8 sHelp_ENEMY_STAB[] = _("Give enemy trainer Pokémon\ngood STAB moves on their types.");
static const u8 sLabel_FULL_COMPATIBILITY[] = _("Move compatibility");
static const u8 sHelp_FULL_COMPATIBILITY[] = _("All Pokémon can learn every\nTM, HM, and tutor move.");
static const u8 sLabel_REUSABLE_TMS[] = _("Reusable TMs");
static const u8 sHelp_REUSABLE_TMS[] = _("Teach TM moves without\nusing up the TM.");
static const u8 sLabel_REMOVE_STORY[] = _("Remove Story");
static const u8 sHelp_REMOVE_STORY[] = _("Skip the main Aqua/Magma plot.\nKeep gyms, rivals, and exploration.");
static const u8 sLabel_EARLY_SURF[] = _("Early Surf");
static const u8 sHelp_EARLY_SURF[] = _("Receive Surf in Rusturf Tunnel.\nUse it after defeating Wattson.");
static const u8 sLabel_EARLY_FLY[] = _("Early Fly");
static const u8 sHelp_EARLY_FLY[] = _("Receive Fly from the Route 110 rival.\nNo badge needed for field use.");
static const u8 sLabel_AUTO_HEAL[] = _("Auto heal");
static const u8 sHelp_AUTO_HEAL[] = _("Heal surviving party Pokémon\nand recharge Tera after battles.");
static const u8 sLabel_INSTANT_CATCH[] = _("Instant catch");
static const u8 sHelp_INSTANT_CATCH[] = _("Guarantee valid wild catches with\na short animation. Balls are used.");

const struct RunSetupSettingInfo gRunSetupSettings[RUN_SETUP_SETTING_COUNT] =
{
    [RUN_SETUP_ENCOUNTERS] = {sLabel_ENCOUNTERS, sHelp_ENCOUNTERS, sBooleanChoices, FLAG_RUN_RULE_ENCOUNTERS, RUN_SETUP_CATEGORY_POKEMON, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_ABILITIES] = {sLabel_ABILITIES, sHelp_ABILITIES, sBooleanChoices, FLAG_RUN_RULE_ABILITIES, RUN_SETUP_CATEGORY_POKEMON, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_SETUP_MOVE_PP] = {sLabel_SETUP_MOVE_PP, sHelp_SETUP_MOVE_PP, sBooleanChoices, FLAG_RUN_RULE_SETUP_MOVE_PP, RUN_SETUP_CATEGORY_MOVES, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 1, 1}},
    [RUN_SETUP_LEARNSETS] = {sLabel_LEARNSETS, sHelp_LEARNSETS, sBooleanChoices, FLAG_RUN_RULE_LEARNSETS, RUN_SETUP_CATEGORY_MOVES, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_TMS_TUTORS] = {sLabel_TMS_TUTORS, sHelp_TMS_TUTORS, sBooleanChoices, FLAG_RUN_RULE_TMS_TUTORS, RUN_SETUP_CATEGORY_MOVES, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_GOOD_MOVE_CHANCE] = {sLabel_GOOD_MOVE_CHANCE, sHelp_GOOD_MOVE_CHANCE, sChanceChoices, VAR_RUN_RULE_GOOD_MOVE_CHANCE, RUN_SETUP_CATEGORY_MOVES, ARRAY_COUNT(sChanceChoices), RUN_SETUP_DEPENDENCY_MOVES, {0, 0, 3}},
    [RUN_SETUP_DIFFICULTY] = {sLabel_DIFFICULTY, sHelp_DIFFICULTY, sDifficultyChoices, VAR_RUN_RULE_DIFFICULTY, RUN_SETUP_CATEGORY_TRAINERS, ARRAY_COUNT(sDifficultyChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 1, 2}},
    [RUN_SETUP_TRAINERS] = {sLabel_TRAINERS, sHelp_TRAINERS, sBooleanChoices, FLAG_RUN_RULE_TRAINERS, RUN_SETUP_CATEGORY_TRAINERS, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_ENEMY_STAB] = {sLabel_ENEMY_STAB, sHelp_ENEMY_STAB, sBooleanChoices, FLAG_RUN_RULE_ENEMY_STAB, RUN_SETUP_CATEGORY_TRAINERS, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_TRAINERS, {0, 0, 1}},
    [RUN_SETUP_LEVEL_CAPS] = {sLabel_LEVEL_CAPS, sHelp_LEVEL_CAPS, sBooleanChoices, FLAG_RUN_RULE_LEVEL_CAPS, RUN_SETUP_CATEGORY_BATTLE, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 1, 1}},
    [RUN_SETUP_FROSTBITE] = {sLabel_FROSTBITE, sHelp_FROSTBITE, sBooleanChoices, FLAG_RUN_RULE_FROSTBITE, RUN_SETUP_CATEGORY_BATTLE, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_OPPONENT_HP_PERCENTAGE] = {sLabel_OPPONENT_HP_PERCENTAGE, sHelp_OPPONENT_HP_PERCENTAGE, sBooleanChoices, FLAG_RUN_RULE_OPPONENT_HP_PERCENTAGE, RUN_SETUP_CATEGORY_BATTLE, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 1, 1}},
    [RUN_SETUP_NO_EV_GAIN] = {sLabel_NO_EV_GAIN, sHelp_NO_EV_GAIN, sBooleanChoices, FLAG_RUN_RULE_NO_EV_GAIN, RUN_SETUP_CATEGORY_TRAINING, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 1, 1}},
    [RUN_SETUP_FULL_COMPATIBILITY] = {sLabel_FULL_COMPATIBILITY, sHelp_FULL_COMPATIBILITY, sBooleanChoices, FLAG_RUN_RULE_FULL_COMPATIBILITY, RUN_SETUP_CATEGORY_TRAINING, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_REUSABLE_TMS] = {sLabel_REUSABLE_TMS, sHelp_REUSABLE_TMS, sBooleanChoices, FLAG_RUN_RULE_REUSABLE_TMS, RUN_SETUP_CATEGORY_TRAINING, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_ITEMS] = {sLabel_ITEMS, sHelp_ITEMS, sBooleanChoices, FLAG_RUN_RULE_ITEMS, RUN_SETUP_CATEGORY_ITEMS, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_BAN_SLATEPORT] = {sLabel_BAN_SLATEPORT, sHelp_BAN_SLATEPORT, sBooleanChoices, FLAG_RUN_RULE_BAN_SLATEPORT, RUN_SETUP_CATEGORY_ITEMS, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_ITEMS, {0, 0, 1}},
    [RUN_SETUP_BAN_GIMMICKS] = {sLabel_BAN_GIMMICKS, sHelp_BAN_GIMMICKS, sBooleanChoices, FLAG_RUN_RULE_BAN_GIMMICKS, RUN_SETUP_CATEGORY_ITEMS, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_ITEMS, {0, 0, 0}},
    [RUN_SETUP_BAN_BATTLE_ITEMS] = {sLabel_BAN_BATTLE_ITEMS, sHelp_BAN_BATTLE_ITEMS, sBooleanChoices, FLAG_RUN_RULE_BAN_BATTLE_ITEMS, RUN_SETUP_CATEGORY_ITEMS, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_ITEMS, {0, 0, 1}},
    [RUN_SETUP_REMOVE_STORY] = {sLabel_REMOVE_STORY, sHelp_REMOVE_STORY, sStoryChoices, FLAG_RUN_RULE_REMOVE_STORY, RUN_SETUP_CATEGORY_PROGRESSION, ARRAY_COUNT(sStoryChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_EARLY_SURF] = {sLabel_EARLY_SURF, sHelp_EARLY_SURF, sBooleanChoices, FLAG_RUN_RULE_EARLY_SURF, RUN_SETUP_CATEGORY_PROGRESSION, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_EARLY_FLY] = {sLabel_EARLY_FLY, sHelp_EARLY_FLY, sBooleanChoices, FLAG_RUN_RULE_EARLY_FLY, RUN_SETUP_CATEGORY_PROGRESSION, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_AUTO_HEAL] = {sLabel_AUTO_HEAL, sHelp_AUTO_HEAL, sBooleanChoices, FLAG_RUN_RULE_AUTO_HEAL, RUN_SETUP_CATEGORY_MISC, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
    [RUN_SETUP_INSTANT_CATCH] = {sLabel_INSTANT_CATCH, sHelp_INSTANT_CATCH, sBooleanChoices, FLAG_RUN_RULE_INSTANT_CATCH, RUN_SETUP_CATEGORY_MISC, ARRAY_COUNT(sBooleanChoices), RUN_SETUP_DEPENDENCY_NONE, {0, 0, 1}},
};

enum RunSetupState
{
    RUN_SETUP_INACTIVE,
    RUN_SETUP_DRAFT,
    RUN_SETUP_CONFIRMED,
};

static EWRAM_DATA struct
{
    u8 values[RUN_SETUP_SETTING_COUNT];
    enum RunSetupState state;
    enum RunSetupPreset preferredPreset;
} sRunSetupDraft = {0};

void RunSetup_SetPreset(enum RunSetupPreset preset)
{
    if (sRunSetupDraft.state != RUN_SETUP_DRAFT || (u32)preset >= RUN_SETUP_PRESET_COUNT)
        return;
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
        sRunSetupDraft.values[i] = gRunSetupSettings[i].presets[preset];
    sRunSetupDraft.preferredPreset = preset;
}

static bool32 RunSetup_MatchesPreset(enum RunSetupPreset preset)
{
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        if (sRunSetupDraft.values[i] != gRunSetupSettings[i].presets[preset])
            return FALSE;
    }
    return TRUE;
}

enum RunSetupPreset RunSetup_GetPreset(void)
{
    if (RunSetup_MatchesPreset(sRunSetupDraft.preferredPreset))
        return sRunSetupDraft.preferredPreset;
    for (u32 i = 0; i < RUN_SETUP_PRESET_COUNT; i++)
    {
        if (RunSetup_MatchesPreset(i))
            return i;
    }
    return RUN_SETUP_PRESET_CUSTOM;
}

void RunSetup_Begin(void)
{
    sRunSetupDraft.state = RUN_SETUP_DRAFT;
    RunSetup_SetPreset(RUN_SETUP_PRESET_VANILLA);
}

void RunSetup_Discard(void)
{
    memset(&sRunSetupDraft, 0, sizeof(sRunSetupDraft));
}

void RunSetup_Confirm(void)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT)
        sRunSetupDraft.state = RUN_SETUP_CONFIRMED;
}

u32 RunSetup_GetValue(enum RunSetupSetting setting)
{
    if ((u32)setting >= RUN_SETUP_SETTING_COUNT)
        return 0;
    return sRunSetupDraft.values[setting];
}

bool32 RunSetup_IsAvailable(enum RunSetupSetting setting)
{
    if ((u32)setting >= RUN_SETUP_SETTING_COUNT)
        return FALSE;
    switch (gRunSetupSettings[setting].dependency)
    {
    case RUN_SETUP_DEPENDENCY_ITEMS:
        return RunSetup_GetValue(RUN_SETUP_ITEMS) != 0;
    case RUN_SETUP_DEPENDENCY_TRAINERS:
        return RunSetup_GetValue(RUN_SETUP_TRAINERS) != 0;
    case RUN_SETUP_DEPENDENCY_MOVES:
        return RunSetup_GetValue(RUN_SETUP_LEARNSETS) || RunSetup_GetValue(RUN_SETUP_TMS_TUTORS);
    default:
        return TRUE;
    }
}

void RunSetup_SetValue(enum RunSetupSetting setting, u32 value)
{
    if (sRunSetupDraft.state == RUN_SETUP_DRAFT && RunSetup_IsAvailable(setting)
     && value < gRunSetupSettings[setting].choiceCount)
        sRunSetupDraft.values[setting] = value;
}

void RunSetup_ApplyToNewGame(void)
{
#if !IS_FRLG
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        u32 value = 0;
        u16 storage = gRunSetupSettings[i].storageId;

        if (sRunSetupDraft.state == RUN_SETUP_CONFIRMED && RunSetup_IsAvailable(i))
            value = RunSetup_GetValue(i);
        if (storage >= VARS_START)
            VarSet(storage, i == RUN_SETUP_GOOD_MOVE_CHANCE ? value * 10 : value);
        else if (value)
            FlagSet(storage);
        else
            FlagClear(storage);
    }
#endif
    RunSetup_Discard();
}

u32 RunSetup_GetCategoryCount(u32 category)
{
    u32 count = 0;
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        if (gRunSetupSettings[i].category == category)
            count++;
    }
    return count;
}

enum RunSetupSetting RunSetup_GetCategorySetting(u32 category, u32 row)
{
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        if (gRunSetupSettings[i].category == category && row-- == 0)
            return i;
    }
    return RUN_SETUP_SETTING_COUNT;
}

void RunSetup_MoveSelection(struct RunSetupNavigation *navigation, bool32 backwards)
{
    u32 category = navigation->category;
    u32 count = RunSetup_GetCategoryCount(category) + 2;
    u32 selection = navigation->selection[category];

    selection = backwards ? (selection + count - 1) % count : (selection + 1) % count;
    navigation->selection[category] = selection;
}

void RunSetup_SwitchCategory(struct RunSetupNavigation *navigation, bool32 backwards, bool32 keepCategoryFocus)
{
    navigation->category = (navigation->category + (backwards ? RUN_SETUP_CATEGORY_COUNT - 1 : 1)) % RUN_SETUP_CATEGORY_COUNT;
    if (keepCategoryFocus)
        navigation->selection[navigation->category] = 1;
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
    return RunSetup_GetValue(RUN_SETUP_FULL_COMPATIBILITY);
}

void RunSetup_SetFullCompatibility(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_FULL_COMPATIBILITY, enabled != FALSE);
}

bool32 RunSetup_GetReusableTMs(void)
{
    return RunSetup_GetValue(RUN_SETUP_REUSABLE_TMS);
}

void RunSetup_SetReusableTMs(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_REUSABLE_TMS, enabled != FALSE);
}

bool32 RunSetup_GetNoEVGain(void)
{
    return RunSetup_GetValue(RUN_SETUP_NO_EV_GAIN);
}

void RunSetup_SetNoEVGain(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_NO_EV_GAIN, enabled != FALSE);
}

bool32 RunSetup_GetOpponentHPPercentage(void)
{
    return RunSetup_GetValue(RUN_SETUP_OPPONENT_HP_PERCENTAGE);
}

void RunSetup_SetOpponentHPPercentage(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_OPPONENT_HP_PERCENTAGE, enabled != FALSE);
}

bool32 RunSetup_GetLevelCaps(void)
{
    return RunSetup_GetValue(RUN_SETUP_LEVEL_CAPS);
}

void RunSetup_SetLevelCaps(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_LEVEL_CAPS, enabled != FALSE);
}

bool32 RunSetup_GetFrostbite(void)
{
    return RunSetup_GetValue(RUN_SETUP_FROSTBITE);
}

void RunSetup_SetFrostbite(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_FROSTBITE, enabled != FALSE);
}

bool32 RunSetup_GetSetupMovePP(void)
{
    return RunSetup_GetValue(RUN_SETUP_SETUP_MOVE_PP);
}

void RunSetup_SetSetupMovePP(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_SETUP_MOVE_PP, enabled != FALSE);
}

bool32 RunSetup_GetInstantCatch(void)
{
    return RunSetup_GetValue(RUN_SETUP_INSTANT_CATCH);
}

void RunSetup_SetInstantCatch(bool32 enabled)
{
    RunSetup_SetValue(RUN_SETUP_INSTANT_CATCH, enabled != FALSE);
}

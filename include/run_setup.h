#ifndef GUARD_RUN_SETUP_H
#define GUARD_RUN_SETUP_H

#define RUN_SETUP_VISIBLE_ROWS 4

enum RunSetupPreset
{
    RUN_SETUP_PRESET_VANILLA,
    RUN_SETUP_PRESET_NUZLOCKE,
    RUN_SETUP_PRESET_BISHEY,
    RUN_SETUP_PRESET_COUNT,
    RUN_SETUP_PRESET_CUSTOM = RUN_SETUP_PRESET_COUNT,
};

enum RunSetupCategory
{
    RUN_SETUP_CATEGORY_POKEMON,
    RUN_SETUP_CATEGORY_MOVES,
    RUN_SETUP_CATEGORY_TRAINERS,
    RUN_SETUP_CATEGORY_BATTLE,
    RUN_SETUP_CATEGORY_TRAINING,
    RUN_SETUP_CATEGORY_ITEMS,
    RUN_SETUP_CATEGORY_PROGRESSION,
    RUN_SETUP_CATEGORY_MISC,
    RUN_SETUP_CATEGORY_COUNT,
};

enum RunSetupSetting
{
    RUN_SETUP_ENCOUNTERS,
    RUN_SETUP_ABILITIES,
    RUN_SETUP_SETUP_MOVE_PP,
    RUN_SETUP_LEARNSETS,
    RUN_SETUP_TMS_TUTORS,
    RUN_SETUP_GOOD_MOVE_CHANCE,
    RUN_SETUP_DIFFICULTY,
    RUN_SETUP_TRAINERS,
    RUN_SETUP_ENEMY_STAB,
    RUN_SETUP_LEVEL_CAPS,
    RUN_SETUP_FROSTBITE,
    RUN_SETUP_OPPONENT_HP_PERCENTAGE,
    RUN_SETUP_NO_EV_GAIN,
    RUN_SETUP_FULL_COMPATIBILITY,
    RUN_SETUP_REUSABLE_TMS,
    RUN_SETUP_ITEMS,
    RUN_SETUP_BAN_SLATEPORT,
    RUN_SETUP_BAN_GIMMICKS,
    RUN_SETUP_BAN_BATTLE_ITEMS,
    RUN_SETUP_REMOVE_STORY,
    RUN_SETUP_EARLY_SURF,
    RUN_SETUP_EARLY_FLY,
    RUN_SETUP_AUTO_HEAL,
    RUN_SETUP_INSTANT_CATCH,
    RUN_SETUP_SETTING_COUNT,
};

enum RunSetupDependency
{
    RUN_SETUP_DEPENDENCY_NONE,
    RUN_SETUP_DEPENDENCY_ITEMS,
    RUN_SETUP_DEPENDENCY_MOVES,
    RUN_SETUP_DEPENDENCY_TRAINERS,
};

struct RunSetupSettingInfo
{
    const u8 *label;
    const u8 *help;
    const u8 *const *choices;
    u16 storageId;
    u8 category;
    u8 choiceCount;
    u8 dependency;
    u8 presets[RUN_SETUP_PRESET_COUNT];
};

struct RunSetupNavigation
{
    u8 category;
    u8 selection[RUN_SETUP_CATEGORY_COUNT];
};

extern const struct RunSetupSettingInfo gRunSetupSettings[RUN_SETUP_SETTING_COUNT];
u32 RunSetup_GetValue(enum RunSetupSetting setting);
void RunSetup_SetValue(enum RunSetupSetting setting, u32 value);
bool32 RunSetup_IsAvailable(enum RunSetupSetting setting);
u32 RunSetup_GetCategoryCount(u32 category);
enum RunSetupSetting RunSetup_GetCategorySetting(u32 category, u32 row);
void RunSetup_MoveSelection(struct RunSetupNavigation *navigation, bool32 backwards);
void RunSetup_SwitchCategory(struct RunSetupNavigation *navigation, bool32 backwards, bool32 keepCategoryFocus);

void RunSetup_SetPreset(enum RunSetupPreset preset);
enum RunSetupPreset RunSetup_GetPreset(void);

void RunSetup_Begin(void);
void RunSetup_Discard(void);
void RunSetup_Confirm(void);
void RunSetup_ApplyToNewGame(void);
void RunSetup_PrepareDisplay(void);
void RunSetup_ClearDisplayTilemap(void);
void RunSetup_ClearDisplayGraphics(void);
bool32 RunSetup_GetFullCompatibility(void);
void RunSetup_SetFullCompatibility(bool32 enabled);

bool32 RunSetup_GetReusableTMs(void);
void RunSetup_SetReusableTMs(bool32 enabled);

bool32 RunSetup_GetNoEVGain(void);
void RunSetup_SetNoEVGain(bool32 enabled);

bool32 RunSetup_GetOpponentHPPercentage(void);
void RunSetup_SetOpponentHPPercentage(bool32 enabled);

bool32 RunSetup_GetLevelCaps(void);
void RunSetup_SetLevelCaps(bool32 enabled);

bool32 RunSetup_GetFrostbite(void);
void RunSetup_SetFrostbite(bool32 enabled);

bool32 RunSetup_GetSetupMovePP(void);
void RunSetup_SetSetupMovePP(bool32 enabled);

bool32 RunSetup_GetInstantCatch(void);
void RunSetup_SetInstantCatch(bool32 enabled);

#endif // GUARD_RUN_SETUP_H

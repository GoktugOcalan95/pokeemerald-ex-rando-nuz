#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "run_setup.h"
#include "save.h"
#include "text.h"
#include "constants/vars.h"
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

TEST("Run setup defaults, presets and edits stay draft-only until START")
{
    enum RunSetupPreset preset;
    PARAMETRIZE { preset = RUN_SETUP_PRESET_VANILLA; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_NUZLOCKE; }
    PARAMETRIZE { preset = RUN_SETUP_PRESET_BISHEY; }

    InitEventData();
    RunSetup_Begin();
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_VANILLA);
    RunSetup_SetPreset(preset);
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        const struct RunSetupSettingInfo *info = &gRunSetupSettings[i];
        EXPECT_EQ(RunSetup_GetValue(i), info->presets[preset]);
        if (info->storageId < VARS_START)
            EXPECT_EQ(FlagGet(info->storageId), FALSE);
        else
            EXPECT_EQ(VarGet(info->storageId), 0);
    }
    RunSetup_SetReusableTMs(!RunSetup_GetReusableTMs());
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetPreset(preset);
    RunSetup_Confirm();
    RunSetup_SetPreset(RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetPreset((enum RunSetupPreset)-1);
    RunSetup_SetPreset((preset + 1) % RUN_SETUP_PRESET_COUNT);
    RunSetup_SetReusableTMs(!RunSetup_GetReusableTMs());
    EXPECT_EQ(RunSetup_GetPreset(), preset);
    RunSetup_ApplyToNewGame();
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        const struct RunSetupSettingInfo *info = &gRunSetupSettings[i];
        u32 expected = info->presets[preset];
        if (info->storageId < VARS_START)
            EXPECT_EQ(FlagGet(info->storageId), expected);
        else
            EXPECT_EQ(VarGet(info->storageId), i == RUN_SETUP_GOOD_MOVE_CHANCE ? expected * 10 : expected);
    }
    InitEventData();
}

TEST("Run setup discards drafts and refuses to apply settings without START")
{
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_VANILLA);
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        u16 storage = gRunSetupSettings[i].storageId;
        EXPECT_EQ(RunSetup_GetValue(i), 0);
        if (storage < VARS_START)
            EXPECT_EQ(FlagGet(storage), FALSE);
        else
            EXPECT_EQ(VarGet(storage), 0);
    }
    RunSetup_SetFullCompatibility(TRUE);
    EXPECT_EQ(RunSetup_GetFullCompatibility(), FALSE);
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_Discard();
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_FULL_COMPATIBILITY), FALSE);
}

TEST("Run setup disabled children retain choices but have no saved effect")
{
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_SetValue(RUN_SETUP_ITEMS, 0);
    RunSetup_SetValue(RUN_SETUP_LEARNSETS, 0);
    RunSetup_SetValue(RUN_SETUP_TMS_TUTORS, 0);
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_BAN_SLATEPORT));
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_BAN_GIMMICKS));
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_BAN_BATTLE_ITEMS));
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_GOOD_MOVE_CHANCE));
    RunSetup_SetValue(RUN_SETUP_BAN_SLATEPORT, 0);
    RunSetup_SetValue(RUN_SETUP_GOOD_MOVE_CHANCE, 10);
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_BAN_SLATEPORT), 1);
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_GOOD_MOVE_CHANCE), 3);
    RunSetup_SetValue(RUN_SETUP_ITEMS, 1);
    RunSetup_SetValue(RUN_SETUP_TMS_TUTORS, 1);
    EXPECT(RunSetup_IsAvailable(RUN_SETUP_GOOD_MOVE_CHANCE));
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_BAN_SLATEPORT), 1);
    RunSetup_SetValue(RUN_SETUP_TMS_TUTORS, 0);
    RunSetup_SetValue(RUN_SETUP_LEARNSETS, 1);
    EXPECT(RunSetup_IsAvailable(RUN_SETUP_GOOD_MOVE_CHANCE));
    RunSetup_SetValue(RUN_SETUP_ITEMS, 0);
    RunSetup_SetValue(RUN_SETUP_LEARNSETS, 0);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    EXPECT(!FlagGet(FLAG_RUN_RULE_BAN_SLATEPORT));
    EXPECT(!FlagGet(FLAG_RUN_RULE_BAN_BATTLE_ITEMS));
    EXPECT_EQ(VarGet(VAR_RUN_RULE_GOOD_MOVE_CHANCE), 0);
    InitEventData();
}

TEST("Run setup preset replacement clears inactive child choices")
{
    RunSetup_Begin();
    RunSetup_SetValue(RUN_SETUP_ITEMS, 1);
    RunSetup_SetValue(RUN_SETUP_BAN_SLATEPORT, 1);
    RunSetup_SetValue(RUN_SETUP_ITEMS, 0);
    EXPECT_EQ(RunSetup_GetPreset(), RUN_SETUP_PRESET_CUSTOM);
    RunSetup_SetPreset(RUN_SETUP_PRESET_VANILLA);
    RunSetup_SetValue(RUN_SETUP_ITEMS, 1);
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_BAN_SLATEPORT), 0);
    RunSetup_SetValue(RUN_SETUP_ITEMS, 2);
    RunSetup_SetValue(RUN_SETUP_SETTING_COUNT, 1);
    RunSetup_SetValue((enum RunSetupSetting)-1, 1);
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_ITEMS), 1);
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_SETTING_COUNT));
    RunSetup_SetValue(RUN_SETUP_DIFFICULTY, 3);
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_DIFFICULTY), 0);
    RunSetup_Discard();
}

TEST("Run setup saves and reloads every setting and numeric choice")
{
    u32 chance;
    u32 difficulty;
    PARAMETRIZE { chance = 0; difficulty = 0; }
    PARAMETRIZE { chance = 3; difficulty = 1; }
    PARAMETRIZE { chance = 10; difficulty = 2; }

    InitEventData();
    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_SetValue(RUN_SETUP_BAN_GIMMICKS, 1);
    RunSetup_SetValue(RUN_SETUP_GOOD_MOVE_CHANCE, chance);
    RunSetup_SetValue(RUN_SETUP_DIFFICULTY, difficulty);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    InitEventData();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        if (gRunSetupSettings[i].storageId < VARS_START)
            EXPECT(FlagGet(gRunSetupSettings[i].storageId));
    }
    EXPECT_EQ(VarGet(VAR_RUN_RULE_GOOD_MOVE_CHANCE), chance * 10);
    EXPECT_EQ(VarGet(VAR_RUN_RULE_DIFFICULTY), difficulty);
    InitEventData();
}

TEST("Run setup category navigation covers all rows and remembers positions")
{
    struct RunSetupNavigation navigation = {0};
    u32 total = 0;
    bool8 seen[RUN_SETUP_SETTING_COUNT] = {0};

    for (u32 category = 0; category < RUN_SETUP_CATEGORY_COUNT; category++)
    {
        u32 count = RunSetup_GetCategoryCount(category);
        EXPECT_EQ(navigation.category, category);
        EXPECT_EQ(navigation.selection[category], 0);
        for (u32 row = 0; row < count; row++)
        {
            enum RunSetupSetting setting = RunSetup_GetCategorySetting(category, row);
            EXPECT(setting < RUN_SETUP_SETTING_COUNT);
            EXPECT(!seen[setting]);
            seen[setting] = TRUE;
            total++;
        }
        RunSetup_MoveSelection(&navigation, TRUE);
        EXPECT_EQ(navigation.selection[category], count + 1);
        EXPECT(count <= RUN_SETUP_VISIBLE_ROWS);
        RunSetup_SwitchCategory(&navigation, FALSE, FALSE);
    }
    EXPECT_EQ(total, RUN_SETUP_SETTING_COUNT);
    EXPECT_EQ(navigation.category, 0);
    EXPECT_EQ(navigation.selection[0], RunSetup_GetCategoryCount(0) + 1);
    RunSetup_MoveSelection(&navigation, FALSE);
    EXPECT_EQ(navigation.selection[0], 0);
    RunSetup_MoveSelection(&navigation, FALSE);
    EXPECT_EQ(navigation.selection[0], 1);
    RunSetup_SwitchCategory(&navigation, TRUE, TRUE);
    EXPECT_EQ(navigation.category, RUN_SETUP_CATEGORY_MISC);
    EXPECT_EQ(navigation.selection[RUN_SETUP_CATEGORY_MISC], 1);
    RunSetup_MoveSelection(&navigation, FALSE);
    EXPECT_EQ(navigation.selection[RUN_SETUP_CATEGORY_MISC], 2);
    EXPECT_EQ(RunSetup_GetCategorySetting(RUN_SETUP_CATEGORY_COUNT, 0), RUN_SETUP_SETTING_COUNT);
}

TEST("Run setup labels, values and help fit the four-row layout")
{
    for (u32 i = 0; i < RUN_SETUP_SETTING_COUNT; i++)
    {
        const struct RunSetupSettingInfo *info = &gRunSetupSettings[i];
        u32 x = info->dependency == RUN_SETUP_DEPENDENCY_NONE ? 16 : 24;
        u32 labelWidth = GetStringWidth(FONT_NORMAL, info->label, 0);
        EXPECT(GetStringWidth(FONT_SMALL, info->help, 0) <= 208);
        for (u32 choice = 0; choice < info->choiceCount; choice++)
            EXPECT(x + labelWidth + 8 + GetStringWidth(FONT_NORMAL, info->choices[choice], 0) <= 208);
        for (u32 j = i + 1; j < RUN_SETUP_SETTING_COUNT; j++)
            EXPECT(info->storageId != gRunSetupSettings[j].storageId);
    }
}

TEST("Run setup Enemy STAB requires trainer randomization")
{
    bool32 trainers;
    PARAMETRIZE { trainers = FALSE; }
    PARAMETRIZE { trainers = TRUE; }

    RunSetup_Begin();
    RunSetup_SetPreset(RUN_SETUP_PRESET_BISHEY);
    RunSetup_SetValue(RUN_SETUP_TRAINERS, FALSE);
    EXPECT(!RunSetup_IsAvailable(RUN_SETUP_ENEMY_STAB));
    RunSetup_SetValue(RUN_SETUP_ENEMY_STAB, FALSE);
    EXPECT_EQ(RunSetup_GetValue(RUN_SETUP_ENEMY_STAB), TRUE);
    RunSetup_SetValue(RUN_SETUP_TRAINERS, trainers);
    EXPECT_EQ(RunSetup_IsAvailable(RUN_SETUP_ENEMY_STAB), trainers);
    RunSetup_Confirm();
    RunSetup_ApplyToNewGame();
    EXPECT_EQ(FlagGet(FLAG_RUN_RULE_ENEMY_STAB), trainers);
    InitEventData();
}

#include "global.h"
#include "remove_story.h"
#include "event_data.h"
#include "field_move.h"
#include "item.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "constants/map_event_ids.h"
#include "constants/map_groups.h"
#include "save.h"
#include "script.h"
#include "test/test.h"
#include "test/overworld_script.h"
#include "constants/field_move.h"
#include "constants/script_commands.h"

extern const u8 EventScript_ResetAllMapFlags[];
extern const u8 RemoveStory_GiveDevonTools[];
extern const u8 RemoveStory_StevenLetter[];
extern const u8 RemoveStory_Wallace[];
extern const u8 RemoveStory_StevenDive[];
extern const u8 RemoveStory_GiveMagmaEmblem[];
extern const u8 RemoveStory_WoodsResearcher[];
extern const u8 RemoveStory_MrStone[];
extern ScrCmdFunc gScriptCmdTable[];
extern ScrCmdFunc gScriptCmdTableEnd[];
void ScriptShowItemDescription(struct ScriptContext *ctx);
void ScriptHideItemDescription(struct ScriptContext *ctx);

static void StartStoryMode(bool32 enabled)
{
    InitEventData();
    RunScriptImmediately(EventScript_ResetAllMapFlags);
    ClearBag();
    if (enabled)
        FlagSet(FLAG_RUN_RULE_REMOVE_STORY);
    InitRemoveStory();
}

static bool8 SkipPresentation(struct ScriptContext *ctx)
{
    switch (ctx->scriptPtr[-1])
    {
    case SCR_OP_MESSAGE: ctx->scriptPtr += 4; break;
    case SCR_OP_DELAY:
    case SCR_OP_PLAYFANFARE:
    case SCR_OP_REMOVEOBJECT: ctx->scriptPtr += 2; break;
    default: break;
    }
    return TRUE;
}

static bool8 RunGiftNative(struct ScriptContext *ctx)
{
    u32 pointer = ScriptReadWord(ctx) & ~0x02000000;
    if (pointer == (u32)ScriptShowItemDescription)
        ctx->scriptPtr++;
    else if (pointer != (u32)ScriptHideItemDescription)
    {
        ctx->scriptPtr -= 4;
        return gScriptCmdTable[SCR_OP_CALLNATIVE](ctx);
    }
    return TRUE;
}

// Execute real receipt and branch commands while omitting screen and sound effects.
static void RunGift(const u8 *script, u32 receiptFlag)
{
    ScrCmdFunc commands[256];
    struct ScriptContext ctx;
    u32 count = gScriptCmdTableEnd - gScriptCmdTable;
    static const u8 presentation[] = {
        SCR_OP_MESSAGE, SCR_OP_WAITMESSAGE, SCR_OP_WAITFANFARE, SCR_OP_WAITBUTTONPRESS,
        SCR_OP_PLAYFANFARE, SCR_OP_DELAY, SCR_OP_LOCK, SCR_OP_LOCKALL,
        SCR_OP_RELEASE, SCR_OP_RELEASEALL, SCR_OP_FACEPLAYER, SCR_OP_CLOSEMESSAGE,
        SCR_OP_REMOVEOBJECT,
    };
    memcpy(commands, gScriptCmdTable, count * sizeof(*commands));
    for (u32 i = 0; i < ARRAY_COUNT(presentation); i++)
        commands[presentation[i]] = SkipPresentation;
    commands[SCR_OP_CALLNATIVE] = RunGiftNative;
    InitScriptContext(&ctx, commands, commands + count);
    SetupBytecodeScript(&ctx, script);
    u32 steps = 0;
    while (RunScriptCommand(&ctx))
    {
        if (receiptFlag && FlagGet(receiptFlag))
            break;
        EXPECT(++steps < 500);
    }
}

TEST("Remove Story leaves normal new games unchanged and skips only plot progression")
{
    StartStoryMode(FALSE);
    EXPECT(!FlagGet(FLAG_RETURNED_DEVON_GOODS));
    EXPECT(!FlagGet(FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE));
    EXPECT_EQ(VarGet(VAR_PETALBURG_WOODS_STATE), 0);
    EXPECT(FlagGet(FLAG_HIDE_REMOVE_STORY_MUSEUM_ITEM));
    StartStoryMode(TRUE);
    EXPECT(FlagGet(FLAG_HIDE_PETALBURG_WOODS_AQUA_GRUNT));
    EXPECT(!FlagGet(FLAG_HIDE_PETALBURG_WOODS_DEVON_EMPLOYEE));
    EXPECT(FlagGet(FLAG_HIDE_AQUA_HIDEOUT_GRUNTS));
    EXPECT(FlagGet(FLAG_HIDE_ROUTE_110_TEAM_AQUA));
    EXPECT(FlagGet(FLAG_HIDE_ROUTE_121_TEAM_AQUA_GRUNTS));
    EXPECT_EQ(VarGet(VAR_ROUTE121_STATE), 1);
    EXPECT(FlagGet(FLAG_HIDE_MAGMA_HIDEOUT_GRUNTS));
    EXPECT(FlagGet(FLAG_HIDE_SEAFLOOR_CAVERN_AQUA_GRUNTS));
    EXPECT(FlagGet(FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE));
    EXPECT(!FlagGet(FLAG_HIDE_REMOVE_STORY_MUSEUM_ITEM));
    EXPECT_EQ(VarGet(VAR_JAGGED_PASS_STATE), 2);
    EXPECT_EQ(VarGet(VAR_ROUTE119_STATE), 0);
    EXPECT_EQ(VarGet(VAR_MOSSDEEP_CITY_STATE), 0);
    EXPECT_EQ(VarGet(VAR_SOOTOPOLIS_CITY_STATE), 0);
    EXPECT_EQ(VarGet(VAR_SKY_PILLAR_STATE), 0);
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT(!FlagGet(FLAG_DELIVERED_STEVEN_LETTER));
    EXPECT(!FlagGet(FLAG_RECEIVED_EXP_SHARE));
    EXPECT(!FlagGet(FLAG_RECEIVED_CASTFORM));
    EXPECT(!FlagGet(FLAG_RECEIVED_METEORITE));
    EXPECT(!FlagGet(FLAG_RECEIVED_TM_RETURN));
    EXPECT(!FlagGet(FLAG_ITEM_AQUA_HIDEOUT_B1F_MASTER_BALL));
    EXPECT(FlagGet(FLAG_HIDE_MOSSDEEP_CITY_STEVENS_HOUSE_BELDUM_POKEBALL));
    StartStoryMode(FALSE);
}

TEST("Remove Story opens Briney and retains the Rustboro rival only after Roxanne")
{
    StartStoryMode(TRUE);
    RemoveStoryAfterRoxanne();
    EXPECT(FlagGet(FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY));
    EXPECT_EQ(VarGet(VAR_RUSTBORO_CITY_STATE), 0);
    FlagSet(FLAG_BADGE01_GET);
    RemoveStoryAfterRoxanne();
    EXPECT(!FlagGet(FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY));
    EXPECT(!FlagGet(FLAG_HIDE_ROUTE_116_DEVON_EMPLOYEE));
    EXPECT(!FlagGet(FLAG_HIDE_RUSTBORO_CITY_RIVAL));
    EXPECT_EQ(VarGet(VAR_RUSTBORO_CITY_STATE), 7);
    EXPECT(!FlagGet(FLAG_DELIVERED_STEVEN_LETTER));
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    StartStoryMode(FALSE);
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(FlagGet(FLAG_RUN_RULE_REMOVE_STORY));
    EXPECT_EQ(VarGet(VAR_RUSTBORO_CITY_STATE), 7);
    EXPECT(!FlagGet(FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY));
    StartStoryMode(FALSE);
}

TEST("Remove Story Letter receipt retries after a full Bag independently of PokeNav")
{
    StartStoryMode(TRUE);
    RunGift(RemoveStory_GiveDevonTools, 0);
    EXPECT(!FlagGet(FLAG_RECEIVED_POKENAV));
    FlagSet(FLAG_BADGE01_GET);
    while (AddBagItem(ITEM_TOGGLE_REPEL, 999))
        ;
    RunGift(RemoveStory_GiveDevonTools, 0);
    EXPECT(FlagGet(FLAG_RECEIVED_POKENAV));
    EXPECT(!FlagGet(FLAG_REMOVE_STORY_RECEIVED_LETTER));
    ClearBag();
    RunGift(RemoveStory_GiveDevonTools, FLAG_REMOVE_STORY_RECEIVED_LETTER);
    EXPECT(CheckBagHasItem(ITEM_LETTER, 1));
    RunGift(RemoveStory_GiveDevonTools, 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LETTER), 1);
    EXPECT(!FlagGet(FLAG_DELIVERED_STEVEN_LETTER));
    StartStoryMode(FALSE);
}

TEST("Remove Story replacement HM and Emblem gifts retry without bypassing field badges")
{
    u32 item, flag;
    const u8 *script;
    PARAMETRIZE { item = ITEM_HM_DIVE; flag = FLAG_RECEIVED_HM_DIVE; script = RemoveStory_StevenDive; }
    PARAMETRIZE { item = ITEM_HM_WATERFALL; flag = FLAG_RECEIVED_HM_WATERFALL; script = RemoveStory_Wallace; }
    PARAMETRIZE { item = ITEM_MAGMA_EMBLEM; flag = FLAG_RECEIVED_RED_OR_BLUE_ORB; script = RemoveStory_GiveMagmaEmblem; }
    StartStoryMode(TRUE);
    EXPECT(AddBagItem(item, 999));
    if (item == ITEM_MAGMA_EMBLEM)
        while (AddBagItem(ITEM_TOGGLE_REPEL, 999))
            ;
    RunGift(script, 0);
    EXPECT(!FlagGet(flag));
    EXPECT(RemoveBagItem(item, 1));
    RunGift(script, flag);
    EXPECT(FlagGet(flag));
    EXPECT_EQ(CountTotalItemQuantityInBag(item), 999);
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_DIVE));
    EXPECT(!IsFieldMoveUnlocked(FIELD_MOVE_WATERFALL));
    FlagSet(FLAG_BADGE07_GET);
    EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_DIVE), item == ITEM_HM_DIVE);
    FlagSet(FLAG_BADGE08_GET);
    EXPECT_EQ(IsFieldMoveUnlocked(FIELD_MOVE_WATERFALL), item == ITEM_HM_WATERFALL);
    StartStoryMode(FALSE);
}

TEST("Remove Story Steel Wing retries independently after delivering the Letter")
{
    StartStoryMode(TRUE);
    RunGift(RemoveStory_StevenLetter, 0);
    EXPECT(!FlagGet(FLAG_DELIVERED_STEVEN_LETTER));
    EXPECT(AddBagItem(ITEM_LETTER, 1));
    EXPECT(AddBagItem(ITEM_TM_STEEL_WING, 999));
    RunGift(RemoveStory_StevenLetter, 0);
    EXPECT(FlagGet(FLAG_DELIVERED_STEVEN_LETTER));
    EXPECT(!CheckBagHasItem(ITEM_LETTER, 1));
    EXPECT(!FlagGet(FLAG_REMOVE_STORY_STEEL_WING));
    EXPECT(RemoveBagItem(ITEM_TM_STEEL_WING, 1));
    RunGift(RemoveStory_StevenLetter, FLAG_REMOVE_STORY_STEEL_WING);
    EXPECT(FlagGet(FLAG_REMOVE_STORY_STEEL_WING));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_STEEL_WING), 999);
    RunGift(RemoveStory_StevenLetter, 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_TM_STEEL_WING), 999);
    StartStoryMode(FALSE);
}

TEST("Remove Story keeps the Woods and Rustboro Great Balls independent from completed plot flags")
{
    u32 receipt;
    const u8 *script;
    PARAMETRIZE { receipt = FLAG_HIDE_PETALBURG_WOODS_DEVON_EMPLOYEE; script = RemoveStory_WoodsResearcher; }
    PARAMETRIZE { receipt = FLAG_REMOVE_STORY_RUSTBORO_GIFT; script = RemoveStory_MrStone; }
    StartStoryMode(TRUE);
    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_REMOVE_STORY_RECEIVED_LETTER);
    FlagSet(FLAG_RECEIVED_POKENAV);
    while (AddBagItem(ITEM_GREAT_BALL, 999))
        ;
    RunGift(script, 0);
    EXPECT(!FlagGet(receipt));
    EXPECT(RemoveBagItem(ITEM_GREAT_BALL, 1));
    RunGift(script, receipt);
    EXPECT(FlagGet(receipt));
    EXPECT_EQ(GetBagItemQuantity(BAG_POKE_BALLS, 0), 999);
    EXPECT(FlagGet(FLAG_RETURNED_DEVON_GOODS));
    EXPECT(!FlagGet(FLAG_DELIVERED_STEVEN_LETTER));
    StartStoryMode(FALSE);
}

extern const u8 RemoveStory_PositionWallace[];

TEST("Remove Story keeps Waterfall available after Juan and uses safe Sootopolis positions")
{
    StartStoryMode(TRUE);
    const struct MapHeader *map = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_SOOTOPOLIS_CITY), MAP_NUM(MAP_SOOTOPOLIS_CITY));
    struct MapHeader savedMap = gMapHeader;
    struct ObjectEventTemplate savedObjects[OBJECT_EVENT_TEMPLATES_COUNT];
    memcpy(savedObjects, gSaveBlock1Ptr->objectEventTemplates, sizeof(savedObjects));
    gMapHeader = *map;
    memcpy(gSaveBlock1Ptr->objectEventTemplates, map->events->objectEvents, map->events->objectEventCount * sizeof(*savedObjects));
    const struct MapLayout *layout = map->mapLayout;
    EXPECT_EQ(UNPACK_COLLISION(layout->map[33 * layout->width + 32]), 0);
    EXPECT_EQ(UNPACK_COLLISION(layout->map[18 * layout->width + 30]), 0);
    u32 door = GetAttributeByMetatileIdAndMapLayout(UNPACK_METATILE(layout->map[32 * layout->width + 31]), METATILE_ATTRIBUTE_BEHAVIOR, layout->isFrlg);
    EXPECT(MetatileBehavior_IsWarpDoor(door));
    VarSet(VAR_SOOTOPOLIS_CITY_STATE, 6);
    FlagSet(FLAG_BADGE08_GET);
    FlagSet(FLAG_HIDE_SOOTOPOLIS_CITY_WALLACE);
    RunScriptImmediately(RemoveStory_PositionWallace);
    EXPECT(!FlagGet(FLAG_HIDE_SOOTOPOLIS_CITY_WALLACE));
    RunGift(RemoveStory_Wallace, FLAG_RECEIVED_HM_WATERFALL);
    EXPECT(CheckBagHasItem(ITEM_HM_WATERFALL, 1));
    EXPECT(IsFieldMoveUnlocked(FIELD_MOVE_WATERFALL));
    EXPECT(FlagGet(FLAG_HIDE_CAVE_OF_ORIGIN_B1F_WALLACE));
    EXPECT_EQ(gSaveBlock1Ptr->objectEventTemplates[LOCALID_SOOTOPOLIS_WALLACE - 1].x, 32);
    EXPECT_EQ(gSaveBlock1Ptr->objectEventTemplates[LOCALID_SOOTOPOLIS_EXPERT - 1].x, 30);
    gMapHeader = savedMap;
    memcpy(gSaveBlock1Ptr->objectEventTemplates, savedObjects, sizeof(savedObjects));
    StartStoryMode(FALSE);
}

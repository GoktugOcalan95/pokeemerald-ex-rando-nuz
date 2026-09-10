#include "global.h"
#include "boss_rewards.h"
#include "battle_pyramid.h"
#include "constants/battle_pyramid.h"
#include "event_data.h"
#include "item.h"
#include "script.h"
#include "constants/opponents.h"
#include "constants/vars.h"

extern const u8 EventScript_BossReward[];

static const enum Item sRewards[] = {
    ITEM_ABILITY_CAPSULE,
    ITEM_ABILITY_PATCH,
    ITEM_ABILITY_CAPSULE,
    ITEM_ABILITY_PATCH,
    ITEM_ABILITY_CAPSULE,
    ITEM_ABILITY_PATCH,
    ITEM_ABILITY_CAPSULE,
    ITEM_ABILITY_PATCH,
    ITEM_ABILITY_CAPSULE,
    ITEM_ABILITY_PATCH,
    ITEM_ABILITY_CAPSULE,
    ITEM_ABILITY_PATCH,
    ITEM_ABILITY_CAPSULE,
};

static u32 GetRewardIndex(u16 trainerId)
{
    switch (trainerId)
    {
    case TRAINER_ROXANNE_1:
        return 0;
    case TRAINER_BRENDAN_RUSTBORO_TREECKO:
    case TRAINER_BRENDAN_RUSTBORO_TORCHIC:
    case TRAINER_BRENDAN_RUSTBORO_MUDKIP:
    case TRAINER_MAY_RUSTBORO_TREECKO:
    case TRAINER_MAY_RUSTBORO_TORCHIC:
    case TRAINER_MAY_RUSTBORO_MUDKIP:
        return 1;
    case TRAINER_BRAWLY_1:
        return 2;
    case TRAINER_BRENDAN_ROUTE_110_TREECKO:
    case TRAINER_BRENDAN_ROUTE_110_TORCHIC:
    case TRAINER_BRENDAN_ROUTE_110_MUDKIP:
    case TRAINER_MAY_ROUTE_110_TREECKO:
    case TRAINER_MAY_ROUTE_110_TORCHIC:
    case TRAINER_MAY_ROUTE_110_MUDKIP:
        return 3;
    case TRAINER_WATTSON_1:
        return 4;
    case TRAINER_FLANNERY_1:
        return 5;
    case TRAINER_NORMAN_1:
        return 6;
    case TRAINER_BRENDAN_ROUTE_119_TREECKO:
    case TRAINER_BRENDAN_ROUTE_119_TORCHIC:
    case TRAINER_BRENDAN_ROUTE_119_MUDKIP:
    case TRAINER_MAY_ROUTE_119_TREECKO:
    case TRAINER_MAY_ROUTE_119_TORCHIC:
    case TRAINER_MAY_ROUTE_119_MUDKIP:
        return 7;
    case TRAINER_WINONA_1:
        return 8;
    case TRAINER_BRENDAN_LILYCOVE_TREECKO:
    case TRAINER_BRENDAN_LILYCOVE_TORCHIC:
    case TRAINER_BRENDAN_LILYCOVE_MUDKIP:
    case TRAINER_MAY_LILYCOVE_TREECKO:
    case TRAINER_MAY_LILYCOVE_TORCHIC:
    case TRAINER_MAY_LILYCOVE_MUDKIP:
        return 9;
    case TRAINER_TATE_AND_LIZA_1:
        return 10;
    case TRAINER_JUAN_1:
        return 11;
    case TRAINER_WALLY_VR_1:
        return 12;
    default:
        return ARRAY_COUNT(sRewards);
    }
}

void BossRewards_RecordVictory(u16 trainerId)
{
    u32 index = GetRewardIndex(trainerId);

    if (index < ARRAY_COUNT(sRewards))
        VarSet(VAR_BOSS_REWARDS_EARNED, VarGet(VAR_BOSS_REWARDS_EARNED) | (1 << index));
}

enum Item BossRewards_TryDeliver(void)
{
    u32 earned = VarGet(VAR_BOSS_REWARDS_EARNED);
    u32 delivered = VarGet(VAR_BOSS_REWARDS_DELIVERED);

    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || FlagGet(FLAG_STORING_ITEMS_IN_PYRAMID_BAG))
        return ITEM_NONE;

    for (u32 i = 0; i < ARRAY_COUNT(sRewards); i++)
    {
        if ((earned & (1 << i)) && !(delivered & (1 << i))
         && CheckBagHasSpace(sRewards[i], 1) && AddBagItem(sRewards[i], 1))
        {
            VarSet(VAR_BOSS_REWARDS_DELIVERED, delivered | (1 << i));
            return sRewards[i];
        }
    }
    return ITEM_NONE;
}

bool32 BossRewards_TryStartScript(void)
{
    enum Item item = BossRewards_TryDeliver();

    if (item == ITEM_NONE)
        return FALSE;

    gSpecialVar_0x8000 = item;
    gSpecialVar_0x8001 = 1;
    gSpecialVar_0x8006 = item;
    gSpecialVar_0x8007 = TRUE;
    ScriptContext_SetupScript(EventScript_BossReward);
    return TRUE;
}

bool32 IsAbilityCustomizationItem(enum Item item)
{
    return item == ITEM_ABILITY_CAPSULE || item == ITEM_ABILITY_PATCH;
}

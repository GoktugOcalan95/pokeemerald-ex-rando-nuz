#include "global.h"
#include "machro_bike.h"
#include "bike.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "follower_npc.h"
#include "item.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "constants/songs.h"
#include "constants/game_stat.h"

bool32 IsBikeItem(u16 item)
{
    return item == ITEM_MACH_BIKE || item == ITEM_ACRO_BIKE || item == ITEM_BICYCLE;
}

u8 GetBikeItemMode(u16 item)
{
    if (item == ITEM_ACRO_BIKE || (item == ITEM_BICYCLE && FlagGet(FLAG_MACHRO_ACRO_MODE)))
        return PLAYER_AVATAR_FLAG_ACRO_BIKE;
    if (item == ITEM_MACH_BIKE || item == ITEM_BICYCLE)
        return PLAYER_AVATAR_FLAG_MACH_BIKE;
    return PLAYER_AVATAR_FLAG_ON_FOOT;
}

void UseBikeItem(u16 item)
{
    if (!TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_BIKE))
        VarSet(VAR_MOUNTED_BIKE, item);
    GetOnOffBike(GetBikeItemMode(item));
}

bool32 CanSwitchMachroBike(void)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 behavior = MapGridGetMetatileBehaviorAt(player->currentCoords.x, player->currentCoords.y);
    if (VarGet(VAR_MOUNTED_BIKE) != ITEM_BICYCLE || !CheckBagHasItem(ITEM_BICYCLE, 1)
     || !TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_BIKE)
     || TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_FORCED_MOVE)
     || gPlayerAvatar.preventStep || ArePlayerFieldControlsLocked()
     || gPlayerAvatar.runningState != NOT_MOVING
     || gPlayerAvatar.tileTransitionState == T_TILE_TRANSITION
     || gPlayerAvatar.acroBikeState != ACRO_STATE_NORMAL
     || (player->heldMovementActive && !player->heldMovementFinished)
     || player->singleMovementActive
     || FlagGet(FLAG_SYS_CYCLING_ROAD)
     || !Overworld_IsBikingAllowed() || IsBikingDisallowedByPlayer())
        return FALSE;
    return !MetatileBehavior_IsForcedMovementTile(behavior)
        && !MetatileBehavior_IsMuddySlope(behavior)
        && !MetatileBehavior_IsBumpySlope(behavior)
        && !MetatileBehavior_IsVerticalRail(behavior)
        && !MetatileBehavior_IsHorizontalRail(behavior)
        && !MetatileBehavior_IsIsolatedVerticalRail(behavior)
        && !MetatileBehavior_IsIsolatedHorizontalRail(behavior)
        && !MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
        && !MapGridGetCollisionAt(player->currentCoords.x, player->currentCoords.y);
}

bool32 TrySwitchMachroBike(u16 newKeys, u16 heldKeys)
{
    if (!(newKeys & B_BUTTON) || !(heldKeys & L_BUTTON) || !CanSwitchMachroBike())
        return FALSE;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_ACRO_BIKE))
        FlagClear(FLAG_MACHRO_ACRO_MODE);
    else
        FlagSet(FLAG_MACHRO_ACRO_MODE);
    ObjectEventClearHeldMovementIfFinished(&gObjectEvents[gPlayerAvatar.objectEventId]);
    SetPlayerAvatarTransitionFlags(GetBikeItemMode(ITEM_BICYCLE));
    FollowerNPC_HandleBike();
    PlaySE(SE_BIKE_BELL);
    return TRUE;
}

u32 GiveOrExchangeBike(u16 item)
{
    static const u16 bikes[] = {ITEM_MACH_BIKE, ITEM_ACRO_BIKE, ITEM_BICYCLE};
    u16 oldItem = ITEM_NONE;
    if (!IsBikeItem(item))
        return 0;
    if (CheckBagHasItem(item, 1))
        return 2;
    for (u32 i = 0; i < ARRAY_COUNT(bikes); i++)
        if (CheckBagHasItem(bikes[i], 1))
        {
            oldItem = bikes[i];
            break;
        }
    if (oldItem == ITEM_NONE && FlagGet(FLAG_RECEIVED_BIKE))
        return 3;
    if (oldItem != ITEM_NONE)
        RemoveBagItem(oldItem, 1);
    if (!AddBagItem(item, 1))
    {
        if (oldItem != ITEM_NONE)
            AddBagItem(oldItem, 1);
        return 0;
    }
    SwapRegisteredBike();
    if (VarGet(VAR_MOUNTED_BIKE) == oldItem && oldItem != ITEM_NONE)
    {
        if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_BIKE))
            GetOnOffBike(0);
        VarSet(VAR_MOUNTED_BIKE, ITEM_NONE);
    }
    if (oldItem != ITEM_NONE)
        IncrementGameStat(GAME_STAT_TRADED_BIKES);
    FlagSet(FLAG_RECEIVED_BIKE);
    return 1;
}

void GiveOrExchangeBikeFromScript(void)
{
    gSpecialVar_Result = GiveOrExchangeBike(gSpecialVar_0x8004);
}

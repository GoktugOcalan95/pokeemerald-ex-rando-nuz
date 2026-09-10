#include "global.h"
#include "mach_bike_assist.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_control_avatar.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "follower_npc.h"
#include "metatile_behavior.h"
#include "script.h"
#include "constants/event_object_movement.h"
#include "constants/layouts.h"

static bool32 ProbeBikeStep(struct ObjectEvent *probe, enum Direction direction)
{
    s16 x = probe->currentCoords.x, y = probe->currentCoords.y;
    u8 behavior, elevation;
    MoveCoords(direction, &x, &y);
    if (GetMapBorderIdAt(x, y) != CONNECTION_NONE || MapGridGetCollisionAt(x, y)
     || IsElevationMismatchAt(probe->currentElevation, x, y)
     || IsMetatileDirectionallyImpassable(probe, x, y, direction))
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    elevation = MapGridGetElevationAt(x, y);
    if (elevation == ELEVATION_MULTI_LEVEL)
        elevation = probe->currentElevation;
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
     || MetatileBehavior_IsRunningDisallowed(behavior)
     || (MetatileBehavior_IsFortreeBridge(behavior) && !(elevation & 1))
     || MetatileBehavior_IsBumpySlope(behavior)
     || MetatileBehavior_IsVerticalRail(behavior)
     || MetatileBehavior_IsHorizontalRail(behavior)
     || MetatileBehavior_IsIsolatedVerticalRail(behavior)
     || MetatileBehavior_IsIsolatedHorizontalRail(behavior)
     || MetatileBehavior_IsSidewaysStairsLeftSideAny(behavior)
     || MetatileBehavior_IsSidewaysStairsRightSideAny(behavior))
        return FALSE;
    for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *object = &gObjectEvents[i];
        if (!object->active || i == gPlayerAvatar.objectEventId
         || object->movementType == MOVEMENT_TYPE_FOLLOW_PLAYER
         || FollowerNPC_IsCollisionExempt(object, &gObjectEvents[gPlayerAvatar.objectEventId]))
            continue;
        if (((object->currentCoords.x == x && object->currentCoords.y == y)
          || (object->previousCoords.x == x && object->previousCoords.y == y))
         && AreElevationsCompatible(elevation, object->currentElevation))
            return FALSE;
    }
    probe->currentCoords.x = x;
    probe->currentCoords.y = y;
    probe->currentMetatileBehavior = behavior;
    probe->currentElevation = elevation;
    return TRUE;
}

enum Direction GetMachBikeAssistanceDirection(enum Direction forward, u16 heldKeys)
{
    static const enum Direction rightDirections[] = {
        [DIR_SOUTH] = DIR_WEST, [DIR_NORTH] = DIR_EAST,
        [DIR_WEST] = DIR_NORTH, [DIR_EAST] = DIR_SOUTH,
    };
    struct ObjectEvent probes[2], ahead;
    enum Direction sides[2];
    bool8 reachable[2] = {TRUE, TRUE};
    if (!(heldKeys & B_BUTTON) || !TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE)
     || (VarGet(VAR_MOUNTED_BIKE) == ITEM_BICYCLE && (heldKeys & L_BUTTON))
     || forward < DIR_SOUTH || forward > DIR_EAST
     || gMapHeader.mapLayoutId == LAYOUT_FORTREE_CITY_GYM
     || gPlayerAvatar.preventStep || ArePlayerFieldControlsLocked()
     || TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_FORCED_MOVE))
        return DIR_NONE;
    probes[0] = probes[1] = gObjectEvents[gPlayerAvatar.objectEventId];
    probes[0].currentMetatileBehavior = probes[1].currentMetatileBehavior = MapGridGetMetatileBehaviorAt(probes[0].currentCoords.x, probes[0].currentCoords.y);
    ahead = probes[0];
    if (ProbeBikeStep(&ahead, forward) || CanUseAutomaticHMInDirection(forward))
        return DIR_NONE;
    sides[0] = rightDirections[forward];
    sides[1] = GetOppositeDirection(sides[0]);
    for (u32 distance = 1; distance <= 10; distance++)
    {
        for (u32 side = 0; side < 2; side++)
        {
            if (!reachable[side])
                continue;
            reachable[side] = ProbeBikeStep(&probes[side], sides[side]);
            if (!reachable[side])
                continue;
            ahead = probes[side];
            if (ProbeBikeStep(&ahead, forward))
                return sides[side];
        }
    }
    return DIR_NONE;
}

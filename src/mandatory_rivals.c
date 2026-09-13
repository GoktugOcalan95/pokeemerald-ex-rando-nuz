#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "mandatory_rivals.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "region_map.h"
#include "script.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/map_event_ids.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"
#include "constants/vars.h"

extern const u8 MandatoryRivals_EventScript_Blocked[];
extern const u8 LilycoveCity_EventScript_InterceptRival[];

bool32 HasWonMandatoryRival(u32 rival)
{
    switch (rival)
    {
    case 1: return FlagGet(FLAG_DEFEATED_RIVAL_ROUTE103);
    case 2: return FlagGet(FLAG_DEFEATED_RIVAL_RUSTBORO) || FlagGet(FLAG_DEFEATED_RIVAL_ROUTE_104);
    case 3: return VarGet(VAR_ROUTE110_STATE) != 0;
    case 4: return VarGet(VAR_ROUTE119_STATE) != 0;
    case 5: return FlagGet(FLAG_MET_RIVAL_LILYCOVE);
    default: return TRUE;
    }
}

u32 GetMandatoryRivalBoundary(u16 map, s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    u32 rival = 0;
    switch (map)
    {
    case MAP_OLDALE_TOWN:
        if (fromX >= 0 && toX < 0)
            rival = 1;
        break;
    case MAP_ROUTE104:
    {
        const struct MapLayout *layout = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(map), MAP_NUM(map))->mapLayout;
        if (fromY < layout->height && toY >= layout->height)
            rival = 2;
        break;
    }
    case MAP_ROUTE110:
        if (fromY >= 0 && toY < 0)
            rival = 3;
        break;
    case MAP_ROUTE119:
    case MAP_ROUTE118:
    {
        const struct MapLayout *layout = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(map), MAP_NUM(map))->mapLayout;
        if (fromX < layout->width && toX >= layout->width)
            rival = 4;
        break;
    }
    case MAP_LILYCOVE_CITY:
        if (fromX <= 30 && toX > 30)
            rival = 5;
        break;
    default: break;
    }
    return HasWonMandatoryRival(rival) ? 0 : rival;
}

bool32 TryStartMandatoryRivalBoundaryScript(u32 direction)
{
    static const u16 locations[] = {MAPSEC_NONE, MAPSEC_ROUTE_103, MAPSEC_ROUTE_104, MAPSEC_ROUTE_110, MAPSEC_ROUTE_119, MAPSEC_LILYCOVE_CITY};
    s16 x, y, nextX, nextY;
    PlayerGetDestCoords(&x, &y);
    nextX = x;
    nextY = y;
    MoveCoords(direction, &nextX, &nextY);
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 rival = GetMandatoryRivalBoundary(map, x - MAP_OFFSET, y - MAP_OFFSET, nextX - MAP_OFFSET, nextY - MAP_OFFSET);
    if (!rival)
        return FALSE;
    GetMapName(gStringVar1, locations[rival], 0);
    if (rival == 5 && !TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING)
        && MapGridGetCollisionAt(x + 1, y) == 0 && MapGridGetCollisionAt(x + 2, y) == 0
        && !MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x + 1, y))
        && !MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x + 2, y)))
    {
        bool32 clear = TRUE;
        for (u32 i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gObjectEvents[i].active && !gObjectEvents[i].isPlayer && gObjectEvents[i].localId != LOCALID_LILYCOVE_RIVAL
                && gObjectEvents[i].currentCoords.y == y
                && (gObjectEvents[i].currentCoords.x == x + 1 || gObjectEvents[i].currentCoords.x == x + 2))
                clear = FALSE;
        if (clear)
        {
            ScriptContext_SetupScript(LilycoveCity_EventScript_InterceptRival);
            return TRUE;
        }
    }
    ScriptContext_SetupScript(MandatoryRivals_EventScript_Blocked);
    return TRUE;
}

bool8 MandatoryRivals_SelectLilycoveRival(struct ScriptContext *ctx)
{
    u8 id;
    gSpecialVar_Result = FALSE;
    if (!TryGetObjectEventIdByLocalIdAndMap(LOCALID_LILYCOVE_RIVAL, MAP_NUM(MAP_LILYCOVE_CITY), MAP_GROUP(MAP_LILYCOVE_CITY), &id))
    {
        gSelectedObjectEvent = id;
        gSpecialVar_LastTalked = LOCALID_LILYCOVE_RIVAL;
        gObjectEvents[id].currentElevation = gObjectEvents[gPlayerAvatar.objectEventId].currentElevation;
        gObjectEvents[id].previousElevation = gObjectEvents[id].currentElevation;
        gSpecialVar_Result = TRUE;
    }
    return FALSE;
}

bool8 MandatoryRivals_GetLilycoveSprite(struct ScriptContext *ctx)
{
    u8 id;
    gSpecialVar_Result = 0;
    if (!TryGetObjectEventIdByLocalIdAndMap(LOCALID_LILYCOVE_RIVAL, MAP_NUM(MAP_LILYCOVE_CITY), MAP_GROUP(MAP_LILYCOVE_CITY), &id))
        gSpecialVar_Result = gObjectEvents[id].spriteId;
    return FALSE;
}

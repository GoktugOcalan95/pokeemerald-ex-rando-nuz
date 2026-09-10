#include "global.h"
#include "route_fly.h"
#include "field_move.h"
#include "overworld.h"

#define DEST(route, x, y, label, water, minX, minY, maxX, maxY) \
    {MAPSEC_ROUTE_##route, MAP_GROUP(MAP_ROUTE##route), MAP_NUM(MAP_ROUTE##route), x, y, water, minX, minY, maxX, maxY, COMPOUND_STRING(label)}

static const struct RouteFlyDestination sRouteFlyDestinations[] =
{
    DEST(101, 5, 10, "Route 101", FALSE, 0, 0, 19, 19),
    DEST(102, 40, 10, "Route 102", FALSE, 0, 0, 49, 19),
    DEST(103, 11, 10, "Route 103 - West", FALSE, 2, 2, 23, 21),
    DEST(103, 68, 10, "Route 103 - East", FALSE, 44, 5, 79, 15),
    DEST(104, 5, 19, "Route 104 - North", FALSE, 0, 0, 39, 29),
    DEST(104, 18, 51, "Route 104 - South", FALSE, 0, 43, 39, 79),
    DEST(105, 15, 58, "Route 105", FALSE, 15, 53, 17, 61),
    DEST(106, 59, 14, "Route 106", FALSE, 49, 11, 69, 17),
    DEST(107, 3, 11, "Route 107", FALSE, 0, 10, 6, 14),
    DEST(108, 27, 6, "Route 108", TRUE, 17, 0, 37, 16),
    DEST(109, 12, 6, "Route 109", FALSE, 0, 0, 39, 62),
    DEST(110, 15, 17, "Route 110 - North", FALSE, 0, 0, 39, 40),
    DEST(110, 11, 67, "Route 110 - South", FALSE, 0, 50, 39, 99),
    DEST(111, 26, 19, "Route 111 - North", FALSE, 0, 0, 39, 50),
    DEST(111, 31, 114, "Route 111 - South", FALSE, 0, 100, 39, 139),
    DEST(112, 22, 38, "Route 112", FALSE, 18, 31, 30, 41),
    DEST(113, 33, 6, "Route 113", FALSE, 0, 0, 99, 19),
    DEST(114, 27, 37, "Route 114", FALSE, 14, 29, 34, 74),
    DEST(115, 16, 65, "Route 115", FALSE, 6, 60, 31, 79),
    DEST(116, 40, 10, "Route 116", FALSE, 0, 7, 60, 19),
    DEST(117, 51, 6, "Route 117", FALSE, 0, 0, 59, 19),
    DEST(118, 13, 7, "Route 118", FALSE, 0, 5, 17, 15),
    DEST(119, 6, 33, "Route 119 - North", FALSE, 2, 31, 30, 60),
    DEST(119, 33, 110, "Route 119 - South", FALSE, 0, 90, 39, 139),
    DEST(120, 27, 4, "Route 120 - North", FALSE, 20, 1, 39, 35),
    DEST(120, 38, 89, "Route 120 - South", FALSE, 0, 70, 39, 99),
    DEST(121, 10, 9, "Route 121 - West", FALSE, 0, 2, 24, 18),
    DEST(121, 37, 6, "Route 121 - East", FALSE, 31, 2, 79, 18),
    DEST(122, 22, 30, "Route 122", FALSE, 20, 29, 25, 31),
    DEST(123, 22, 7, "Route 123 - West", FALSE, 0, 1, 41, 19),
    DEST(123, 122, 13, "Route 123 - East", FALSE, 79, 0, 139, 19),
    DEST(124, 70, 49, "Route 124", FALSE, 67, 44, 75, 52),
    DEST(125, 22, 20, "Route 125", FALSE, 5, 16, 24, 27),
    DEST(126, 44, 69, "Route 126", TRUE, 0, 0, 79, 79),
    DEST(127, 45, 25, "Route 127", FALSE, 42, 13, 64, 26),
    DEST(128, 41, 22, "Route 128", FALSE, 32, 13, 43, 23),
    DEST(129, 42, 20, "Route 129", TRUE, 0, 0, 79, 39),
    DEST(130, 45, 27, "Route 130", TRUE, 0, 0, 79, 39),
    DEST(131, 35, 27, "Route 131", TRUE, 0, 0, 59, 39),
    DEST(132, 27, 25, "Route 132", FALSE, 19, 21, 35, 29),
    DEST(133, 41, 16, "Route 133", FALSE, 26, 15, 46, 17),
    DEST(134, 77, 29, "Route 134", FALSE, 69, 25, 79, 33),
};
#undef DEST

STATIC_ASSERT(ARRAY_COUNT(sRouteFlyDestinations) <= sizeof(((struct SaveBlock1 *)0)->visitedRouteFly) * 8, RouteFlyVisitCapacity);

u32 GetRouteFlyDestinationCount(void)
{
    return ARRAY_COUNT(sRouteFlyDestinations);
}

const struct RouteFlyDestination *GetRouteFlyDestination(u32 id)
{
    return id < ARRAY_COUNT(sRouteFlyDestinations) ? &sRouteFlyDestinations[id] : NULL;
}

void RecordRouteFlyVisit(u32 mapGroup, u32 mapNum, s32 x, s32 y, bool32 onWater)
{
    for (u32 i = 0; i < ARRAY_COUNT(sRouteFlyDestinations); i++)
    {
        const struct RouteFlyDestination *dest = &sRouteFlyDestinations[i];
        if (dest->mapGroup == mapGroup && dest->mapNum == mapNum
         && x >= dest->minX && x <= dest->maxX && y >= dest->minY && y <= dest->maxY
         && (dest->requiresSurf || !onWater))
            gSaveBlock1Ptr->visitedRouteFly[i / 8] |= 1 << (i % 8);
    }
}

bool32 IsRouteFlyVisited(u32 id)
{
    return id < ARRAY_COUNT(sRouteFlyDestinations) && (gSaveBlock1Ptr->visitedRouteFly[id / 8] & (1 << (id % 8))) != 0;
}

bool32 CanFlyToRoute(u32 id)
{
    return IsRouteFlyVisited(id) && (!sRouteFlyDestinations[id].requiresSurf || IsFieldMoveUnlocked(FIELD_MOVE_SURF));
}

u32 GetVisitedRouteFlyChoices(u32 mapsec, u8 *choices)
{
    u32 count = 0;
    for (u32 i = 0; i < ARRAY_COUNT(sRouteFlyDestinations); i++)
        if (sRouteFlyDestinations[i].mapsec == mapsec && IsRouteFlyVisited(i))
        {
            if (choices != NULL && count < ROUTE_FLY_MAX_CHOICES)
                choices[count] = i;
            count++;
        }
    return count;
}

bool32 TrySetRouteFlyDestination(u32 id)
{
    if (!CanFlyToRoute(id))
        return FALSE;
    const struct RouteFlyDestination *dest = &sRouteFlyDestinations[id];
    SetWarpDestination(dest->mapGroup, dest->mapNum, WARP_ID_NONE, dest->x, dest->y);
    return TRUE;
}

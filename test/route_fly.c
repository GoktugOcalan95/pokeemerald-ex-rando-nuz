#include "global.h"
#include "route_fly.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "save.h"
#include "text.h"
#include "string_util.h"
#include "test/test.h"

u8 Test_GetFlyArrivalTransition(u16 behavior);

static void ClearRouteVisits(void)
{
    memset(gSaveBlock1Ptr->visitedRouteFly, 0, sizeof(gSaveBlock1Ptr->visitedRouteFly));
}

TEST("Route Fly destinations have safe target terrain and no overlapping warps or objects")
{
    struct MapHeader savedMap = gMapHeader;
    for (u32 i = 0; i < GetRouteFlyDestinationCount(); i++)
    {
        const struct RouteFlyDestination *dest = GetRouteFlyDestination(i);
        gMapHeader = *Overworld_GetMapHeaderByGroupAndId(dest->mapGroup, dest->mapNum);
        const struct MapLayout *layout = gMapHeader.mapLayout;
        EXPECT(dest->x < layout->width);
        EXPECT(dest->y < layout->height);
        u16 block = layout->map[dest->y * layout->width + dest->x];
        EXPECT_EQ(UNPACK_COLLISION(block), 0);
        u32 behavior = GetAttributeByMetatileIdAndMapLayout(UNPACK_METATILE(block), METATILE_ATTRIBUTE_BEHAVIOR, layout->isFrlg);
        EXPECT_EQ(MetatileBehavior_IsSurfableWaterOrUnderwater(behavior), dest->requiresSurf);
        EXPECT_EQ(Test_GetFlyArrivalTransition(behavior), dest->requiresSurf ? PLAYER_AVATAR_FLAG_SURFING : PLAYER_AVATAR_FLAG_ON_FOOT);
        EXPECT(!MetatileBehavior_IsJumpNorth(behavior));
        EXPECT(!MetatileBehavior_IsJumpSouth(behavior));
        EXPECT(!MetatileBehavior_IsJumpEast(behavior));
        EXPECT(!MetatileBehavior_IsJumpWest(behavior));
        for (u32 j = 0; j < gMapHeader.events->warpCount; j++)
            EXPECT(gMapHeader.events->warps[j].x != dest->x || gMapHeader.events->warps[j].y != dest->y);
        for (u32 j = 0; j < gMapHeader.events->objectEventCount; j++)
            EXPECT(gMapHeader.events->objectEvents[j].x != dest->x || gMapHeader.events->objectEvents[j].y != dest->y);
    }
    gMapHeader = savedMap;
}

TEST("Route Fly unlocks surrounding sections independently without requiring the landing tile")
{
    for (u32 i = 0; i < GetRouteFlyDestinationCount(); i++)
    {
        const struct RouteFlyDestination *dest = GetRouteFlyDestination(i);
        ClearRouteVisits();
        RecordRouteFlyVisit(dest->mapGroup, dest->mapNum, dest->x, dest->y + 1, dest->requiresSurf);
        EXPECT(IsRouteFlyVisited(i));
        for (u32 j = 0; j < GetRouteFlyDestinationCount(); j++)
            if (j != i)
                EXPECT(!IsRouteFlyVisited(j));
        u8 choices[ROUTE_FLY_MAX_CHOICES];
        EXPECT_EQ(GetVisitedRouteFlyChoices(dest->mapsec, choices), 1);
        EXPECT_EQ(choices[0], i);
        ClearRouteVisits();
        RecordRouteFlyVisit(dest->mapGroup, dest->mapNum, -1, -1, FALSE);
        EXPECT(!IsRouteFlyVisited(i));
        if (!dest->requiresSurf)
        {
            RecordRouteFlyVisit(dest->mapGroup, dest->mapNum, dest->x, dest->y, TRUE);
            EXPECT(!IsRouteFlyVisited(i));
        }
    }
}

TEST("Route Fly retains water visits while Surf is locked and lands at exact selected coordinates")
{
    ClearBag();
    ClearRouteVisits();
    FlagClear(FLAG_RUN_RULE_EARLY_SURF);
    FlagClear(FLAG_BADGE05_GET);
    for (u32 i = 0; i < GetRouteFlyDestinationCount(); i++)
    {
        const struct RouteFlyDestination *dest = GetRouteFlyDestination(i);
        EXPECT(!TrySetRouteFlyDestination(i));
        RecordRouteFlyVisit(dest->mapGroup, dest->mapNum, dest->x, dest->y, dest->requiresSurf);
        EXPECT(IsRouteFlyVisited(i));
        EXPECT_EQ(CanFlyToRoute(i), !dest->requiresSurf);
    }
    EXPECT(AddBagItem(ITEM_HM_SURF, 1));
    FlagSet(FLAG_BADGE05_GET);
    for (u32 i = 0; i < GetRouteFlyDestinationCount(); i++)
    {
        const struct RouteFlyDestination *dest = GetRouteFlyDestination(i);
        EXPECT(TrySetRouteFlyDestination(i));
        WarpIntoMap();
        EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, dest->mapGroup);
        EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, dest->mapNum);
        EXPECT_EQ(gSaveBlock1Ptr->pos.x, dest->x);
        EXPECT_EQ(gSaveBlock1Ptr->pos.y, dest->y);
    }
    EXPECT(!TrySetRouteFlyDestination(ROUTE_FLY_NONE));
    FlagClear(FLAG_BADGE05_GET);
    ClearBag();
    ClearRouteVisits();
}

TEST("Route Fly preserves independent destination visits through saving and loading")
{
    ClearRouteVisits();
    for (u32 i = 0; i < GetRouteFlyDestinationCount(); i += 2)
    {
        const struct RouteFlyDestination *dest = GetRouteFlyDestination(i);
        RecordRouteFlyVisit(dest->mapGroup, dest->mapNum, dest->x, dest->y, dest->requiresSurf);
    }
    Save_ResetSaveCounters();
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_OK);
    ClearRouteVisits();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    for (u32 i = 0; i < GetRouteFlyDestinationCount(); i++)
        EXPECT_EQ(IsRouteFlyVisited(i), i % 2 == 0);
    ClearRouteVisits();
}

TEST("Route Fly destination lists fit the Fly window and retain all section choices")
{
    u8 text[64];
    memset(gSaveBlock1Ptr->visitedRouteFly, 0xFF, sizeof(gSaveBlock1Ptr->visitedRouteFly));
    for (u32 mapsec = MAPSEC_ROUTE_101; mapsec <= MAPSEC_ROUTE_134; mapsec++)
    {
        u8 choices[ROUTE_FLY_MAX_CHOICES];
        u32 count = GetVisitedRouteFlyChoices(mapsec, choices);
        EXPECT(count > 0 && count <= ROUTE_FLY_MAX_CHOICES);
        for (u32 i = 0; i < count; i++)
        {
            const struct RouteFlyDestination *dest = GetRouteFlyDestination(choices[i]);
            StringCopy(text, dest->name);
            if (dest->requiresSurf)
                StringAppend(text, COMPOUND_STRING(" (Surf needed)"));
            EXPECT(GetStringWidth(FONT_NORMAL, text, 0) <= 184);
        }
    }
    ClearRouteVisits();
}

TEST("Route Fly does not unlock landings from disconnected banks and ledges")
{
    const struct {u16 map; u16 x; u16 y;} blocked[] = {
        {MAP_ROUTE105, 14, 50}, {MAP_ROUTE106, 57, 18}, {MAP_ROUTE112, 22, 28},
        {MAP_ROUTE114, 11, 26}, {MAP_ROUTE115, 20, 20}, {MAP_ROUTE116, 75, 10},
        {MAP_ROUTE118, 35, 7}, {MAP_ROUTE119, 35, 60}, {MAP_ROUTE120, 19, 23},
        {MAP_ROUTE121, 29, 7},
    };
    for (u32 i = 0; i < ARRAY_COUNT(blocked); i++)
    {
        ClearRouteVisits();
        RecordRouteFlyVisit(MAP_GROUP(blocked[i].map), MAP_NUM(blocked[i].map), blocked[i].x, blocked[i].y, FALSE);
        for (u32 j = 0; j < GetRouteFlyDestinationCount(); j++)
            EXPECT(!IsRouteFlyVisited(j));
    }
}

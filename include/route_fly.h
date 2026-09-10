#ifndef GUARD_ROUTE_FLY_H
#define GUARD_ROUTE_FLY_H

#define ROUTE_FLY_NONE 0xFF
#define ROUTE_FLY_MAX_CHOICES 4

struct RouteFlyDestination
{
    mapsec_u16_t mapsec;
    u8 mapGroup;
    u8 mapNum;
    u8 x;
    u8 y;
    bool8 requiresSurf;
    u16 minX;
    u16 minY;
    u16 maxX;
    u16 maxY;
    const u8 *name;
};

u32 GetRouteFlyDestinationCount(void);
const struct RouteFlyDestination *GetRouteFlyDestination(u32 id);
void RecordRouteFlyVisit(u32 mapGroup, u32 mapNum, s32 x, s32 y, bool32 onWater);
bool32 IsRouteFlyVisited(u32 id);
bool32 CanFlyToRoute(u32 id);
u32 GetVisitedRouteFlyChoices(u32 mapsec, u8 *choices);
bool32 TrySetRouteFlyDestination(u32 id);

#endif

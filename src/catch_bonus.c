#include "global.h"
#include "catch_bonus.h"
#include "run_setup.h"

u64 CalculateCatchBonusRescueThreshold(u32 shakeThreshold, u32 criticalThreshold, u32 tier)
{
    static const u8 multipliers[] = {2, 3, 6, 10};
    const u64 one = 1ULL << 32;
    u64 shake = (u64)shakeThreshold << 16;
    u64 ordinary = shake;
    u64 probability, boosted;

    if (tier == CATCH_BONUS_NONE || tier >= CATCH_BONUS_INSTANT || shake == 0)
        return 0;
    if (shake >= one)
        return one;
    for (u32 i = 1; i < 4; i++)
        ordinary = (ordinary * shake) >> 32;
    probability = (shake * criticalThreshold + ordinary * (256 - criticalThreshold)) >> 8;
    boosted = probability * multipliers[tier] / 2;
    if (boosted >= one)
        return one;
    // Rescue only failed throws, scaling the combined critical/ordinary probability.
    return ((boosted - probability) << 32) / (one - probability);
}

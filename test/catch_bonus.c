#include "global.h"
#include "catch_bonus.h"
#include "test/test.h"

TEST("Catch bonus matches exact combined probability vectors")
{
    static const struct { u32 shake, critical, tier; u64 expected; } cases[] =
    {
        {0, 0, 1, 0ULL},
        {0, 0, 2, 0ULL},
        {0, 0, 3, 0ULL},
        {100, 0, 1, 0ULL},
        {100, 0, 2, 0ULL},
        {100, 0, 3, 0ULL},
        {16384, 0, 1, 8421504ULL},
        {16384, 0, 2, 33686018ULL},
        {16384, 0, 3, 67372036ULL},
        {32768, 0, 1, 143165576ULL},
        {32768, 0, 2, 572662306ULL},
        {32768, 0, 3, 1145324612ULL},
        {32768, 42, 1, 333084040ULL},
        {32768, 42, 2, 1332336160ULL},
        {32768, 42, 3, 2664672321ULL},
        {50000, 0, 1, 1100438995ULL},
        {50000, 0, 2, 4294967296ULL},
        {50000, 0, 3, 4294967296ULL},
        {50000, 42, 1, 1482453074ULL},
        {50000, 42, 2, 4294967296ULL},
        {50000, 42, 3, 4294967296ULL},
        {65535, 42, 1, 4294967296ULL},
        {65535, 42, 2, 4294967296ULL},
        {65535, 42, 3, 4294967296ULL},
        {65536, 0, 1, 4294967296ULL},
        {65536, 0, 2, 4294967296ULL},
        {65536, 0, 3, 4294967296ULL},
    };
    for (u32 i = 0; i < ARRAY_COUNT(cases); i++)
    {
        u64 actual = CalculateCatchBonusRescueThreshold(cases[i].shake, cases[i].critical, cases[i].tier);
        EXPECT_LE(actual, cases[i].expected + 32);
        EXPECT_LE(cases[i].expected, actual + 32);
    }
}

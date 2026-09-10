#include "global.h"
#include "run_randomizer.h"

static u32 Mix(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352D;
    value ^= value >> 15;
    value *= 0x846CA68B;
    return value ^ (value >> 16);
}

u32 RunRandomizerHash(u32 domain, u32 source, u32 slot)
{
    const u8 *id = gSaveBlock2Ptr->playerTrainerId;
    u32 seed = id[0] | (id[1] << 8) | (id[2] << 16) | ((u32)id[3] << 24);
    return Mix(seed ^ Mix(domain) ^ Mix(source + 0x9E3779B9) ^ Mix(slot + 0x85EBCA6B));
}

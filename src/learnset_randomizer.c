#include "global.h"
#include "event_data.h"
#include "learnset_randomizer.h"
#include "move.h"
#include "move_randomizer.h"
#include "pokemon.h"
#include "run_randomizer.h"
#include "constants/flags.h"

#define LEARNSET_DOMAIN 0x400
#define LEARNSET_CAPACITY 64
#define LEARNSET_CACHE_SIZE 4

static EWRAM_DATA struct {
    struct LevelUpMove moves[LEARNSET_CAPACITY];
    u32 seed;
    u16 species;
    u8 chance;
    bool8 ready;
} sCache[LEARNSET_CACHE_SIZE] = {0};
static EWRAM_DATA u8 sNextCache = 0;

static void GenerateLearnset(struct LevelUpMove *moves, const struct LevelUpMove *original, u16 species)
{
    bool8 used[MOVES_COUNT] = {0};
    u32 count = 0;
    for (u32 level = 0; level <= MAX_LEVEL; level++)
    {
        u32 entries = level == 1 ? MAX_MON_MOVES : 0;
        if (level != 1)
            for (u32 i = 0; original[i].move != LEVEL_UP_MOVE_END; i++)
                entries += original[i].level == level;
        if (level >= 10 && level <= 60 && level % 5 == 0)
        {
            bool32 occupied = FALSE;
            for (u32 i = 0; original[i].move != LEVEL_UP_MOVE_END; i++)
                occupied |= original[i].level > level - 5 && original[i].level <= level;
            if (!occupied)
                entries++;
        }
        for (u32 i = 0; i < entries; i++)
        {
            assertf(count < LEARNSET_CAPACITY - 1, "Randomized learnset too long: %d", species);
            if (level == 1 && i == 0)
                for (u32 move = 1; move < MOVES_COUNT; move++)
                    used[move] |= !IsRandomizerDirectAttack(move);
            u32 move = ChooseRandomizerMove(LEARNSET_DOMAIN, species, count, used);
            if (level == 1 && i == 0)
            {
                memset(used, 0, sizeof(used));
                for (u32 j = 0; j < count; j++)
                    used[moves[j].move] = TRUE;
            }
            moves[count++] = (struct LevelUpMove){move, level};
            used[move] = TRUE;
        }
    }
    u32 first = 0;
    while (moves[first].level == 0)
        first++;
    // Evolution offers have no learning level; sort only ordinary damaging entries.
    for (u32 i = first; i < count; i++)
    {
        if (gMovesInfo[moves[i].move].category == DAMAGE_CATEGORY_STATUS)
            continue;
        for (u32 j = i + 1; j < count; j++)
        {
            if (gMovesInfo[moves[j].move].category != DAMAGE_CATEGORY_STATUS
                && GetRandomizerMovePower(moves[j].move) < GetRandomizerMovePower(moves[i].move))
            {
                u16 move = moves[i].move;
                moves[i].move = moves[j].move;
                moves[j].move = move;
            }
        }
    }
    bool32 hasAttack = FALSE;
    for (u32 i = first; i < first + MAX_MON_MOVES; i++)
        hasAttack |= IsRandomizerDirectAttack(moves[i].move);
    if (!hasAttack)
    {
        u32 slot = first;
        while (slot < first + MAX_MON_MOVES && gMovesInfo[moves[slot].move].category == DAMAGE_CATEGORY_STATUS)
            slot++;
        if (slot == first + MAX_MON_MOVES)
            slot = first;
        u32 ceiling = 0xFFFF;
        for (u32 i = slot; i < count; i++)
            if (gMovesInfo[moves[i].move].category != DAMAGE_CATEGORY_STATUS)
            {
                ceiling = GetRandomizerMovePower(moves[i].move);
                break;
            }
        u32 candidates = 0;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (!used[move] && IsRandomizerDirectAttack(move) && GetRandomizerMovePower(move) <= ceiling)
                candidates++;
        assertf(candidates != 0, "No starting attack for species %d", species);
        u32 choice = RunRandomizerHash(LEARNSET_DOMAIN, species, 0xFFFF) % candidates;
        for (u32 move = 1; move < MOVES_COUNT; move++)
            if (!used[move] && IsRandomizerDirectAttack(move) && GetRandomizerMovePower(move) <= ceiling && choice-- == 0)
            {
                moves[slot].move = move;
                break;
            }
    }
    moves[count] = (struct LevelUpMove){LEVEL_UP_MOVE_END, 0};
}

const struct LevelUpMove *GetRandomizedLearnset(u16 species, const struct LevelUpMove *original)
{
    if (!FlagGet(FLAG_RUN_RULE_LEARNSETS) || species == SPECIES_NONE || species == SPECIES_EGG)
        return original;
    u32 seed = RunRandomizerHash(LEARNSET_DOMAIN, 0, 0);
    u32 chance = GetRandomizerGoodMoveChance();
    for (u32 i = 0; i < LEARNSET_CACHE_SIZE; i++)
        if (sCache[i].ready && sCache[i].seed == seed && sCache[i].species == species && sCache[i].chance == chance)
            return sCache[i].moves;
    u32 index = sNextCache++ % LEARNSET_CACHE_SIZE;
    GenerateLearnset(sCache[index].moves, original, species);
    sCache[index].seed = seed;
    sCache[index].species = species;
    sCache[index].chance = chance;
    sCache[index].ready = TRUE;
    return sCache[index].moves;
}

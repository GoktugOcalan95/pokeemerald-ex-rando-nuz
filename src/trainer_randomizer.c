#include "global.h"
#include "battle.h"
#include "data.h"
#include "event_data.h"
#include "pokemon.h"
#include "run_randomizer.h"
#include "species_randomizer.h"
#include "trainer_randomizer.h"
#include "trainer_util.h"
#include "constants/flags.h"

const struct Trainer *GetRunTrainer(u16 trainerId)
{
#if TESTING
    extern const struct Trainer gRunTrainerTestData[DIFFICULTY_COUNT][TRAINERS_COUNT];
    return &gRunTrainerTestData[DIFFICULTY_NORMAL][trainerId];
#else
    return &gTrainers[DIFFICULTY_NORMAL][trainerId];
#endif
}

#define TRAINER_SPECIES_DOMAIN 0x500
#define RIVAL_EVOLUTION_DOMAIN 0x501

bool32 IsRunTrainerBattle(u16 trainerId)
{
    return trainerId > TRAINER_NONE && trainerId < TRAINERS_COUNT && !gIsDebugBattle
        && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_CATCH_TUTORIAL
            | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SECRET_BASE
            | BATTLE_TYPE_RECORDED));
}

u16 GetRunTrainerSpecies(u16 trainerId, u32 slot, u16 original)
{
    if (!FlagGet(FLAG_RUN_RULE_TRAINERS))
        return original;
    const struct Trainer *trainer = GetRunTrainer(trainerId);
    if (slot < trainer->partySize && trainer->trainerClass == TRAINER_CLASS_RIVAL
        && (trainer->trainerPic == TRAINER_PIC_BRENDAN || trainer->trainerPic == TRAINER_PIC_MAY))
    {
        u32 starter = 0;
        u32 stage = 0;
        switch (original)
        {
        case SPECIES_TREECKO: starter = 0; stage = 0; break;
        case SPECIES_GROVYLE: starter = 0; stage = 1; break;
        case SPECIES_SCEPTILE: starter = 0; stage = 2; break;
        case SPECIES_TORCHIC: starter = 1; stage = 0; break;
        case SPECIES_COMBUSKEN: starter = 1; stage = 1; break;
        case SPECIES_BLAZIKEN: starter = 1; stage = 2; break;
        case SPECIES_MUDKIP: starter = 2; stage = 0; break;
        case SPECIES_MARSHTOMP: starter = 2; stage = 1; break;
        case SPECIES_SWAMPERT: starter = 2; stage = 2; break;
        default: starter = 3; break;
        }
        if (starter < 3)
        {
            const u16 bases[] = {SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP};
            u16 species = RandomizeStarterSpecies(bases[starter], starter);
            for (u32 step = 0; step < stage; step++)
            {
                const struct Evolution *evolutions = GetSpeciesEvolutions(species);
                u32 count = 0;
                if (evolutions == NULL)
                    break;
                for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
                    count += IsRandomizerSpeciesEligible(evolutions[i].targetSpecies);
                if (!count)
                    break;
                u32 choice = RunRandomizerHash(RIVAL_EVOLUTION_DOMAIN, starter, step) % count;
                for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
                    if (IsRandomizerSpeciesEligible(evolutions[i].targetSpecies) && choice-- == 0)
                    {
                        species = evolutions[i].targetSpecies;
                        break;
                    }
            }
            return species;
        }
    }
    return PickRandomizerSpecies(FALSE, TRAINER_SPECIES_DOMAIN, trainerId, slot, SPECIES_NONE, SPECIES_NONE);
}

bool32 CreateRunTrainerParty(struct Pokemon *party, u16 trainerId)
{
    if (!IsRunTrainerBattle(trainerId) || !FlagGet(FLAG_RUN_RULE_TRAINERS))
        return FALSE;
    const struct Trainer *trainer = GetRunTrainer(trainerId);
    const struct Trainer *source = trainer->overrideTrainer ? GetRunTrainer(trainer->overrideTrainer) : trainer;
    u32 count = trainer->partySize ? trainer->partySize : source->partySize;
    count = min(count, PARTY_SIZE);
    if ((gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) && (B_MULTI_HALF_TEAMS || trainer->multiTeamSize == MULTI_TEAM_SIZE_HALF))
        count = min(count, PARTY_SIZE / 2);
    struct TrainerGenerator generator = {0};
    MakeTrainerGenerator(&generator, trainer);
    generator.randomizerSource = trainerId;
    generator.smartTera = FALSE;
    ZeroPartyMons(party);
    for (u32 i = 0; i < count; i++)
    {
        struct TrainerMon entry = source->party[i];
        entry.species = GetRunTrainerSpecies(trainerId, i, entry.species);
        entry.ability = ABILITY_NONE;
        entry.gender = TRAINER_MON_RANDOM_GENDER;
        entry.nickname = NULL;
        entry.teraType = TYPE_MYSTERY;
        entry.shouldUseDynamax = FALSE;
        entry.gigantamaxFactor = FALSE;
        memset(entry.moves, 0, sizeof(entry.moves));
        GenerateMonFromTrainerMon(&party[i], &entry, &generator);
    }
    return TRUE;
}

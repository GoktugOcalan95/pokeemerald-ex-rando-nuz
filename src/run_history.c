#include "global.h"
#include "run_history.h"
#include "battle.h"
#include "main.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "teaching_randomizer.h"

extern const u16 gTutorMoves[];

static EWRAM_DATA u32 sCaughtPersonality = 0;
static EWRAM_DATA bool8 sCaughtWildMon = FALSE;

static bool32 IsRecordedWildBattle(void)
{
    return !(gBattleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER
        | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_CATCH_TUTORIAL
        | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SECRET_BASE | BATTLE_TYPE_POKEDUDE));
}

void RunHistory_Init(void)
{
    struct RunHistory *history = &gPokemonStoragePtr->runHistory;
    memset(history, 0, sizeof(*history));
    history->magic = RUN_HISTORY_MAGIC;
    history->version = RUN_HISTORY_VERSION;
    history->runId = Random32();
    memset(&gSaveBlock1Ptr->runDiscovery, 0, sizeof(gSaveBlock1Ptr->runDiscovery));
    sCaughtWildMon = FALSE;
}

void RunHistory_RecordArea(void)
{
    u32 area = GetCurrentRegionMapSectionId();
    if (area < sizeof(gSaveBlock1Ptr->runDiscovery.visitedAreas) * 8)
        gSaveBlock1Ptr->runDiscovery.visitedAreas[area / 8] |= 1 << (area % 8);
}

static void RecordMon(struct Pokemon *mon, u32 outcome)
{
    struct RunHistory *history = &gPokemonStoragePtr->runHistory;
    u32 species = GetMonData(mon, MON_DATA_SPECIES);
    if (history->magic != RUN_HISTORY_MAGIC || history->version != RUN_HISTORY_VERSION
        || species == SPECIES_NONE)
        return;
    RunHistory_RecordArea();
    if (history->count >= RUN_HISTORY_CAPACITY)
    {
        if (history->dropped != UINT32_MAX)
            history->dropped++;
        return;
    }
    struct RunHistoryRecord *record = &history->records[history->count++];
    record->personality = GetMonData(mon, MON_DATA_PERSONALITY);
    record->trainerId = GetMonData(mon, MON_DATA_OT_ID);
    record->species = species;
    record->area = GetCurrentRegionMapSectionId();
    record->outcome = outcome;
}

void RunHistory_RecordAcquisition(struct Pokemon *mon)
{
    if (gMain.inBattle && IsRecordedWildBattle())
    {
        sCaughtPersonality = GetMonData(mon, MON_DATA_PERSONALITY);
        sCaughtWildMon = TRUE;
    }
    else
    {
        RecordMon(mon, GetMonData(mon, MON_DATA_IS_EGG) ? RUN_HISTORY_EGG : RUN_HISTORY_GIFT);
    }
}

void RunHistory_RecordSplitEvolution(struct Pokemon *mon)
{
    RecordMon(mon, RUN_HISTORY_SPLIT_EVOLUTION);
}

void RunHistory_RecordWildBattle(void)
{
    if (IsRecordedWildBattle())
    {
        // Resolve uncaught opponents before the capture in a double battle.
        for (u32 i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE
                || (sCaughtWildMon && GetMonData(mon, MON_DATA_PERSONALITY) == sCaughtPersonality))
                continue;
            u32 outcome = RUN_HISTORY_FLED;
            if (GetMonData(mon, MON_DATA_HP) == 0)
                outcome = RUN_HISTORY_KNOCKED_OUT;
            else if (gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW)
                outcome = RUN_HISTORY_LOST;
            RecordMon(mon, outcome);
        }
        if (sCaughtWildMon)
            for (u32 i = 0; i < PARTY_SIZE; i++)
            {
                struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_A][i];
                if (GetMonData(mon, MON_DATA_PERSONALITY) == sCaughtPersonality)
                {
                    RecordMon(mon, RUN_HISTORY_CAUGHT);
                    break;
                }
            }
    }
    sCaughtWildMon = FALSE;
}

void RunHistory_RecordHatch(struct Pokemon *mon)
{
    struct RunHistory *history = &gPokemonStoragePtr->runHistory;
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);
    u32 trainerId = GetMonData(mon, MON_DATA_OT_ID);
    for (u32 i = 0; i < min(history->count, RUN_HISTORY_CAPACITY); i++)
        if (history->records[i].personality == personality && history->records[i].trainerId == trainerId
            && history->records[i].outcome == RUN_HISTORY_EGG)
            history->records[i].outcome = RUN_HISTORY_HATCHED;
}

void RunHistory_RecordTutor(u16 move)
{
    for (u32 i = 0; i < RUN_HISTORY_TUTOR_COUNT && gTutorMoves[i] != MOVE_UNAVAILABLE; i++)
        if (gTutorMoves[i] == move)
        {
            gSaveBlock1Ptr->runDiscovery.tutorsSeen |= 1u << i;
            gSaveBlock1Ptr->runDiscovery.tutorAreas[i] = GetCurrentRegionMapSectionId();
            break;
        }
}

bool8 ScriptRecordTutorOffer(struct ScriptContext *ctx)
{
    RunHistory_RecordTutor(gSpecialVar_0x8005);
    return FALSE;
}

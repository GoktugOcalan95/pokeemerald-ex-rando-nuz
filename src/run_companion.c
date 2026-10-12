#include "global.h"
#include "run_companion.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "teaching_randomizer.h"
#include "constants/region_map_sections.h"

extern const u16 gTutorMoves[];

void RunCompanion_Init(void)
{
    struct CompanionSaveMetadata *metadata = &gPokemonStoragePtr->companion;
    memset(metadata, 0, sizeof(*metadata));
    metadata->magic = COMPANION_SAVE_MAGIC;
    metadata->layoutHash = COMPANION_SAVE_LAYOUT_HASH;
    metadata->runId = Random32();
    memset(&gSaveBlock1Ptr->runDiscovery, 0, sizeof(gSaveBlock1Ptr->runDiscovery));
}

static void RecordTutorInArea(u16 move, u32 mapsec)
{
    for (u32 i = 0; i < RUN_DISCOVERY_TUTOR_COUNT && gTutorMoves[i] != MOVE_UNAVAILABLE; i++)
        if (gTutorMoves[i] == move)
        {
            gSaveBlock1Ptr->runDiscovery.tutorsSeen |= 1u << i;
            gSaveBlock1Ptr->runDiscovery.tutorAreas[i] = mapsec;
            break;
        }
}

void RunCompanion_RecordTutor(u16 move)
{
    RecordTutorInArea(move, GetCurrentRegionMapSectionId());
}

void RunCompanion_RecordTutorsInArea(u32 mapsec)
{
    static const struct { u16 move; u8 mapsec; } sOverworldTutors[] =
    {
        {MOVE_SWAGGER, MAPSEC_SLATEPORT_CITY},
        {MOVE_ROLLOUT, MAPSEC_MAUVILLE_CITY},
        {MOVE_FURY_CUTTER, MAPSEC_VERDANTURF_TOWN},
        {MOVE_MIMIC, MAPSEC_LAVARIDGE_TOWN},
        {MOVE_METRONOME, MAPSEC_FALLARBOR_TOWN},
        {MOVE_SLEEP_TALK, MAPSEC_FORTREE_CITY},
        {MOVE_SUBSTITUTE, MAPSEC_LILYCOVE_CITY},
        {MOVE_DYNAMIC_PUNCH, MAPSEC_MOSSDEEP_CITY},
        {MOVE_DOUBLE_EDGE, MAPSEC_SOOTOPOLIS_CITY},
        {MOVE_EXPLOSION, MAPSEC_PACIFIDLOG_TOWN},
    };
    for (u32 i = 0; i < ARRAY_COUNT(sOverworldTutors); i++)
        if (sOverworldTutors[i].mapsec == mapsec)
            RecordTutorInArea(sOverworldTutors[i].move, mapsec);
    if (mapsec == MAPSEC_BATTLE_FRONTIER)
        for (u32 tutor = 0; tutor < 2; tutor++)
            for (u32 i = 0; i < 10; i++)
                RecordTutorInArea(GetOriginalFrontierTutorMove(tutor, i), mapsec);
}

bool8 ScriptRecordTutorOffer(struct ScriptContext *ctx)
{
    RunCompanion_RecordTutor(gSpecialVar_0x8005);
    return FALSE;
}

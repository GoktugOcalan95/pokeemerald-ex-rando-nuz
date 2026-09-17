#include "global.h"
#include "run_companion.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"

extern const u16 gTutorMoves[];

void RunCompanion_Init(void)
{
    struct CompanionSaveMetadata *metadata = &gPokemonStoragePtr->companion;
    memset(metadata, 0, sizeof(*metadata));
    metadata->magic = COMPANION_SAVE_MAGIC;
    metadata->version = COMPANION_SAVE_VERSION;
    metadata->runId = Random32();
    memset(&gSaveBlock1Ptr->runDiscovery, 0, sizeof(gSaveBlock1Ptr->runDiscovery));
}

void RunCompanion_RecordTutor(u16 move)
{
    for (u32 i = 0; i < RUN_DISCOVERY_TUTOR_COUNT && gTutorMoves[i] != MOVE_UNAVAILABLE; i++)
        if (gTutorMoves[i] == move)
        {
            gSaveBlock1Ptr->runDiscovery.tutorsSeen |= 1u << i;
            gSaveBlock1Ptr->runDiscovery.tutorAreas[i] = GetCurrentRegionMapSectionId();
            break;
        }
}

bool8 ScriptRecordTutorOffer(struct ScriptContext *ctx)
{
    RunCompanion_RecordTutor(gSpecialVar_0x8005);
    return FALSE;
}

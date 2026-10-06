#ifndef GUARD_RUN_COMPANION_H
#define GUARD_RUN_COMPANION_H

#define COMPANION_SAVE_MAGIC 0x434F4D50
#define RUN_DISCOVERY_TUTOR_COUNT 32

// FNV-1a over the save struct sizes, so the companion rejects saves from builds with a different layout.
#define COMPANION_LAYOUT_STEP(hash, size) ((((u32)(hash)) ^ (u32)(size)) * 16777619u)
#define COMPANION_SAVE_LAYOUT_HASH                                                     \
    COMPANION_LAYOUT_STEP(COMPANION_LAYOUT_STEP(COMPANION_LAYOUT_STEP(                  \
    COMPANION_LAYOUT_STEP(COMPANION_LAYOUT_STEP(COMPANION_LAYOUT_STEP(                  \
    COMPANION_LAYOUT_STEP(2166136261u, sizeof(struct SaveBlock1)),                      \
        sizeof(struct SaveBlock2)), sizeof(struct SaveBlock3)),                         \
        sizeof(struct PokemonStorage)), sizeof(struct BoxPokemon)),                     \
        BAG_SAVE1_BYTES), BAG_SAVE3_BYTES)

struct CompanionSaveMetadata
{
    u32 magic;
    u32 layoutHash;
    u32 runId;
};

struct RunDiscovery
{
    u8 tutorAreas[RUN_DISCOVERY_TUTOR_COUNT];
    u32 tutorsSeen;
};

struct ScriptContext;

void RunCompanion_Init(void);
void RunCompanion_RecordTutor(u16 move);
bool8 ScriptRecordTutorOffer(struct ScriptContext *ctx);

#endif

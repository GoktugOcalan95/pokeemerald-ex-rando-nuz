#ifndef GUARD_RUN_COMPANION_H
#define GUARD_RUN_COMPANION_H

#define COMPANION_SAVE_MAGIC 0x434F4D50
#define COMPANION_SAVE_VERSION 1
#define RUN_DISCOVERY_TUTOR_COUNT 32

struct CompanionSaveMetadata
{
    u32 magic;
    u16 version;
    u16 reserved;
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

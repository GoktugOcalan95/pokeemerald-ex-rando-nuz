#ifndef GUARD_RUN_HISTORY_H
#define GUARD_RUN_HISTORY_H

#define RUN_HISTORY_MAGIC 0x4852554E
#define RUN_HISTORY_VERSION 1
#define RUN_HISTORY_CAPACITY 128
#define RUN_HISTORY_TUTOR_COUNT 32

enum RunHistoryOutcome
{
    RUN_HISTORY_CAUGHT = 1,
    RUN_HISTORY_KNOCKED_OUT,
    RUN_HISTORY_FLED,
    RUN_HISTORY_LOST,
    RUN_HISTORY_EGG,
    RUN_HISTORY_GIFT,
    RUN_HISTORY_HATCHED,
    RUN_HISTORY_SPLIT_EVOLUTION,
};

struct RunHistoryRecord
{
    u32 personality;
    u32 trainerId;
    u16 species;
    u8 area;
    u8 outcome;
};

struct RunHistory
{
    u32 magic;
    u16 version;
    u16 count;
    u32 runId;
    u32 dropped;
    struct RunHistoryRecord records[RUN_HISTORY_CAPACITY];
};

struct RunDiscovery
{
    u8 visitedAreas[32];
    u8 tutorAreas[RUN_HISTORY_TUTOR_COUNT];
    u32 tutorsSeen;
};

struct Pokemon;
struct ScriptContext;

void RunHistory_Init(void);
void RunHistory_RecordAcquisition(struct Pokemon *mon);
void RunHistory_RecordSplitEvolution(struct Pokemon *mon);
void RunHistory_RecordWildBattle(void);
void RunHistory_RecordArea(void);
void RunHistory_RecordHatch(struct Pokemon *mon);
void RunHistory_RecordTutor(u16 move);
bool8 ScriptRecordTutorOffer(struct ScriptContext *ctx);

#endif

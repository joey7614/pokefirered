#ifndef GUARD_OPENCLAW_AI_H
#define GUARD_OPENCLAW_AI_H

#include "global.h"

#define OPENCLAW_TEXT_SIZE 128

struct OpenClawIPC {
    u8 request_ready;               // ROM sets 1 when NPC dialog triggers
    u8 response_ready;              // Python sets 1 when AI response is ready
    u8 pad[2];
    u8 npc_orig_text[OPENCLAW_TEXT_SIZE]; // original NPC text (Gen3 encoded)
    u8 ai_text[OPENCLAW_TEXT_SIZE];       // AI response (Gen3 encoded)
};

extern struct OpenClawIPC gOpenClawIPC;

#endif // GUARD_OPENCLAW_AI_H

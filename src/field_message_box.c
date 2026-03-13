#include "global.h"
#include "field_message_box.h"
#include "gflib.h"
#include "new_menu_helpers.h"
#include "quest_log.h"
#include "script.h"
#include "text_window.h"
#include "string_util.h"
#include "openclaw_ai.h"

static EWRAM_DATA u8 sMessageBoxType = 0;

static void ExpandStringAndStartDrawFieldMessageBox(const u8 *str);
static void StartDrawFieldMessageBox(void);

void InitFieldMessageBox(void)
{
    sMessageBoxType = FIELD_MESSAGE_BOX_HIDDEN;
    gTextFlags.canABSpeedUpPrint = FALSE;
    gTextFlags.useAlternateDownArrow = FALSE;
    gTextFlags.autoScroll = FALSE;
}

static void Task_DrawFieldMessageBox(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    switch (task->data[0])
    {
    case 0:
        if (gQuestLogState == QL_STATE_PLAYBACK)
        {
            gTextFlags.autoScroll = TRUE;
            LoadQuestLogWindowTiles(0, 0x200);
        }
        else if (!IsMsgSignpost())
            LoadStdWindowFrameGfx();
        else
            LoadSignpostWindowFrameGfx();
        task->data[0]++;
        break;
    case 1:
        DrawDialogueFrame(0, TRUE);
        task->data[0]++;
        break;
    case 2:
        if (RunTextPrinters_CheckPrinter0Active() != TRUE)
        {
            sMessageBoxType = FIELD_MESSAGE_BOX_HIDDEN;
            DestroyTask(taskId);
        }
        break;
    }
}

static void CreateTask_DrawFieldMessageBox(void)
{
    CreateTask(Task_DrawFieldMessageBox, 80);
}

static void DestroyTask_DrawFieldMessageBox(void)
{
    u8 taskId = FindTaskIdByFunc(Task_DrawFieldMessageBox);
    if (taskId != 0xFF)
        DestroyTask(taskId);
}

#define OPENCLAW_TIMEOUT_FRAMES 600  // 10 seconds at 60 fps

static void Task_OpenClawWaitForAI(u8 taskId)
{
    s16 *frames = &gTasks[taskId].data[0];
    (*frames)++;

    // Player stays locked (sMessageBoxType = FIELD_MESSAGE_BOX_NORMAL from ShowFieldMessage).
    // No intermediate dialog — just wait silently until AI responds or timeout.

    if (gOpenClawIPC.response_ready)
    {
        StringCopyN(gStringVar4, gOpenClawIPC.ai_text, OPENCLAW_TEXT_SIZE);
        gOpenClawIPC.response_ready = 0;
        StartDrawFieldMessageBox();
        DestroyTask(taskId);
        return;
    }

    if (*frames >= OPENCLAW_TIMEOUT_FRAMES)
    {
        StringCopyN(gStringVar4, gOpenClawIPC.npc_orig_text, OPENCLAW_TEXT_SIZE);
        gOpenClawIPC.request_ready = 0;
        StartDrawFieldMessageBox();
        DestroyTask(taskId);
    }
}

bool8 ShowFieldMessage(const u8 *str)
{
    if (sMessageBoxType != FIELD_MESSAGE_BOX_HIDDEN)
        return FALSE;
    StringCopyN(gOpenClawIPC.npc_orig_text, str, OPENCLAW_TEXT_SIZE - 1);
    gOpenClawIPC.npc_orig_text[OPENCLAW_TEXT_SIZE - 1] = 0xFF;
    gOpenClawIPC.request_ready = 1;
    sMessageBoxType = FIELD_MESSAGE_BOX_NORMAL;
    CreateTask(Task_OpenClawWaitForAI, 80);
    return TRUE;
}

bool8 ShowFieldAutoScrollMessage(const u8 *str)
{
    if (sMessageBoxType != FIELD_MESSAGE_BOX_HIDDEN)
        return FALSE;
    sMessageBoxType = FIELD_MESSAGE_BOX_AUTO_SCROLL;
    ExpandStringAndStartDrawFieldMessageBox(str);
    return TRUE;
}

// Unused
static bool8 ForceShowFieldAutoScrollMessage(const u8 *str)
{
    sMessageBoxType = FIELD_MESSAGE_BOX_AUTO_SCROLL;
    ExpandStringAndStartDrawFieldMessageBox(str);
    return TRUE;
}

// Unused
// Same as ShowFieldMessage, but instead of accepting a string argument,
// it just prints whatever that's already in gStringVar4
static bool8 ShowFieldMessageFromBuffer(void)
{
    if (sMessageBoxType != FIELD_MESSAGE_BOX_HIDDEN)
        return FALSE;
    sMessageBoxType = FIELD_MESSAGE_BOX_NORMAL;
    StartDrawFieldMessageBox();
    return TRUE;
}

static void ExpandStringAndStartDrawFieldMessageBox(const u8 *str)
{
    StringExpandPlaceholders(gStringVar4, str);
    AddTextPrinterDiffStyle(TRUE);
    CreateTask_DrawFieldMessageBox();
}

static void StartDrawFieldMessageBox(void)
{
    AddTextPrinterDiffStyle(TRUE);
    CreateTask_DrawFieldMessageBox();
}

void HideFieldMessageBox(void)
{
    DestroyTask_DrawFieldMessageBox();
    ClearDialogWindowAndFrame(0, TRUE);
    sMessageBoxType = FIELD_MESSAGE_BOX_HIDDEN;
}

u8 GetFieldMessageBoxType(void)
{
    return sMessageBoxType;
}

bool8 IsFieldMessageBoxHidden(void)
{
    if (sMessageBoxType == FIELD_MESSAGE_BOX_HIDDEN)
        return TRUE;
    else
        return FALSE;
}

// Unused
static void ReplaceFieldMessageWithFrame(void)
{
    DestroyTask_DrawFieldMessageBox();
    DrawStdWindowFrame(0, TRUE);
    sMessageBoxType = FIELD_MESSAGE_BOX_HIDDEN;
}

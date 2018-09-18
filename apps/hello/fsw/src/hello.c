#include "hello.h"
#include "hello_events.h"
#include "hello_msg.h"
#include "hello_version.h"
#include "hello_perfids.h"
#include "hello_msgids.h"
#include "sample_app_msgids.h"
#include "/home/talos/cFE/apps/sample_app/fsw/src/sample_app_msg.h"

hello_hk_tlm_t     HELLO_HkTelemetryPkt;

static CFE_EVS_BinFilter_t HELLO_EventFilters[] = 
{
    {HELLO_STARTUP_INF_EID,    0x0000},
    {HELLO_COMMAND_ERR_EID,    0x0000},
    {HELLO_COMMANDNOP_INF_EID, 0x0000},
    {HELLO_COMMANDRST_INF_EID, 0x0000},
};

void helloMain(void)
{
    int32 status;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(HELLO_PERF_ID);

    helloInit();

    while(CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(HELLO_PERF_ID);
        status = CFE_SB_RcvMsg(&HelloAppData.HELLOMsgPtr, HelloAppData.HELLO_CommandPipe, 500);

        if(status == CFE_SUCCESS)
        {
            hello_ProcessCommandPacket();
        }
    }

    CFE_ES_ExitApp(RunStatus);
    
}

void helloInit(void)
{
    memset(&HelloAppData, 0, sizeof(HelloAppData_t));
    CFE_ES_RegisterApp(); /* must register with ES when started */

    CFE_EVS_Register(HELLO_EventFilters,
    sizeof(HELLO_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
    CFE_EVS_BINARY_FILTER);

    CFE_SB_CreatePipe(&HelloAppData.HELLO_CommandPipe, HELLO_PIPE_DEPTH, "hello_cmd_pipe");
    CFE_SB_Subscribe(HELLO_CMD_MID, HelloAppData.HELLO_CommandPipe);
    CFE_SB_Subscribe(HELLO_SEND_HK_MID, HelloAppData.HELLO_CommandPipe);
    CFE_SB_Subscribe(SAMLPE_TO_HELLO_MID, HelloAppData.HELLO_CommandPipe);

    hello_ResetCounters();

    CFE_SB_InitMsg(&HELLO_HkTelemetryPkt,
    HELLO_HK_TLM_MID,
    HELLO_APP_HK_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent(HELLO_STARTUP_INF_EID, CFE_EVS_INFORMATION, 
    "hello initialized. version %d. %d. %d. %d",
    HELLO_MAJOR_VERSION,
    HELLO_MINOR_VERSION,
    HELLO_REVISION,
    HELLO_MISSION_REV);
}

void hello_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId;
    MsgId = CFE_SB_GetMsgId(HelloAppData.HELLOMsgPtr);

    switch(MsgId)
    {
        case HELLO_CMD_MID:
        hello_ProcessGroundCommand();
        break;

        case HELLO_SEND_HK_MID:
        hello_ReportHousekeeping();
        break;

        case SAMLPE_TO_HELLO_MID:
        hello_ProcessSample();
        break;

        default:
        HELLO_HkTelemetryPkt.hello_command_count++;
        CFE_EVS_SendEvent(HELLO_COMMAND_ERR_EID, CFE_EVS_ERROR,
        "hello: invalid command packet, MID = 0x%x", MsgId);
        break;
    }

    return;
}

void hello_ProcessGroundCommand(void)
{
    uint16 CommandCode;
    CommandCode = CFE_SB_GetCmdCode(HelloAppData.HELLOMsgPtr);

    switch(CommandCode)
    {
        case HELLO_NOOP_CC:
        HELLO_HkTelemetryPkt.hello_command_count++;
        CFE_EVS_SendEvent(HELLO_COMMANDNOP_INF_EID, CFE_EVS_INFORMATION,
        "hello, NOOP command");
        break;

        case HELLO_RESET_COUNTERS_CC:
        hello_ResetCounters();
        break;

        default:
        break;
    }

    return;
}

void hello_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *)&HELLO_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *)&HELLO_HkTelemetryPkt);

    return;
}

void hello_ResetCounters(void)
{
    HELLO_HkTelemetryPkt.hello_command_count = 0;
    HELLO_HkTelemetryPkt.hello_command_error_count = 0;

    CFE_EVS_SendEvent(HELLO_COMMANDRST_INF_EID, CFE_EVS_INFORMATION, "hello: RESET command");

    return;
}

void hello_ProcessSample(void)
{
    static Test_t *msg;
    static int count = 0;
    count++;
    msg = (Test_t *)(HelloAppData.HELLOMsgPtr);
    printf("use data is %d, count is %d \n",(*msg).data, count);
}

boolean hello_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    if(ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID = CFE_SB_GetMsgId(msg);
        uint16 CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(HELLO_LEN_ERR_EID, CFE_EVS_ERROR,
        "Invalid msg length: ID = 0x%x, CC = %d, Len = %d, Expected = %d", 
        MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        HELLO_HkTelemetryPkt.hello_command_error_count++;
    }

    return(result);
}

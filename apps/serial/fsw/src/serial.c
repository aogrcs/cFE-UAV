/*******************************************************************************
** File: serial.c
**
** Purpose:
**   This file contains the source code for the Serial.
**
*******************************************************************************/

/*
**   Include Files:
*/

#include "serial.h"
#include "serial_perfids.h"
#include "serial_msgids.h"
#include "serial_msg.h"
#include "serial_events.h"
#include "serial_version.h"
#include "/home/talos/cFE/apps/hello/fsw/src/hello.h"

/*
** global data
*/

serial_hk_tlm_t    SERIAL_HkTelemetryPkt;

static CFE_EVS_BinFilter_t  SERIAL_EventFilters[] =
       {  /* Event ID    mask */
          {SERIAL_STARTUP_INF_EID,       0x0000},
          {SERIAL_COMMAND_ERR_EID,       0x0000},
          {SERIAL_COMMANDNOP_INF_EID,    0x0000},
          {SERIAL_COMMANDRST_INF_EID,    0x0000},
       };

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* SerialMain() -- Application entry point and main process loop          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void SerialMain( void )
{
    int32  status;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(SERIAL_PERF_ID);

    SerialInit();

    /*
    ** Serial Runloop
    */
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(SERIAL_PERF_ID);

        CFE_SB_SendMsg((CFE_SB_Msg_t *)(&testData));

        /* Pend on receipt of command packet -- timeout set to 500 millisecs */
        status = CFE_SB_RcvMsg(&SerialData.SerialMsgPtr, SerialData.Serial_CommandPipe, 500);
        
        CFE_ES_PerfLogEntry(SERIAL_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            SERIAL_ProcessCommandPacket();
        }

    }

    CFE_ES_ExitApp(RunStatus);

} /* End of SerialMain() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* SerialInit() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void SerialInit(void)
{
    /*
    ** Register the app with Executive services
    */
    CFE_ES_RegisterApp() ;

    /*
    ** Register the events
    */ 
    CFE_EVS_Register(SERIAL_EventFilters,
                     sizeof(SERIAL_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);

    /*
    ** Create the Software Bus command pipe and subscribe to housekeeping
    **  messages
    */
    CFE_SB_CreatePipe(&SerialData.Serial_CommandPipe, SERIAL_PIPE_DEPTH,"SERIAL_CMD_PIPE");
    CFE_SB_Subscribe(SERIAL_CMD_MID, SerialData.Serial_CommandPipe);
    CFE_SB_Subscribe(SERIAL_SEND_HK_MID, SerialData.Serial_CommandPipe);

    SERIAL_ResetCounters();

    CFE_SB_InitMsg(&SERIAL_HkTelemetryPkt,
                   SERIAL_HK_TLM_MID,
                   SERIAL_HK_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent (SERIAL_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "SERIAL Initialized. Version %d.%d.%d.%d",
                SERIAL_MAJOR_VERSION,
                SERIAL_MINOR_VERSION, 
                SERIAL_REVISION, 
                SERIAL_MISSION_REV);
				
} /* End of SerialInit() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SERIAL_ProcessCommandPacket                                        */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SERIAL    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SERIAL_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t  MsgId;

    MsgId = CFE_SB_GetMsgId(SerialData.Serial_CommandPipe);

    switch (MsgId)
    {
        case SERIAL_CMD_MID:
            SERIAL_ProcessGroundCommand();
            break;

        case SERIAL_SEND_HK_MID:
            SERIAL_ReportHousekeeping();
            break;

        default:
            SERIAL_HkTelemetryPkt.serial_command_error_count++;
            CFE_EVS_SendEvent(SERIAL_COMMAND_ERR_EID,CFE_EVS_ERROR,
			"SERIAL: invalid command packet,MID = 0x%x", MsgId);
            break;
    }

    return;

} /* End SERIAL_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SERIAL_ProcessGroundCommand() -- SERIAL ground commands                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void SERIAL_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(SerialData.SerialMsgPtr);

    /* Process "known" Serial ground commands */
    switch (CommandCode)
    {
        case SERIAL_NOOP_CC:
            SERIAL_HkTelemetryPkt.serial_command_count++;
            CFE_EVS_SendEvent(SERIAL_COMMANDNOP_INF_EID,CFE_EVS_INFORMATION,
			"SERIAL: NOOP command");
            break;

        case SERIAL_RESET_COUNTERS_CC:
            SERIAL_ResetCounters();
            break;

        /* default case already found during FC vs length test */
        default:
            break;
    }
    return;

} /* End of SERIAL_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SERIAL_ReportHousekeeping                                              */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SERIAL_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &SERIAL_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &SERIAL_HkTelemetryPkt);
    return;

} /* End of SERIAL_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  SERIAL_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void SERIAL_ResetCounters(void)
{
    /* Status of commands processed by the Serial */
    SERIAL_HkTelemetryPkt.serial_command_count       = 0;
    SERIAL_HkTelemetryPkt.serial_command_error_count = 0;

    CFE_EVS_SendEvent(SERIAL_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
		"SERIAL: RESET command");
    return;

} /* End of SERIAL_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SERIAL_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
boolean SERIAL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{     
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID   = CFE_SB_GetMsgId(msg);
        uint16         CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(SERIAL_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        SERIAL_HkTelemetryPkt.serial_command_error_count++;
    }

    return(result);

} /* End of SERIAL_VerifyCmdLength() */


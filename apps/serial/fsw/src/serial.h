/*******************************************************************************
** File: serial.h
**
** Purpose:
**   This file is main hdr file for the Serial application.
**
**
*******************************************************************************/

#ifndef _serial_app_h_
#define _serial_app_h_

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"
#include "test_msg.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

/***********************************************************************/

#define SERIAL_PIPE_DEPTH                     32

typedef struct{
    CFE_SB_PipeId_t             Serial_CommandPipe;
    CFE_SB_MsgPtr_t             SerialMsgPtr;
}SerialData_t;

/************************************************************************
** Type Definitions
*************************************************************************/

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (SAMPLE_AppMain), these
**       functions are not called from any other source module.
*/
void SerialMain(void);
void SerialInit(void);
void SERIAL_ProcessCommandPacket(void);
void SERIAL_ProcessGroundCommand(void);
void SERIAL_ReportHousekeeping(void);
void SERIAL_ResetCounters(void);

SerialData_t    SerialData;
Test_t          testData;   

boolean SERIAL_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif /* _serial_app_h_ */

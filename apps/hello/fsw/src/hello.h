#ifndef _hello_h_
#define _hello_h_

#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"
#include "test_msg.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#define HELLO_PIPE_DEPTH 32

typedef struct{
    CFE_SB_PipeId_t    HELLO_CommandPipe;
    CFE_SB_MsgPtr_t    HELLOMsgPtr;
}HelloAppData_t;

void helloMain(void);
void helloInit(void);
void hello_ProcessCommandPacket(void);
void hello_ProcessGroundCommand(void);
void hello_ReportHousekeeping(void);
void hello_ResetCounters(void);
void hello_ProcessSample(void);

HelloAppData_t HelloAppData;

boolean hello_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif

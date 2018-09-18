#ifndef _hello_msg_h_
#define _hello_msg_h_

#define HELLO_NOOP_CC            0
#define HELLO_RESET_COUNTERS_CC  1
#define HELLO_SAMPLE_CC          2

typedef struct
{
    uint8 CmdHeader[CFE_SB_CMD_HDR_SIZE];
} hello_NoArgsCmd_t;

typedef struct
{
    uint8 TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8 hello_command_error_count;
    uint8 hello_command_count;
    uint8 spare[2];
} OS_PACK hello_hk_tlm_t;

#define HELLO_APP_HK_TLM_LNGTH sizeof(hello_hk_tlm_t)

#endif
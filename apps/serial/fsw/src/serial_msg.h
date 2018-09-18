/*******************************************************************************
** File:
**   serial_msg.h 
**
** Purpose: 
**  Define SERIAL  Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _serial_msg_h_
#define _serial_msg_h_

/*
** Serial command codes
*/
#define SERIAL_NOOP_CC                 0
#define SERIAL_RESET_COUNTERS_CC       1

/*************************************************************************/
/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
   uint8    CmdHeader[CFE_SB_CMD_HDR_SIZE];

} SERIAL_NoArgsCmd_t;

/*************************************************************************/
/*
** Type definition (SAMPLE App housekeeping)
*/
typedef struct 
{
    uint8              TlmHeader[CFE_SB_TLM_HDR_SIZE];
    uint8              serial_command_error_count;
    uint8              serial_command_count;
    uint8              spare[2];

}   OS_PACK serial_hk_tlm_t  ;

#define SERIAL_HK_TLM_LNGTH   sizeof ( serial_hk_tlm_t )

#endif /* _serial_msg_h_ */

/************************/
/*  End of File Comment */
/************************/

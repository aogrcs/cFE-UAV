#ifndef TEST_MSG_H
#define TEST_MSG_H

#include "cfe.h"

typedef struct{
    uint8   TlmHeader[CFE_SB_TLM_HDR_SIZE];  /**< cFS header information */
    uint8  data;
}Test_t; 

#endif
/*******************************************************************************
*                Copyright 2007, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
*******************************************************************************/
///////////////////////////////////////////////////////////////////////////////
//! \file tsp_dtcm_parser.h
//!
//! \Implementation of a console for tsp_dtcm_parser Tool
//!
//! Author:   Weizhao Jiang
//! Version:  1.0
//! Date:     Sep. 2011
//!
///////////////////////////////////////////////////////////////////////////////


#ifndef __TSP_DTCM_PARSER_H__ 
#define __TSP_DTCM_PARSER_H__ 

#include "tsp_fw_spec.h"

typedef	unsigned int        BOOL;
#define TRUE                (1)
#define FALSE               (0)


#define     TspDtcmGlobal_LA_Mask            0x00000001
#define     TspDtcmGlobal_LA_Shift           0

#define     TspDtcmGlobal_SCHED_Mask         0x00000002
#define     TspDtcmGlobal_SCHED_Shift        1

#define     TspDtcmGlobal_PID_Mask           0x00000004
#define     TspDtcmGlobal_PID_Shift          2

#define 	TspDtcmGlobal_PES_Mask 		     0x00000008
#define 	TspDtcmGlobal_PES_Shift          3

#define     TspDtcmGlobal_SEF_Mask 		     0x00000010
#define     TspDtcmGlobal_SEF_Shift          4

#define 	TspDtcmGlobal_TS_Mask 		     0x00000020
#define 	TspDtcmGlobal_TS_Shift           5

#define 	TspDtcmGlobal_HBOIN_Mask 	     0x00000040
#define 	TspDtcmGlobal_HBOIN_Shift        6

#define     TspDtcmGlobal_INDEX_Mask         0x00000080
#define     TspDtcmGlobal_INDEX_Shift        7

#define     TspDtcmGlobal_HBOOUT_Mask        0x00000100
#define     TspDtcmGlobal_HBOOUT_Shift       8

#define     TspDtcmGlobal_DSOUT_Mask         0x00000200
#define     TspDtcmGlobal_DSOUT_Shift        9

#define     TspDtcmGlobal_DSIN_Mask          0x00000400
#define     TspDtcmGlobal_DSIN_Shift         10

#define     TspDtcmGlobal_INPOOL_Mask        0x00000800
#define     TspDtcmGlobal_INPOOL_Shift       11

#define     TspDtcmGlobal_DTCMSTRM_Mask      0x00001000
#define     TspDtcmGlobal_DTCMSTRM_Shift     12

#define     TspDtcmGlobal_OUTPOOL_Mask       0x00002000
#define     TspDtcmGlobal_OUTPOOL_Shift      13

#define     TspDtcmGlobal_OUTMODULE_Mask     0x00004000
#define     TspDtcmGlobal_OUTMODULE_Shift    14

#define     TspDtcmGlobal_STACK_Mask         0x00008000
#define     TspDtcmGlobal_STACK_Shift        15

#define     TspDtcmGlobal_DMX_Mask           0x00010000
#define     TspDtcmGlobal_DMX_Shift          16

#define     TspDtcmGlobal_BYPASS_Mask        0x00020000
#define     TspDtcmGlobal_BYPASS_Shift       17

#define     TspDtcmGlobal_MCARDMUX_Mask      0x00040000
#define     TspDtcmGlobal_MCARDMUX_Shift     18

#define     TspDtcmGlobal_PARTSEC_Mask       0x00080000
#define     TspDtcmGlobal_PARTSEC_Shift      19

#define     TspDtcmGlobal_CONST_Mask         0x00100000
#define     TspDtcmGlobal_CONST_Shift        20

#define     TspDtcmGlobal_PARTPES_Mask       0x00200000
#define     TspDtcmGlobal_PARTPES_Shift      21

#define     TspDtcmGlobal_AACS_Mask          0x00400000
#define     TspDtcmGlobal_AACS_Shift         22

#define     TspDtcmGlobal_WM_Mask            0x00800000
#define     TspDtcmGlobal_WM_Shift           23

typedef enum _dtcm_show_mode_
{
    DETECTED_MODE = 0x0,
    ONE_SHOT_MODE = 0x1,
} DTCM_SHOW_MODE;

typedef int (*CMDFUNC)(int argc, char *argv[]);
typedef int (*ItemShowFunc)(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);


typedef struct tagCmdHandler
{
    char *pCmd;
    CMDFUNC Handler;
    char *pHelp;
} CMD_HANDLER;

typedef struct tagItemCtx
{
    unsigned int index;
    unsigned char ItemName[128];
    ItemShowFunc tspdtcm_item_show;
} DTCM_ITEM_CTX;



#endif

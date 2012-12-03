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

////////////////////////////////////////////////////////////////////////////////
//! \file main.c
//!
//! \brief This is ES stream and decoded frames dump tool,.
//!
//! Purpose:
//!
//!
//!	Version, Date and Author :
//!   V 1.00,    August 18 2012,    Tanyu
//!
//! Note:
////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "vdec_mgr.h"
#include "vdmdump.h"
#define MAX_CMD_LEN       80
#define MAX_ARG_CNT       10
#define MVVDMDUMP_REV "0.1"

CMD_HANDLER gCmdHandler_vdmdump[] = {
    {"help", cmd_handler_help_vdmdump, "List all supported commands"},
    {"estreams", cmd_handler_dump_estreams_vdmdump, "Dump es streams to file" },
    {"frames", cmd_handler_dump_frames_vdmdump, "Dump decoed frames to file" },
};
int iNumCmd_vdmdump = sizeof(gCmdHandler_vdmdump)/sizeof(CMD_HANDLER);

int cmd_handler_help_vdmdump(int argc, char *argv[])
{
    int i;
    printf("Marvell Galois Galois VDM Dump Control\n\n");
    printf("  Revision: %s\n", MVVDMDUMP_REV);
    printf("  Usage: 1. vdmdump command [arg1] [arg2] ...\n\n");
    printf("         2. vdmdump; \n             command [arg1] [arg2] ...\n\n");
    printf("All supported commands:\n\n");
	
    for (i = 0; i < iNumCmd_vdmdump; i++)
    {
        printf("  %-12s- %s\n", gCmdHandler_vdmdump[i].pCmd, gCmdHandler_vdmdump[i].pHelp);
    }
    return 0;
}

 int cmd_handler_dump_estreams_vdmdump(int argc, char *argv[])
{
    if( strcmp( argv[1], "enable" ) == 0 )
    {
        VdecMgr_Dump_Enable(VDM_DUMP_ESTEAMS, 1, NULL);
    }
    else if(strcmp( argv[1], "disable" ) == 0)
    {
        VdecMgr_Dump_Enable(VDM_DUMP_ESTEAMS, 0, NULL);
    }
    else
    {
        printf("  Usage: 1. vdmdump estreams [enable|disable] [filename] ...\n\n");
        printf("         2. vdmdump; \n             estreams [enable|disable] [filename] ...\n\n");
    }
    return 0;
}

 int cmd_handler_dump_frames_vdmdump(int argc, char *argv[])
{
    if( strcmp( argv[1], "enable" ) == 0 )
    {
        VdecMgr_Dump_Enable(VDM_DUMP_FRAMES, 1, NULL);
    }
    else if(strcmp( argv[1], "disable" ) == 0)
    {
        VdecMgr_Dump_Enable(VDM_DUMP_FRAMES, 0, NULL);
    }
    else
    {
        printf("  Usage: 1. vdmdump frames [enable|disable] [filename] ...\n\n");
        printf("         2. vdmdump; \n             frames [enable|disable] [filename] ...\n\n");
    }
    return 0;
}



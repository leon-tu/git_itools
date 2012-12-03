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
#define PROMPT            "ITOOLS:$ => VDM_DUMP:$ "
#define MVVDMDUMP_REV "0.1"
typedef int (*CMDFUNC)(int argc, char *argv[]);
typedef struct tagCmdHandler
{
    char *pCmd;
    CMDFUNC Handler;
    char *pHelp;
} CMD_HANDLER;

static int cmd_handler_help(int argc, char *argv[]);
static int cmd_handler_dump_estreams(int argc, char *argv[]);
static int cmd_handler_dump_frames(int argc, char *argv[]);

static CMD_HANDLER gCmdHandler[] = {
    {"help", cmd_handler_help, "List all supported commands"},
    {"estreams", cmd_handler_dump_estreams, "Dump es streams to file" },
    {"frames", cmd_handler_dump_frames, "Dump decoed frames to file" },
};
static int iNumCmd = sizeof(gCmdHandler)/sizeof(CMD_HANDLER);

static int cmd_handler_help(int argc, char *argv[])
{
    int i;
    printf("Marvell Galois Galois VDM Dump Control\n\n");
    printf("  Revision: %s\n", MVVDMDUMP_REV);
    printf("  Usage: 1. vdmdump command [arg1] [arg2] ...\n\n");
    printf("         2. vdmdump; \n             command [arg1] [arg2] ...\n\n");
    printf("All supported commands:\n\n");
    for (i = 0; i < iNumCmd; i++)
    {
        printf("  %-12s- %s\n", gCmdHandler[i].pCmd, gCmdHandler[i].pHelp);
    }
    return 0;
}

static int cmd_handler_dump_estreams(int argc, char *argv[])
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

static int cmd_handler_dump_frames(int argc, char *argv[])
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


static int get_command(char *pCmd, int iMaxSize)
{
    int count;
    char ch;

    if ((pCmd == 0) || (iMaxSize == 0))
    {
        return 0;
    }

    count = 0;
    iMaxSize--;
    while ((ch = getchar()) != 0x0a)
    {
        if (count < iMaxSize)
        {
            pCmd[count++] = ch;
        }
    }

    pCmd[count] = '\0';

    return count;
}

static BOOL parse_command(char *pCmd)
{
    char *pArg[MAX_ARG_CNT];
    int argc = 0;
    BOOL bValid = FALSE;
    int i;

    while (*pCmd == 0x20) pCmd++;

    if (strlen(pCmd) == 0)
    {
        return TRUE;
    }

    if (strcmp(pCmd, "quit") == 0)
    {
        return FALSE;
    }

    pArg[argc] = pCmd;
    argc++;
    while (*pCmd != 0)
    {
        while ((*pCmd != 0x20) && (*pCmd != 0))
        {
            pCmd++;
        }

        if (*pCmd == 0x20)
        {
            *pCmd++ = 0;
            while (*pCmd == 0x20) pCmd++;
            if (*pCmd != 0)
            {
                pArg[argc++] = pCmd;
            }
        }
    }

    for (i = 0; i < iNumCmd; i++)
    {
        if (strcmp(pArg[0], gCmdHandler[i].pCmd) == 0)
        {
            bValid = TRUE;
            gCmdHandler[i].Handler(argc, pArg);
        }
    }

    if (!bValid)
    {
        printf("Invalid command! Type help to see all supported command!\n");
    }

    return TRUE;
}

static CMD_HANDLER* parse_command_from_shell(char *pCmd)
{
    int i;

    while (*pCmd == 0x20) pCmd++;

    if (strlen(pCmd) == 0)
    {
        return TRUE;
    }

    for (i = 0; i < iNumCmd; i++)
    {
        if (strcmp(pCmd, gCmdHandler[i].pCmd) == 0)
        {
            return &gCmdHandler[i];
        }
    }

    printf("Invalid command! Type help to see all supported command!\n");

    return NULL;
}


int VdmDumpCtrlEntry()
{
    HRESULT rc = 0;
    char	Cmd[MAX_CMD_LEN];
    int len = 0;

    printf("***********************************************\n");
    printf("*       MARVELL Galois VDM Dump Control       *\n");
    printf("***********************************************\n");

    do
    {
        printf(PROMPT);
        len = get_command(Cmd, MAX_CMD_LEN);
        //printf("Command = %s, len = %d\n", Cmd, len);
    } while (parse_command(Cmd));

    return rc;
}


int VdmDumpCommandline(int argc, char *argv[])
{
    HRESULT rc = 0;
    char    Cmd[MAX_CMD_LEN];
    CMD_HANDLER *pCmdHandler = 0;

    printf("***********************************************\n");
    printf("*       MARVELL Galois VDM Dump Control       *\n");
    printf("***********************************************\n");

    pCmdHandler = parse_command_from_shell(argv[1]);
    if (pCmdHandler)
    {
        pCmdHandler->Handler(argc-1, &argv[1]);
    }
    else
    {
        cmd_handler_help(argc, argv);
        rc = E_INVALIDARG;
    }

    return rc;
}


int VdmDumpMain(int argc, char *argv[])
{
    if (argc >= 2)
        VdmDumpCommandline(argc, argv);
    else
        VdmDumpCtrlEntry();
    return 0;
}

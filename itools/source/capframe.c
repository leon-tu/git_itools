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
//! \file main.c
//!
//! \brief Implementation of a console for capture a video frame
//!
//! Author:   Chang Liu
//! Version:  1.1
//! Date:     September. 2012
//!
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "capframe.h"

#include "galois_debug.h"
#ifndef WIN32
#include "pe_api.h"
#include "pe_debug.h"
#endif

#define MAX_CMD_LEN       80
#define MAX_ARG_CNT       10
#define CAPFRAME_REV         "1.1"
#define INVALID_ARGUMENT  printf("Invalid argument!\n")

typedef struct CapturePara
{
    UINT32 CapCh;
    UINT32 CapMode;
    MV_PE_RECT CapWin;
    UINT32 CapNum;
    char CapPath[256];
} CAPTURE_PARA;
static CAPTURE_PARA gCurrentPara;

extern HANDLE gPe;
extern HANDLE ghVIP;

int cmd_handler_setmode_capframe(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "1") == 0)
            gCurrentPara.CapMode = 1;
        else if (strcmp(argv[1], "0") == 0)
            gCurrentPara.CapMode = 0;
	else
            INVALID_ARGUMENT;
    }
    else
	INVALID_ARGUMENT;

    return 0;
}

int cmd_handler_setwin_capframe(int argc, char *argv[])
{

    int x,y,w,h;

    if (argc > 1)
    {
        if ((x = atoi(argv[1])) >= 0){
            gCurrentPara.CapWin.x= (INT16)x;
	    }
        else
        {
            printf("x:");
            INVALID_ARGUMENT;
        }

        if ((y = atoi(argv[2])) >= 0)
            gCurrentPara.CapWin.y= (INT16)y;
        else
        {
            printf("y:");
            INVALID_ARGUMENT;
        }

        if ((w = atoi(argv[3])) > 0)
            gCurrentPara.CapWin.w= (INT16)w;
        else
        {
            printf("w:");
            INVALID_ARGUMENT;
        }

        if ((h = atoi(argv[4])) > 0)
            gCurrentPara.CapWin.h= (INT16)h;
        else
        {
            printf("h:");
            INVALID_ARGUMENT;
        }

    }
    else
        INVALID_ARGUMENT;

    return 0;
}

int cmd_handler_setpath_capframe(int argc, char *argv[])
{

    if (argc > 1)
    {
       strcpy(gCurrentPara.CapPath,argv[1]);
    }
    else
        INVALID_ARGUMENT;

    return 0;
}

int cmd_handler_setnum_capframe(int argc, char *argv[])
{
    int num;

    if (argc > 1)
    {
       if ((num = atoi(argv[1])) >= 0)
           gCurrentPara.CapNum = num;
       else
	   INVALID_ARGUMENT;
    }
    else
        INVALID_ARGUMENT;

    return 0;
}

int cmd_handler_getarg_capframe(int argc, char *argv[])
{
    printf("Current arguments:\n");
    printf("CapMode: %d\n",gCurrentPara.CapMode);
    printf("CapWin:  (%d, %d, %d, %d)\n", gCurrentPara.CapWin.x, gCurrentPara.CapWin.y, gCurrentPara.CapWin.w, gCurrentPara.CapWin.h);
    printf("CapPath: %s\n", gCurrentPara.CapPath);
    printf("CapNum:  %d\n", gCurrentPara.CapNum);

    return 0;
}

int cmd_handler_encap_capframe(int argc, char *argv[])
{
    unsigned char* addr = NULL;
    UINT32 shm_offset = 0;
    UINT32 size = 0;
    MV_PE_RECT rect;
    MV_TimePTS_t pts;
    MV_PE_VIP_TG_PARAMS struct_fetg;
    MV_PE_VIP_TG_PARAMS *fetg = &struct_fetg;
    UINT32 hStrmBuf = 0;
    UINT32 ExpectBytes=0;
    UINT32 pDstBuf = 0;
    UINT32 ReadSize = 0;
    FILE *fp=NULL;
    UINT32 frm_cnt=0;
    UINT32 cap_cnt=0;
    UINT8 pDstFileName[128] = {0};
    UINT8 *pReadPtr;
    UINT32 rc;

    cap_cnt = gCurrentPara.CapNum;

    rect.x = gCurrentPara.CapWin.x;
    rect.y = gCurrentPara.CapWin.y;
    rect.w = gCurrentPara.CapWin.w;
    rect.h = gCurrentPara.CapWin.h;
    ExpectBytes = rect.w * rect.h *2;
    rc = MV_PE_StreamBufAllocate(gPe,(ExpectBytes)*4,&hStrmBuf);

    MV_PE_VIP_Video_Get_FETG(gPe, ghVIP, &struct_fetg);
  //  printf("fetg (x:%d,%d,y: %d,%d)\n", fetg->start_x, fetg->end_x, fetg->start_y, fetg->end_y);
  //  printf("about to capture a frame rect (%d, %d, %d, %d)\n", rect.x, rect.y, rect.w, rect.h);

    while(cap_cnt-- > 0){
        rc = MV_PE_VIP_CaptureFrameExt(gPe, hStrmBuf, 0, &rect, &pts);

//        printf("%s %d: rc = %08x\n",__FUNCTION__,__LINE__,rc);
        if (rc != S_OK){
            printf("MV_PE_VIP_CaptureFrameExt error! rc =%08x\n",rc);
            break;
        }

        //Get the read ptr of c_buf
        ExpectBytes = rect.w * rect.h * 2;
        ReadSize = ExpectBytes;
        rc = MV_PE_StreamBufGetReadPtr(gPe, hStrmBuf, &ReadSize, (void **)&pReadPtr);
        if (ExpectBytes != ReadSize || pReadPtr == NULL || rc != S_OK){
            printf("MV_PE_StreamBufGetReadPtr error! rc=%d\n");
            printf("%s %d: rc = %08x, pReadPtr=%08x, ReadSize=%d, ExpectBytes=%d \n",__FUNCTION__,__LINE__,rc,pReadPtr,ReadSize,ExpectBytes);
            break;
        }

        //Save the captured frame.
        snprintf(pDstFileName, 256, "%scapvip_%dx%d_mode%d_%d.yuv",gCurrentPara.CapPath,rect.w,rect.h,gCurrentPara.CapMode,frm_cnt++);
	if(NULL != (fp = fopen(pDstFileName, "wb")))
	{
            fwrite(pReadPtr, 1, ExpectBytes, fp);
            fflush(fp);
            fclose(fp);
	}
	else
	{
            printf("Failed to open %s\n",pDstFileName);
	}
        pDstBuf = NULL;
        rc = MV_PE_StreamBufRead(gPe, hStrmBuf, pDstBuf, ExpectBytes);
        printf("save the capture frame. pReadPtr = %08x, ReadSize = %d, pts_high = %08x, pts_low = %08x\n",pReadPtr,ReadSize,pts.m_high,pts.m_low);
    }

    printf ("Captured %d frames. \n",frm_cnt);

    return 0;
}

CMD_HANDLER gCmdHandler_capframe[] = {
    {"help", cmd_handler_help_capframe, "List all supported commands"},
    {"setmode", cmd_handler_setmode_capframe, "Set the video capture mode. 0(capture partial of the video frame); 1(capture the downscaled version of the video frame) ex: setmode 0"},
    {"setwin", cmd_handler_setwin_capframe, "Set the window size for video capture. The format is (x, y, width, height). ex: setwin 0 0 320 240"},
    {"setpath", cmd_handler_setpath_capframe, "Set the path and prefix to save the captured frames. ex: setpath /data/"},
    {"setnum", cmd_handler_setnum_capframe, "Set the number of captured frame. ex: setnum 10"},
    {"getarg", cmd_handler_getarg_capframe, "Get current arg setting."},
    {"cap", cmd_handler_encap_capframe, "Enable and start the capture frame function."},
    //extension and alias
};

int iNumCmd_capframe = sizeof(gCmdHandler_capframe)/sizeof(CMD_HANDLER);

int cmd_handler_help_capframe(int argc, char *argv[])
{
    int i;
    printf("Marvell GaloisSoftware PE Debug Tool: capture frame from VIP\n\n");
    printf("  Revision: %s\n", CAPFRAME_REV);
    printf("  Author: Chang Liu <liuchang@marvell.com>\n");
    printf("  Usage:  capframe; \n            command [arg1] [arg2] ...\n\n");
    printf("All supported commands:\n\n");
    for (i = 0; i < iNumCmd_capframe; i++)
    {
        printf("  %-12s- %s\n", gCmdHandler_capframe[i].pCmd, gCmdHandler_capframe[i].pHelp);
    }
    return 0;
}


void init_cap_arg()
{
    strcpy(gCurrentPara.CapPath,"/data/");
    gCurrentPara.CapCh = 0;
    gCurrentPara.CapMode = 0;
    gCurrentPara.CapWin.x = 0;
    gCurrentPara.CapWin.y = 0;
    gCurrentPara.CapWin.w = 160;
    gCurrentPara.CapWin.h = 120;
    gCurrentPara.CapNum = 10;

    return;
}



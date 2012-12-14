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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#include "tsp_fw_spec.h"
#include "tsp_dtcm_parser.h"

#define INVALID_ARGUMENT  printf("Invalid argument!\n")
#define TSP_VER  1.0

unsigned int gPrintCtrlWord = 0xFFFFFFFF;
unsigned char* gptrMemMap = NULL;
FILE    *fpSaveFile = NULL;
unsigned int    Count = 0;
SIE_TspDtcmGlobal gDTCMBackup;

#define TSP_OUT(...)  {                                         \   
                        if(fpSaveFile)                          \
                        {                                       \
                            fprintf(fpSaveFile, __VA_ARGS__);   \
                            fflush(fpSaveFile);                 \       
                        }                                       \
                        printf(__VA_ARGS__);                    \
                    }

static int tspdtcm_Localdata_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_Tasklist_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_PidFilterCtxTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_PesFilterCtxTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_WmFilter_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_DSOutBufTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_DsInBufTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_SecFilterCtxTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);
static int tspdtcm_AACSDecFilter_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow);

static void TSPDTCM_ItemShow(DTCM_SHOW_MODE mode, unsigned char *ptr, unsigned char *item);


CMD_HANDLER gCmdHandler_tsp[] = {

    {"setctrlwd",cmd_handler_SetCtrlWord_tsp,"Set control word"},
    {"getctrlwd",cmd_handler_GetCtrlWord_tsp,"Get control word"},
    {"openfile",cmd_handler_openfile_tsp,"Open one log file to save"},
    {"show",cmd_handler_show_tsp,"Show DTCM content"},
    {"detect",cmd_handler_detect_tsp,"Detect the change of DTCM content"},
    {"help",cmd_handler_help_tsp,"List all supported commands"},

};

int iNumCmd_tsp = sizeof(gCmdHandler_tsp)/sizeof(CMD_HANDLER);


DTCM_ITEM_CTX DTCMItemTable[] = 
                    {
                        {0, "Localdata",    tspdtcm_Localdata_show},
                        {1, "Tasklist",     tspdtcm_Tasklist_show},
                        {2, "PidFilter",    tspdtcm_PidFilterCtxTable_show},
                        {3, "PesFilter",    tspdtcm_PesFilterCtxTable_show},
                        {4, "WmFilter",     tspdtcm_WmFilter_show},
                        {5, "DSOutBuf",     tspdtcm_DSOutBufTable_show},
                        {6, "DsInBuf",      tspdtcm_DsInBufTable_show},
                        {7, "SecFilter",    tspdtcm_SecFilterCtxTable_show},
                        {8, "AACSDecFilter",tspdtcm_AACSDecFilter_show},                            
                    };
#define DTCM_ITEM_NUM  (sizeof(DTCMItemTable)/sizeof(DTCM_ITEM_CTX))

int cmd_handler_SetCtrlWord_tsp(int argc, char *argv[])
{
    if(argc != 2)
    {
        INVALID_ARGUMENT;
        return -1;
    }

    int val = Str2Num(argv[1]); 
    
    gPrintCtrlWord = val;
    return 0;
}


unsigned int cmd_handler_GetCtrlWord_tsp(int argc, char *argv[])
{
    if(argc != 1)
    {
        INVALID_ARGUMENT;
        return -1;

    }

    printf("Ctrl Word = 0x%08x\n",gPrintCtrlWord);

    return gPrintCtrlWord;
}

static int tspdtcm_Localdata_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_LocalArea_Ext *pFigoLocalArea;
    unsigned char *p;
    unsigned int i = 0;

    pFigoLocalArea = &(pShadow->ie_FigoLocalArea);
    p = (unsigned char *)pFigoLocalArea;
    if(mode == ONE_SHOT_MODE || 
        memcmp(&gDTCMBackup.ie_FigoLocalArea, pFigoLocalArea, sizeof(SIE_LocalArea_Ext)))
    {
        TSP_OUT("[LocalArea Mem Dump]");
        for(i=0; i<sizeof(SIE_LocalArea_Ext); i++)
        {
            if(i%16 == 0) TSP_OUT("\n----");
            TSP_OUT("0x%02x ", p[i]);
        }
        TSP_OUT("\n")

        memcpy(&gDTCMBackup.ie_FigoLocalArea, pFigoLocalArea, sizeof(SIE_LocalArea_Ext));
        return 1;
    }

    return 0;
}

static int tspdtcm_Tasklist_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_UINT32 	*TaskList = &(pShadow->ie_TaskList[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_TaskList, TaskList, 8*sizeof(SIE_UINT32)))
    {
        TSP_OUT("[TaskList]\n");
        for(i=0; i<8; i++)
        {   if(TaskList[i].uV32_uV16L != 0xFFFF || TaskList[i].uV32_uV16H != 0xFFFF)
            {
                TSP_OUT("----TaskList[%d] = 0x%x\n", i, TaskList[i]);
            }
        }
        memcpy(gDTCMBackup.ie_TaskList, TaskList, 8*sizeof(SIE_UINT32));
        return 1;
    }

    return 0;
}

static int tspdtcm_PidFilterCtxTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_PidFilterCtx *pPidFilterCtxTable = &(pShadow->ie_PidFilterCtxTable[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_PidFilterCtxTable, pPidFilterCtxTable, 256*sizeof(SIE_PidFilterCtx)))
    {
        TSP_OUT("[ie_PidFilterCtxTable]\n");
        for(i=0; i<256; i++)
        {
            pPidFilterCtxTable = &(pShadow->ie_PidFilterCtxTable[i]);
            if(pPidFilterCtxTable->u_IsActive)
            {
                TSP_OUT("----Filter Index:%d \n", i);
                TSP_OUT("----[u_OnOff=%d] \n", pPidFilterCtxTable->u_OnOff);
                TSP_OUT("----[u_IsPcr=%d] \n", pPidFilterCtxTable->u_IsPcr);
                TSP_OUT("----[u_IsPsi=%d] \n", pPidFilterCtxTable->u_IsPsi);
                TSP_OUT("----[u_IsTsOut=%d] \n", pPidFilterCtxTable->u_IsTsOut);
                TSP_OUT("----[u_DecrytionBlockSize=%d] \n", pPidFilterCtxTable->u_DecrytionBlockSize);
                TSP_OUT("----[u_SystemKeyOffsetFromKey=%d]\n", pPidFilterCtxTable->u_SystemKeyOffsetFromKey);

                TSP_OUT("----[u_ScrambMode=%d] \n", pPidFilterCtxTable->u_ScrambMode);
                TSP_OUT("----[u_TS_ID=%d] \n", pPidFilterCtxTable->u_TS_ID);
                TSP_OUT("----[u_TspDecryptType=%d] \n", pPidFilterCtxTable->u_TspDecryptType);
                TSP_OUT("----[u_TspDecryptModeOrRound=%d] \n", pPidFilterCtxTable->u_TspDecryptModeOrRound);
                TSP_OUT("----[u_Pid=0x%x] \n", pPidFilterCtxTable->u_Pid);
                TSP_OUT("----[u_TspDecryptDataLen=%d] \n", pPidFilterCtxTable->u_TspDecryptDataLen);

                TSP_OUT("----[u_pDecryptKeyCtx=%d] \n", pPidFilterCtxTable->u_pDecryptKeyCtx);
                TSP_OUT("----[u_pDecryptIVCtx=%d] \n", pPidFilterCtxTable->u_pDecryptIVCtx);

                TSP_OUT("----[u_uSefState=%d] \n", pPidFilterCtxTable->u_uSefState);
                TSP_OUT("----[u_uCcnt=%d] \n", pPidFilterCtxTable->u_uCcnt);
                TSP_OUT("----[u_CurrScrambBits=%d] \n", pPidFilterCtxTable->u_CurrScrambBits);
                TSP_OUT("----[u_EvenKeyOffsetFromOddKey=0x%x] \n", pPidFilterCtxTable->u_EvenKeyOffsetFromOddKey);
                TSP_OUT("----[u_pPartSecHdrInfo=%d] \n", pPidFilterCtxTable->u_pPartSecHdrInfo);

            }
        }
        memcpy(gDTCMBackup.ie_PidFilterCtxTable, &(pShadow->ie_PidFilterCtxTable[0]), 256*sizeof(SIE_PidFilterCtx));
        return 1;

    }

    return 0;

}

static int tspdtcm_SecFilterCtxTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_SecFilterCtx *pSecFilterCtxTable = &(pShadow->ie_SecFilterCtxTable[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_SecFilterCtxTable, pSecFilterCtxTable, 128*sizeof(SIE_PidFilterCtx)))
    {
        TSP_OUT("[ie_SecFilterCtxTable]\n");
        for(i=0; i<128; i++)
        {
            pSecFilterCtxTable = &(pShadow->ie_SecFilterCtxTable[i]);
            if(pSecFilterCtxTable->u_pSectionBuf != 0xFFFF && pSecFilterCtxTable->u_pSectionBuf != 0x0)
            {
                TSP_OUT("----Filter Index:%d \n", i);
                TSP_OUT("----[u_pSectionBuf=%d] \n", pSecFilterCtxTable->u_pSectionBuf);
                TSP_OUT("----[u_BytesLeft=%d] \n", pSecFilterCtxTable->u_BytesLeft);
                TSP_OUT("----[u_IsCrcCheck=%d] \n", pSecFilterCtxTable->u_IsCrcCheck);
                TSP_OUT("----[u_OnOff=%d] \n", pSecFilterCtxTable->u_OnOff);
                TSP_OUT("----[u_CrcState=%d] \n", pSecFilterCtxTable->u_CrcState);
            }
        }
        memcpy(gDTCMBackup.ie_PidFilterCtxTable, &(pShadow->ie_PidFilterCtxTable[0]), 256*sizeof(SIE_PidFilterCtx));
        return 1;

    }

    return 0;

}


static int tspdtcm_AACSDecFilter_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_AACSDecFilter *pAACSDecFilter = &(pShadow->ie_AacsFilters[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_AacsFilters, pAACSDecFilter, arr_TspDtcmGlobal_AacsFilters*sizeof(SIE_AACSDecFilter)))
    {
        TSP_OUT("[ie_AacsFilters]\n");
        for(i=0; i<arr_TspDtcmGlobal_AacsFilters; i++)
        {
            pAACSDecFilter = &(pShadow->ie_AacsFilters[i]);
            if(pAACSDecFilter->ie_TskHdr.u_pFunc != 0xFFFF && pAACSDecFilter->ie_TskHdr.u_pFunc != 0x0)
            {
                TSP_OUT("----Filter Index:%d \n", i);
                TSP_OUT("--------ie_TskHdr:\n");
                TSP_OUT("------------[u_pFunc=0x%x]\n", pAACSDecFilter->ie_TskHdr.u_pFunc);
                TSP_OUT("------------[u_TaskState=0x%x]\n", pAACSDecFilter->ie_TskHdr.u_TaskState);
                TSP_OUT("------------[u_TaskType=0x%x]\n", pAACSDecFilter->ie_TskHdr.u_TaskType);
                TSP_OUT("------------[u_uSrcId=0x%x]\n", pAACSDecFilter->ie_TskHdr.u_uSrcId);
                TSP_OUT("--------[uFLAG_isBypass=0x%x]\n", pAACSDecFilter->uFLAG_isBypass);
                TSP_OUT("--------[uFLAG_isEncrypted=0x%x]\n", pAACSDecFilter->uFLAG_isEncrypted);
                TSP_OUT("--------[uFLAG_isBeOn=0x%x]\n", pAACSDecFilter->uFLAG_isBeOn);
                TSP_OUT("--------[uFLAG_CryptoFifoID=0x%x]\n", pAACSDecFilter->uFLAG_CryptoFifoID);
                TSP_OUT("--------[uIO_pStrmInBuf=0x%x]\n", pAACSDecFilter->uIO_pStrmInBuf);
                TSP_OUT("--------[uIO_pStrmOutBuf=0x%x]\n", pAACSDecFilter->uIO_pStrmOutBuf);
                TSP_OUT("--------[u_pLocalData=0x%x]\n", pAACSDecFilter->u_pLocalData);                
                TSP_OUT("--------[u_packStartOffset=0x%x]\n", pAACSDecFilter->u_packStartOffset);
                TSP_OUT("--------[u_packSizeToRd=0x%x]\n", pAACSDecFilter->u_packSizeToRd);
                TSP_OUT("--------[u_beBlockCnt=0x%x]\n", pAACSDecFilter->u_beBlockCnt);
                TSP_OUT("--------[u_bePackSizeToRd=0x%x]\n", pAACSDecFilter->u_bePackSizeToRd);
                TSP_OUT("--------[uCTR_DataCnt=0x%x]\n", pAACSDecFilter->uCTR_DataCnt);
                TSP_OUT("--------[uKEY_pTitleKey=0x%x]\n", pAACSDecFilter->uKEY_pTitleKey);
                TSP_OUT("--------[uKEY_pBusKey=0x%x]\n", pAACSDecFilter->uKEY_pBusKey);
            }
        }
        memcpy(gDTCMBackup.ie_PidFilterCtxTable, &(pShadow->ie_PidFilterCtxTable[0]), 256*sizeof(SIE_PidFilterCtx));
        return 1;

    }

    return 0;

}



static int tspdtcm_PesFilterCtxTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_PesFilterCtx *pPesFilterCtxTable = &(pShadow->ie_PesFilterCtxTable[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_PesFilterCtxTable, pPesFilterCtxTable, 32*sizeof(SIE_PidFilterCtx)))
    {
        TSP_OUT("[ie_PesFilterCtxTable]\n");
        for(i=0; i<32; i++)
        {
            pPesFilterCtxTable = &(pShadow->ie_PesFilterCtxTable[i]);
            
            if(pPesFilterCtxTable->u_StrmState || pPesFilterCtxTable->u_OnOff)
            {
                TSP_OUT("----Filter Index:%d\n", i);
                TSP_OUT("----[u_StrmState=%d] \n", pPesFilterCtxTable->u_StrmState);
                TSP_OUT("----[u_HdrLeftSize=%d] \n", pPesFilterCtxTable->u_HdrLeftSize);
                TSP_OUT("----[u_OnOff=%d] \n", pPesFilterCtxTable->u_OnOff);
                TSP_OUT("----[u_uSrcId=%d] \n", pPesFilterCtxTable->u_uSrcId);
                TSP_OUT("----[u_NonZeroPesLen=%d] \n", pPesFilterCtxTable->u_NonZeroPesLen);
                TSP_OUT("----[u_IsCrcCheck=%d]\n", pPesFilterCtxTable->u_IsCrcCheck);
                TSP_OUT("----[u_PadToDword=%d]\n", pPesFilterCtxTable->u_PadToDword);
                TSP_OUT("----[u_PadToDword=%d]\n", pPesFilterCtxTable->u_PadToDword);
                TSP_OUT("----[u_SmallFramePadSize=%d]\n", pPesFilterCtxTable->u_SmallFramePadSize);

            }
        }
        memcpy(gDTCMBackup.ie_PesFilterCtxTable, &(pShadow->ie_PesFilterCtxTable[0]), 32*sizeof(SIE_PidFilterCtx));
        
        return 1;
    }
    return 0;
}


static int tspdtcm_WmFilter_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    
    SIE_WmDecFilter *pWmDecFilter=&pShadow->ie_WmFilter;
    if(mode == ONE_SHOT_MODE || 
        memcmp(&gDTCMBackup.ie_WmFilter, pWmDecFilter, sizeof(SIE_WmDecFilter)))
    {
        TSP_OUT("[ie_WmFilter]\n");
        TSP_OUT("----[SIE_TaskHeader] \n");
        TSP_OUT("--------[u_TaskState=%d] \n", pWmDecFilter->ie_TskHdr.u_TaskState);
        TSP_OUT("--------[u_TaskType=%d] \n", pWmDecFilter->ie_TskHdr.u_TaskType);
        TSP_OUT("--------[u_uSrcId=%d]\n", pWmDecFilter->ie_TskHdr.u_uSrcId);
        TSP_OUT("----[u_payloadOffset = 0x%x] \n", pWmDecFilter->u_payloadOffset);
        TSP_OUT("----[u_payloadSize = 0x%x]\n", pWmDecFilter->u_payloadSize);
        TSP_OUT("----[ie_marlin_param]\n");
        TSP_OUT("--------[u_b_start = 0x%x]\n", pWmDecFilter->ie_marlin_param.u_b_start);
        TSP_OUT("--------[u_b_complete = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_b_complete);
        TSP_OUT("--------[u_FifoCH = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_FifoCH);
        TSP_OUT("--------[u_source_addr = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_source_addr);
        TSP_OUT("--------[u_b_start = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_b_start);
        TSP_OUT("--------[u_b_complete = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_b_complete); 
        TSP_OUT("--------[u_input_size = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_input_size);
        TSP_OUT("--------[u_dest_addr = 0x%x] \n", pWmDecFilter->ie_marlin_param.u_dest_addr);
        memcpy(&gDTCMBackup.ie_WmFilter, pWmDecFilter, sizeof(SIE_WmDecFilter));
        
        return 1;
    }
    return 0;
}


static int tspdtcm_DSOutBufTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_DSStreamOutEx *pDSOutBuf = &(pShadow->ie_DSOutBufTable[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_DSOutBufTable, pDSOutBuf, 160*sizeof(SIE_DSStreamOutEx)))
    {
        TSP_OUT("[ie_DSOutBufTable]\n");
        for(i=0; i<160; i++)
        {
            pDSOutBuf = &(pShadow->ie_DSOutBufTable[i]);
            if(pDSOutBuf->uCNT_DataCnt || pDSOutBuf->ie_dsFifo.u_pbBase)
            {
                TSP_OUT("----[ie_DSOutBufTable]\n");
                TSP_OUT("----[uCNT_DataCnt = 0x%x]\n", pDSOutBuf->uCNT_DataCnt);
                TSP_OUT("----[ie_dsFifo]\n");
                TSP_OUT("--------%d. u_nRdOff=x%x u_pbBase=0x%x u_nWrOff=0x%x u_nMaxSize=0x%x\n", 
                                        i,
                                        pDSOutBuf->ie_dsFifo.u_nRdOff,
                                        pDSOutBuf->ie_dsFifo.u_pbBase,
                                        pDSOutBuf->ie_dsFifo.u_nWrOff,
                                        pDSOutBuf->ie_dsFifo.u_nMaxSize);

            }
        }
        memcpy(gDTCMBackup.ie_DSOutBufTable, &(pShadow->ie_DSOutBufTable[0]), 160*sizeof(SIE_DSStreamOutEx));
        
        return 1;
    }

    return 0;
}


static int tspdtcm_DsInBufTable_show(DTCM_SHOW_MODE mode, SIE_TspDtcmGlobal *pShadow)
{
    SIE_DSStreamIn *pDsInBuf = &(pShadow->ie_DsInBufTable[0]);
    unsigned int i = 0;

    if(mode == ONE_SHOT_MODE || 
        memcmp(gDTCMBackup.ie_DsInBufTable, pDsInBuf, 8*sizeof(SIE_DSStreamIn)))
    {

        TSP_OUT("[ie_DsInBufTable]\n");
        for(i=0; i<8; i++)
        {
            pDsInBuf = &(pShadow->ie_DsInBufTable[i]);
            if(pDsInBuf->ie_dsFifo.u_pbBase)
            {
    			TSP_OUT("----ie_dsFifo:\n");
                TSP_OUT("--------%d. u_nRdOff=x%x u_pbBase=0x%x u_nWrOff=0x%x u_nMaxSize=0x%x\n", 
                                    i,
                                    pDsInBuf->ie_dsFifo.u_nRdOff,
                                    pDsInBuf->ie_dsFifo.u_pbBase,
                                    pDsInBuf->ie_dsFifo.u_nWrOff,
                                    pDsInBuf->ie_dsFifo.u_nMaxSize);
            }
            if(pDsInBuf->ie_ctrlFIFoIn.u_pbBase)
            {
    			TSP_OUT("----ie_ctrlFIFoIn:\n");
                TSP_OUT("--------%d. u_nRdOff=x%x u_pbBase=0x%x u_nWrOff=0x%x u_nMaxSize=0x%x\n", 
                                    i,
                                    pDsInBuf->ie_ctrlFIFoIn.u_nRdOff,
                                    pDsInBuf->ie_ctrlFIFoIn.u_pbBase,
                                    pDsInBuf->ie_ctrlFIFoIn.u_nWrOff,
                                    pDsInBuf->ie_ctrlFIFoIn.u_nMaxSize);
            }
        }
        memcpy(gDTCMBackup.ie_DsInBufTable, &(pShadow->ie_DsInBufTable[0]), 8*sizeof(SIE_DSStreamIn));
        return 1;
    }

    return 0;
}


static DTCM_ITEM_CTX *tspdtcm_inquire_item_entry(unsigned char *item)
{
    int i = 0;

    if(!item)
    {
        return NULL;
    }
    
    for(i=0; i<DTCM_ITEM_NUM; i++)
    {
        if(strcmp(tolower(item), tolower(DTCMItemTable[i].ItemName)) == 0)
        {
            return &DTCMItemTable[i];
        }
    }

    return NULL;
}
static void TSPDTCM_ItemShow(DTCM_SHOW_MODE mode, unsigned char *ptr, unsigned char *item)
{
    SIE_TspDtcmGlobal *pShadow = (SIE_TspDtcmGlobal*)ptr;
    unsigned int i = 0;
    DTCM_ITEM_CTX *pDTCMItemCtx = 0;

    pDTCMItemCtx = tspdtcm_inquire_item_entry(item);

    if(pDTCMItemCtx)
    {
        pDTCMItemCtx->tspdtcm_item_show(mode, ptr);
    }
    else
    {
        TSP_OUT("|---------support Item show as below-----------------|\n");
        for(i=0; i<DTCM_ITEM_NUM; i++)
        {
            TSP_OUT("     %d. %s\n", DTCMItemTable[i].index, DTCMItemTable[i].ItemName);
        }
        TSP_OUT("\nYou can use the command like \"show Localdata\"");
        TSP_OUT("\n\n");
    }
    
    return;
}

static void TSPDTCM_ContentShow(DTCM_SHOW_MODE mode, unsigned char *ptr)
{
    SIE_TspDtcmGlobal *pShadow = (SIE_TspDtcmGlobal*)ptr;
    unsigned int i = 0;
    unsigned int anything_show = 0;
    
    if(gPrintCtrlWord&TspDtcmGlobal_LA_Mask)
    {
        anything_show += tspdtcm_Localdata_show(mode, pShadow);
    }

    if(gPrintCtrlWord&TspDtcmGlobal_SCHED_Mask)
    {
        anything_show += tspdtcm_Tasklist_show(mode, pShadow);
    }

    if(gPrintCtrlWord&TspDtcmGlobal_PID_Mask)
    {
        anything_show += tspdtcm_PidFilterCtxTable_show(mode, pShadow);
    }

    if(gPrintCtrlWord&TspDtcmGlobal_PES_Mask)
    {
        anything_show += tspdtcm_PesFilterCtxTable_show(mode, pShadow);
    }

    if(gPrintCtrlWord&TspDtcmGlobal_WM_Mask)
    {
        anything_show += tspdtcm_WmFilter_show(mode, pShadow);
    }

    if(gPrintCtrlWord&TspDtcmGlobal_DSOUT_Mask)
    {
        anything_show += tspdtcm_DSOutBufTable_show(mode, pShadow);
    }

    if(gPrintCtrlWord&TspDtcmGlobal_DSIN_Mask)
    {
        anything_show += tspdtcm_DsInBufTable_show(mode, pShadow);
    }
    if(anything_show)
    {
        Count++;
        TSP_OUT("|---------The--%d--times----TspDtcmGlobal-End-----------------|\n", Count);
        TSP_OUT("\n\n");
    }
    return;
}

unsigned char* tsp_dtcm_memmap(unsigned int size)
{
    unsigned int addr = 0xf7a40000;
    unsigned int mmem_addr=0, test_addr = addr, range_size = size, scaned = 0;
    char c;
    unsigned char* ptr=0;
    unsigned int i =0;

    addr = 0xf7a40000;
    
    mmem_addr = do_devmem_map(addr, size);
    if (mmem_addr == 0xffffffff)
    {
        printf("wrong address return\n");
        return NULL;
    }
    
    ptr = (unsigned char*)(mmem_addr);

    return ptr;
    
}

int Str2Num(char str[])
{ 
    int i; 
    char c; 
    int p=0;
    char *h = str;

    if(str[0] == '0' && ((str[1] == 'x')||(str[1] == 'X')))
    {
        h = &str[2];
        for(i=0;h[i]!='\0';i++)
        {   
            c=h[i];
            if(c>='0'&&c<='9') 
            {
        	    p=p*16+c-'0';
            }
            else if(c>='A'&&c<='F') 
            {
            	p=p*16+10+(c-'A');
            }
            else if(c>='a'&&c<='f') 
            {
        	    p=p*16+10+(c-'a');
            }
            else 
            {
                printf("error!\n");
            }
        }

    }
    else
    {
        p = atoi(h);
    }

    return(p);
} 


int cmd_handler_openfile_tsp(int argc, char *argv[])
{
    if(argc != 2)
    {
        INVALID_ARGUMENT;
        return -1;
    }
    
    if(fpSaveFile)
    {
        fclose(fpSaveFile);
    }
    
    fpSaveFile = fopen(argv[1], "wt+");
    return 0;
}    


int cmd_handler_show_tsp(int argc ,char *argv[])
{
    TSPDTCM_ItemShow(ONE_SHOT_MODE, gptrMemMap, tolower(argv[1]));
    return 0;
}


int cmd_handler_detect_tsp(int argc, char *argv[])
{
    while(1)
    {
            /*Show on Detect mode*/
        TSPDTCM_ContentShow(DETECTED_MODE, gptrMemMap);
    }

    
} 


int cmd_handler_help_tsp(int argc, char *argv[])
{
    if(argc != 1)
    {
        INVALID_ARGUMENT;
        return -1;
    }
    
    int i;
    printf("Marvell Galois PE Debug Control System\n\n");
    printf("  Version: %s\n",TSP_VER);
    printf("  Author: Weizhao Jiang\n");
    printf("  Usage: 1. tsp command [arg1] [arg2] ...\n\n");
    printf("         2. tsp; \n             command [arg1] [arg2] ...\n\n");
    printf("All supported commands:\n\n");
    for (i = 0; i < iNumCmd_tsp; i++)
    {
        printf("         %-12s- %s\n", gCmdHandler_tsp[i].pCmd, gCmdHandler_tsp[i].pHelp);
    }

   return 0;
}


static unsigned int MemMapSize = sizeof(SIE_TspDtcmGlobal)+0x100;

int InitTsp()
{
    unsigned int addr = 0xf7a40000;
    printf("do_devmem_map(0x%x, 0x%x)\n", addr, MemMapSize);
    gptrMemMap = do_devmem_map(addr, MemMapSize);
    if (gptrMemMap == 0xffffffff)
    {
        printf("wrong address return\n");
        return -1;
    }

    return 0;
}

void ExitTsp()
{
    do_devmem_unmap(gptrMemMap, MemMapSize);
}


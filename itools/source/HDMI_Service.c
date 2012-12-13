#include "com_type.h"
#include "OSAL_api.h"
#include "pe_type.h"
#include "pe_api.h"
#include "com_type.h"
#include "ErrorCode.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>


#include <string.h>

#include "setres.h"

#define DBG_ENABLE

#ifdef DBG_ENABLE
#define DEBUG_LOG   printf("\n"); printf
#else
#define DEBUG_LOG
#endif


#define MAX_CMD_LEN       80
#define MAX_ARG_CNT       10
#define SetRes_REV         "1.0"
#define INVALID_ARGUMENT  printf("Invalid argument!\n")
#define PE_RES_TO_VPP_RES(peRes) ((peRes)-1)
#define HDMI_MSGQ_SIZE    32

typedef struct {
        UINT32 ucHdmiColorFormat;
        UINT32 ucHdmiBitDepth;
        UCHAR ucHdmiPixelRep;
}HdmiConfig;

CMD_HANDLER gCmdHandler_setres[] = {
		{"help", cmd_handler_help_setres, "ShowHelp()"},
		{"switch", cmd_handler_switch_setres, "parameter router"},
};

//int iNumCmd_setres = sizeof(gCmdHandler_setres)/sizeof(CMD_HANDLER);

extern HANDLE  gPe;

static const int RES_INVALID = -1;
static MV_CC_HANDLE_MsgQEx_t s_HdmiMsgQ; // HDMI receiving message queue
static MV_OSAL_HANDLE_TASK_t s_HdmiCecTask;//HDMI envent handling task
HdmiConfig HdmiCfg = {0};
INT BoxResID = MV_PE_VOUT_FORMAT_1920_1080_P_5994;

static VOID SetResolutionbyKey(char key);
static VOID ShowHelp();

static HRESULT HdmiCecEvtHandler(UINT32 EventCode, void *EventInfo, void *Context)
{
    if (EventCode == MV_PE_EVENT_VPP_HDMI) {
        MV_CC_MsgQEx_PostMsg(s_HdmiMsgQ, EventInfo);
    }
    return S_OK;
}

static HRESULT GetPreferredResolution(MV_PE_VPP_HDMI_SINK_CAPS* pSinkCaps, INT* pHdmiRes)
{
    *pHdmiRes = RES_INVALID;

    if ( pSinkCaps->prefResInfo.hActive == 720 &&
            pSinkCaps->prefResInfo.vActive == 480 &&
            pSinkCaps->prefResInfo.refreshRate == 60)
    {
        *pHdmiRes = (pSinkCaps->prefResInfo.interlaced == 1) ? MV_PE_VOUT_FORMAT_720_480_I_5994 : MV_PE_VOUT_FORMAT_720_480_P_5994;

    } else if ( pSinkCaps->prefResInfo.hActive == 720 &&
            pSinkCaps->prefResInfo.vActive == 576 &&
            pSinkCaps->prefResInfo.refreshRate == 50)
    {
        *pHdmiRes = (pSinkCaps->prefResInfo.interlaced == 1) ? MV_PE_VOUT_FORMAT_720_576_I_50 : MV_PE_VOUT_FORMAT_720_576_P_50;

    } else if ( pSinkCaps->prefResInfo.hActive == 1280 &&
                pSinkCaps->prefResInfo.vActive == 720)
    {
        *pHdmiRes = pSinkCaps->prefResInfo.refreshRate == 30 ? MV_PE_VOUT_FORMAT_1280_720_P_2997 :
                    pSinkCaps->prefResInfo.refreshRate == 25 ? MV_PE_VOUT_FORMAT_1280_720_P_25 :
                    pSinkCaps->prefResInfo.refreshRate == 60 ? MV_PE_VOUT_FORMAT_1280_720_P_5994 :
                    pSinkCaps->prefResInfo.refreshRate == 50 ? MV_PE_VOUT_FORMAT_1280_720_P_50 : RES_INVALID;

    } else if ( pSinkCaps->prefResInfo.hActive == 1920 &&
                pSinkCaps->prefResInfo.vActive == 1080)
    {
        switch (pSinkCaps->prefResInfo.refreshRate)
        {
        case 30:
            *pHdmiRes = MV_PE_VOUT_FORMAT_1920_1080_P_2997;
            break;
        case 25:
            *pHdmiRes = MV_PE_VOUT_FORMAT_1920_1080_P_25;
            break;
        case 24:
            *pHdmiRes = MV_PE_VOUT_FORMAT_1920_1080_P_24;
            break;
        case 60:
            *pHdmiRes = pSinkCaps->prefResInfo.interlaced == 1 ? MV_PE_VOUT_FORMAT_1920_1080_I_5994 : MV_PE_VOUT_FORMAT_1920_1080_P_5994;
            break;
        case 50:
            *pHdmiRes = pSinkCaps->prefResInfo.interlaced == 1 ? MV_PE_VOUT_FORMAT_1920_1080_I_50 : MV_PE_VOUT_FORMAT_1920_1080_P_50;
            break;
        default:
            break;
        }
    }
    return S_OK;
}

static HRESULT GetMaxResolution(MV_PE_VPP_HDMI_SINK_CAPS* pSinkCaps, INT* pHdmiRes)
{
    HRESULT hr = E_FAIL;
    INT resID;

    *pHdmiRes = RES_INVALID;

    if (pSinkCaps->validEdid)
    {
        for (resID = MV_PE_VOUT_FORMAT_1920_1080_P_50; resID >= MV_PE_VOUT_FORMAT_720_480_I_60; resID--)
        {
            if (pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(resID)))
            {
                *pHdmiRes = resID;
                break;
            }
        }

        /* if both 50Hz and 60Hz are supported, select the 60Hz one */
        if (resID == MV_PE_VOUT_FORMAT_1920_1080_P_50 &&
            (pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(MV_PE_VOUT_FORMAT_1920_1080_P_60)) ||
            pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(MV_PE_VOUT_FORMAT_1920_1080_P_5994))))
        {
            *pHdmiRes = MV_PE_VOUT_FORMAT_1920_1080_P_5994;
        }

        if (resID == MV_PE_VOUT_FORMAT_1920_1080_I_50 &&
            (pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(MV_PE_VOUT_FORMAT_1920_1080_I_60)) ||
            pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(MV_PE_VOUT_FORMAT_1920_1080_I_5994))))
        {
            *pHdmiRes = MV_PE_VOUT_FORMAT_1920_1080_I_5994;
        }

        if (resID == MV_PE_VOUT_FORMAT_1280_720_P_50 &&
            (pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(MV_PE_VOUT_FORMAT_1280_720_P_60)) ||
            pSinkCaps->sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(MV_PE_VOUT_FORMAT_1280_720_P_5994))))
        {
            *pHdmiRes = MV_PE_VOUT_FORMAT_1280_720_P_5994;
        }
    }
    return hr;
}

static void SelectHDMIVideoFormat(MV_PE_VPP_HDMI_SINK_CAPS* pSinkCaps)
{
    //select HDMI video format.
    if (pSinkCaps->DC36bitSprt) {
        HdmiCfg.ucHdmiBitDepth = MV_PE_VOUT_BITDEPTH_12BIT;
        HdmiCfg.ucHdmiColorFormat = \
        pSinkCaps->YCbCr444AtDC ? MV_PE_VIDEO_COLOR_FORMAT_YCBCR444 : MV_PE_VIDEO_COLOR_FORMAT_RGB888;
    } else if (pSinkCaps->DC30bitSprt) {
        HdmiCfg.ucHdmiBitDepth = MV_PE_VOUT_BITDEPTH_10BIT;
        HdmiCfg.ucHdmiColorFormat = \
        pSinkCaps->YCbCr444AtDC ? MV_PE_VIDEO_COLOR_FORMAT_YCBCR444 : MV_PE_VIDEO_COLOR_FORMAT_RGB888;
    } else {
        HdmiCfg.ucHdmiBitDepth = MV_PE_VOUT_BITDEPTH_8BIT;
        HdmiCfg.ucHdmiColorFormat = \
        pSinkCaps->YCbCr444Sprt ? MV_PE_VIDEO_COLOR_FORMAT_YCBCR444 :
        pSinkCaps->YCbCr422Sprt ? MV_PE_VIDEO_COLOR_FORMAT_YCBCR422 :
        MV_PE_VIDEO_COLOR_FORMAT_RGB888;
    }
    return;
}

static HRESULT GetMaxPixelReptByResolution(MV_PE_VPP_HDMI_SINK_CAPS* pSinkCaps, INT resID, UCHAR* pPixelRept)
{
    switch (resID) {
    case MV_PE_VOUT_FORMAT_720_480_I_60:
    case MV_PE_VOUT_FORMAT_720_480_I_5994:
        if (pSinkCaps->prInfo[0].prSupport & (1<<4))
            *pPixelRept = 4;
        else
            *pPixelRept = 2;
        break;
    case MV_PE_VOUT_FORMAT_720_576_I_50:
        if (pSinkCaps->prInfo[1].prSupport & (1<<4))
            *pPixelRept = 4;
        else
            *pPixelRept = 2;
        break;
    case MV_PE_VOUT_FORMAT_720_480_P_60:
    case MV_PE_VOUT_FORMAT_720_480_P_5994:
        if (pSinkCaps->prInfo[2].prSupport & (1<<4))
            *pPixelRept = 4;
        else if (pSinkCaps->prInfo[2].prSupport & (1<<2))
            *pPixelRept = 2;
        else
            *pPixelRept = 1;
        break;
    case MV_PE_VOUT_FORMAT_720_576_P_50:
        if (pSinkCaps->prInfo[3].prSupport & (1<<4))
            *pPixelRept = 4;
        else if (pSinkCaps->prInfo[3].prSupport & (1<<2))
            *pPixelRept = 2;
        else
            *pPixelRept = 1;
        break;
    default:
        *pPixelRept = 1;
        break;
    }

    return S_OK;
}

int SetResolutionbyID(INT targetRes)
{
    HRESULT hr = E_FAIL;
    MV_PE_VPP_HDMI_SINK_CAPS SinkCaps = {0};

    //read HDMI sink capability.
    MV_PE_VOutHDMIGetSinkCaps(gPe, MV_PE_CHANNEL_PRIMARY_VIDEO, &SinkCaps);

    if (SinkCaps.validEdid)
    {
        if(BoxResID == targetRes)
        {
           return S_OK;
        }
	
        //select HDMI video format.
        SelectHDMIVideoFormat(&SinkCaps);

        //HDMI cable is connected and EDID is valid.
        if (SinkCaps.sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(targetRes)))
        {
            //mute HDMI output
            MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_HDMI, 0);
            MV_OSAL_Task_Sleep(100);

            //box resolution is supported by HDMI receiver. update box resolution.
            hr = MV_PE_VOutSetResolutionBDEx(gPe, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, targetRes, HdmiCfg.ucHdmiBitDepth, NULL);

            if (targetRes >= MV_PE_VOUT_FORMAT_1920_1080_P_60)
            {
                // set component output to 1080I
                hr = MV_PE_VOutSetInput(gPe, MV_PE_OUTPUT_VIDEO_PRIM, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, MV_PE_VOUT_INTERLACED);
                hr = MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_PRIM, TRUE);
            }
            else if ((targetRes >= MV_PE_VOUT_FORMAT_1920_1080_P_30 && targetRes <= MV_PE_VOUT_FORMAT_1920_1080_P_2398) ||
                (targetRes >= MV_PE_VOUT_FORMAT_1280_720_P_30 && targetRes <= MV_PE_VOUT_FORMAT_1280_720_P_25))
            {
                // mute component output
                hr = MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_PRIM, FALSE);
            }
            else
            {
                hr = MV_PE_VOutSetInput(gPe, MV_PE_OUTPUT_VIDEO_PRIM, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, MV_PE_VOUT_AUTO_SELECT);
                hr = MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_PRIM, TRUE);
            }

            BoxResID = targetRes;

            DEBUG_LOG("output resolution is %d!\n", BoxResID);

            /* select pixel repetition */
            GetMaxPixelReptByResolution(&SinkCaps, BoxResID, &HdmiCfg.ucHdmiPixelRep);
            DEBUG_LOG("pixel repetition is %d!\n", HdmiCfg.ucHdmiPixelRep);
			
            /* set HDMI Video format */
            MV_PE_VOutHDMISetVideoFormat(gPe, MV_PE_OUTPUT_VIDEO_PRIM, 
                HdmiCfg.ucHdmiColorFormat, HdmiCfg.ucHdmiBitDepth, HdmiCfg.ucHdmiPixelRep);
            DEBUG_LOG("color format is %d!\n", HdmiCfg.ucHdmiColorFormat);
            DEBUG_LOG("bit depth is %d!\n", HdmiCfg.ucHdmiBitDepth);

            //set HDMI audio format.
            MV_PE_AOutSetHDMIFormat(gPe, MV_PE_OUTPUT_AUDIO_PATH_HDMI, MV_PE_AOUT_HDMI_FORMAT_MULTI);

            MV_PE_VOutHDMISetAudioFormat(gPe, MV_PE_OUTPUT_VIDEO_PRIM, BoxResID, HdmiCfg.ucHdmiPixelRep, NULL);
        }else{
            printf("\nThis resolution is not supported by the connected device\n");
        }
    }
    else
    {
        // HDMI cable is not connected or EDID is not valid
        printf("\nHDMI cable is not connected or EDID is not valid\n");
    }

    return hr;
}


/* HDMI message handling function */
static void ProcessHDMIMessage(UINT32 HdmiMsgID)
{
    MV_PE_VPP_HDMI_SINK_CAPS SinkCaps;
    HRESULT hr;

    switch (HdmiMsgID) {
        case MV_PE_VPP_EVENT_HDMI_SINK_CONNECTED:
            /* set HDMI output resolution and AV format */
            printf("\n<<<<<HDMI cable is connected!!\n");

            //get sink capability (EDID)
            MV_PE_VOutHDMIGetSinkCaps(gPe, MV_PE_CHANNEL_PRIMARY_VIDEO, &SinkCaps);

            if (SinkCaps.validEdid)
            {
                //get pixel repetition value
                SelectHDMIVideoFormat(&SinkCaps);

                //mute HDMI output
                MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_HDMI, 0);
                MV_OSAL_Task_Sleep(100);

                //HDMI cable is connected and EDID && BoxResID are valild.
                if (RES_INVALID != BoxResID && (SinkCaps.sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(BoxResID))) )
                {
                    //box resolution is supported by HDMI receiver.
                    DEBUG_LOG("box resolution is supported!");
                    hr  = MV_PE_VOutSetResolutionBDEx(gPe, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, BoxResID, HdmiCfg.ucHdmiBitDepth, NULL);
		  }
                else
                {
                    //Box resolution is not supported by HDMI receiver.
                    INT hdmiRes;
                    //select a preferred resolution or maximum supported resolution for receiver.
                    GetPreferredResolution(&SinkCaps, &hdmiRes);
                    if (hdmiRes == RES_INVALID)
                    {
                        /* HDMI reeiver preferred resolution is not support by player.
                        *  try to get maximum resolution supported by both player and receiver.
                        */
                        GetMaxResolution(&SinkCaps, &hdmiRes);
                    }

                    if (hdmiRes == RES_INVALID)
                        hdmiRes =  MV_PE_VOUT_FORMAT_720_480_P_5994;

                    //set HDMI/Component resolution.
                    hr = MV_PE_VOutSetResolutionBDEx(gPe, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, hdmiRes, HdmiCfg.ucHdmiBitDepth, NULL);

                    //record the selected resolution as box resolution
                    BoxResID = hdmiRes;
                }

                DEBUG_LOG("output resolution is %d!", BoxResID);

                /* select pixel repetition */
                GetMaxPixelReptByResolution(&SinkCaps, BoxResID, &HdmiCfg.ucHdmiPixelRep);
                DEBUG_LOG("pixel repetition is %d!", HdmiCfg.ucHdmiPixelRep);
			
                /* set HDMI Video format */
                MV_PE_VOutHDMISetVideoFormat(gPe, MV_PE_OUTPUT_VIDEO_PRIM, 
                    HdmiCfg.ucHdmiColorFormat, HdmiCfg.ucHdmiBitDepth, HdmiCfg.ucHdmiPixelRep);
                DEBUG_LOG("color format is %d!", HdmiCfg.ucHdmiColorFormat);
                DEBUG_LOG("bit depth is %d!\n", HdmiCfg.ucHdmiBitDepth);

                //set HDMI audio format.
                MV_PE_AOutSetHDMIFormat(gPe, MV_PE_OUTPUT_AUDIO_PATH_HDMI, MV_PE_AOUT_HDMI_FORMAT_MULTI);

                MV_PE_VOutHDMISetAudioFormat(gPe, MV_PE_OUTPUT_VIDEO_PRIM, BoxResID, HdmiCfg.ucHdmiPixelRep, NULL);
                MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_HDMI, 1);
            }
            break;

        case MV_PE_VPP_EVENT_HDMI_SINK_DISCONNECTED:
	     printf("\n>>>>>HDMI cable is dis-connected!!\n");
            break;

        default:
            break;
    }
}

 /* HDMI message handling task */
static VOID * HdmiCecTaskProc (VOID *arg)
{
    HRESULT ret;
    UINT32 HdmiMsgID;

    do {
        /* try get/handle HDMI message */
        do {
            ret = MV_CC_MsgQEx_TryGetMsg(s_HdmiMsgQ, &HdmiMsgID);
            if (ret == S_OK)
                ProcessHDMIMessage(HdmiMsgID);
        } while (ret == S_OK);

        MV_OSAL_Task_Sleep(100);

    } while (1);
    return NULL;
}

int InitHdmiServe()
{	
	HRESULT rc;
	MV_PE_VPP_HDMI_SINK_CAPS SinkCaps = {0};
	
	MV_PE_ShowLogo(gPe, TRUE);
        
        /* Create HDMI message queue */
        rc = MV_CC_MsgQEx_Create(&s_HdmiMsgQ, MV_CC_MsgQType_ExITC, 0, HDMI_MSGQ_SIZE, sizeof(INT)+10);
        
        /* Create HDMI message handling task */
        MV_OSAL_Task_Create(&s_HdmiCecTask, HdmiCecTaskProc, NULL);
        
        /* load HDCP keys */
        rc = MV_PE_VOutHDMILoadHDCPKeys(gPe, MV_PE_OUTPUT_VIDEO_PRIM);
        
        if (rc == S_OK)
        {
        	/* enable HDCP */
        	DEBUG_LOG("Enable HDCP!\n");
        	MV_PE_VOutHDMISetHDCP(gPe, MV_PE_OUTPUT_VIDEO_PRIM, TRUE);
        }
        else
        {
        	/* disable HDCP */
        	DEBUG_LOG("Disable HDCP!\n");
        	MV_PE_VOutHDMISetHDCP(gPe, MV_PE_OUTPUT_VIDEO_PRIM, FALSE);
        }
        
        MV_PE_RegisterEventCallBack(gPe, MV_PE_EVENT_VPP_HDMI, HdmiCecEvtHandler, 0, 0);
        
        //get sink capability (EDID)
        MV_PE_VOutHDMIGetSinkCaps(gPe, MV_PE_CHANNEL_PRIMARY_VIDEO, &SinkCaps);
        
        if (SinkCaps.validEdid)
        {
        	DEBUG_LOG("EDID is valid!\n");
        	//get pixel repetition value
        	SelectHDMIVideoFormat(&SinkCaps);
        
        	//mute HDMI output
        	MV_PE_VOutSetEnable(gPe, MV_PE_OUTPUT_VIDEO_HDMI, 0);
        	MV_OSAL_Task_Sleep(100);
        
        	//HDMI cable is connected and EDID && BoxResID are valild.
        	if (RES_INVALID != BoxResID && (SinkCaps.sprtedDispRes & (0x1 << PE_RES_TO_VPP_RES(BoxResID))) )
        	{
        		//box resolution is supported by HDMI receiver.
        		DEBUG_LOG("box resolution is supported!\n");
        		rc = MV_PE_VOutSetResolutionBDEx(gPe, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, BoxResID, HdmiCfg.ucHdmiBitDepth, NULL);
        	}
        	else
        	{
        		//Box resolution is not supported by HDMI receiver.
        		DEBUG_LOG("box resolution is not supported!\n");
        		INT hdmiRes;
        		//select a preferred resolution or maximum supported resolution for receiver.
        		GetPreferredResolution(&SinkCaps, &hdmiRes);
        		if (hdmiRes == RES_INVALID)
        		{
        			/* HDMI reeiver preferred resolution is not support by player.
        			*  try to get maximum resolution supported by both player and receiver.
        			*/
        			GetMaxResolution(&SinkCaps, &hdmiRes);
        		}
        
        		if (hdmiRes == RES_INVALID)
        		hdmiRes =  MV_PE_VOUT_FORMAT_720_480_P_5994;
        
        		//set HDMI/Component resolution.
        		rc = MV_PE_VOutSetResolutionBDEx(gPe, MV_PE_CPCB_OUTPUT_VIDEO_PRIM, hdmiRes, HdmiCfg.ucHdmiBitDepth, NULL);
        
        		//record the selected resolution as box resolution
        		BoxResID = hdmiRes;
        	}
        
        	DEBUG_LOG("output resolution is %d!\n", BoxResID);
        	
        	/* select pixel repetition */
        	GetMaxPixelReptByResolution(&SinkCaps, BoxResID, &HdmiCfg.ucHdmiPixelRep);
        	DEBUG_LOG("pixel repetition is %d!\n", HdmiCfg.ucHdmiPixelRep);
        		
        	/* set HDMI Video format */
        	MV_PE_VOutHDMISetVideoFormat(gPe, MV_PE_OUTPUT_VIDEO_PRIM, 
        	HdmiCfg.ucHdmiColorFormat, HdmiCfg.ucHdmiBitDepth, HdmiCfg.ucHdmiPixelRep);
        	DEBUG_LOG("color format is %d!\n", HdmiCfg.ucHdmiColorFormat);
        	DEBUG_LOG("bit depth is %d!\n", HdmiCfg.ucHdmiBitDepth);
        
        	//set HDMI audio format.
        	MV_PE_AOutSetHDMIFormat(gPe, MV_PE_OUTPUT_AUDIO_PATH_HDMI, MV_PE_AOUT_HDMI_FORMAT_MULTI);
        
        	MV_PE_VOutHDMISetAudioFormat(gPe, MV_PE_OUTPUT_VIDEO_PRIM, BoxResID, HdmiCfg.ucHdmiPixelRep, NULL);

	}

}

int cmd_handler_help_setres(int argc, char * argv [ ])
{
	ShowHelp();
	return 0;
}

int cmd_handler_switch_setres(int argc, char * argv [ ])
{
	HRESULT rc;
	
	if (argc == 2)
		{
			if (argv[1][0] != '-')
				{
					printf("Invalid arguments\n");
					rc = -1;
				}
			SetResolutionbyKey(argv[1][1]);
			rc = 0;
		}

	else
		{
			printf("Invalid arguments\n");
			rc = -1;

		}			

	return rc;
}

static VOID SetResolutionbyKey(char key)
{
    switch (key)
    {
        case 'a':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_NTSC_M);
            break;
        case 'b':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_NTSC_J);
            break;
        case 'c':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_PAL_M);
            break;
        case 'd':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_PAL_BGH);
            break;
        case 'e':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_720_480_I_60);
            break;
        case 'f':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_720_480_I_5994);
            break;
        case 'g':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_720_576_I_50);
            break;
        case 'h':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_720_480_P_60);
            break;
        case 'i':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_720_480_P_5994);
            break;
        case 'j':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_720_576_P_50);
            break;
        case 'k':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1280_720_P_2997);
            break;
        case 'l':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1280_720_P_25);
            break;
        case 'm':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1280_720_P_60);
            break;
        case 'n':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1280_720_P_5994);
            break;
        case 'o':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1280_720_P_50);
            break;			
        case 'p':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_I_60);
            break;
        case 'q':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_I_5994);
            break;
        case 'r':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_I_50);
            break;
        case 's':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_30);
            break;
        case 't':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_2997);
            break;
        case 'u':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_25);
            break;
        case 'v':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_24);
            break;
        case 'w':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_25);
            break;
        case 'x':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_2398);
            break;
        case 'y':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_60);
            break;
        case 'z':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_5994);
            break;
        case '1':
            SetResolutionbyID(MV_PE_VOUT_FORMAT_1920_1080_P_50);
            break;
        case 'H':
            ShowHelp();
            break;
        case '\n':
            break;
        default:
            printf("\nInvalid option!\n");
            break;
    }
}

static VOID ShowHelp()
{
    puts(
        "\n"
        "         Arguments::\n"
        "         help     	      Show keyboard to resolution mapping\n"		//-H
        "         quit     	      Exit the program gracefully\n"				//-Q
        "         switch -a       NTSC_M          \n"
        "         switch -b       NTSC_J          \n"
        "         switch -c       PAL_M           \n"
        "         switch -d       PAL_BGH         \n"
        "         switch -e       720x480I   60Hz    \n"
        "         switch -f       720x480I   59.94Hz  \n"
        "         switch -g       720x576I   50Hz    \n"
        "         switch -h       720x480P   60Hz    \n"
        "         switch -i       720x480P   59.94Hz  \n"
        "         switch -j       720x576P   50Hz    \n"
        "         switch -k       1280x720P  29.97Hz \n"
        "         switch -l       1280x720P  25Hz   \n"
        "         switch -m       1280x720P  60Hz   \n"
        "         switch -n       1280x720P  59.94Hz \n"
        "         switch -o       1280x720P  50Hz   \n"
        "         switch -p       1920x1080I 60Hz  \n"
        "         switch -q       1920x1080I 59.94Hz\n"
        "         switch -r       1920x1080I 50Hz  \n"
        "         switch -s       1920x1080P 30Hz  \n"
        "         switch -t       1920x1080P 29.97Hz\n"
        "         switch -u       1920x1080P 25Hz  \n"
        "         switch -v       1920x1080P 24Hz  \n"
        "         switch -w       1920x1080P 25Hz  \n"
        "         switch -x       1920x1080P 23.98Hz\n"
        "         switch -y       1920x1080P 60Hz  \n"
        "         switch -z       1920x1080P 59.94Hz\n"
        "         switch -1       1920x1080P 50Hz  \n"
        "\n"
        "         Notes:\n"
        "         This HDMI service code can be served as HDMI demostration and resolution setting utility\n\n"
        "         1. For HDMI demostration:\n"
        "             When the application is excuted without argument, it acts as fully HDMI service code, OSAL and PE will be initialized in the beginning, \
and services like automatically resolution selection, HDMI cable plug/un-plug handling and resolution setting from user will be provided. \
When the application is terminated with \"quit\", PE and OSAL will be destroyed. \n"
        "             Samples:\n"
        "                 #Enter demostration mode\n"
        "                 root@Galois:~# ./SetRes\n"
        "                 #Set HDMI output resolution to 1920X1080 60Hz\n"
        "                 HDMI_RES> -y\n"
        "                 #Set HDMI output resolution to 1280X720 60Hz\n"
        "                 HDMI_RES> -m\n"
        "                 #Exit the demostration\n"
        "                 HDMI_RES> quit\n"
        "         2. For resolution setting utility:\n"
        "             When the application is excuted with argument, it acts as a HDMI output resolution setting tool, and only takes care of the resolution setting from user. \
It exits automatically when the resolution setting is finished and can be used both in command line and scripts, Please be notified that \
 the resolution setting only take effect when the connected TV support that resolution.\n"
        "             Samples:\n"
        "                 #Enter utility mode, and set HDMI output resolution to 1920X1080 60Hz\n"
        "                 root@Galois:~# ./SetRes -y\n"
        "                 #Enter utility mode, and set HDMI output resolution to 1280X720 60Hz\n"
        "                 root@Galois:~# ./SetRes -m\n"
    );  
}       
 

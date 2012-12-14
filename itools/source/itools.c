#include "itools.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "com_type.h"
#include "ErrorCode.h"

#define MAX_CMD_LEN       80
#define MAX_ARG_CNT       10
#define PROMPT_ITOOLS            "ITOOLS:$ "
#define ITOOLS_REV         "1.0"
#define INVALID_ARGUMENT  printf("Invalid argument!\n")

HANDLE gPe;

static int GetCommand(char *pCmd, int iMaxSize);
static BOOL ParseCommand(char *argv[],char *pCmd);
static CMD_HANDLER* ParseCommandFromShell(char *argv[]);
static int ItoolsCtrlEntry(char *argv[]);
static int ItoolsCommandline(int argc, char *argv[]);

static PROMPT gPrompt[] = {
    {"itools", "ITOOLS:$ "},
    {"mvlog", "ITOOLS:$ =>MVLOG:$ "},
    {"capframe", "ITOOLS:$ =>CAPFRAME:$ "},
    {"vdmdump","ITOOLS:$ =>VDMDUMP:$ "},
    {"regctl","ITOOLS:$ =>REGCTL:$ "},
    {"setres", "ITOOLS:$ =>SETRES:$ "},
    {"setplane","ITOOLS:$ =>SETPLANE:$ "},
    {"tsp","ITOOLS:$ =>TSP-Figo:$ "},

};

static int iNumPormpt = sizeof(gPrompt)/sizeof(PROMPT);

static CMD_HANDLER gCmdHandler_itools[] = {
    {"help", cmd_handler_help_itools, "List all supported tools"},
    {"mvlog", cmd_handler_mvlog_itools, "mvlog tool"},
    {"vdmdump", cmd_handler_vdmdump_itools, "vdm_dump tool"},
    {"setres", cmd_handler_setres_itools, "set_res tool(set HDMI output resolution)"},
    {"capframe", cmd_handler_capframe_itools, "capframe tool"},
    {"setplane", cmd_handler_setplane_itools, "setplane tool"},
    {"regctl", cmd_handler_regctl_itools, "regctl tool"},
    {"tsp",cmd_handler_tsp_itools,"Implementation of a console for tsp_dtcm_parser Tool"},
    	//{"quit",NULL,"return to previous status"},

    //extension and alias
    // {"NULL", cmd_handler_NULL, ""},
};

static int iNumCmd_itools = sizeof(gCmdHandler_itools)/sizeof(CMD_HANDLER);

//note: iNumCmd no use now, since it is a variation and can't initialize static(extern) variation
extern CMD_HANDLER gCmdHandler_mvlog[];   //wrong for CMD_HANDLER *gCmdHandler_mvlog (ÖØÉùÃ÷)
//extern int iNumCmd_mvlog;

extern CMD_HANDLER gCmdHandler_capframe[];
//extern int iNumCmd_capframe;

extern CMD_HANDLER gCmdHandler_vdmdump[];
//extern int iNumCmd_vdmdump;

extern CMD_HANDLER gCmdHandler_regctl[];
//extern int iNumCmd_regctl;

extern CMD_HANDLER gCmdHandler_setres[];
//extern int iNumCmd_setres;

extern CMD_HANDLER gCmdHandler_setplane[];
//extern int iNumCmd_setplane;

extern CMD_HANDLER gCmdHandler_tsp[];

static TOOL_CMD_HANDLERs gToolCmdHandler[]={
    {"mvlog", gCmdHandler_mvlog, 16 },    //iNumCmd_mvlog 16 ; 
    //try sizeof(gCmdHandler_mvlog)/sizeof(CMD_HANDLER) failed, since using extern CMD_HANDLER gCmdHandler_mvlog[], gCmdHandler_mvlog is a variable
    {"itools", gCmdHandler_itools, 7 },     //iNumCmd_itools 7
    {"capframe", gCmdHandler_capframe, 7 },  //7
    {"vdmdump", gCmdHandler_vdmdump, 3},	//3
    {"regctl", gCmdHandler_regctl, 4},  //3 
    {"setres", gCmdHandler_setres, 2},  //2
    {"setplane", gCmdHandler_setplane, 2 },
    {"tsp",gCmdHandler_tsp,6},
    
};

static int iNumToolCmd = sizeof(gToolCmdHandler)/sizeof(TOOL_CMD_HANDLERs);

static int INIT()
{
  	HRESULT hr;

  #ifndef WIN32
    	MV_OSAL_Init();
   	 if ((hr = MV_PE_Init(&gPe)) != S_OK)
   	 {
        		printf("PE Initialize failed! hr = 0x%x\n", hr);
        		MV_PE_Remove(gPe);
        		MV_OSAL_Exit();
       		 return -1;
    	}
	
  #else
    		MV_Debug_Initialize(DBG_ERROR);

 #endif    

     	return 0;
}

static void EXIT()
{ 
  #ifndef WIN32
    MV_PE_Remove(gPe);
    MV_OSAL_Exit();

  #endif
}


static int cmd_handler_help_itools(int argc, char *argv[])
{
    int i;
    printf("Marvell Galois Integrated Tools\n\n");
    printf("  Version: %s\n", ITOOLS_REV);
    //printf("  Author: \n");
    printf("  Usage: 1. itools tool_name command [arg1] [arg2] ...\n\n");
    printf("         2. itools; \n             tool_name command [arg1] [arg2] ...\n\n");
    printf("         3. itools; \n             tool_name; \n		  command [arg1] [arg2] ...\n\n");
    printf("	All tools contain the unified commands \"help\" and \"quit\"\n\n");
    printf("	All integrated tools:\n\n");
    for (i = 0; i < iNumCmd_itools; i++)
    {
   	printf("         %-12s- %s\n", gCmdHandler_itools[i].pCmd, gCmdHandler_itools[i].pHelp);
    }
    printf("\n");
    return 0;
}

static int cmd_handler_mvlog_itools(int argc, char *argv[])
{   
    printf("***********************************************\n");
    printf("*       MARVELL Galois PE Debug Control       *\n");
    printf("***********************************************\n\n");
	
    if (argc >= 2)
        ItoolsCommandline(argc, argv);
    else
        ItoolsCtrlEntry(argv);

    return 0;

}

static int cmd_handler_capframe_itools(int argc,char * argv [])
{
	printf("*****************************************************\n");
    	printf("*  Marvell Galois Capture Frame from VIP  *\n");
   	 printf("******************************************************\n\n");
	if(Init_capframe() == -1)
	{
		return -1;
	}
	
	if (argc >= 2)
        		ItoolsCommandline(argc, argv);
    	else
        		ItoolsCtrlEntry(argv);

    	return 0;

}

static int cmd_handler_vdmdump_itools(int argc, char * argv [ ])
{
	printf("***********************************************\n");
    printf("*       MARVELL Galois VDM Dump Control       *\n");
    printf("***********************************************\n\n");
		
	if (argc >= 2)
        		ItoolsCommandline(argc, argv);
    	else
        		ItoolsCtrlEntry(argv);

    	return 0;
}

static int cmd_handler_regctl_itools(int argc, char * argv [ ])
{
	printf("***********************************************\n");
    printf("*	Berlin Register Control	*\n");
    printf("***********************************************\n\n");
		
	if (argc >= 2)
        		ItoolsCommandline(argc, argv);
    	else
        		ItoolsCtrlEntry(argv);

    	return 0;

}

static int cmd_handler_setres_itools(int argc, char * argv [ ])
{
	printf("***********************************************\n");
    printf("*	Berlin HDMI Service Sample Code	*\n");
    printf("***********************************************\n\n");

	if (argc >= 2)
        		ItoolsCommandline(argc, argv);
    	else
    	{			
		InitHdmiServe();  	//without arguments,supply full HDMI service
		ItoolsCtrlEntry(argv);
    	}

    	return 0;
}

static int cmd_handler_setplane_itools(int argc, char * argv [ ])
{
	printf("***********************************************\n");
    printf("*      Set VPP Plane Status  *\n");
    printf("***********************************************\n\n");
		
	if (argc >= 2)
        ItoolsCommandline(argc, argv);
    else
        ItoolsCtrlEntry(argv);

    return 0;
}


static int cmd_handler_tsp_itools(int argc,char * argv [ ])
{
    printf("***********************************************\n");
    printf("*            TSP_DTCMParserEntry              *\n");
    printf("***********************************************\n");

    if(InitTsp())
        return -1;

    if (argc >= 2)
        ItoolsCommandline(argc, argv);
    else
        ItoolsCtrlEntry(argv);
    
    ExitTsp();
    return 0;
}

static BOOL ParseCommand(char *argv[],char *pCmd)
{
    char *pArg[MAX_ARG_CNT];
    int argc = 0;
    BOOL bValid = FALSE;
    int i;

    while (*pCmd == 0x20) pCmd++;
    
    if (strlen(pCmd) == 0)
        return TRUE; 

    if (strcmp(pCmd, "quit") == 0)
        return FALSE;

    pArg[argc] = *argv;
    argc++;

    pArg[argc] = pCmd;
    argc++;
    while(*pCmd != 0)
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
    
    CMD_HANDLER *pCmdHandler = 0;
    int iNumCmd = 0;
    int j;
    for (j = 0; j< iNumToolCmd; j++)
    {   
        //printf("ParseCommand: %s, %s\n",pArg[0],gToolCmdHandler[j].pTool);
        if (strcmp(pArg[0], gToolCmdHandler[j].pTool) == 0)
        {   
            pCmdHandler = gToolCmdHandler[j].pCmdHandler;
            iNumCmd = gToolCmdHandler[j].iNumCmd;

        }

    }

    for (i = 0; i < iNumCmd; i++)
    {
       // printf("%s,	%s\n",pArg[1],pCmdHandler[i].pCmd);
        if (strcmp(pArg[1], pCmdHandler[i].pCmd) == 0)
        {
            bValid = TRUE;
            /*
            int j;
            printf("paramater in:\n");
            for (j = 1; j <=argc-1; j++)
            {
                printf("%s\n",pArg[j]);
            }
            */
            pCmdHandler[i].Handler(argc-1, &pArg[1]);
        }
    }

    if (!bValid)
    {
        printf("Invalid command! Type help to see all supported command!\n");
    }

    /*for (i = 0; i < argc; i++)
    {
        printf("arg%d = %s\n", i, pArg[i]);
    }*/

    return TRUE;
}

static int GetCommand(char *pCmd, int iMaxSize)
{
    int count;
    char ch;

    if ((pCmd == 0) || (iMaxSize == 0))
    {
        return 0;
    }

    count = 0;
    iMaxSize--;
    while((ch = getchar()) != 0x0a)
    {
        if (count < iMaxSize)
        {
            pCmd[count++] = ch;
        }
    }

    pCmd[count] = '\0';

    return count;
}

int ItoolsCtrlEntry(char *argv[])
{
    HRESULT rc = 0;
    char    Cmd[MAX_CMD_LEN];
    int len;
	
    do
    {   
        int i;
        for(i = 0; i<iNumPormpt; i++)
        {
            if(strcmp(*argv,gPrompt[i].pTool) == 0)
                printf(gPrompt[i].pPrompt);
        }
        len = GetCommand(Cmd, MAX_CMD_LEN);
        //printf("Command = %s, len = %d\n", Cmd, len);
    }
    while (ParseCommand(argv,Cmd));
	return rc;
}


static CMD_HANDLER* ParseCommandFromShell(char *argv[]) //char *pCmd
{
    int i;
    char *pCmd = argv[0];

    while (*pCmd == 0x20) pCmd++;
    while (*argv[1] == 0x20) argv[1]++;

    if (strlen(pCmd) == 0)
        return TRUE;   //make clear

    CMD_HANDLER *pCmdHandler = 0;
    int iNumCmd = 0;

    int j;
    for (j = 0; j< iNumToolCmd; j++)
    {
        //printf("From Shell: %s, %s\n",pCmd,gToolCmdHandler[j].pTool);
        if (strcmp(pCmd, gToolCmdHandler[j].pTool) == 0)
        {  
            pCmdHandler = gToolCmdHandler[j].pCmdHandler;
            iNumCmd = gToolCmdHandler[j].iNumCmd;
            
            for (i = 0; i < iNumCmd; i++)
            {   
                //printf("%s,  %s\n",(argv[1]), pCmdHandler[i].pCmd);
                if (strcmp(argv[1], pCmdHandler[i].pCmd) == 0)    //pCmd+1 ->argv[1]
                {
                    return &pCmdHandler[i];
                }
            }


        }
        
    }


    printf("Invalid command! Type help to see all supported command!\n");

    return NULL;
}

int ItoolsCommandline(int argc, char *argv[])
{
    HRESULT rc = 0;
    CMD_HANDLER *pCmdHandler = 0;

    pCmdHandler = ParseCommandFromShell(&argv[0]);   //argv[1] ->[0]
    if (pCmdHandler)
    {
        pCmdHandler->Handler(argc-1, &argv[1]);
    }

    /*
    else
    {
        cmd_handler_help_itools(argc, argv);
        rc = E_INVALIDARG;
    }
    */
    return rc;
}

/*
static void debug_signal_handler()
{
    printf("please use \"quit\"\n");
}

*/
int main(int argc, char *argv[])
{
    //signal(SIGINT, debug_signal_handler);
    printf("***********************************************\n");
    printf("*       MARVELL Galois Itools  *\n");
    printf("***********************************************\n");
	
    if(INIT() == -1)
    {
        return -1;
    }

    if(strcmp(argv[0], "itools") != 0)
    {
    	argv[0] = "itools";
    }
	
    if (argc >= 2)
        ItoolsCommandline(argc, argv);
    else
        ItoolsCtrlEntry(argv);

    EXIT();
    return 0;
}

#ifndef _TYPE_H_
#define _TYPE_H_

typedef int (*CMDFUNC)(int argc, char *argv[]);

typedef struct tagPrompt
{
	char *pTool;
	char *pPrompt;
}PROMPT;

typedef struct tagCmdHandler
{ 
	char *pCmd;
	CMDFUNC Handler;
	char *pHelp;
} CMD_HANDLER;


typedef struct tagToolCmdHandler
{
    char * pTool;
    CMD_HANDLER *pCmdHandler;
    int iNumCmd;

}TOOL_CMD_HANDLER;


#endif


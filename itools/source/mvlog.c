#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "mvlog.h"
#include "galois_debug.h"
#ifndef WIN32
#include "pe_api.h"
#include "pe_debug.h"
#endif


#define MAX_CMD_LEN       80
#define MAX_ARG_CNT       10
#define MVLOG_REV         "1.2"
#define INVALID_ARGUMENT  printf("Invalid argument!\n")

CMD_HANDLER gCmdHandler_mvlog[] = {
    {"help", cmd_handler_help_mvlog, "List all supported commands"},
    {"log", cmd_handler_log_mvlog, "Turn on/off the whole log"},
    {"levup", cmd_handler_levup_mvlog, "Increase debug level of all modules"},
    {"levdown", cmd_handler_levdown_mvlog, "Decrease debug level of all modules"},
    {"getmodname", cmd_handler_getmodname_mvlog, "Show the module name by ID"},
    {"setmodname", cmd_handler_setmodname_mvlog, "Set the module name by ID"},
    {"getmodl", cmd_handler_getmodl_mvlog, "Get debug level of specified module"},
    {"setmodl", cmd_handler_setmodl_mvlog, "Set debug level of specified module"},
    {"enmodl", cmd_handler_enablemodl_mvlog, "Enable a specified level for a module"},
    {"dismodl", cmd_handler_disablemodl_mvlog, "Disable a specified level for a module"},
    {"lsmod", cmd_handler_lsmod_mvlog, "List all modules and its current debug level"},
    {"pelog", cmd_handler_pelog_mvlog, "Show PE detailed source/stream/avsync information."},
    {"cmod", cmd_handler_cmod_mvlog, "Set current module which is operated upon by cmd + and -"},
    {"+", cmd_handler_modlevup_mvlog, "Enable the highest level among the levels that are not enabled"},
    {"-", cmd_handler_modlevdown_mvlog, "Disable the lowest level among the levels that are enabled"},
    {"status", cmd_handler_status_mvlog, "Show running status of each modules (dev)"},
};

int iNumCmd_mvlog = sizeof(gCmdHandler_mvlog)/sizeof(CMD_HANDLER);

extern HANDLE gPe;
static unsigned int gCurrentMod;
static char *levstr[] = {
    "FATAL",
    "ERROR",
    "HIGH",
    "MID",
    "LOW",
    "INFO",
    "USER1",
    "USER2",
};

int cmd_handler_help_mvlog(int argc, char *argv[]);
static int send_debug_control_msg(int cmd, int arg1, int arg2, char *pMod);

static BOOL get_level_from_string(char *pLevStr, int *pLev)
{
    int i;

    if ((pLevStr != 0) && (pLev != 0))
    {
        for (i = 0; i < DBG_LEVEL_MAX; i++)
        {
            if (strcmp(levstr[i], pLevStr) == 0)
            {
                *pLev = i;
                return TRUE;
            }
        }

        if ((i = atoi(pLevStr)) > 0)
        {
            *pLev = i;
            return TRUE;
        }

        if (strcmp(pLevStr, "0") == 0)
        {
            *pLev = 0;
            return TRUE;
        }
    }

    return FALSE;
}

static char *get_modname_from_arg(char *pArg)
{
    static char modname[16];

    if (pArg != 0)
    {
        int i;
        modname[0] = 0;
        if (((i = atoi(pArg)) > 0) && (i < MOD_MAX))
        {
            send_debug_control_msg(CMD_GET_MODULE_NAME, i, 0, modname); 
            return modname;
        }

        if (strcmp(pArg, "0") == 0)
        {
            send_debug_control_msg(CMD_GET_MODULE_NAME, 0, 0, modname); 
            return modname;
        }

        return pArg;

    }

    return 0;
}


static int send_debug_control_msg(int cmd, int arg1, int arg2, char *pMod)
{
    HRESULT hr;
#ifndef WIN32
    MV_PE_DEBUG_INPUT in;
    MV_PE_DEBUG_OUTPUT out;

    in.Arg[0] = cmd;
    in.Arg[1] = arg1;
    in.Arg[2] = arg2;

    in.InModule = FALSE;
    in.OutModule = FALSE;

    if (pMod)
    {
        int len = strlen(pMod);
        if (len > 15)
        {
            len = 15;
        }
        memcpy(in.Module, pMod, len);
        in.Module[len] = '\0';
        if (cmd != CMD_GET_MODULE_NAME)
            in.InModule = TRUE;
        else
            in.OutModule = TRUE;
    }
    hr = MV_PE_DebugControl(gPe, MV_PE_MODULE_DBG_CTRL, &in, &out);
    //printf("out.arg=0x%x\n", out.Arg);
    if (hr == S_OK)
    {
        if (arg1 && ((cmd == CMD_DEBUG_LEVEL_UP) ||
            (cmd == CMD_DEBUG_LEVEL_DOWN) ||
            (cmd == CMD_GET_DBG_LEVEL_BY_NAME) ||
            (cmd == CMD_GET_NUM_MODULES)))
        {
            *(UINT*)arg1 = out.Arg;
        }
        else if (arg2 && (cmd == CMD_GET_DBG_LEVEL))
        {
            *(UINT*)arg2 = out.Arg;
        }
        else if (cmd == CMD_GET_MODULE_NAME)
        {
            strcpy(pMod, out.Module);
        }
    }
    else
    {
        printf("Operation Failed!, it seems debug is not enabled in PE side"
            " or wrong parameter is passed.\n");
    }
#else
    hr = MV_Debug_Control((DBG_CMD)cmd, arg1, arg2, pMod);
    if (hr != S_OK)
    {
        printf("Operation Failed!, it seems debug is not enabled in PE side"
            " or wrong parameter is passed.\n");
    }
#endif
    return hr;
}

int cmd_handler_log_mvlog(int argc, char *argv[])
{
    int enabled = -1;

    if (argc > 1)
    {
        if (strcmp(argv[1], "enable") == 0)
            enabled = 1;
        else if (strcmp(argv[1], "disable") == 0)
            enabled = 0;
        else if (strcmp(argv[1], "on") == 0)
            enabled = 1;
        else if (strcmp(argv[1], "off") == 0)
            enabled = 0;
        else if (strcmp(argv[1], "1") == 0)
            enabled = 1;
        else if (strcmp(argv[1], "0") == 0)
            enabled = 0;
    }

    if (enabled >= 0)
    {
        if (send_debug_control_msg(CMD_DEBUG_ENABLE, enabled, 0, 0) == S_OK)
        {
            printf("LOG %s\n", enabled ? "Enabled" : "Disabled");
        }
    }
    else
    {
        INVALID_ARGUMENT;
    }

    return 0;
}

int cmd_handler_levup_mvlog(int argc, char *argv[])
{
    send_debug_control_msg(CMD_DEBUG_LEVEL_UP, 0, 0, 0);
    return 0;
}

int cmd_handler_levdown_mvlog(int argc, char *argv[])
{
    send_debug_control_msg(CMD_DEBUG_LEVEL_DOWN, 0, 0, 0);
    return 0;
}

int cmd_handler_getmodname_mvlog(int argc, char *argv[])
{
    char modname[MAX_MODULE_NAME_LEN + 1];

    if (argc > 1)
    {
        if (S_OK == send_debug_control_msg(CMD_GET_MODULE_NAME, atoi(argv[1]),
            0, modname))
        {
            printf("The name of module[%d] is %s\n", atoi(argv[1]), modname);
        }
    }
    else
    {
        INVALID_ARGUMENT;
    }

    return 0;
}

int cmd_handler_setmodname_mvlog(int argc, char *argv[])
{
    if (argc > 2)
    {
        if (S_OK == send_debug_control_msg(CMD_SET_MODULE_NAME, atoi(argv[1]),
            0, argv[2]))
        {
            printf("The name of module[%d] is set to %s\n", atoi(argv[1]), argv[2]);
        }
    }
    else
    {
        INVALID_ARGUMENT;
    }

    return 0;
}

int cmd_handler_getmodl_mvlog(int argc, char *argv[])
{
    unsigned int level = 0;
    unsigned int mod = 0;

    if (argc > 1)
    {
        if (S_OK == send_debug_control_msg(CMD_GET_DBG_LEVEL_BY_NAME,
            (UINT)&level, 0, get_modname_from_arg(argv[1])))
        {
            printf("The level of module [%s] is 0x%02x\n", get_modname_from_arg(argv[1]), level);
            return 0;
        }
    }

    INVALID_ARGUMENT;
    return 0;
}

int cmd_handler_setmodl_mvlog(int argc, char *argv[])
{
    if (argc > 2)
    {
        if (atoi(argv[2]) != 0 || (strcmp(argv[2], "0") == 0))
        {
            if (S_OK == send_debug_control_msg(CMD_SET_DBG_LEVEL_BY_NAME,
                (UINT)atoi(argv[2]), 0, get_modname_from_arg(argv[1])))
            {
                printf("The level of module [%s] is set to 0x%02x\n", get_modname_from_arg(argv[1]), atoi(argv[2]));
                return 0;
            }
        }
    }

    INVALID_ARGUMENT;
    return 0;
}

int cmd_handler_enablemodl_mvlog(int argc, char *argv[])
{
    if (argc > 2)
    {
        int lev = 0;
        if (get_level_from_string(argv[2], &lev))
        {
            if (S_OK == send_debug_control_msg(CMD_ENABLE_DBG_LEVEL_BY_NAME,
                (UINT)lev, 0, get_modname_from_arg(argv[1])))
            {
                printf("Level %s is enabled for module [%s]\n", levstr[lev], get_modname_from_arg(argv[1]));
                return 0;
            }
        }
    }

    INVALID_ARGUMENT;
    return 0;
}

int cmd_handler_disablemodl_mvlog(int argc, char *argv[])
{
    if (argc > 2)
    {
        int lev = 0;
        if (get_level_from_string(argv[2], &lev))
        {
            if (S_OK == send_debug_control_msg(CMD_DISABLE_DBG_LEVEL_BY_NAME,
                (UINT)lev, 0, get_modname_from_arg(argv[1])))
            {
                printf("Level %s is disabled for module [%s]\n", levstr[lev], get_modname_from_arg(argv[1]));
                return 0;
            }
        }
    }

    INVALID_ARGUMENT;
    return 0;
}

int cmd_handler_pelog_mvlog(int argc, char *argv[])
{
#ifndef WIN32
    MV_PE_DEBUG_INPUT in;
    MV_PE_DEBUG_OUTPUT out;

    in.InModule = FALSE;
    in.OutModule = FALSE;

    MV_PE_DebugControl(gPe, MV_PE_DBGCTRL_SHOWLOG, &in, &out);
#endif
    return 0;
}

int cmd_handler_status_mvlog(int argc, char *argv[])
{
    MV_PE_DEBUG_INPUT in = {0,};
    MV_PE_DEBUG_OUTPUT out = {0,};
    UINT i, total_mod = 0;
    HRESULT rc = S_OK;

    in.InModule = FALSE;
    in.OutModule = FALSE;

    rc = send_debug_control_msg(CMD_GET_NUM_MODULES, (UINT)&total_mod, 0, 0);

    // add each module status report
#ifdef MV_CONFIG_GENERIC_DEBUG_LOG
    if (argc == 1)
        for (i = 0; i < total_mod;i++)
        {
            in.InModule = TRUE;
            in.Arg[0] = i;
            MV_PE_DebugControl(gPe, MV_PE_DBGCTRL_SHOWLOG, &in, &out);
        }
    else
    {
        in.InModule = TRUE;
        in.Arg[0] = atoi(argv[1]);
        MV_PE_DebugControl(gPe, MV_PE_DBGCTRL_SHOWLOG, &in, &out);

    }
#else

    MV_PE_DebugControl(gPe, MV_PE_DBGCTRL_SHOWLOG, &in, &out);
    MV_PE_DebugControl(gPe, MV_PE_DBGCTRL_TSP_DUMPSTATUS, &in, &out);
#endif
    return 0;
}

int cmd_handler_lsmod_mvlog(int argc, char *argv[])
{
    INT  hr, hr1;
    UINT i, total_mod = 0;
    UINT level;
    char modname[MAX_MODULE_NAME_LEN + 1];

    hr = send_debug_control_msg(CMD_GET_NUM_MODULES, (UINT)&total_mod, 0, 0);
    if (hr == S_OK)
    {
        printf("MODULE ID   NAME             DEBUG LEVEL\n");
        printf("----------------------------------------\n");
        for (i = 0; i < total_mod; i++)
        {
            UINT k;
            modname[0] = 0;
            hr = send_debug_control_msg(CMD_GET_MODULE_NAME, i, 0, modname);
            hr1 = send_debug_control_msg(CMD_GET_DBG_LEVEL, i, (UINT)&level, 0);
            if ((hr != S_OK) || (hr1 != S_OK))
            {
                continue;
            }
            printf("%5d       %-12s    0x%02x (", i, modname, level);
            if (level == 0)
            {
                printf(" NONE");
            }
            else
            {
                for (k = 0; k < DBG_LEVEL_MAX; k++)
                {
                    if ((1 << k) & level)
                    {
                        printf(" %s", levstr[k]);
                    }
                }
            }
            printf(" )\n");
        }
    }

    return 0;
}

int cmd_handler_cmod_mvlog(int argc, char *argv[])
{
    INT  hr;
    UINT i, total_mod = 0;
    char modname[MAX_MODULE_NAME_LEN + 1];
    char *pMod;

    if (argc == 1)
    {
        hr = send_debug_control_msg(CMD_GET_MODULE_NAME, gCurrentMod, 0, modname);
        if (hr == S_OK)
        {
            printf("Current Module = %s\n", modname);
        }

        return 0;
    }
    else if (argc != 2)
    {
        INVALID_ARGUMENT;
        return 0;
    }

    hr = send_debug_control_msg(CMD_GET_NUM_MODULES, (UINT)&total_mod, 0, 0);
    if (hr == S_OK)
    {
        pMod = get_modname_from_arg(argv[1]);
        for (i = 0; i < total_mod; i++)
        {
            UINT k;
            modname[0] = 0;
            hr = send_debug_control_msg(CMD_GET_MODULE_NAME, i, 0, modname);
            if (strcmp(pMod, modname) == 0)
            {
                gCurrentMod = i;
                printf("Current module set to %d [%s]\n", i, pMod);
                break;
            }
        }

        if (i == total_mod)
        {
            printf("%s is an invalid module name or index!\n", argv[1]);
        }
    }

    return 0;
}

static int modlevupdown(char *pMod, BOOL bUp)
{
    INT  hr;
    UINT i;
    UINT level, new_lev = DBG_LEVEL_MAX;
    char *pModName;
    char modname[MAX_MODULE_NAME_LEN + 1];

    if (pMod)
    {
        pModName = get_modname_from_arg(pMod);
    }
    else
    {
        pModName = modname;
        hr = send_debug_control_msg(CMD_GET_MODULE_NAME, gCurrentMod, 0, modname);
        if (hr != S_OK)
        {
            return 0;
        }
    }

    hr = send_debug_control_msg(CMD_GET_DBG_LEVEL_BY_NAME, (UINT)&level, 0, pModName);
    if (hr != S_OK)
    {
        return 0;
    }

    if (bUp)
    {
        for (i = 0; i < DBG_LEVEL_MAX; i++)
        {
            if (((1 << i) & level) == 0)
            {
                break;
            }
        }
        new_lev = i;
    }
    else
    {
        for (i = DBG_LEVEL_MAX; i > 0; i--)
        {
            if (((1 << (i - 1)) & level) != 0)
            {
                break;
            }
        }

        if (i > 0)
        {
            new_lev = i - 1;
        }
    }

    if (new_lev < DBG_LEVEL_MAX)
    {
        hr = send_debug_control_msg(bUp ? CMD_ENABLE_DBG_LEVEL_BY_NAME : CMD_DISABLE_DBG_LEVEL_BY_NAME, (UINT)new_lev, 0, pModName);
        if (hr == S_OK)
        {
            printf("Level %s is %sABLED for module [%s]\n", levstr[new_lev], bUp ? "EN" : "DIS", pModName);
        }
    }
    else
    {
        printf("All levels of the module [%s] are %s\n", pModName, bUp ? "Enabled" : "Disabled");
    }

    return 0;
}

int cmd_handler_modlevup_mvlog(int argc, char *argv[])
{
    if (argc == 1)
    {
        modlevupdown(0, TRUE);
    }
    else if (argc == 2)
    {
        modlevupdown(argv[1], TRUE);
    }
    else
    {
        INVALID_ARGUMENT;
    }

    return 0;
}

int cmd_handler_modlevdown_mvlog(int argc, char *argv[])
{
    if (argc == 1)
    {
        modlevupdown(0, FALSE);
    }
    else if (argc == 2)
    {
        modlevupdown(argv[1], FALSE);
    }
    else
    {
        INVALID_ARGUMENT;
    }

    return 0;
}

int cmd_handler_help_mvlog(int argc, char *argv[])
{
    int i;
    printf("Marvell Galois PE Debug Control System\n\n");
    printf("  Revision: %s\n", MVLOG_REV);
    printf("  Author: Jun Ma <junma@marvell.com>\n");
    printf("  Usage: 1. mvlog command [arg1] [arg2] ...\n\n");
    printf("         2. mvlog; \n             command [arg1] [arg2] ...\n\n");
    printf("All supported commands:\n\n");
    for (i = 0; i < iNumCmd_mvlog; i++)
    {
        printf("         %-12s- %s\n", gCmdHandler_mvlog[i].pCmd, gCmdHandler_mvlog[i].pHelp);
    }
    printf("\nSupported debug level:\n");
    printf("  0 - FATAL, 1 - ERROR, 2 - HIGH,  3 - MID\n");
    printf("  4 - LOW,   5 - INFO,  6 - USER1, 7 - USER2\n\n");
    return 0;
}

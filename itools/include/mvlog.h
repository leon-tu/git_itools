#ifndef _MVLOG_H_
#define _MVLOG_H_

#include "itools_type.h"

int cmd_handler_help_mvlog(int argc, char *argv[]);
int cmd_handler_log_mvlog(int argc, char *argv[]);
int cmd_handler_levup_mvlog(int argc, char *argv[]);
int cmd_handler_levdown_mvlog(int argc, char *argv[]);
int cmd_handler_getmodname_mvlog(int argc, char *argv[]);
int cmd_handler_setmodname_mvlog(int argc, char *argv[]);
int cmd_handler_getmodl_mvlog(int argc, char *argv[]);
int cmd_handler_setmodl_mvlog(int argc, char *argv[]);
int cmd_handler_enablemodl_mvlog(int argc, char *argv[]);
int cmd_handler_disablemodl_mvlog(int argc, char *argv[]);
int cmd_handler_lsmod_mvlog(int argc, char *argv[]);
int cmd_handler_pelog_mvlog(int argc, char *argv[]);
int cmd_handler_cmod_mvlog(int argc, char *argv[]);
int cmd_handler_modlevup_mvlog(int argc, char *argv[]);
int cmd_handler_modlevdown_mvlog(int argc, char *argv[]);
int cmd_handler_status_mvlog(int argc, char *argv[]);

#endif

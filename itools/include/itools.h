#ifndef _ITOOLS_H_
#define _ITOOLS_H_

#include "mvlog.h"
#include "itools_type.h"
#include "vdmdump.h"
#include "regctl.h"
#include "setres.h"
#include "setplane.h"
#include "capframe.h"
#include "tsp_dtcm_parser.h"

static int cmd_handler_help_itools(int argc, char *argv[]);
static int cmd_handler_mvlog_itools(int argc, char *argv[]);
static int cmd_handler_vdmdump_itools(int argc, char *argv[]);
static int cmd_handler_setres_itools(int argc, char *argv[]);
static int cmd_handler_capframe_itools(int argc,char * argv []);
static int cmd_handler_setplane_itools(int argc,char * argv []);
static int cmd_handler_regctl_itools(int argc,char * argv []);
static int cmd_handler_tsp_itools(int argc,char * argv[]);

#endif

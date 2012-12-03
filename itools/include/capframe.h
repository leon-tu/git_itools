#ifndef _CAPFRAME_H_
#define _CAPFRAME_H_

#include "itools_type.h"

int cmd_handler_help_capframe(int argc,char * argv []);
int cmd_handler_setmode_capframe(int argc,char * argv []);
int cmd_handler_setwin_capframe(int argc,char * argv []);
int cmd_handler_setpath_capframe(int argc,char * argv []);
int cmd_handler_setnum_capframe(int argc,char * argv []);
int cmd_handler_getarg_capframe(int argc,char * argv []);
int cmd_handler_encap_capframe(int argc,char * argv []);
void init_cap_arg();	//init in capframe 


#endif
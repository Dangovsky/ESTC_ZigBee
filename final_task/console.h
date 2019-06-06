#ifndef CONSOLE_H
#define CONSOLE_H

#define CLEAR_LINE                                         \
    "\033[2K" /* ESC seq for clear entire line          */ \
    "\r"      /* ESC seq for move cursor at and of line */

#define _CMD_HELP "help"
#define _CMD_CLEAR "clear"
#define _CMD_IEEE_ADDR "ieee"
#define _CMD_ACTIVE_EP "ep"
#define _CMD_SIMPLE_DISC "simple"
#define _CMD_NEIGHBORS "neighbors"
#define _CMD_NWK_ADDR "nwk"
#define _CMD_LEAVE "leave"
#define _CMD_PERMIT_JOIN "permit_join"
#define _CMD_BEACON_REQ "beacon_req"
#define _CMD_DATA_REQ "send"

#define _NUM_OF_CMD 11

#include "./microrl/include/microrl.h"

/** 
 * Instance of a "micro readline" library.
 * Global to allow calls from cmd_handlers.c
 */
microrl_t microrl = {0};

/** 
 * "execute" function argument - num of words in command line.
 * Global to allow use of delayed get_out_buf and to call from cmd_handlers.c
 */
volatile int argc_g;

/**
 * "execute" function argument - words in command line.
 * Global to allow use of delayed get_out_buf and to call from cmd_handlers.c.
 *
 * More importantly it is allow to use delayed get_out_buf at the cost of memory. See "execute" function.
 */
char argv_g[_COMMAND_TOKEN_NMB][(zb_uint_t)((_COMMAND_LINE_LEN - 1) / _COMMAND_TOKEN_NMB + 1)];

void print(const char *str);

#endif /* CONSOLE_H */

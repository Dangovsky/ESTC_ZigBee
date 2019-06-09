#ifndef CONSOLE_H
#define CONSOLE_H

#include "./microrl/include/microrl.h"

/** used to correctly end command execution */
#define ON_RETURN \
    WRITE_PROMPT  \
    set_current_command(NULL);

/** used for clearing prompt message */
#define CLEAR_LINE                                \
    "\033[2K" /* ESC seq for clear entire line */ \
    "\r"

/** used for writing prompt message */
#define WRITE_PROMPT                            \
    print("\n\r");                              \
    print((get_current_microrl())->prompt_str); \
    (get_current_microrl())->cursor = 0;

/**
 Format for 64-bit address
 Duplicate ZB`s TRACE_FORMAT_64, cose it is defined for double addr
*/
#define FORMAT_64 "%hx.%hx.%hx.%hx.%hx.%hx.%hx.%hx"

/**
 Format arguments for 64-bit address
 Duplicate ZB`s TRACE_FORMAT_64, cose it is defined for double addr
*/
#define ARG_64(a) (zb_uint8_t)((a)[7]), (zb_uint8_t)((a)[6]), (zb_uint8_t)((a)[5]), (zb_uint8_t)((a)[4]), \
                  (zb_uint8_t)((a)[3]), (zb_uint8_t)((a)[2]), (zb_uint8_t)((a)[1]), (zb_uint8_t)((a)[0])

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

/** 
 * @brief Return instance of a "micro readline" library.
 * 
 * Global to allow calls from cmd_handlers.c
 */
microrl_t *get_current_microrl();

/** 
 * @brief Return "execute" function argument - num of words in command line.
 * 
 * Global to allow use of delayed get_out_buf and to call from cmd_handlers.c
 */
int get_current_argc();

/**
 * @brief Return "execute" function argument - i-th word in command line.
 *
 * Global to allow use of delayed get_out_buf and to call from cmd_handlers.c.
 * More importantly it is allow to use delayed get_out_buf at the cost of memory. See "execute" function.
 */
char *get_current_argv(zb_uint8_t i);

void set_current_command(char* command_name);

void print(const char *str);

#endif /* CONSOLE_H */

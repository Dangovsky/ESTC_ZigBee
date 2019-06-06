#ifndef CONSOLE_H
#define CONSOLE_H

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
 * More importantly it is allow to use delayed get_out_buf by cost of memory. See execute function.
 */
//char argv_g [_COMMAND_TOKEN_NMB][(zb_uint_t)((_COMMAND_LINE_LEN - 1) / _COMMAND_TOKEN_NMB + 1)];
char** argv_g;

void print(const char *str);

#endif /* CONSOLE_H */

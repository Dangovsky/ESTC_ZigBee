/** Delay before rx_buffer_flush call in milliseconds */
#define RX_DELAY 100

/** String size used to store one word from console */
#define WORD_LEN 20

/** Ring buffer length of zb_uint8_t */
#define RING_BUFFER_LENGTH 16
ZB_RING_BUFFER_DECLARE(ring_buffer, zb_uint8_t, RING_BUFFER_LENGTH);

/**
 * warning: actual baudrate will be in 1.5 times bigger.
 * @see BAUD_RATE_CORRECTION(b)
 */
#define BAUD_RATE 38200

/** Because of zb clock, baudrate must be in 1.5 times bigger then on pc */
#define BAUD_RATE_CORRECTION(b) (zb_uint16_t)((b) + (b) / 2)

/** duplicate ZB macro DISABLE_SERIAL_INTER, but with different USART */
#define CONSOLE_DISABLE_SERIAL_INTER() NVIC_DisableIRQ(USART2_IRQn)
/** duplicate ZB macro ENABLE_SERIAL_INTER, but with different USART */
#define CONSOLE_ENABLE_SERIAL_INTER() NVIC_EnableIRQ(USART2_IRQn)

/** duplicate ZB macro ENABLE_SERIAL_TR_INTER, but with different USART */
#define CONSOLE_ENABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, ENABLE)
/** duplicate ZB macro DISABLE_SERIAL_TR_INTER, but with different USART */
#define CONSOLE_DISABLE_SERIAL_TR_INTER() USART_ITConfig(USART2, USART_IT_TXE, DISABLE)

/** Static variables for console */
typedef struct console_context_s{
    char *variants[_NUM_OF_CMD + 1]; /*!< array for "complet" function */

    ring_buffer_t ring_buffer_tx; /*!< ring buffer for writing to usart */

    ring_buffer_t ring_buffer_rx; /*!< ring buffer for sending char to microrl */

    microrl_t microrl; /*!< Instance of a "micro readline" library. */

    volatile int current_argc; /*!< "execute" function argument - num of words in command line. */

    char current_argv[_COMMAND_TOKEN_NMB][WORD_LEN + 1]; /*!< "execute" function argument - words in command line. */

    char *current_command; /*!< command which is currently executing or NULL */

    volatile zb_uint8_t tx_in_progress : 1; /*!< "tx is busy" flag */

    volatile zb_uint8_t rx_in_progress : 1; /*!< "rx_buffer_flush is scheduled" flag */
} console_context_t;
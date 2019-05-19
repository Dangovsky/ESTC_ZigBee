#ifndef ZIGBEE_BULB_H
#define ZIGBEE_BULB_H

#include <stm32f4xx.h>
#include "zb_common.h"
#include "zb_aps.h"

/*! \file zbulb.h
 *  \brief API for libzbulb
 *
 *  Library provide a way of communication between light bulb and remote control over ZigBee network.
 *  Also it set a variety of command in commands_t and
 *  calls function from bulb_handlers_t when corresponding command received.
 */

/*!
 *  Enumeration to represent available commands.
 */
typedef enum commands_e {
    ON_COMMAND = 0,          /*!< Turn bulb on */
    OFF_COMMAND,             /*!< Turn bulb off */
    TOGGLE_COMMAND,          /*!< Toggle bulb state */
    BRIGHTNESS_UP_COMMAND,   /*!< Set brightness on step higher */
    BRIGHTNESS_DOWN_COMMAND, /*!< Set brightness on step lower */
    BRIGHTNESS_COMMAND,      /*!< Set brightness. This command is the only one which send payload - 8-bit brightness. */
    TOGGLE_COLOR_COMMAND,    /*!< Switch bulb color */

} commands_t;

/*!
 *  Sructure to get access to data in buffer tail.
 *  Used on sendind device.
 */
typedef struct bulb_tail_s {
    zb_uint8_t brightness; /*!< Payload for BRIGHTNESS_COMMAND. */
    zb_uint16_t addr;      /*!< Address on which packet will be sended. */
} ZB_PACKED_STRUCT bulb_tail_t;

/*!
 *  Sructure to get access to packet payload.
 *  Used on receiving device.
 */
typedef struct bulb_payload_s {
    zb_uint8_t command;    /*!< Received command (always used) */
    zb_uint8_t brightness; /*!< Payload for BRIGHTNESS_COMMAND (used only when BRIGHTNESS_COMMAND received) */
} bulb_payload_t;

/*!
 *  Sructure to hold callbacks for each command on receiving device.
 */
typedef struct bulb_handlers_s {
    zb_callback_t bulb_receive_on_command;              /*!< Callback to schedule when ON_COMMAND received*/
    zb_callback_t bulb_receive_off_command;             /*!< Callback to schedule when OFF_COMMAND received*/
    zb_callback_t bulb_receive_toggle_command;          /*!< Callback to schedule when TOGGLE_COMMAND received*/
    zb_callback_t bulb_receive_brightness_up_command;   /*!< Callback to schedule when BRIGHTNESS_UP_COMMAND received*/
    zb_callback_t bulb_receive_brightness_down_command; /*!< Callback to schedule when BRIGHTNESS_DOWN_COMMAND received*/
    zb_callback_t bulb_receive_brightness_command;      /*!< Callback to schedule when BRIGHTNESS_COMMAND received*/
    zb_callback_t bulb_receive_toggle_color_command;    /*!< Callback to schedule when TOGGLE_COLOR_COMMAND received*/
} bulb_handlers_t;

/*! \brief Initialise function
 *
 *  Set global variable buttons_handlers = handlers.
 *  \param handlers - callbacks to schedule when receive corresponding command.
 */
void init_zbulb(bulb_handlers_t* handlers);

/*! \brief Data indication on receiving device.
 *
 *  Parses packet and schedules handler for corresponding command.
 *  \param param - buffer param.
 */
void bulb_parse_packet(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain 16-bit address at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_on_command(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain 16-bit address at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_off_command(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain 16-bit address at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_toggle_command(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain 16-bit address at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_brightness_up_command(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain 16-bit address at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_brightness_down_command(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain bulb_tail_t struct at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_brightness_command(zb_uint8_t param) ZB_CALLBACK;

/*! \brief Send function for sending device
 *
 *  Buffer should contain 16-bit address at it`s tail.
 *  \param param - buffer param.
 */
void bulb_send_toggle_color_command(zb_uint8_t param) ZB_CALLBACK;

#endif /* ZIGBEE_BULB_H */

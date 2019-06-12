# USART command prompt

Command prompt to work with zboss

 * Current speed is 38400 on PC. Baudrate on STM should be in 1.5 times bigger then on PC.
 
## Files

Project splitted is files:
 * [zdo_console.c](zdo_console.c) - [microrl](https://github.com/Helius/microrl) and usart setup
 * [console.h](console.h) - header for local use
 * [cmd_handlers.c](cmd_handlers.c) - functions to call on recieved command
 * [cmd_callbacks.c](cmd_callbacks.c) - callbacks for remote responses

## Pins setup

It use USART2 as transport with:
 * PD5 for TX
 * PD6 for RX

## Serial port setup

```bash
    lsusb
    modprobe usbserial vendor= product=
    dmesg | grep ttyUSB
    sudo chmod 777 /dev/ttyUSB_
    cu -l /dev/ttyUSB_ -s 38400
```

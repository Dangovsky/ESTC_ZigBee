# ESTC_ZigBee
Second part of DSR course

## [lab_3](lab_3)
Simple protocol for controlling a light bulb. [ZE](lab_3/ze.c) send commands with optional payload (color or brightness). [ZC](lab_3/zc.c) print input in log file.
[`led.h`](lab_3/led.h) file for stucts end enumerator.

Commands:
  * Enable/disable light bulb
  * Toggle light bulb
  * Brightness: step up, step down, set to level
  * Toggle the color (we’ll assume that the bulb has a predetermined set of colors)

## [zdo_descriptors](zdo_descriptors)
ZR use ZDO's discovery function to find active EP on ZC, his input cluster and send him a packet.

## [stm_bulb](stm_bulb)
[Bulb](stm_bulb/zc.c) and [remote control](stm_bulb/ze.c) implementation on two stm32 dicsovery with zboss.

There are three library:

### [libled](stm_bulb/libled)
Is libled from [first part](https://github.com/Dangovsky/ESTC) of the course with gamma correction.

### [libbuttons](stm_bulb/libbuttons)
Initialize buttons and interrupts, uses callbacks passed to `init_buttons` as action handlers.
Can work on timers or on zboss alarms, based on defines `BUTTONS_TIMER` and `BUTTONS_ZB_ALARMS`.

### [libzbulb](stm_bulb/libzbulb)
Provides protocol for bulb-remote control interaction over zigbee.
 * Bulb use `parse_packet` as data indicaction and past callbacks for nended commands to `init_zbulb`
 * Remote control send command wia provided `bulb_send_*_command` functions.

## [fiinal task](fiinal_task)
Command prompt to work with zboss. See [README](final_task/README.md) for more info

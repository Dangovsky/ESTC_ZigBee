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

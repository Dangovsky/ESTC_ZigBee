# USART command prompt

 * Current speed is 38400 on PC. Baudrate on STM should be in 1.5 times bigger then on PC.
 * Try `ieee 0`. IEEE_TEST defined to test how console response without second stm plate

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

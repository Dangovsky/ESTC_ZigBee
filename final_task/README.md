# USART command prompt

## Enter currently does not work, use Ctrl+J (^J) instead.

## Serial port setup

```bash
    lsusb
    modprobe usbserial vendor= product=
    dmesg | grep ttyUSB
    sudo chmod 777 /dev/ttyUSB_
    cu -l /dev/ttyUSB_ -s 38400
```
## Baudrate on STM should be in 1.5 times bigger then on PC
# USART comand promt

```bash
    lsusb
    modprobe usbserial vendor= product=
    dmesg | grep ttyUSB
    sudo chmod 777 /dev/ttyUSB_
    cu -l /dev/ttyUSB_ -s 6400 (for 9600 baudrate on stm)
```

baudrate on STM should be in 1.5 times bigger then on PC

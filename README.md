# voltage_counter - read data from a voltage counter
## overview

This project involves a 4G network-enabled MCU (Microcontroller Unit) control program designed for real-time battery status monitoring. The system utilizes cellular connectivity to transmit operational parameters including voltage, current, temperature, and impedance data from battery units. The embedded firmware implements dynamic power management algorithms and includes safety protocols for overcharge/discharge protection. Initial field validation has been successfully conducted on lead-acid battery packs, demonstrating stable data acquisition and remote communication capabilities. Detailed circuit schematics and hardware implementation specifications will be provided in subsequent documentation.

## for serial port access in Linux, pls refer : https://pyserial.readthedocs.io/en/latest/ .

## Tested in CentOS 7.9.2009

## install python package "pyserial"
```
pip3 install pyserial
```

## run the command
```
python3 get-voltage.py
```

## 4G模块的资料
https://www.tastek.cn/Product/73

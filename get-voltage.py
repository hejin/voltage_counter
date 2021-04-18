import serial



def get_voltage_by_chan(chan_id):
    voltage = ""
    counter = ""
    cmd_str = 'YSR' + str(chan_id + 1) + '\n'
    print(("channel_str : " + cmd_str).strip()) 
    try:
        counter = serial.Serial(port="/dev/ttyUSB0", baudrate=115200, parity='N', stopbits=1, timeout=3)
        #counter.write(repr("YSR1\n").encode('ascii'))
        counter.write(ascii(cmd_str).encode())
        voltage = counter.readline()
    except Exception as e:
        print("Exception in get_voltage_by_chan() : "+ str(e)) 
    finally:
        if counter:
            counter.close()
        return voltage

def main():
    channel_number  = 0
    enable_channels = [True, True]
    print('Trying to get data from counter ...\n')
    for enable_channel in enable_channels:
        if enable_channel:
            voltage_counter = get_voltage_by_chan(channel_number)
            if not voltage_counter:
                print("failed to retrieve voltage counter from channel #"  + str(channel_number));
            else:
                print("channel #" + str(channel_number) + " voltage is : " + str(voltage_counter))
        print('\n')
        channel_number += 1


if __name__ == "__main__":
    main()

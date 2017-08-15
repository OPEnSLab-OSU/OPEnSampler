## Udell 2017. OPEnS LAb
## This program uses pySerial Library to send data to arduino
## https://github.com/pyserial/pyserial
## https://stackoverflow.com/questions/22271309/sending-a-character-to-the-arduino-serial-port-using-pythons-pyserial

## using time library .sleep
import time

## Import the Serial Library
import serial

## Number of bags in system
numBags = 24

## Bag to begin pumping in or out
firstBag = 2

## pump time out, how long to pump out each bag in seconds
pumpTime = 50

## Set this to 0 if you want to fill bags, or 1 if you want to drain
FillOrDrain = 1


## Open your Arduino Serial port.
## Look at the "Tools" menu in the Arduino IDE, check your Port setting and modify the first parameter to match that
## Then ensure your Baud rate in the second param matches the Baud rate in the Arduino sketch
## You can also check your Device Manager to see what COM port the Arduino is on
ser = serial.Serial('/dev/cu.usbmodem1411', 115200)


time.sleep(3)

"""
## open Valve 1
ser.write(b'V1 1\r')
time.sleep(1)
## Turn Motor on reverse
ser.write(b'M -1'\r)

for x in range(0, 23):
    time.sleep(20)

"""
ser.reset_input_buffer()

if FillOrDrain:
    motorOn = 'M-1 \r\n'
else:
    motorOn = 'M1 \r\n'


FlushOff = 'V0 0\r\n'
FirstValveOn = 'V1 1\r\n'
motorOff = 'M0 \r\n'


"""
while True:
    ##print(ser.readline())
    ## open Valve 0
    ser.write(mesg1.encode('ascii'))
    print('Valve 0 On')
    time.sleep(5)

    ## close Valve 0
    ser.write(msg.encode('ascii'))
    print('Valve 0 Off')
    time.sleep(5)
"""
## Turn off flush valve just in case open
ser.write(FlushOff.encode('ascii'))
time.sleep(1)

## Turn On Valve 1
ser.write(FirstValveOn.encode('ascii'))
print("First valve opened")
time.sleep(1)

## Start pump reverse
ser.write(motorOn.encode('ascii'))

## Wait for first bag to drain
time.sleep(pumpTime)

for i in range(firstBag, numBags+1):
    ##Make new valve string, Vn set based on lumber interation into loop
    ## found on https://stackoverflow.com/questions/5309978/sprintf-like-functionality-in-python
    buf = 'V%d 1\r\n' % i
    print(buf)
    ser.write(buf.encode('ascii'))
    ## resend motor on signal in case it turnes off accidentally
    time.sleep(3)
    ser.write(motorOn.encode('ascii'))
    time.sleep(pumpTime)

ser.write(motorOff.encode('ascii'))


while True:
    print('All Finished!')
    time.sleep(10)

import serial
from threading import Thread
from time import sleep

try:
    portaSerial = serial.Serial()
    portaSerial.port = '/dev/ttyUSB0'
    portaSerial.baudrate = 9600
    portaSerial.parity=serial.PARITY_NONE
    portaSerial.stopbits=serial.STOPBITS_ONE
    portaSerial.bytesize=serial.EIGHTBITS
    portaSerial.xonxoff = True
    portaSerial.timeout=None
    portaSerial.open()
except Exception as e:
    print(e)


class recebe(Thread):
    def run(self):
        while(True):
            try:
                inMsg = portaSerial.readline()
                inMsg = inMsg.decode("UTF-8")
                print(inMsg)
            except Exception as e:
                print(e)

thRecebe = recebe()
thRecebe.start()

while(True):
    try:
        x = 20000
        while(x<70000):
            x = x + 500
            msg = '2/1/' + str(x) + '\n'
            print(msg)
            portaSerial.write(bytes(msg, 'UTF-8'))
            portaSerial.flushOutput()
            sleep(.1)
    except KeyboardInterrupt:
        portaSerial.close()
        exit()

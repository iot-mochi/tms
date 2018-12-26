import serial
import time

con = serial.Serial('/dev/ttyACM0',9600)
 
def send_serial(command_str):
    time.sleep(1)
    print (con.portstr)
    buf = bytes(command_str, 'utf-8')
    con.write(buf)
    time.sleep(1)
    con.write(bytes(command_str+'/n','utf-8'))

    con_str = con.readline()
    print(con_str)
    con_str = con.readline()
    print(con_str)

if __name__ == '__main__':
    send_serial('read')
 
import socket
import time

target = "192.168.1.9"
ports = range(8000, 8020)

for port in ports:
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(1)
        result = sock.connect_ex((target, port))
        if result == 0:
            print(f"Port {port}: Open")
        else: 
            print(f"Port {port}: Closed")
        sock.close()
    except:
        pass
    time.sleep(0.1)
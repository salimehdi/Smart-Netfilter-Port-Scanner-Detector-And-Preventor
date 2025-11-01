import socket
import time
import struct


MCAST_GRP = '224.1.1.1'
MCAST_PORT = 5007
NUM_MESSAGES = 10000
MSG_PAYLOAD = "X" * 100  


print(f"Starting multicast sender to {MCAST_GRP}:{MCAST_PORT}")
print(f"Sending {NUM_MESSAGES} messages...")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.settimeout(2.0)
ttl = struct.pack('b', 1) 
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

start_time = time.time()

try:
    for i in range(NUM_MESSAGES):
        message = f"{i}:{time.time()}:{MSG_PAYLOAD}"
        sock.sendto(message.encode('utf-8'), (MCAST_GRP, MCAST_PORT))
        time.sleep(0.001) 

    sock.sendto(b"STOP", (MCAST_GRP, MCAST_PORT))

except socket.timeout:
    print("Socket timed out")
finally:
    sock.close()
    end_time = time.time()
    total_time = end_time - start_time
    print(f"\n--- UDP Sender Finished ---")
    print(f"Sent {NUM_MESSAGES} messages in {total_time:.4f} seconds.")
    print(f"Throughput: {NUM_MESSAGES / total_time:.2f} msgs/sec")
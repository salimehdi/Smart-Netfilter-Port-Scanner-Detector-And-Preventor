import socket
import time


SERVER_IP = "10.0.2.15"  
SERVER_PORT = 5007
NUM_MESSAGES = 10000
MSG_PAYLOAD = "X" * 100  


print(f"Starting TCP client to {SERVER_IP}:{SERVER_PORT}")
print(f"Sending {NUM_MESSAGES} messages...")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    sock.connect((SERVER_IP, SERVER_PORT))
except Exception as e:
    print(f"Failed to connect to server: {e}")
    exit()

start_time = time.time()

try:
    for i in range(NUM_MESSAGES):
        message = f"{i}:{time.time()}:{MSG_PAYLOAD}\n"
        sock.sendall(message.encode('utf-8'))

    
    sock.sendall(b"STOP\n")

finally:
    sock.close()
    end_time = time.time()
    total_time = end_time - start_time
    print(f"\n--- TCP Sender Finished ---")
    print(f"Sent {NUM_MESSAGES} messages in {total_time:.4f} seconds.")
    print(f"Throughput: {NUM_MESSAGES / total_time:.2f} msgs/sec")
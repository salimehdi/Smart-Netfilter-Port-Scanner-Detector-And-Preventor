import socket
import time


LISTEN_IP = '0.0.0.0' 
LISTEN_PORT = 5007
EXPECTED_MESSAGES = 10000


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((LISTEN_IP, LISTEN_PORT))
sock.listen(1)

print(f"TCP Server listening on {LISTEN_IP}:{LISTEN_PORT}...")

conn, addr = sock.accept()
print(f"Client connected from {addr}")

latencies = []
received_seqs = set()
total_received = 0
test_running = True
buffer = b""

with conn:
    while test_running:
        try:
            
            data = conn.recv(1024)
            if not data:
                test_running = False 
                break
                
            buffer += data
            
            
            while b'\n' in buffer:
                message, buffer = buffer.split(b'\n', 1)
                
                if message == b"STOP":
                    test_running = False
                    continue
                
                if not message:
                    continue

                receive_time = time.time()
                
                
                parts = message.decode('utf-8').split(':', 2)
                seq_num = int(parts[0])
                send_time = float(parts[1])
                
                
                latency_ms = (receive_time - send_time) * 1000
                
                latencies.append(latency_ms)
                received_seqs.add(seq_num)
                total_received += 1
                
        except Exception as e:
            print(f"Error: {e}")
            test_running = False

sock.close()


if total_received > 0:
    
    expected_packets = EXPECTED_MESSAGES
    packets_lost = expected_packets - total_received
    loss_percentage = (packets_lost / expected_packets) * 100

    avg_latency = sum(latencies) / len(latencies)
    max_latency = max(latencies)
    min_latency = min(latencies)

    print(f"\n--- TCP Receiver Finished ---")
    print(f"Total Packets Received: {total_received}")
    print(f"Packet Loss:            {packets_lost} / {expected_packets} ({loss_percentage:.2f}%)")
    print(f"--- Latency (ms) ---")
    print(f"Average: {avg_latency:.4f} ms")
    print(f"Min:     {min_latency:.4f} ms")
    print(f"Max:     {max_latency:.4f} ms")
else:
    print("No messages received.")
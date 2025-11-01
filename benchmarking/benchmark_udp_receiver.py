import socket
import struct
import time


MCAST_GRP = '224.1.1.1'
MCAST_PORT = 5007
LISTEN_IP = '0.0.0.0' 
EXPECTED_MESSAGES = 10000


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((LISTEN_IP, MCAST_PORT))
mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

print(f"UDP Receiver listening on {MCAST_GRP}:{MCAST_PORT}...")

latencies = []
received_seqs = set()
total_received = 0
test_running = True

while test_running:
    try:
        sock.settimeout(5.0) 
        data, addr = sock.recvfrom(1024)

        if data == b"STOP":
            test_running = False
            continue

        receive_time = time.time()
        parts = data.decode('utf-8').split(':', 2)
        seq_num = int(parts[0])
        send_time = float(parts[1])
        
        latency_ms = (receive_time - send_time) * 1000
        
        latencies.append(latency_ms)
        received_seqs.add(seq_num)
        total_received += 1
        
    except socket.timeout:
        test_running = False
    except Exception:
        pass 

sock.close()


if total_received > 0:
    
    expected_packets = max(received_seqs) + 1 if received_seqs else 0
    packets_lost = expected_packets - len(received_seqs)
    loss_percentage = (packets_lost / expected_packets) * 100 if expected_packets > 0 else 0

    avg_latency = sum(latencies) / len(latencies)
    max_latency = max(latencies)
    min_latency = min(latencies)

    print(f"\n--- UDP Receiver Finished ---")
    print(f"Total Packets Received: {total_received}")
    print(f"Packet Loss:            {packets_lost} / {expected_packets} ({loss_percentage:.2f}%)")
    print(f"--- Latency (ms) ---")
    print(f"Average: {avg_latency:.4f} ms")
    print(f"Min:     {min_latency:.4f} ms")
    print(f"Max:     {max_latency:.4f} ms")
else:
    print("No messages received.")
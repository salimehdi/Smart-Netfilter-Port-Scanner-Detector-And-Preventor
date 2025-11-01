# Smart Netfilter Port Scanner Detector

## Overview

This is a Linux kernel module that provides intelligent detection and blocking of network port scanners. It uses the Netfilter framework to inspect incoming packets and identify suspicious behavior characteristic of port scanning, while employing smart filtering techniques to avoid flagging legitimate traffic (like web browsing) as malicious.

The module is highly configurable and provides detailed statistics and a list of currently blocked IP addresses through the `/proc` filesystem.

## Features

- **Intelligent Scan Detection**: Uses heuristics like the number of unique ports contacted, connection success rate, and time windows to identify scanners.
- **False Positive Reduction**: "Smart filtering" mode can distinguish between a port scan and normal network activity (e.g., a web browser opening multiple connections).
- **Runtime Monitoring**: Provides real-time statistics and a list of blocked IPs via the `/proc` filesystem.
- **Automatic Unblocking**: Blocked IPs are automatically unblocked after a configurable duration.
- **Comprehensive Makefile**: Includes targets for easy compilation, installation, testing, and monitoring.

## Requirements

- A Linux system with kernel headers installed (e.g., `linux-headers-$(uname -r)`)
- `make` and `gcc`

## Compilation

To compile the module, simply run the make command in the project's root directory:

```bash
make
```

This will produce the kernel module file: `netfilter_smart_port_scan_detector.ko`.

## Installation & Usage

### Loading the Module

```bash
sudo insmod netfilter_smart_port_scan_detector.ko
# Or use the Makefile target
make install
```

### Unloading the Module

```bash
sudo rmmod netfilter_smart_port_scan_detector
# Or use the Makefile target
make uninstall
```

### Viewing Kernel Logs

```bash
dmesg
# Or use the Makefile targets
make logs      # Shows the last 50 lines
make watch-logs # Shows logs in real-time
```

## Monitoring

You can monitor the module's activity by reading the files in `/proc/netfilter_scanner/`.

### View Current Configuration

```bash
make config
# Or
cat /proc/netfilter_scanner/config
```

### View Statistics

```bash
make stats
# Or
cat /proc/netfilter_scanner/stats
```

Statistics include:
- **`packets_processed`**: Total packets inspected by the module
- **`scans_detected`**: Number of port scan events detected
- **`ips_blocked`**: Total number of unique IPs blocked
- **`packets_blocked`**: Total packets dropped from blocked IPs
- **`tcp_packets`**: Total TCP packets processed
- **`legitimate_traffic`**: Number of times traffic was classified as legitimate and ignored

### View Currently Blocked IPs

```bash
make blocked
# Or
cat /proc/netfilter_scanner/blocked_ips
```

This will show a list of blocked IPs, the reason for the block, and the time remaining until they are unblocked.

## Testing

This test simulates an attack from a different machine on your network.

### 1. Find Your Linux Machine's IP Address

On the machine where the kernel module is installed, find its local IP address:

```bash
# This is the recommended command
hostname -I
# Or, for more detail
ip addr show
```

Look for an IP address like `192.168.1.9`. This is your target IP.

### 2. Run the Port Scan Script from a Different Machine

From another computer on the same network, save the following Python code as a file (e.g., `scanner.py`). **Do not run this on the machine with the kernel module.**

```python
# scanner.py
import socket
import time

# !!! IMPORTANT: Change this to your target machine's IP address !!!
target = "192.168.1.9"
ports = range(8000, 8020)

print(f"Scanning target: {target}")

for port in ports:
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(0.5)  # Faster timeout
        result = sock.connect_ex((target, port))
        if result == 0:
            print(f"Port {port}: Open")
        else:
            print(f"Port {port}: Closed")
        sock.close()
    except socket.error as err:
        print(f"Couldn't connect to server: {err}")
    time.sleep(0.1)

print("Scan finished.")
```

Before running, update the `target` variable in the script to the IP address you found in step 1. Then, run the script:

```bash
python3 scanner.py
```

### 3. Verify the Block

As the script runs, it will quickly exceed the `scan_threshold` set in the module. The module will then block the IP address of the machine running the Python script.

On the machine with the kernel module, check the stats and the blocked list:

```bash
# Check the stats - you should see scans_detected and ips_blocked incremented
make stats

# Check the blocked list - you should see the scanner's IP address
make blocked
```

Once the scanner's IP is blocked, all further connection attempts from it will be dropped by the kernel module. To the scanner, it will appear as if all ports are closed or unreachable.

## License

This project is licensed under the GPL (GNU General Public License).

## Resources

( Curious?! Want to make similar kernel module yourself? )
| Resource Name                   | Description                               | Link                                                      |
|----------------------------------|-------------------------------------------|-----------------------------------------------------------|
| Linux Command                    | A guide to Linux shell scripting          | [linuxcommand.org](https://linuxcommand.org/index.php)     |
| Kernel Module Programming (Old)  | An older version of the Kernel Module Programming guide (2001) | [LDP LKMPG (Old)](https://tldp.org/LDP/lkmpg/)             |
| Kernel Module Programming (Updated) | Frequently updated Kernel Module Programming guide | [Fennecj LKMPG (Updated)](https://fennecj.github.io/lkmpg/) |


# Simple Python Port Scanner (test.py)

## Overview

This is a basic Python script designed to perform a TCP port scan against a target IP address. Its primary purpose is to serve as a testing tool for the Smart Netfilter Port Scanner Detector kernel module. By simulating a port scan, it allows you to verify that the kernel module is working correctly by detecting the scan and blocking the source IP.

## How It Works

The script operates using the following logic:

- It iterates through a predefined range of TCP ports (8000 to 8020 by default)
- For each port, it creates a new TCP socket
- It uses the non-blocking `socket.connect_ex()` method to attempt a connection. This method returns 0 if the port is open and an error code otherwise
- A short timeout (1 second) is set on the socket to prevent the script from hanging on unresponsive ports
- A small delay (0.1 seconds) is added between each connection attempt to control the scan speed
- The status of each port ("Open" or "Closed") is printed to the console

## Requirements

- **Python 3**

The `socket` and `time` libraries are part of the standard Python library, so no external packages are needed.

## Configuration

Before running the script, you must edit the `target` variable to match the IP address of the machine you want to scan (the machine running the kernel module).

Open `test.py` and modify this line:

```python
# !!! IMPORTANT: Change this to your target machine's IP address !!!
target = "192.168.1.9"
```

## Usage

Once you have configured the target IP address, you can run the script from your terminal **from a machine other than your target**.

```bash
python3 test.py
```

## Expected Outcome

When you run this script against a machine with the Smart Netfilter Port Scanner Detector module loaded:

1. The script will begin scanning ports and printing their status
2. After it checks a number of ports equal to or greater than the `scan_threshold` configured in the kernel module, the module will identify this activity as a port scan
3. The kernel module will then block the IP address of the machine running the script
4. You can verify this by running `make stats` and `make blocked` on the target machine

## Important Notes

- **Do not run this script on the same machine as the kernel module** - run it from a different computer on the same network
- Make sure to update the target IP address before running
- This script is intended for testing purposes only on networks you own or have permission to test

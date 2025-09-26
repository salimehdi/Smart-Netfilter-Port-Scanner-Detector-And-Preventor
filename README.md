Smart Netfilter Port Scanner DetectorOverviewThis is a Linux kernel module that provides intelligent detection and blocking of network port scanners. It uses the Netfilter framework to inspect incoming packets and identify suspicious behavior characteristic of port scanning, while employing smart filtering techniques to avoid flagging legitimate traffic (like web browsing) as malicious.The module is highly configurable and provides detailed statistics and a list of currently blocked IP addresses through the /proc filesystem.FeaturesIntelligent Scan Detection: Uses heuristics like the number of unique ports contacted, connection success rate, and time windows to identify scanners.False Positive Reduction: "Smart filtering" mode can distinguish between a port scan and normal network activity (e.g., a web browser opening multiple connections).Runtime Monitoring: Provides real-time statistics and a list of blocked IPs via the /proc filesystem.Automatic Unblocking: Blocked IPs are automatically unblocked after a configurable duration.Comprehensive Makefile: Includes targets for easy compilation, installation, testing, and monitoring.RequirementsA Linux system with kernel headers installed (e.g., linux-headers-$(uname -r)).make and gcc.nmap for testing (optional, but recommended).CompilationTo compile the module, simply run the make command in the project's root directory:make
This will produce the kernel module file: netfilter_smart_port_scan_detector.ko.Installation & UsageTo load the module:sudo insmod netfilter_smart_port_scan_detector.ko
# Or use the Makefile target
make install
To unload the module:sudo rmmod netfilter_smart_port_scan_detector
# Or use the Makefile target
make uninstall
To view kernel logs for module activity:dmesg
# Or use the Makefile targets
make logs       # Shows the last 50 lines
make watch-logs # Shows logs in real-time
ConfigurationThe module can be configured at runtime by writing to /proc/netfilter_scanner/config. You must have root privileges to do this.Example: To change the scan threshold to 20 ports:echo "scan_threshold 20" | sudo tee /proc/netfilter_scanner/config
You can also use an equals sign:echo "block_duration=600" | sudo tee /proc/netfilter_scanner/config
Available Parameters:scan_threshold: The number of unique ports an IP must contact within the time_window to be considered a potential scanner. (Default: 15)time_window: The time period in seconds to monitor an IP for suspicious activity. (Default: 60)block_duration: The duration in seconds to block a detected scanner's IP address. (Default: 300)enable_blocking: Set to 1 to block detected scanners, 0 to only log them. (Default: 1)enable_logging: Set to 1 for more verbose logging of packet activity. (Default: 0)enable_smart_filtering: Set to 1 to use heuristics to avoid blocking legitimate traffic, 0 for basic detection. (Default: 1)MonitoringYou can monitor the module's activity by reading the files in /proc/netfilter_scanner/.View current configuration:make config
# Or
cat /proc/netfilter_scanner/config
View statistics:make stats
# Or
cat /proc/netfilter_scanner/stats
packets_processed: Total packets inspected by the module.scans_detected: Number of port scan events detected.ips_blocked: Total number of unique IPs blocked.packets_blocked: Total packets dropped from blocked IPs.tcp_packets: Total TCP packets processed.legitimate_traffic: Number of times traffic was classified as legitimate and ignored.View currently blocked IPs:make blocked
# Or
cat /proc/netfilter_scanner/blocked_ips
This will show a list of blocked IPs, the reason for the block, and the time remaining until they are unblocked.TestingThis test simulates an attack from a different machine on your network.1. Find your Linux Machine's IP AddressOn the machine where the kernel module is installed, find its local IP address. You can use one of these commands:# This is the recommended command
hostname -I

# Or, for more detail
ip addr show
Look for an IP address like 192.168.1.9. This is your target IP.2. Run the Port Scan Script from a Different MachineFrom another computer on the same network, save the following Python code as a file (e.g., scanner.py). Do not run this on the machine with the kernel module.# scanner.py
import socket
import time

# !!! IMPORTANT: Change this to your target machine's IP address !!!
target = "192.168.1.9" 
ports = range(8000, 8020)

print(f"Scanning target: {target}")

for port in ports:
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(0.5) # Faster timeout
        result = sock.connect_ex((target, port))
        if result == 0:
            print(f"Port {port}: Open")
        # We don't need to print 'Closed' for this test
        sock.close()
    except socket.error as err:
        print(f"Couldn't connect to server: {err}")
    time.sleep(0.1)

print("Scan finished.")
Before running, update the target variable in the script to the IP address you found in step 1. Then, run the script:python3 scanner.py
3. Verify the BlockAs the script runs, it will quickly exceed the scan_threshold set in the module. The module will then block the IP address of the machine running the Python script.On the machine with the kernel module, check the stats and the blocked list:# Check the stats - you should see scans_detected and ips_blocked incremented
make stats

# Check the blocked list - you should see the scanner's IP address
make blocked
Once the scanner's IP is blocked, all further connection attempts from it will be dropped by the kernel module. To the scanner, it will appear as if all ports are closed or unreachable.LicenseThis project is licensed under the GPL (GNU General Public License).
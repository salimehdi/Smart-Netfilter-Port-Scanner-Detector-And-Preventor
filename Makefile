
obj-m += netfilter_smart_port_scan_detector.o

netfilter_smart_port_scan_detector-y := detector/detector.o procfs/proc_handler.o utils/helper.o

KDIR := /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all: module

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install: module
	sudo insmod netfilter_smart_port_scan_detector.ko

uninstall:
	sudo rmmod netfilter_smart_port_scan_detector

logs:
	sudo dmesg | tail -50

watch-logs:
	sudo dmesg -wH

status:
	lsmod | grep netfilter_smart_port_scan_detector || echo "Module not loaded"

stats:
	@cat /proc/netfilter_scanner/stats 2>/dev/null || echo "Module not loaded or proc file not found"

config:
	@cat /proc/netfilter_scanner/config 2>/dev/null || echo "Module not loaded or proc file not found"

blocked:
	@cat /proc/netfilter_scanner/blocked_ips 2>/dev/null || echo "Module not loaded or proc file not found"

.PHONY: all module clean install uninstall logs watch-logs status stats config blocked test-scan full-test
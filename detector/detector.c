#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include "headers/detector.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ali Mehdi Syed");
MODULE_DESCRIPTION("Smart Netfilter-based port scanner detector");
MODULE_VERSION("1.0");

struct scanner_stats stats = {0};
struct list_head port_hash_table[HASH_SIZE];
struct list_head blocked_hash_table[HASH_SIZE];
DEFINE_SPINLOCK(port_tracker_lock);
DEFINE_SPINLOCK(blocked_ip_lock);
struct nf_hook_ops nfho;

int scan_threshold = 15;
int time_window = 60;
int block_duration = 300;
int enable_blocking = 1;
int enable_logging = 0;
int enable_smart_filtering = 1;


static unsigned int hook_func(void *priv,
                              struct sk_buff *skb,
                              const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    u32 src_ip;
    u16 dest_port = 0;
    struct port_tracker *tracker;
    unsigned long flags;
    bool connection_successful = false;

    if (!skb) return NF_ACCEPT;

    ip_header = ip_hdr(skb);
    if (!ip_header) return NF_ACCEPT;

    src_ip = ntohl(ip_header->saddr);

    stats.packets_processed++;

    spin_lock_irqsave(&blocked_ip_lock, flags);
    if (enable_blocking && is_ip_blocked(src_ip)) {
        spin_unlock_irqrestore(&blocked_ip_lock, flags);
        stats.packets_blocked++;
        return NF_DROP;
    }
    spin_unlock_irqrestore(&blocked_ip_lock, flags);

    if (ip_header->protocol == IPPROTO_TCP) {
        tcp_header = tcp_hdr(skb);
        if (tcp_header) {
            dest_port = ntohs(tcp_header->dest);
            stats.tcp_packets++;

            connection_successful = (tcp_header->syn && tcp_header->ack) ||
                                  (!tcp_header->syn && tcp_header->ack);

            spin_lock_irqsave(&port_tracker_lock, flags);

            tracker = find_or_create_tracker(src_ip);
            if (tracker) {
                if (time_after(jiffies, tracker->first_activity + time_window * HZ)) {
                    tracker->unique_ports = 0;
                    tracker->first_activity = jiffies;
                    tracker->is_scanner = false;
                }

                add_port_to_tracker(tracker, dest_port, connection_successful);

                if (enable_smart_filtering && !tracker->is_scanner) {
                    if (looks_like_legitimate_browsing(tracker)) {
                         stats.legitimate_traffic++;
                    } else if (looks_like_port_scan(tracker)) {
                        tracker->is_scanner = true;
                        stats.scans_detected++;

                        if (enable_blocking) {
                            spin_unlock_irqrestore(&port_tracker_lock, flags);
                            spin_lock_irqsave(&blocked_ip_lock, flags);
                            block_ip(src_ip, "Suspicious port scanning");
                            spin_unlock_irqrestore(&blocked_ip_lock, flags);
                            return NF_DROP;
                        }
                    }
                }
            }

            spin_unlock_irqrestore(&port_tracker_lock, flags);
        }
    }

    return NF_ACCEPT;
}

static int __init smart_scanner_init(void)
{
    printk(KERN_INFO "Smart Port Scanner Detector Loading...\n");
    init_hash_tables();
    init_proc();

    nfho.hook = hook_func;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &nfho) != 0) {
        printk(KERN_ERR "Failed to register netfilter hook\n");
        cleanup_proc();
        return -1;
    }

    printk(KERN_INFO "Smart Port Scanner Detector Loaded Successfully\n");
    return 0;
}

static void __exit smart_scanner_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
    cleanup_proc();
    cleanup_hash_tables();
    printk(KERN_INFO "Smart Port Scanner Detector Unloaded\n");
}

module_init(smart_scanner_init);
module_exit(smart_scanner_exit);

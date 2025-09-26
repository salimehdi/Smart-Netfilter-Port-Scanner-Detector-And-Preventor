#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/hash.h>
#include "../detector/headers/detector.h"

static u16 legitimate_ports[] = { 80, 443, 53, 25, 587, 110, 995, 143, 993, 21, 0 };

static bool is_legitimate_port(u16 port) {
    int i;
    for (i = 0; legitimate_ports[i] != 0; i++) {
        if (port == legitimate_ports[i]) return true;
    }
    return false;
}

bool looks_like_legitimate_browsing(struct port_tracker *tracker) {
    int legitimate_port_count = 0;
    int i;
    for (i = 0; i < tracker->unique_ports; i++) {
        if (is_legitimate_port(tracker->ports[i].port)) {
            legitimate_port_count++;
        }
    }
    return legitimate_port_count > (tracker->unique_ports / 2);
}

bool looks_like_port_scan(struct port_tracker *tracker) {
    return tracker->unique_ports >= scan_threshold;
}

static unsigned int ip_hash(u32 ip) {
    return hash_32(ip, 10) & HASH_MASK;
}

void init_hash_tables(void) {
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        INIT_LIST_HEAD(&port_hash_table[i]);
        INIT_LIST_HEAD(&blocked_hash_table[i]);
    }
}

void cleanup_hash_tables(void) {
    int i;
    struct port_tracker *tracker, *tmp_tracker;
    struct blocked_ip *blocked, *tmp_blocked;

    for (i = 0; i < HASH_SIZE; i++) {
        list_for_each_entry_safe(tracker, tmp_tracker, &port_hash_table[i], list) {
            list_del(&tracker->list);
            kfree(tracker);
        }
        list_for_each_entry_safe(blocked, tmp_blocked, &blocked_hash_table[i], list) {
            list_del(&blocked->list);
            kfree(blocked);
        }
    }
}

bool is_ip_blocked(u32 ip) {
    unsigned int hash_val = ip_hash(ip);
    struct blocked_ip *blocked;
    list_for_each_entry(blocked, &blocked_hash_table[hash_val], list) {
        if (blocked->ip == ip) {
            if (time_after(jiffies, blocked->block_time + block_duration * HZ)) {
                list_del(&blocked->list);
                kfree(blocked);
                return false;
            }
            return true;
        }
    }
    return false;
}

void block_ip(u32 ip, const char *reason) {
    unsigned int hash_val = ip_hash(ip);
    struct blocked_ip *blocked;

    if (is_ip_blocked(ip)) return;

    blocked = kmalloc(sizeof(struct blocked_ip), GFP_ATOMIC);
    if (!blocked) return;

    blocked->ip = ip;
    blocked->block_time = jiffies;
    strncpy(blocked->reason, reason, sizeof(blocked->reason) - 1);
    blocked->reason[sizeof(blocked->reason) - 1] = '\0';
    INIT_LIST_HEAD(&blocked->list);
    list_add(&blocked->list, &blocked_hash_table[hash_val]);
    stats.ips_blocked++;
}

struct port_tracker *find_or_create_tracker(u32 src_ip) {
    unsigned int hash_val = ip_hash(src_ip);
    struct port_tracker *tracker;

    list_for_each_entry(tracker, &port_hash_table[hash_val], list) {
        if (tracker->src_ip == src_ip) return tracker;
    }

    tracker = kmalloc(sizeof(struct port_tracker), GFP_ATOMIC);
    if (!tracker) return NULL;

    tracker->src_ip = src_ip;
    tracker->unique_ports = 0;
    tracker->first_activity = jiffies;
    tracker->is_scanner = false;
    INIT_LIST_HEAD(&tracker->list);
    list_add(&tracker->list, &port_hash_table[hash_val]);
    return tracker;
}

void add_port_to_tracker(struct port_tracker *tracker, u16 port, bool successful) {
    int i;
    for (i = 0; i < tracker->unique_ports; i++) {
        if (tracker->ports[i].port == port) return;
    }

    if (tracker->unique_ports < MAX_PORTS_PER_IP) {
        tracker->ports[tracker->unique_ports].port = port;
        tracker->ports[tracker->unique_ports].successful_connection = successful;
        tracker->unique_ports++;
    }
}

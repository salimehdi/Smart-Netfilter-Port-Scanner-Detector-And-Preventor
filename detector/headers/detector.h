#ifndef _DETECTOR_H
#define _DETECTOR_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/skbuff.h>

#define HASH_SIZE 1024
#define HASH_MASK (HASH_SIZE - 1)
#define MAX_PORTS_PER_IP 100

extern int scan_threshold;
extern int time_window;
extern int block_duration;
extern int enable_blocking;
extern int enable_logging;
extern int enable_smart_filtering;

struct scanner_stats {
    unsigned long packets_processed;
    unsigned long scans_detected;
    unsigned long ips_blocked;
    unsigned long packets_blocked;
    unsigned long tcp_packets;
    unsigned long legitimate_traffic;
};

struct port_tracker {
    u32 src_ip;
    struct {
        u16 port;
        bool successful_connection;
    } ports[MAX_PORTS_PER_IP];
    int unique_ports;
    unsigned long first_activity;
    bool is_scanner;
    struct list_head list;
};

struct blocked_ip {
    u32 ip;
    unsigned long block_time;
    char reason[64];
    struct list_head list;
};

extern struct scanner_stats stats;
extern struct list_head port_hash_table[HASH_SIZE];
extern struct list_head blocked_hash_table[HASH_SIZE];
extern spinlock_t port_tracker_lock;
extern spinlock_t blocked_ip_lock;


void init_hash_tables(void);
void cleanup_hash_tables(void);
bool is_ip_blocked(u32 ip);
void block_ip(u32 ip, const char *reason);
struct port_tracker *find_or_create_tracker(u32 src_ip);
void add_port_to_tracker(struct port_tracker *tracker, u16 port, bool successful);
bool looks_like_legitimate_browsing(struct port_tracker *tracker);
bool looks_like_port_scan(struct port_tracker *tracker);

int init_proc(void);
void cleanup_proc(void);


#endif

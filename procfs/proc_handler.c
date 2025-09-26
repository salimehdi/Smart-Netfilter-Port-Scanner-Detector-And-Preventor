
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include "../detector/headers/detector.h"

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_config;
static struct proc_dir_entry *proc_stats;
static struct proc_dir_entry *proc_blocked_ips;

static ssize_t config_write(struct file *file, const char __user *buffer,
                           size_t count, loff_t *pos)
{
    char buf[256];
    char key[64];
    int value;

    if (count >= sizeof(buf)) return -EINVAL;
    if (copy_from_user(buf, buffer, count)) return -EFAULT;
    buf[count] = '\0';

    if (sscanf(buf, "%63s %d", key, &value) == 2) {
        if (strcmp(key, "scan_threshold") == 0) scan_threshold = value;
        else if (strcmp(key, "time_window") == 0) time_window = value;
        else if (strcmp(key, "block_duration") == 0) block_duration = value;
        else if (strcmp(key, "enable_blocking") == 0) enable_blocking = value;
        else if (strcmp(key, "enable_logging") == 0) enable_logging = value;
        else if (strcmp(key, "enable_smart_filtering") == 0) enable_smart_filtering = value;
    }

    return count;
}


static ssize_t config_read(struct file *file, char __user *buffer,
                          size_t count, loff_t *pos)
{
    char buf[512];
    int len;

    if (*pos > 0) return 0;

    len = scnprintf(buf, sizeof(buf),
        "scan_threshold=%d\n"
        "time_window=%d\n"
        "block_duration=%d\n"
        "enable_blocking=%d\n"
        "enable_logging=%d\n"
        "enable_smart_filtering=%d\n",
        scan_threshold, time_window, block_duration,
        enable_blocking, enable_logging, enable_smart_filtering);

    if (copy_to_user(buffer, buf, len)) return -EFAULT;
    *pos = len;

    return len;
}

static ssize_t stats_read(struct file *file, char __user *buffer,
                         size_t count, loff_t *pos)
{
    char buf[512];
    int len;

    if (*pos > 0) return 0;

    len = scnprintf(buf, sizeof(buf),
        "packets_processed=%lu\n"
        "scans_detected=%lu\n"
        "ips_blocked=%lu\n"
        "packets_blocked=%lu\n"
        "tcp_packets=%lu\n"
        "legitimate_traffic=%lu\n",
        stats.packets_processed, stats.scans_detected, stats.ips_blocked,
        stats.packets_blocked, stats.tcp_packets, stats.legitimate_traffic);

    if (copy_to_user(buffer, buf, len)) return -EFAULT;
    *pos = len;

    return len;
}

static ssize_t blocked_ips_read(struct file *file, char __user *buffer,
                               size_t count, loff_t *pos)
{
    char *buf;
    int len = 0;
    int i;
    struct blocked_ip *blocked;
    unsigned long current_time = jiffies;

    if (*pos > 0) return 0;

    buf = kmalloc(2048, GFP_KERNEL);
    if (!buf) return -ENOMEM;

    len += scnprintf(buf + len, 2048 - len, "Currently Blocked IPs:\n");

    spin_lock_bh(&blocked_ip_lock);
    for (i = 0; i < HASH_SIZE && len < 2048 - 100; i++) {
        list_for_each_entry(blocked, &blocked_hash_table[i], list) {
            unsigned long remaining = 0;
            if (time_before(current_time, blocked->block_time + block_duration * HZ)) {
                remaining = (blocked->block_time + block_duration * HZ - current_time) / HZ;
                len += scnprintf(buf + len, 2048 - len,
                                 "%pI4 (expires in %lu seconds) - %s\n",
                                 &blocked->ip, remaining, blocked->reason);
            }
        }
    }
    spin_unlock_bh(&blocked_ip_lock);

    if (len == strlen("Currently Blocked IPs:\n")) {
        len += scnprintf(buf + len, 2048 - len, "No IPs currently blocked.\n");
    }

    if (copy_to_user(buffer, buf, len)) {
        kfree(buf);
        return -EFAULT;
    }

    kfree(buf);
    *pos = len;
    return len;
}

static const struct proc_ops config_proc_ops = {
    .proc_read = config_read,
    .proc_write = config_write,
};

static const struct proc_ops stats_proc_ops = {
    .proc_read = stats_read,
};

static const struct proc_ops blocked_ips_proc_ops = {
    .proc_read = blocked_ips_read,
};

int init_proc(void)
{
    proc_dir = proc_mkdir("netfilter_scanner", NULL);
    if (!proc_dir) return -ENOMEM;

    proc_config = proc_create("config", 0644, proc_dir, &config_proc_ops);
    proc_stats = proc_create("stats", 0444, proc_dir, &stats_proc_ops);
    proc_blocked_ips = proc_create("blocked_ips", 0444, proc_dir, &blocked_ips_proc_ops);

    if (!proc_config || !proc_stats || !proc_blocked_ips) {
        cleanup_proc();
        return -ENOMEM;
    }
    return 0;
}

void cleanup_proc(void)
{
    if (proc_dir) {
        remove_proc_subtree("netfilter_scanner", NULL);
    }
}


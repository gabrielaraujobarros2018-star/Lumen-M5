/* logcatSample01.c - Simple Android logcat reader for Lumen OS debugging */
/* Reads from Android log devices - perfect for Galaxy A05s Phosh development */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

/* Android logger structures */
struct logger_entry {
    uint32_t len;      /* length of the payload */
    uint16_t __pad;
    uint16_t tid;      /* Android thread ID */
    uint16_t pid;      /* Android process ID */
    uint16_t tid2;     /* same as tid */
};

#define LOGGER_ENTRY_MAX_LEN  4096
#define LOG_NAME_MAX_LEN      64

/* Log buffer names */
const char *log_devices[] = {
    "/dev/log/main",
    "/dev/log/system", 
    "/dev/log/radio",
    "/dev/log/events",
    NULL
};

/* Log levels */
const char *log_levels[] = {
    "UNKNOWN", "VERBOSE", "DEBUG", "INFO", 
    "WARN", "ERROR", "FATAL", "SILENT"
};

/* Global quit flag */
volatile sig_atomic_t quit = 0;

void signal_handler(int sig) {
    quit = 1;
}

/* Parse Android log entry */
int parse_log_entry(char *buf, size_t len, struct logger_entry *entry) {
    if (len < sizeof(struct logger_entry)) {
        return -1;
    }
    
    memcpy(entry, buf, sizeof(struct logger_entry));
    
    // Ensure payload length is valid
    if (entry->len > LOGGER_ENTRY_MAX_LEN || 
        entry->len > len - sizeof(struct logger_entry)) {
        return -1;
    }
    
    return 0;
}

/* Print log entry in logcat format */
void print_log_entry(const char *device, struct logger_entry *entry, 
                    char *payload, time_t now) {
    char timebuf[32];
    struct tm tm;
    
    localtime_r(&now, &tm);
    strftime(timebuf, sizeof(timebuf), "%m-%d %H:%M:%S", &tm);
    
    // Extract priority from payload (first char is priority)
    int pri = payload[0] - '0';
    if (pri < 0 || pri >= sizeof(log_levels)/sizeof(log_levels[0])) {
        pri = 0;
    }
    
    printf("[%s][%s][%d:%d] %s
", 
           timebuf, log_levels[pri], entry->pid, entry->tid, 
           payload + 1);  // Skip priority byte
    fflush(stdout);
}

/* Read from single log device */
void read_log_device(const char *device_name) {
    int fd = open(device_name, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open log device");
        return;
    }
    
    printf("=== Monitoring %s ===
", device_name);
    
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    char buf[LOGGER_ENTRY_MAX_LEN + 1];
    struct logger_entry entry;
    time_t now;
    
    while (!quit) {
        int ret = poll(&pfd, 1, 1000);  // 1 second timeout
        
        if (ret > 0 && (pfd.revents & POLLIN)) {
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '';
                
                if (parse_log_entry(buf, n, &entry) == 0) {
                    now = time(NULL);
                    print_log_entry(device_name, &entry, buf + sizeof(struct logger_entry), now);
                }
            }
        } else if (ret < 0 && errno != EINTR) {
            perror("poll error");
            break;
        }
    }
    
    close(fd);
}

int main(int argc, char *argv[]) {
    printf("Lumen OS Logcat Reader v0.1 (Galaxy A05s/Phosh)
");
    printf("Press Ctrl+C to exit

");
    
    // Install signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (argc > 1) {
        // Specific device mode
        read_log_device(argv[1]);
    } else {
        // Monitor all devices (parallel would need threads)
        int i = 0;
        while (log_devices[i]) {
            printf("Available log devices:
");
            printf("  %s
", log_devices[i]);
            i++;
        }
        printf("
Use: %s /dev/log/main
", argv[0]);
        
        // Default to main log
        read_log_device("/dev/log/main");
    }
    
    printf("
Logcat stopped.
");
    return 0;
}

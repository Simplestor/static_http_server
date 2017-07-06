#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/klog.h>

#include "log.h"
#include "lock.h"

#define LOG_BUF_MAX 512
#define MAX_LOG_FILE_SIZE (0x200000)

enum LOG_TYPE{
    LOG_TYPE_CONSOLE = 0,
    LOG_TYPE_SYSLOGD,
    LOG_TYPE_TEXT_FILE,
    LOG_TYPE_MAX,
};

static int log_type = LOG_TYPE_CONSOLE;
static int log_file_fd = -1;
static int log_file_size = 0;
static int s_log_level = DBG_CUR_LEVEL;

static char s_log_buffer_lines[DBG_LOG_MAX_LINES][DBG_LOG_MAX_LINE_SIZE];
static int log_base_line = 0;
static int log_curr_line = 0;
static lock_t s_log_lock;

static const unsigned int color[] = {
    [DBG_FATAL] = 208 <<  8 | 0x41,
    [DBG_ERR] = 196 <<  8 | 0x11,
    [DBG_WARN] = 226 <<  8 | 0x03,
    [DBG_INFO] = 253 <<  8 | 0x09,
    [DBG_NOTICE] =  40 <<  8 | 0x02,
    [DBG_DEBUG] =  34 <<  8 | 0x02,
};

#define set_color(x)  fprintf(stderr, "\033[%d;3%dm", (color[x] >> 4) & 15, color[x] & 15)
#define set_256color(x) fprintf(stderr, "\033[48;5;%dm\033[38;5;%dm", (color[x] >> 16) & 0xff, (color[x] >> 8) & 0xff)
#define reset_color() fprintf(stderr, "\033[0m")

void log_write(int level, const char *fmt, ...)
{
    char buf[LOG_BUF_MAX];
    va_list ap;
    int len = 0;

    if (level > s_log_level) return;
    if(log_type == LOG_TYPE_SYSLOGD){
        //syslog(level, fmt);
        return ;
    }

    lock(&s_log_lock);
    va_start(ap, fmt);
    vsnprintf(buf + len, LOG_BUF_MAX, fmt, ap);
    vsnprintf(s_log_buffer_lines[log_curr_line], DBG_LOG_MAX_LINE_SIZE, fmt, ap);
    buf[LOG_BUF_MAX - 1] = 0;
    va_end(ap);
    if(log_curr_line == log_base_line){
        log_curr_line++;
    }else if(((log_curr_line + 1) == log_base_line) || ((log_base_line == 0) && ((log_curr_line + 1) == DBG_LOG_MAX_LINES))){
        log_curr_line++;
        log_base_line++;
    }else{
        log_curr_line++;
    }
    if(log_base_line >= DBG_LOG_MAX_LINES){
        log_base_line = 0;
    }
    if(log_curr_line >= DBG_LOG_MAX_LINES){
        log_curr_line = 0;
    }
    unlock(&s_log_lock);
    if ((log_type == LOG_TYPE_CONSOLE) || (log_file_fd < 0)){
        set_color(level);
        fprintf(stderr, "%s", buf);
        reset_color();
        return ;
    }
    log_file_size += strlen(buf);
    if(log_file_size >= MAX_LOG_FILE_SIZE){
        log_file_size = strlen(buf);
        lseek(log_file_fd, 0, SEEK_SET);
    }
    write(log_file_fd, buf, strlen(buf));
}


int log_level_set(int level)
{
    if(level >= 0){
        s_log_level = level;
    }

    return 0;
}

int log_read(char *log, int size)
{
    int tmp = 0, i;

    if(!log || !size){
        return 0;
    }
    if(log_base_line == log_curr_line){
        log[0] = 0;
        return tmp;
    }
    lock(&s_log_lock);
    if(log_base_line < log_curr_line){
        for(i = (log_curr_line - 1); i >= log_base_line; i--){
            tmp += snprintf(log + tmp, size - tmp, "%s", s_log_buffer_lines[i]);
        }
    }else{
        for(i = (DBG_LOG_MAX_LINES - 1); i >= log_base_line; i--){
            tmp += snprintf(log + tmp, size - tmp, "%s", s_log_buffer_lines[i]);
        }
        for(i = (log_curr_line - 1); i >= 0; i--){
            tmp += snprintf(log + tmp, size - tmp, "%s", s_log_buffer_lines[i]);
        }
    }
    unlock(&s_log_lock);

    return tmp;
}

int log_init(const char *file, int level)
{
    log_file_fd = -1;

    lock_init(&s_log_lock);
    s_log_level = level;

    if(file){
        int temp_fd;
        char dir[128];
        char *tmp;

        if(!strcasecmp(file, "syslogd")){
            log_type = LOG_TYPE_SYSLOGD;
            openlog("broadvis", LOG_NDELAY, LOG_DAEMON);

            return 0;
        }
        if(!strcasecmp(file, "console")){
            log_type = LOG_TYPE_CONSOLE;

            return 0;
        }
        strcpy(dir, file);
        if((tmp = strrchr(dir, '/')) != NULL){
            char cmd[128];

            tmp[0] = 0;
            sprintf(cmd, "mkdir -p %s", dir);
            system(cmd);
        }
        temp_fd = open(file, O_RDWR | O_APPEND | O_SYNC | O_CREAT, 0655);
        if(temp_fd < 0){
            DBG(DBG_ERR, "can't open log file %s\n", file);
            log_type = LOG_TYPE_CONSOLE;

            return -1;
        }
        dup2(temp_fd, 1);
        dup2(temp_fd, 2);
        log_file_fd = temp_fd;
        log_type = LOG_TYPE_TEXT_FILE;
    }

    return 0;
}

int log_exit(void)
{
    if(log_type == LOG_TYPE_SYSLOGD){
        closelog();

        return 0;
    }
    if(log_file_fd > 0){
        close(log_file_fd);
        log_file_fd =  -1;
    }

    return 0;
}

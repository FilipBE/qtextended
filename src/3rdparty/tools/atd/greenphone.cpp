
#ifdef QT_QWS_GREENPHONE

#include "greenphone.h"

#include <linux/rtc.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define BCD_TO_INT(x) ((10 * (((x) >> 4) & 0x0f)) + ((x) & 0x0f))
#define INT_TO_BCD(x) ((16 * (((x) / 10) % 10)) + ((x) % 10))

void write_pcf_reg(const char *reg, unsigned char value)
{
    int regfd = open(reg, O_WRONLY);
    if (regfd == -1) {
#ifdef DAEMON
        syslog(LOG_ERR, "/proc/pcf/RTC*: %m\n");
        closelog();
#else
        perror("/proc/pcf/RTC*");
#endif
        exit(errno);
    }

    char buffer[5];
    snprintf(buffer, 16, "0x%02x", value);

    write(regfd, buffer, 4);

    close(regfd);
}

unsigned char read_pcf_reg(const char *reg)
{
    int regfd = open(reg, O_RDONLY);
    if (regfd == -1) {
#ifdef DAEMON
        syslog(LOG_ERR, "/proc/pcf/RTC*: %m\n");
        closelog();
#else
        perror("/proc/pcf/RTC*");
#endif
        exit(errno);
    }

    char buffer[6];
    read(regfd, buffer, 6);
    buffer[5] = '\0';
    buffer[4] = '\0';

    close(regfd);

    unsigned int value;
    value = strtoul(buffer, NULL, 16);

    return value;
}


void greenphone_read_time(struct rtc_time *tm)
{
    unsigned char tmp;

    tmp = read_pcf_reg(RTCSC);
    tm->tm_sec = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCMN);
    tm->tm_min = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCHR);
    tm->tm_hour = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCWD);
    tm->tm_wday = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCDT);
    tm->tm_mday = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCMT);
    tm->tm_mon = BCD_TO_INT(tmp) - 1;

    tmp = read_pcf_reg(RTCYR);
    tm->tm_year = 68 + BCD_TO_INT(tmp);

    tm->tm_yday = -1;
    tm->tm_isdst = -1;
}

void greenphone_read_alarm(struct rtc_time *tm)
{
    unsigned char tmp;

    tmp = read_pcf_reg(RTCSCA);
    tm->tm_sec = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCMNA);
    tm->tm_min = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCHRA);
    tm->tm_hour = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCWDA);
    tm->tm_wday = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCDTA);
    tm->tm_mday = BCD_TO_INT(tmp);

    tmp = read_pcf_reg(RTCMTA);
    tm->tm_mon = BCD_TO_INT(tmp) - 1;

    tmp = read_pcf_reg(RTCYRA);
    tm->tm_year = 68 + BCD_TO_INT(tmp);

    tm->tm_yday = -1;
    tm->tm_isdst = -1;
}

void greenphone_set_alarm(struct rtc_time *tm)
{
    write_pcf_reg(RTCSCA, INT_TO_BCD(tm->tm_sec));
    write_pcf_reg(RTCMNA, INT_TO_BCD(tm->tm_min));
    write_pcf_reg(RTCHRA, INT_TO_BCD(tm->tm_hour));
    write_pcf_reg(RTCWDA, INT_TO_BCD(tm->tm_wday));
    write_pcf_reg(RTCDTA, INT_TO_BCD(tm->tm_mday));
    write_pcf_reg(RTCMTA, INT_TO_BCD(tm->tm_mon + 1));
    write_pcf_reg(RTCYRA, INT_TO_BCD(tm->tm_year - 68));
}

void greenphone_disable_alarm()
{
    write_pcf_reg(RTCSCA, 0x7f);
    write_pcf_reg(RTCMNA, 0x7f);
    write_pcf_reg(RTCHRA, 0x3f);
    write_pcf_reg(RTCWDA, 0x07);
    write_pcf_reg(RTCDTA, 0x3f);
    write_pcf_reg(RTCMTA, 0x1f);
    write_pcf_reg(RTCYRA, 0xff);
}

#endif


#ifndef GREENPHONE_H
#define GREENPHONE_H

#ifdef QT_QWS_GREENPHONE

#define RTCYR  "/proc/pcf/RTCYR"
#define RTCMT  "/proc/pcf/RTCMT"
#define RTCDT  "/proc/pcf/RTCDT"
#define RTCWD  "/proc/pcf/RTCWD"
#define RTCHR  "/proc/pcf/RTCHR"
#define RTCMN  "/proc/pcf/RTCMN"
#define RTCSC  "/proc/pcf/RTCSC"

#define RTCYRA "/proc/pcf/RTCYRA"
#define RTCMTA "/proc/pcf/RTCMTA"
#define RTCDTA "/proc/pcf/RTCDTA"
#define RTCWDA "/proc/pcf/RTCWDA"
#define RTCHRA "/proc/pcf/RTCHRA"
#define RTCMNA "/proc/pcf/RTCMNA"
#define RTCSCA "/proc/pcf/RTCSCA"

void write_pcf_reg(const char *reg, unsigned char value);
unsigned char read_pcf_reg(const char *reg);
void greenphone_read_time(struct rtc_time *tm);
void greenphone_read_alarm(struct rtc_time *tm);
void greenphone_set_alarm(struct rtc_time *tm);
void greenphone_disable_alarm();

#endif

#endif

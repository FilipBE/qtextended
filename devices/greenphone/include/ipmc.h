#ifndef	__IPMC_H
#define	__IPMC_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <asm/xscale-pmu.h>
#include <asm/arch/omega_sysdevs.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#endif

/* Use 'K' as magic number */
#define IPMC_IOC_MAGIC  'K'

/* To keep compitable with old cpufreqd user application.  */
#if 0
#define IPMC_IOCTL_GET 	_IOR(IPMC_IOC_MAGIC, 1, struct ipm_config) 
#define IPMC_IOCTL_SET  _IOW(IPMC_IOC_MAGIC, 2, struct ipm_config)

#endif
#define IPMC_IOCTL_GET_IPM_CONFIG  _IOR(IPMC_IOC_MAGIC, 3, struct ipm_config)
#define IPMC_IOCTL_SET_IPM_CONFIG  _IOW(IPMC_IOC_MAGIC, 4, int)

#define IPMC_IOCTL_GET_DVFM_CONFIG  _IOR(IPMC_IOC_MAGIC, 3, struct ipm_config)
#define IPMC_IOCTL_SET_DVFM_CONFIG  _IOW(IPMC_IOC_MAGIC, 4, struct ipm_config)

#define IPMC_IOCTL_GET_EVENT   _IOR(IPMC_IOC_MAGIC, 5, int)
#define IPMC_IOCTL_SET_EVENT   _IOW(IPMC_IOC_MAGIC, 6, int)

#define IPMC_IOCTL_RESET_UI_TIMER _IOW(IPMC_IOC_MAGIC, 8, int)

#define	IPMC_IOCTL_GET_SLEEPTIME  _IOR(IPMC_IOC_MAGIC, 9, int) 
#define	IPMC_IOCTL_SET_SLEEPTIME  _IOW(IPMC_IOC_MAGIC, 10, int) 

#define	IPMC_IOCTL_GET_WAKETIME  _IOR(IPMC_IOC_MAGIC, 11, int) 
#define	IPMC_IOCTL_SET_WAKETIME  _IOW(IPMC_IOC_MAGIC, 12, int) 

#define	IPMC_IOCTL_GET_UITIME	_IOR(IPMC_IOC_MAGIC, 13, int)
#define	IPMC_IOCTL_SET_UITIME	_IOW(IPMC_IOC_MAGIC, 14, int)

#define	IPMC_IOCTL_GET_SLEEPLEVEL _IOR(IPMC_IOC_MAGIC, 15, int)
#define	IPMC_IOCTL_SET_SLEEPLEVEL _IOW(IPMC_IOC_MAGIC, 16, int)

#define	IPMC_IOCTL_STARTPMU	 _IOW(IPMC_IOC_MAGIC, 17, int)
#define	IPMC_IOCTL_STOPPMU	_IOW(IPMC_IOC_MAGIC, 18, struct pmu_results)

#define	IPMC_IOCTL_CLEAR_EVENTLIST _IOW(IPMC_IOC_MAGIC, 20, int)

#define	IPMC_IOCTL_SYSSTATER	 _IOR(IPMC_IOC_MAGIC, 21, struct sysdevs)

/*Tony.He 2004-10-13*/
//#define	IPMC_IOCTL_ENABLE_IPMD	_IO(IPMC_IOC_MAGIC,22)	//enable the sleep source and wake up source
//#define	IPMC_IOCTL_SEND_EVENT_MACHTYPE	_IOW(IPMC_IOC_MAGIC,23,struct ipm_event)
/*send the defined command to the device*/
#define	IPMC_IOCTL_SEND_PMCOMM	_IOW(IPMC_IOC_MAGIC,22,int)
#define	IPMC_IOCTL_SET_SLEEPSRC	_IOW(IPMC_IOC_MAGIC,23,unsigned long)
#define	IPMC_IOCTL_GET_DEV_STATUS	_IOR(IPMC_IOC_MAGIC,24,unsigned long)


#define NOVOLCHG    0
#define HIGHER      1
#define LOWER       2

#define OMEGAPM_POWEROFF  0
#define OMEGAPM_U312MHZ   1
#define OMEGAPM_U208MHZ   2
#define OMEGAPM_U104MHZ   3
#define OMEGAPM_U26MHZ1   4
#define OMEGAPM_U26MHZ2   5
#define OMEGAPM_F104MHZ   6
#define OMEGAPM_F26MHZ    7
#define OMEGAPM_SLEEP     8

#define MAX_IPME_NUM 20	/*	20 IPM event max */
/*      IPM events queue */


struct  ipm_config {
    /*  Below  items must be set to set configurations. */
    unsigned int    cpu_mode;
    unsigned int    core_freq;  /*  in khz, run mode.*/
                                /*  If CPU is in Trubo mode. plus N. */
    unsigned int    core_vltg;    /*  in mV.  */
    unsigned int    turbo_mode;     /*  specify the N value.    */
    unsigned int    fast_bus_mode;
    unsigned int    mem;
    /*  Below items may need to get system DPM configurations.  */
    unsigned int    sys_bus_freq;
    unsigned int    mem_bus_freq;
    unsigned int    lcd_freq;
    unsigned int    enabled_device;
    unsigned int    powermode;
    /*The sleep event*/
    unsigned int    sleep_src_event;
};


struct fv_table {
    unsigned int freq;
    unsigned int voltage;
};

struct ipm_event {
	unsigned int type;	/*	What type of IPM events.	*/
	unsigned int kind;	/*	What kind, or sub-type of events.*/
//	void *infodata;		/*	events specific data.	*/
	unsigned int info;
};

#ifdef __KERNEL__
typedef struct{
	unsigned int	dev_initialized;
	/*if wakeup source need send wake up msg to the ipmd*/
	int need_wakeup_src;
	/*if sleep source need send sleep msg to the ipmd*/
	int need_sleep_src;

	/*The last time to press keyboard or press touch panel*/
	unsigned long	prev_act_time;
	/*the time if no action, the device will send a msg to the ipmd*/
	unsigned long	sleep_interval_time;
	/*for saving system resource*/
	unsigned long	precision_time;

	/*timer for sending sleep msg to the ipmd*/
	struct timer_list	action_timer;
	unsigned int		timer_enabled;
	
	spinlock_t		timer_lock;
	
}dev_sleep_data_t;

struct ipme_event_list{
	struct ipm_event	event;
	struct list_head	list;
};

struct ipme_queue{
        int head;
        int tail;
        int len;
        struct ipme_event_list  ipmes[MAX_IPME_NUM];
	struct	list_head	used_list;
	struct	list_head	free_list;
        wait_queue_head_t waitq;
};
#endif

#define	IPM_EVENTS_TYPE(x)   ((x&0xFF000000)>>24)
#define	IPM_EVENTS_KIND(x)   ((x&0x00F00000)>>16)
#define	IPM_EVENTS_INFO(x)   (x&0xFF)

/*	IPM event types.	*/
#define	IPM_EVENT_PROFILER	0x0		/*	Profiler events.	*/
#define	IPM_EVENT_PMU		0x1		/*	PMU events, may not need.	*/
#define	IPM_EVENT_DEVICE	0x2		/*	Device event.	*/
#define	IPM_EVENT_POWER		0x3		/*	Power fail/revocer event.	*/
#define	IPM_EVENT_TIMER		0x4		/*	Device Timer timeout event.	*/
#define	IPM_EVENT_CPU		0x5		/*	CPU utilization change  event.	*/
#define	IPM_EVENT_SYSTEM	0x6

/*	IPM event kinds.	*/
#define	IPM_EVENT_DEVICE_TIMEOUT	0x0
#define	IPM_EVENT_POWER_LOW		0x1
#define	IPM_EVENT_POWER_FAULT	0x2
#define	IPM_EVENT_POWER_OK		0x3
#define	IPM_EVENT_SYSTEM_WAKEUP	0x4
#define IPM_EVENT_DEVICE_FLIP1	0x5
#define IPM_EVENT_DEVICE_FLIP3	0x6
#define IPM_EVENT_DEVICE_FLIP4	0x7
#define IPM_EVENT_DEVICE_CAMERA	0x8
#define IPM_EVENT_DEVICE_AUDIO	0x9
#define IPM_EVENT_DEVICE_OVERLAY2	0xA
#define IPM_EVENT_DEVICE_TVENCODER	0xB
#define IPM_EVENT_DEVICE_CHARGER	0xC

/*Tony.He 2004-10-10*/
//#ifdef CONFIG_KEY_TOUCH_PM_CONTRL
#define	IPM_EVENT_SYSTEM_SLEEP	0xD
//#define	IPM_EVENT_NO_PRESS	0x40
//#endif //
#ifdef CONFIG_PM_GROUP
#define	IPM_EVENT_NO_DEVICE_USED	0xE
#define	IPM_EVENT_DEVICE_USED		0xF
#define	IPM_EVENT_RESTART_PROFILE	0x10
#define	IPM_EVENT_STOP_PROFILE		0x11
#define	IPM_EVENT_DETECT_ACTION		0x12
#define	IPM_EVENT_DEVICE_FLIP14		0x13
#endif //CONFIG_PM_GROUP

#define	IPM_EVENT_IDLE_PROFILER	0x0
#define	IPM_EVENT_PERF_PROFILER	0x1

#define	IPM_EVENT_CPUVERYBUSY		0x0		/*	report CPU is very busy now.	*/
#define	IPM_EVENT_CPUBUSY		0x1		/*	report CPU is very busy now.	*/
#define	IPM_EVENT_CPUFREE		0x2		/*	report CPU is free now.	*/

/*	IPM event infos, not defined yet.	*/
#define	IPM_EVENT_NULLINFO	0x0
#define	IPM_WAKEUPBYRTC		0x1
#define	IPM_WAKEUPBYUI		0x2
#define IPM_DEVICE_ON		0x3
#define IPM_DEVICE_OFF		0x4


/*	IPM functions	*/
#ifdef __KERNEL__
void 	ipm_event_notify(int  type, int kind, int info);
#endif

#define __FREQ_LOCK
#ifdef __FREQ_LOCK

#define CAMERA_DEVPRI	3
#define OVLAY2_DEVPRI	2
#define AUDIO_DEVPRI	1
#define OTHER_DEVPRI	0

void cpu_freq_lock(int L, int N, int fast_bus, int memf_div, int _dev_pri);
void cpu_freq_unlock(void);
#endif

#endif

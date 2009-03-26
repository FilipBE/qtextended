/*
 * used for the detect actions
*/
#ifndef _OMEGA_DETECT_H_
#define	_OMEGA_DETECT_H_

#ifdef __KERNEL__
#include <linux/file.h>
#endif //__KERNEL__
#include <asm/arch/omega_sysdevs.h>
#include <asm/arch/omega.h>

#ifdef __KERNEL__
#define	DETECT_DEBUG(x)	
//The following two lines is commented by Bill Chen 2005.12.6
#define	DETECT_ALERT_ERROR(x,arg...)	//OMEGA_FORMAT_MESSAGE("Omega_DETECT",0,x,##arg)
#define	DETECT_ALERT_WARNING(x,arg...)	//OMEGA_FORMAT_MESSAGE("Omega_DETECT",0,x,##arg)
#endif //__KERNEL__
//#define	DETECT_ALERT_INFO(format,arg...)	printk(KERN_INFO __FILE "(%d):"format"\n",##arg)

#define	OMEGA_DETECT_MAJOR	102

#if 0
#define	DETECT_POWER_OFF	( (OMEGA_DETECT_MAJOR<<8)+POWER_OFF )
#define	DETECT_LOW_POWER	( (OMEGA_DETECT_MAJOR<<8)+LOW_POWER )
#define	DETECT_AV_PLUGIN	( (OMEGA_DETECT_MAJOR<<8)+AV_PLUGIN )
#define	DETECT_HEADSET_PLUGIN	( (OMEGA_DETECT_MAJOR<<8)+HEADSET_PLUGIN )
#define	DETECT_FLIP_SENSE_3	( (OMEGA_DETECT_MAJOR<<8)+FLIP_SENSE_3 )
#define	DETECT_FLIP_SENSE_4	( (OMEGA_DETECT_MAJOR<<8)+FLIP_SENSE_4 )
#define	DETECT_ALARM_TIMEOUT	( (OMEGA_DETECT_MAJOR<<8)+ALARM_TIMEOUT )
#define	DETECT_AV_HEADSET_PLUGOUT	( (OMEGA_DETECT_MAJOR<<8)+AV_HEADSET_PLUGOUT )
#define	DETECT_AV_HEADSET_PLUGERR	( (OMEGA_DETECT_MAJOR<<8)+AV_HEADSET_PLUGERR )
#endif 

#define	REGISTER_SIGNAL		( (OMEGA_DETECT_MAJOR<<8)+0x80 )
#define	SET_STATUS		( (OMEGA_DETECT_MAJOR<<8)+0x90 )

#ifdef __KERNEL__
typedef struct{
	struct list_head	action_list;
	unsigned int 	action_ref_num;
	//int		valid;
	//int		active;
	detect_device_t	*status_p;
	int		(*on)(void);
	int		(*off)(void);
}detect_action_t;

typedef struct{
	pid_t		pid;
	struct task_struct	*tsk;
	int		sig;
	spinlock_t	omega_detect_lock;
	/*action number*/
	int		action_num;
	struct list_head	action_head;
}omega_detect_t;

/*global detect action data*/
extern int register_detect_device(int detect_number,int (*on)(void),int(*off)(void));
extern int unregister_detect_device(int detect_number);
extern int mark_detect_device(int detect_number,int action,int extra);
extern int initialize_detect_device(int detect_number,int action,int extra);
extern uint8_t read_detect_status(int detect_number);

/*
 * set gpio pin data
*/
static inline void setgpio_outhigh(int gpio_num)
{
	if(gpio_num < 32)
		GPSR0 |= (1 << (gpio_num % 32));
	else if(gpio_num < 64)
		GPSR1 |= (1 << (gpio_num % 32));
	else if(gpio_num < 96)
		GPSR2 |= (1 << (gpio_num % 32));
	else if(gpio_num < 120)
		GPSR3 |= (1 << (gpio_num % 32));
}

static inline void setgpio_outlow(int gpio_num)
{
	
	if(gpio_num < 32)
		GPCR0 |= (1 << (gpio_num % 32));
	else if(gpio_num < 64)
		GPCR1 |= (1 << (gpio_num % 32));
	else if(gpio_num < 96)
		GPCR2 |= (1 << (gpio_num % 32));
	else if(gpio_num < 120)
		GPCR3 |= (1 << (gpio_num % 32));
}
#endif //__KERNEL__
#endif //_OMEGA_DETECT_H_

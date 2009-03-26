/*
 * linux/include/asm-arm/arch-pxa/sysdevs.h
 *
 * Created:	Sep 7, 2004
 * Copyright:	
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __OMEGA_SYSDEVS_H
#define __OMEGA_SYSDEVS_H

/* sysdevs->pmmode*/
#define	OMEGAPM_POWEROFF	0
#define	OMEGAPM_U312MHZ	1
#define	OMEGAPM_U208MHZ	2
#define	OMEGAPM_U104MHZ	3
#define	OMEGAPM_U26MHZ1	4
#define	OMEGAPM_U26MHZ2	5
#define	OMEGAPM_F104MHZ	6
#define	OMEGAPM_F26MHZ	7
#define	OMEGAPM_SLEEP	8

/* sysdevs->bootsrc */
#define OMEGABOOT_NORM      0
#define OMEGABOOT_CHG       1
#define OMEGABOOT_ALRM      2
#define OMEGABOOT_WDR       3
#define OMEGABOOT_REBOOT    4

/*
 * detect device id
*/
#define	DEV_ON			1
#define	DEV_OFF			0
#define	TOOLOW_POWER		2
#define	NORMAL_POWER		1
#define	LOW_POWER		0
#define	AV_ON			1
#define	HEADSET_ON		2
#define	AVHEADSET_ERR		3
#define MODEM_OFF		0
#define MODEM_ON		1
#define MODEM_CRASH		2

#define	CHARGER_DETECT		1
#define	AVHEADSET_DETECT	2
#define	SDCARD_DETECT		3
#define	USBIN_DETECT		4
#define	FLIPSENSOR3_DETECT	5
#define	FLIPSENSOR4_DETECT	6
#define	FLIPSENSOR1_DETECT	7
#define	MODEM_DETECT		8
#define	HANDFREE_DETECT		9
#define	LCDSYNC_DETECT		10
#define	LCD_DETECT		11
#define	CALLING_DETECT		12
#define	CAMERA_DETECT		13
#define	BOOTSRC_DETECT		14
#define	TV_DETECT		15
#define	PMMODE_DETECT		16
#define	LOWPOWER_DETECT		17
/**/
#define	SYSTEM_SLEEP_DETECT	18
#define	SYSTEM_WAKEUP_DETECT	19
/*control device*/
#define	LCDBL_DETECT		20
#define	CHGLED_DETECT		21
#define	SYSLED_DETECT		22
#define	KPBL_DETECT		23
#define	VIBRATOR_DETECT		24

#ifndef __KERNEL__
typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
#endif //

typedef struct{
	uint8_t	dev_id:8;
	uint8_t	status:4;
	uint8_t	rdclear:4;
	uint16_t	extra:16;
}detect_device_t;

typedef struct sysdevs
{
	int	sigsrc;		// indicate which signal of accessory plugin detected
	/* signal in detect */
	detect_device_t	chargerin;	// 1-charger plugin	0-charger not plugin

	detect_device_t	avheadsetin;	// 1-av cable plugin	2-headseet plugin	3-cable plugin error	0-off
	//int	headsetin;	// 1-headset plugin	0-heasset not plugin
	//int	avcablein;	// 1-av cable plugin	0-av cable not plugin
	detect_device_t	sdcardin;	// 1-sd card plugin	0-sd card not plugin
	detect_device_t	usbin;		// 1-usb cable plugin	0-usb cable not plugin
	detect_device_t	flipsensor3;	// 1-flipsensor	high	0-flipsensor low
	detect_device_t	flipsensor4;	// 1-flipsensor	high	0-flipsensor low
	detect_device_t	flipsensor1;	// 1-flipsensor	high	0-flipsensor low
	/* status only for check */
	detect_device_t	modemon;	// 1-modem poweron	0-modem poweroff        2-modem crash
	detect_device_t	handfree;	// 1-handfree function on	0-handfree function off
	detect_device_t	lcdsync;	// 1-lcd extern sync	0-lcd internal sync
	detect_device_t	lcd;		// 1-lcd on		0-lcd off
	detect_device_t	calling;	// 1-calling		0-no call
	detect_device_t	camera;		// 1-camera on		0-camera off
	detect_device_t	bootsrc;	// 0-normal boot	1-charger in	3-alarm
	detect_device_t	tv;		// 1-on			0-off
	detect_device_t	pmmode;		// 
	detect_device_t	lowpower;	// 1-normal		0-low
	detect_device_t	system_sleep;	// 1-system need sleep	0-no msg
	detect_device_t	system_wakeup;	// 1-system need wakeup 0-no msg
	/* simple control device */
	detect_device_t	lcdbl;		// 1-lcd backlight on	0-lcd backlight off
	detect_device_t	chgled;		// 1-charger indicator led on	0-charger indicator led off
	detect_device_t	sysled;		// 1-system led on	0-system led off
	detect_device_t	kpbl;		// 1-keypad backlight on	0-keypad backlight off
	detect_device_t	vibrator;	// 1-vibrator on	0-vibrator off
	detect_device_t	enddevice;	//virtual device to end.
}sysdevs_t;

extern sysdevs_t	omega_sysdevs;
#ifdef	__KERNEL__
typedef	struct{
	uint8_t	on_send:4;
	uint8_t	off_send:4;
}dev_action_cmd_t;
#endif //

#endif

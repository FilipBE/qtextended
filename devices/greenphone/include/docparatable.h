#ifndef __DOC_PARAT_H__
#define __DOC_PARAT_H__

#ifdef __KERNEL__

#include <linux/config.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/mtd/compatmac.h>
#undef recalc_sigpending
#undef daemonize
#include <linux/module.h>
#include <linux/uio.h>

#endif /* __KERNEL__ */

#include <linux/ioctl.h>




#define PARA_TABLE_NAME	"DOC Parameter table"

//#define PARA_TABLE_BASEADDR	0x1EE0000
#define PARA_TABLE_LEN		0x40000

#define PARA_BARCODE_OFFSET	0x0
#define PARA_BARCODE_LENGTH	20

#define PARA_AUDIO_OFFSET	0x100
#define PARA_AUDIO_LENGTH	112

#define PARA_ADC_OFFSET	0x200
#define PARA_ADC_LENGTH	16

#define PARA_REVERSE_OFFSET	0x300
#define PARA_REVERSE_LENGTH	20

#define PARA_PHASE_OFFSET	0x400
#define PARA_PHASE_LENGTH	2

#define PARA_ALERT_OFFSET	0x500
#define PARA_ALERT_LENGTH	64

#define PARA_VERSION_OFFSET	0x1000
#define PARA_VERSION_LENGTH	20

#define PARA_TATRESULT_OFFSET	0x1100
#define PARA_TATRESULT_LENGTH	20

#define PARA_PASSWORD_OFFSET	0x1200
#define PARA_PASSWORD_LENGTH	8

#define PARA_PANIC_OFFSET	0x1300
#define PARA_PANIC_LENGTH	0x800

#define PARA_DEBUG_OFFSET	0x2000
#define PARA_DEBUG_LENGTH	2

#define PARA_BT_MAC_ADDR_OFFSET    0x2100
#define PARA_BT_MAC_ADDR_LENGTH    6

#define PARA_PRODUCT_SN_INFO_OFFSET          0x2200
#define PARA_PRODUCT_SN_INFO_LENGTH         32

#define PARA_BOARD_SN_INFO_OFFSET          0x2300
#define PARA_BOARD_SN_INFO_LENGTH         32

#define PARA_HW_VER_OFFSET  0x2400
#define PARA_HW_VER_LENGTH 32

#define PARA_STATION_FLAG_OFFSET  0x2500
#define PARA_STATION_FLAG_LENGTH 32

#define DOCPTGETBARCODE		_IOC(_IOC_READ,0x21,5,PARA_BARCODE_LENGTH)
#define DOCPTGETAUDIOT		_IOC(_IOC_READ,0x21,6,PARA_AUDIO_LENGTH)
#define DOCPTGETADCDACT		_IOC(_IOC_READ,0x21,7,PARA_ADC_LENGTH)
#define DOCPTGETALERTR		_IOC(_IOC_READ,0x21,8,PARA_ALERT_LENGTH)
#define DOCPTSETALERTR		_IOC(_IOC_WRITE,0x21,9,PARA_ALERT_LENGTH)
#define DOCPTSETPASSWD		_IOC(_IOC_WRITE,0x21,10,PARA_PASSWORD_LENGTH)
#define DOCPTGETPASSWD		_IOC(_IOC_READ,0x21,11,PARA_PASSWORD_LENGTH)
#define DOCPTSETDEBUG		_IOC(_IOC_WRITE,0x21,12,PARA_DEBUG_LENGTH)
#define DOCPTGETDEBUG		_IOC(_IOC_READ,0x21,13,PARA_DEBUG_LENGTH)

#define DOCPTSETBTMACADDR		_IOC(_IOC_WRITE,0x21,14,PARA_BT_MAC_ADDR_LENGTH)
#define DOCPTGETBTMACADDR		_IOC(_IOC_READ,0x21,15,PARA_BT_MAC_ADDR_LENGTH)

#define DOCPTSETPRODUCTSN		_IOC(_IOC_WRITE,0x21,16,PARA_PRODUCT_SN_INFO_LENGTH)
#define DOCPTGETPRODUCTSN		_IOC(_IOC_READ,0x21,17,PARA_PRODUCT_SN_INFO_LENGTH)

#define DOCPTSETBOARDSN		_IOC(_IOC_WRITE,0x21,18,PARA_BOARD_SN_INFO_LENGTH)
#define DOCPTGETBOARDSN		_IOC(_IOC_READ,0x21,19,PARA_BOARD_SN_INFO_LENGTH)

#define DOCPTSETHWVER		_IOC(_IOC_WRITE,0x21,20,PARA_HW_VER_LENGTH)
#define DOCPTGETHWVER		_IOC(_IOC_READ,0x21,21,PARA_HW_VER_LENGTH)

#define DOCPTSETSTATIONFLAG		_IOC(_IOC_WRITE,0x21,22,PARA_STATION_FLAG_LENGTH)
#define DOCPTGETSTATIONFLAG		_IOC(_IOC_READ,0x21,23,PARA_STATION_FLAG_LENGTH)

enum eParaTType
{
	eParaT_Barcode = 0,
	eParaT_Audio,
	eParaT_ADC,
	eParaT_Reverse,
	eParaT_Phase,
	eParaT_Alert,
	eParaT_Version,
	eParaT_TatResult,
	eParaT_PassWD,
	eParaT_Panic,
	eParaT_Debug,
	eParaT_BTMacAddr,
	eParaT_ProductSN,
	eParaT_BoardSN,
	eParaT_HWVer,
	eParaT_StationFlag,
	eParaT_Max
};
struct DOC_block_device_operations {
	int (*open) (struct inode *, struct file *);
	int (*release) (struct inode *, struct file *);	
	loff_t (*llseek)(struct file *file, loff_t offset, int origin);
	ssize_t (*read)( struct file *file,
			  char *buffer,
			  size_t len,
			  loff_t *offset );
	ssize_t (*write)( struct file *file,
			  char *buffer,
			  size_t len,
			  loff_t *offset );
	int (*ioctl) (struct inode *, struct file *, unsigned, unsigned long);
	struct module *owner;
};

extern int Docparat_read( enum eParaTType type, unsigned char* buffer, int offset, int len );
extern int Docparat_readADDA3600( unsigned short* val );
extern int Docparat_readADDA4200( unsigned short* val );
#endif /* __DOC_PARAT_H__ */

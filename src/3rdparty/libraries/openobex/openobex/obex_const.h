/*
 *
 *  OpenOBEX - Free implementation of the Object Exchange protocol
 *
 *  Copyright (C) 1999-2000  Dag Brattli <dagb@cs.uit.no>
 *  Copyright (C) 1999-2000  Pontus Fuchs <pontus.fuchs@tactel.se>
 *  Copyright (C) 2001-2002  Jean Tourrilhes <jt@hpl.hp.com>
 *  Copyright (C) 2002-2006  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __OBEX_CONST_H
#define __OBEX_CONST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef union {
	uint32_t bq4;
	uint8_t bq1;
	const uint8_t *bs;
} obex_headerdata_t;

typedef struct {
	int (*connect)(obex_t *handle, void * customdata);
	int (*disconnect)(obex_t *handle, void * customdata);
	int (*listen)(obex_t *handle, void * customdata);
	int (*write)(obex_t *handle, void * customdata, uint8_t *buf, int buflen);
	int (*handleinput)(obex_t *handle, void * customdata, int timeout);
	void * customdata;
} obex_ctrans_t;

/* USB-specific OBEX interface information */
typedef struct {
	/* Manufacturer, e.g. Nokia */
	char *manufacturer;
	/* Product, e.g. Nokia 6680 */
	char *product;
	/* Product serial number */
	char *serial;
	/* USB device configuration description */
	char *configuration;
	/* Control interface description */
	char *control_interface;
	/* Idle data interface description, typically empty */
	char *data_interface_idle;
	/* Active data interface description, typically empty */
	char *data_interface_active;
	/* Internal information for the transport layer in the library */
	struct obex_usb_intf_transport_t *interface;
} obex_usb_intf_t;

/* Generic OBEX interface information */
typedef union {
	/* USB-specific OBEX interface information */
	obex_usb_intf_t usb;
	//obex_bluetooth_intf_t bt; // to be added
} obex_interface_t;

#define OBEX_CLIENT		0
#define OBEX_SERVER		1

/* Possible events */
#define OBEX_EV_PROGRESS	0	/* Progress has been made */
#define OBEX_EV_REQHINT		1	/* An incoming request is about to come */
#define OBEX_EV_REQ		2	/* An incoming request has arrived */
#define OBEX_EV_REQDONE		3	/* Request has finished */
#define OBEX_EV_LINKERR		4	/* Link has been disconnected */
#define OBEX_EV_PARSEERR	5	/* Malformed data encountered */
#define OBEX_EV_ACCEPTHINT	6	/* Connection accepted */
#define OBEX_EV_ABORT		7	/* Request was aborted */
#define OBEX_EV_STREAMEMPTY	8	/* Need to feed more data when sending a stream */
#define OBEX_EV_STREAMAVAIL	9	/* Time to pick up data when receiving a stream */
#define OBEX_EV_UNEXPECTED	10	/* Unexpected data, not fatal */
#define OBEX_EV_REQCHECK	11	/* First packet of an incoming request has been parsed */

/* For OBEX_Init() */
#define OBEX_FL_KEEPSERVER	0x02	/* Keep the server alive */
#define OBEX_FL_FILTERHINT	0x04	/* Filter devices based on hint bit */
#define OBEX_FL_FILTERIAS	0x08	/* Filter devices based on IAS entry */

/* For OBEX_ObjectAddHeader */
#define OBEX_FL_FIT_ONE_PACKET	0x01	/* This header must fit in one packet */
#define OBEX_FL_STREAM_START	0x02	/* Start of streaming body */
#define OBEX_FL_STREAM_DATA	0x04	/* Body-stream data */
#define OBEX_FL_STREAM_DATAEND	0x08	/* Body stream last data */
#define OBEX_FL_SUSPEND		0x10	/* Suspend after sending this header */

/* Transports */
#define OBEX_TRANS_IRDA		1
#define OBEX_TRANS_INET		2
#define OBEX_TRANS_CUST		3	/* Fixme: This will go away in future */
#define OBEX_TRANS_CUSTOM	3
#define OBEX_TRANS_BLUETOOTH	4
#define OBEX_TRANS_FD		5
#define OBEX_TRANS_USB		6

/* Standard headers */
#define OBEX_HDR_EMPTY		0x00 /* Empty header (buggy OBEX servers) */
#define OBEX_HDR_COUNT		0xc0 /* Number of objects (used by connect) */
#define OBEX_HDR_NAME		0x01 /* Name of the object */
#define OBEX_HDR_TYPE		0x42 /* Type of the object */
#define OBEX_HDR_LENGTH		0xc3 /* Total lenght of object */
#define OBEX_HDR_TIME		0x44 /* Last modification time of (ISO8601) */
#define OBEX_HDR_TIME2		0xC4 /* Deprecated use HDR_TIME instead */
#define OBEX_HDR_DESCRIPTION	0x05 /* Description of object */
#define OBEX_HDR_TARGET		0x46 /* Identifies the target for the object */
#define OBEX_HDR_HTTP		0x47 /* An HTTP 1.x header */
#define OBEX_HDR_BODY		0x48 /* Data part of the object */
#define OBEX_HDR_BODY_END	0x49 /* Last data part of the object */
#define OBEX_HDR_WHO		0x4a /* Identifies the sender of the object */
#define OBEX_HDR_CONNECTION	0xcb /* Connection identifier */
#define OBEX_HDR_APPARAM	0x4c /* Application parameters */
#define OBEX_HDR_AUTHCHAL	0x4d /* Authentication challenge */
#define OBEX_HDR_AUTHRESP	0x4e /* Authentication response */
#define OBEX_HDR_CREATOR	0xcf /* indicates the creator of an object */
#define OBEX_HDR_WANUUID	0x50 /* uniquely identifies the network client
					(OBEX server) */
#define OBEX_HDR_OBJECTCLASS	0x51 /* OBEX Object class of object */
#define OBEX_HDR_SESSIONPARAM	0x52 /* Parameters used in session
					commands/responses */
#define OBEX_HDR_SESSIONSEQ	0x93 /* Sequence number used in each OBEX
					packet for reliability */

/* Commands */
#define OBEX_CMD_CONNECT	0x00
#define OBEX_CMD_DISCONNECT	0x01
#define OBEX_CMD_PUT		0x02
#define OBEX_CMD_GET		0x03
#define OBEX_CMD_SETPATH	0x05
#define OBEX_CMD_SESSION	0x07 /* used for reliable session support */
#define OBEX_CMD_ABORT		0x7f
#define OBEX_FINAL		0x80

/* Responses */
#define	OBEX_RSP_CONTINUE		0x10
#define OBEX_RSP_SWITCH_PRO		0x11
#define OBEX_RSP_SUCCESS		0x20
#define OBEX_RSP_CREATED		0x21
#define OBEX_RSP_ACCEPTED		0x22
#define OBEX_RSP_NON_AUTHORITATIVE	0x23
#define OBEX_RSP_NO_CONTENT		0x24
#define OBEX_RSP_RESET_CONTENT		0x25
#define OBEX_RSP_PARTIAL_CONTENT        0x26
#define OBEX_RSP_MULTIPLE_CHOICES	0x30
#define OBEX_RSP_MOVED_PERMANENTLY	0x31
#define OBEX_RSP_MOVED_TEMPORARILY	0x32
#define OBEX_RSP_SEE_OTHER		0x33
#define OBEX_RSP_NOT_MODIFIED		0x34
#define OBEX_RSP_USE_PROXY		0x35
#define OBEX_RSP_BAD_REQUEST		0x40
#define OBEX_RSP_UNAUTHORIZED		0x41
#define OBEX_RSP_PAYMENT_REQUIRED	0x42
#define OBEX_RSP_FORBIDDEN		0x43
#define OBEX_RSP_NOT_FOUND		0x44
#define OBEX_RSP_METHOD_NOT_ALLOWED	0x45
#define OBEX_RSP_NOT_ACCEPTABLE		0x46
#define OBEX_RSP_PROXY_AUTH_REQUIRED	0x47
#define OBEX_RSP_REQUEST_TIME_OUT	0x48
#define OBEX_RSP_CONFLICT		0x49
#define OBEX_RSP_GONE			0x4a
#define OBEX_RSP_LENGTH_REQUIRED	0x4b
#define OBEX_RSP_PRECONDITION_FAILED	0x4c
#define OBEX_RSP_REQ_ENTITY_TOO_LARGE	0x4d
#define OBEX_RSP_REQ_URL_TOO_LARGE	0x4e
#define OBEX_RSP_UNSUPPORTED_MEDIA_TYPE	0x4f
#define OBEX_RSP_INTERNAL_SERVER_ERROR	0x50
#define OBEX_RSP_NOT_IMPLEMENTED	0x51
#define OBEX_RSP_BAD_GATEWAY		0x52
#define OBEX_RSP_SERVICE_UNAVAILABLE	0x53
#define OBEX_RSP_GATEWAY_TIMEOUT	0x54
#define OBEX_RSP_VERSION_NOT_SUPPORTED	0x55
#define OBEX_RSP_DATABASE_FULL		0x60
#define OBEX_RSP_DATABASE_LOCKED	0x61

/* Min, Max and default transport MTU */
#define OBEX_DEFAULT_MTU	1024
#define OBEX_MINIMUM_MTU	255
#define OBEX_MAXIMUM_MTU	65535

/* Optimum MTU for various transport (optimum for throughput).
 * The user/application has to set them via OBEX_SetTransportMTU().
 * If you are worried about safety or latency, stick with the current
 * default... - Jean II */
#define OBEX_IRDA_OPT_MTU	(7 * 2039)	/* 7 IrLAP frames */

#ifdef __cplusplus
}
#endif

#endif /* __OBEX_CONST_H */

#ifndef _INPUT_H_
#define _INPUT_H_
//#include "KeyCode.h"

#define KEY_HOMEPAGE	172	
#define KEY_HOME	3
#define KEY_BACK	4
#define KEY_UP		19
#define KEY_DOWN	20
#define KEY_LEFT	21
#define KEY_RIGHT	22
#define KEY_OK		23
#define KEY_VOL_PLUS	24
#define KEY_VOL_MINUS	25
#define KEY_MENU		82
#define KEY_DELETE		67
#define KEY_CLEAR		28

#define KEY_POWER		116

/*
 * Event types
 */
#define EV_SYN          0x00
#define EV_KEY          0x01
#define EV_REL          0x02
#define EV_ABS          0x03
#define EV_MSC          0x04
#define EV_SW           0x05
#define EV_LED          0x11
#define EV_SND          0x12
#define EV_REP          0x14
#define EV_FF           0x15
#define EV_PWR          0x16
#define EV_FF_STATUS        0x17
#define EV_MAX          0x1f
#define EV_CNT          (EV_MAX+1)


/* abs event */
#define ABS_MT_SLOT		0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */
#define ABS_MT_DISTANCE		0x3b	/* Contact hover distance */
/*
 * Synchronization events.
 */
#define SYN_REPORT      0
#define SYN_CONFIG      1
#define SYN_MT_REPORT       2



#endif 
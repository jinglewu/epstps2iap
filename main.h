#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <time.h>

#define VERSION "1.5"
#define VERSION_SUB "0"

#define POLL_TIMEOUT            180000
#define POLL_TIMEOUT_RETRY      1800

#define START_FLASH             3
#define RETURN_OK               1
#define RETURN_FAIL             0
#define DRIVER_RW_OK            1
#define SERIORAW_RW_OK          1
#define DRIVER_NOT_FOUND        -1
#define DRIVER_CANNOT_OPENDIR   -2
#define DRVCTL_NOT_FOUND        -3
#define DRVCTL_CANNOT_ACCESS    -4
#define DRVCTL_CANNOT_WRITE     -5
#define NO_SERIO_RAW_DRIVER      -6

#define SERIORAW_NOT_CREATED     -10
#define SERIORAW_CANNOT_ACCESS   -11
#define SERIORAW_CANNOT_READ     -12
#define SERIORAW_CANNOT_WRITE    -13
#define SERIORAW_CANNOT_OPEN     -14

#define IS_NOT_ELAN_PST         -20
#define READ_FW_VER_FAIL        -21
#define READ_MODULE_ID_FAIL     -22
#define DISABLE_PST_FAIL        -23
#define ENABLE_PST_FAIL         -24

#define READ_IAP_VER_FAIL       -30
#define ERROR_RANK_NUM          -31
#define ERROR_OPEN_BIN          -32
#define ERROR_BIN_SIZE          -33
#define ERROR_READ_BIN          -34
#define RESET_PST_FAIL          -35

#define ERROR_IAPCOMMAND        -40
#define ERROR_SETPASSWORD       -41
#define ERROR_GETSTATUS         -42
#define ERROR_FLASHKEY          -43

#define ERROR_END_BIN_ADDRESS   -50
#define ERROR_BIN_FILE          -51

#define SET_WRAP_MODE_FAIL      -60
#define WRITE_PAGE_FAIL         -61
#define SEND_CHECKSUM_FAIL      -62
#define RESET_WRAP_MODE_FAIL    -63
#define GETSTATUS_FAIL          -64
#define IAP_PAGE_ERROR          -65
#define IAP_INTERFACE_ERROR     -66

#define ERROR_READ_CHECKSUM          -70
#define ERROR_NOTCORRECT_CHECKSUM    -71


#define WRITE_RET_OK            1
#define WRITE_DATA_FAIL         -81
#define WRITE_NO_RESPONSE       -82
#define WRITE_RET_NO_ACK        -83
#define WRITE_RET_NO_WRAP       -84
#define WRITE_RET_NACK          -85
#define WRITE_RET_ERR           -86

#define READ_RET_OK             2
#define READ_RET_FAIL           -91
#define READ_NO_RESPONSE        -92
#define READ_WAIT_POLL_ERR      -93
#define READ_WAIT_POLL_TIMEOUT  -94

#define PS2MOUSE_BAT1     0xaa
#define PS2MOUSE_BAT2     0x0

#define BOOT_CODE_START_ADDRESS		0x0080
#define PSMOUSE_DRIVER_PATH         "/sys/bus/serio/devices/"
#define SERIO_RAW_PATH              "/dev/serio_raw"

#define IAP_STATE  	 	0
#define GET_FWVER_STATE  	1
#define GET_BINVER_STATE 	2
#define GET_MODULEID_STATE 	3
#define GET_SWVER_STATE		4
#define CREATE_SERIO_STATE	5
#define REMOVE_SERIO_STATE	6

typedef struct _FLASH_HANDLER
{

    int ERROR_CODE;
    unsigned long long  CURRENT_ADDRESS;
    unsigned long long 	TOTAL_FLASH_SIZE;		// 1.0.0.14

} FLASH_HANDLER;

typedef struct _FLASH_INFO
{
    unsigned long long StartWordAddress;
    unsigned long long EndWordAddress;
    const char  *ImagePathFile;
}FLASH_INFO, *PFLASH_INFO;

typedef struct _ROM_HEADER
{
    unsigned short		APP_PROG_START;
    unsigned long long 	BIN_SIZE;

} ROM_HEADER;

enum IAP_CONTROLS
{
    CTRL_StartIAP			= 0x0001,
    CTRL_IAP_ProgBusy		= 0x0002,
    CTRL_Reserve			= 0x0004,
    CTRL_EndIAP				= 0x0008,
    CTRL_IAP_InterfaceError = 0x0010,
    CTRL_IAP_PageError		= 0x0020,
    CTRL_SwitchToIAP		= 0x0040,
    CTRL_CheckPasswordOK	= 0x0080,
    CTRL_LastPage			= 0x0100,
    CTRL_LastWords_Fit		= 0x0200,
    CTRL_OnePage_FULL		= 0x0400,
};
#endif // MAIN_H

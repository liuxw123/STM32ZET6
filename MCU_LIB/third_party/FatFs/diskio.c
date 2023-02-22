/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "bsp/bsp_sd.h"

/* Definitions of physical drive number for each drive */
#define DEV_SDIO_SDCARD     0

#define SD_BLOCK_SIZE       (BLOCKSIZE)
#define SD_SECTOR_SIZE      (4096)
#define SD_SECTOR_COUNT     (1024)          // 4M = SD_SECTOR_COUNT * SD_SECTOR_SIZE


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case DEV_SDIO_SDCARD: return RES_OK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
) {
	switch (pdrv) {
	case DEV_SDIO_SDCARD:
	    if (MSD_OK == BSP_SD_Init()) {
	        return 0;
	    }
	    break;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
) {
	switch (pdrv) {
	case DEV_SDIO_SDCARD:
	    if (MSD_OK == BSP_SD_ReadBlocks_DMA(buff, sector, count)) {
	        return RES_OK;
	    }
	    break;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    switch (pdrv) {
    case DEV_SDIO_SDCARD:
        if (MSD_OK == BSP_SD_WriteBlocks_DMA((BYTE *)buff, sector, count)) {
            return RES_OK;
        }
        break;
    }

    return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv == DEV_SDIO_SDCARD) {
	    switch (cmd) {
	    case CTRL_SYNC:
	        return RES_OK;
	    case GET_SECTOR_SIZE:
	        *(WORD*)buff = SD_SECTOR_SIZE;
	        return RES_OK;
	    case GET_BLOCK_SIZE:
	        *(WORD*)buff = SD_BLOCK_SIZE;
            return RES_OK;
	    case GET_SECTOR_COUNT:
	        *(DWORD*)buff = SD_SECTOR_COUNT;
            return RES_OK;
	    }
	}

	return RES_PARERR;
}

DWORD get_fattime() {
    return 0;
}


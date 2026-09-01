#ifndef __RESOURCE_MANAGER_APPLICATION_HEADER__
#define __RESOURCE_MANAGER_APPLICATION_HEADER__

#define RESMGR_TAGNAME		(CONFIG_RESMGR_TAGNAME)
#define RESMGR_FINDBYID		(CONFIG_RESMGR_FINDBYID)
#define RESMGR_FINDBYTAG	(CONFIG_RESMGR_FINDBYTAG)
#define RESMGR_DATCHK		(CONFIG_RESMGR_DATCHK)

// RESOUCE PROTOCOL VER1.1 FORMAT:
//	|===============================================================|
//	|	00 01 02 03	|	04 05 06 07	|	08 09 0A 0B	|	0C 0D 0E 0F	|
//	|---------------------------------------------------------------|
//	|	RES_TAG		|	HDR_CRC32	|	PROT_VER	|	DATE_TIME	|
//	|	ITEM_CNT	|	 FW_ADDR	|	 FW_LEN	    |	USER_ID		|
//	|	ITEM1.ID	|	ITEM1.ADDR	|	ITEM1.LEN	|	ITEM1.CHK	|
//	|	ITEM2.ID	|	ITEM2.ADDR	|	ITEM2.LEN	|	ITEM2.CHK	|
//	|	.........	|	.........	|	.........	|	.........	|
//	|	ITEMx.ID	|	ITEMx.ADDR	|	ITEMx.LEN	|	ITEMx.CHK	|
//	|...............................................................|
//	|......................... ITEMs DATA ..........................|
//	|...............................................................|
//	|===============================================================|

typedef struct { uint32_t tagid, addr, size, crc32; } item_desc_t;
typedef struct {
	uint32_t hdrtag, hdrcrc;
	struct { uint16_t minor, major; } prot_ver;
	struct {
		uint32_t sec   : 6;
		uint32_t min   : 6;
		uint32_t hour  : 5;
		uint32_t day   : 5;
		uint32_t month : 4;
		uint32_t year  : 6;
	} date_time;
	uint32_t itemcnt, fwaddr, fwlen, userid;
	item_desc_t items[0];
} const *res_hdr_t;

void res_init();
void *res_item_by_id(uint32_t resid, uint32_t *psize);
void *res_item_by_name(uint32_t tagid, uint32_t *psize);

#endif//__RESOURCE_MANAGER_APPLICATION_HEADER__

#coding=utf-8

import os
import sys
import struct
import zlib
import time
import datetime
from configparser import ConfigParser

pver = 0x00010004     # VER=1.4
uuid = 0xA1B2C3D4     # UID
# RESOUCE PROTOCOL VER1.1 FORMAT:
#	|===============================================================|
#	|	00 01 02 03	|	04 05 06 07	|	08 09 0A 0B	|	0C 0D 0E 0F	|
#	|---------------------------------------------------------------|
#	|	RES_TAG		|	HDR_CRC32	|	PROT_VER	|	DATE_TIME	|
#	|	ITEM_CNT	|	 FW_ADDR	|	 FW_LEN	    |	USER_ID		|
#	|	ITEM1.ID	|	ITEM1.ADDR	|	ITEM1.LEN	|	ITEM1.CHK	|
#	|	ITEM2.ID	|	ITEM2.ADDR	|	ITEM2.LEN	|	ITEM2.CHK	|
#	|	.........	|	.........	|	.........	|	.........	|
#	|	ITEMx.ID	|	ITEMx.ADDR	|	ITEMx.LEN	|	ITEMx.CHK	|
#	|...............................................................|
#	|......................... ITEMs DATA ..........................|
#	|...............................................................|
#	|===============================================================|

class IniParser(ConfigParser):
    def __init__(self, defaults=None):
        ConfigParser.__init__(self, defaults=defaults)
    def optionxform(self, optionstr):
        return optionstr

def res_unpack(psrc, pdst, offs=0):
    bitv = lambda val, pos, len: ((val >> pos) & ((1 << len) - 1))
    with open(psrc, "rb") as fsrc:
        fsrc.seek(offs, 0)
        bhdr = fsrc.read(32)
        [tag_v, crc_v, ver_v, tim_v, cnt_v, adr_v, len_v, uid_v] = struct.unpack("<8I", bhdr)
        item = fsrc.read(cnt_v * 16)
        tag_s = "%c%c%c%c" % (bitv(tag_v, 0, 8), bitv(tag_v, 8, 8), bitv(tag_v, 16, 8), bitv(tag_v, 24, 8))
        ver_s = "%d.%d" % (bitv(ver_v, 16, 16), bitv(ver_v, 0, 16))
        tim_s = "%04d-%02d-%02d_%02d:%02d:%02d" % (2000 + bitv(tim_v, 26, 6), bitv(tim_v, 22, 4), bitv(tim_v, 17, 5)
            , bitv(tim_v, 12, 5), bitv(tim_v, 6, 6), bitv(tim_v, 0, 6))
        len_s = "%0.1fM bytes" % (len_v / pow(2, 20)) if len_v >= pow(2, 20) else "%0.1fK bytes" % (len_v / pow(2, 10))
        crc32 = zlib.crc32(bhdr[8:])
        crc32 = zlib.crc32(item, crc32)
        print("================================ header information ================================")
        print("[key]\t\t\t[value]\t\t[information]")
        print("resource header tag\t0x%08X\t%s" % (tag_v, tag_s))
        print("header crc32 value\t0x%08X\t%s" % (crc_v, "ok" if crc_v == crc32 else "[crc:0x%08X]" % crc32))
        print("protocol version\t0x%08X\t%s" % (ver_v, ver_s))
        print("build date time\t\t0x%08X\t%s" % (tim_v, tim_s))
        print("resouce item count\t0x%08X\t%d" % (cnt_v, cnt_v))
        print("firmware information\t0x%08X\t%s" % (adr_v, len_s))
        print("user/application id\t0x%08X\t%d" % (uid_v, uid_v))
        print("================================= item information =================================")
        print("[item]\t[name]\t[addr]\t\t[len]\t\t[crc32]\t\t[error]")
        if len(pdst) and not os.path.exists(pdst):
            os.mkdir(pdst)
        with open("%s/FWBIN.bin" % pdst, "wb") as fdst:
            fsrc.seek(adr_v, 0)
            data = fsrc.read(len_v)
            fdst.write(data)
        for i in range(cnt_v):
            [rid_v, adr_v, len_v, crc_v] = struct.unpack("<4I", item[i * 16: (i + 1) * 16])
            fsrc.seek(adr_v, 0)
            dat = fsrc.read(len_v)
            crc32 = zlib.crc32(dat)
            rid_s = "%c%c%c%c" % (bitv(rid_v, 0, 8), bitv(rid_v, 8, 8), bitv(rid_v, 16, 8), bitv(rid_v, 24, 8))
            if 0 == len_v:
                desc = "skip"
            else:
                desc = ""
                if crc32 != crc_v:
                    desc += "[crc:0x%08X]" % crc32
                if adr_v % 4:
                    desc += "[non-aligned]"
                if not len(desc):
                    desc = "ok"
                    if len(pdst):
                        with open("%s/%s.bin" % (pdst, rid_s), "wb") as fdst:
                            fdst.write(dat)
            print("%d\t%s\t0x%08X\t0x%08X\t0x%08X\t%s" % (i, rid_s, adr_v, len_v, crc_v, desc))
        print("====================================================================================")

def res_pack(pconf):
    align = 32       # resource align in bytes

    conf = IniParser()
    conf.read(pconf)

    fwbin = conf.get("Info", "FirmwareName")
    fdst = conf.get("Info", "NewFileName")
    base = conf.getint("Info", "ResStartAddr")
    tags = conf.get("Info", "ResHeaderTag")
    items = conf.items("List")
    cnts = len(items)
    with open(fdst, "wb") as fdst:
        # save build time
        now = datetime.datetime.now()
        with open("info.txt", "w") as finfo:
            info = "%04d-%02d-%02d_%02d:%02d:%02d" % (now.year, now.month, now.day, now.hour, now.minute, now.second)
            finfo.write(info)
            print("build:", info)
        time = ((now.year - 2000) << 26) + (now.month << 22) + (now.day << 17) + (now.hour << 12) + (now.minute << 6) + (now.second)
    
        # write firmware if exist
        fwsize = 0
        with open(fwbin, "rb") as fsrc:
            fdst.seek(0, 0)
            data = fsrc.read()
            if (fwsize := len(data)) > base:
                raise Exception("fw's size(%d) > ResStartAddr" % fwsize)
            fdst.write(data)

        # seek over firmware space, parese file and fill the item header
        faddr = (base + (32 + cnts * 16) + align - 1) // align * align      # force alignment
        bhdr = struct.pack("<6I", pver, time, cnts, 0, fwsize, uuid)        # header without tag and checksum
        for key, val in items:
            with open(val, "rb") as fsrc:
                data = fsrc.read()
                size = len(data)
                fdst.seek(faddr, 0)
                fdst.write(data)
                bhdr += struct.pack("<4s3I", key.encode(), faddr, size, zlib.crc32(data))
                faddr += (size + align - 1) // align * align  # force alignment

        # write the packet header
        bhdr = struct.pack("<4sI", tags.encode(), zlib.crc32(bhdr)) + bhdr
        fdst.seek(0, 0)
        fdst.write(bhdr)

def main(*argv):
    argc = len(argv)
    cmds = os.path.basename(str(argv[0]))
    if argc >= 3:
        mode = argv[1]
        psrc = argv[2]
        if len(psrc) and os.path.exists(psrc):
            if mode == "--unpack" or mode == "-u":
                pdst = argv[3] if argc > 3 else ""
                res_unpack(psrc, pdst)
                return 0
            if mode == "--pack" or mode == "-p":
                res_pack(psrc)
                return 0
    print("csk resouce pack/unpack tool v%d.%d" % ((pver >> 16) & 0xff, pver & 0xff))
    print("powered by danny(xhtan@listenai.com)")
    print("usage:")
    print("    %s [-h|--help]" % cmds)
    print("    %s {-p|--pack} <config_path>" % cmds)
    print("    %s {-u|--unpack} <pack_path> [unpack_path]" % cmds)
    return 1

if __name__ == '__main__':
    os.chdir(os.path.dirname(sys.argv[0]))
    main(*sys.argv)
    # main(sys.argv[0], "--pack", "config.ini")
    # main(sys.argv[0], "--unpack", "respak.bin", "outs")

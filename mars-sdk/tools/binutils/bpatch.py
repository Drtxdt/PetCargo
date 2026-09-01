#coding=utf-8

import os, sys, struct
from elftools.elf.elffile import ELFFile
from io import SEEK_CUR, SEEK_END, SEEK_SET

def elfpatch(path: str, hpos: int):
	with open(path, "r+b") as file:
		aligned = lambda size, align: (size + align - 1) // align * align
		elfsize = lambda felf: sum(
      		aligned(seg.header.p_filesz, 8) for seg in ELFFile(felf).iter_segments() 
            if seg.header.p_type=='PT_LOAD' and seg.header.p_filesz
        )
		size = os.path.getsize(path) if path.lower().endswith('.bin') else elfsize(file)
		file.seek(0, SEEK_SET)
		base = file.read(10000).find(b"MARS_APP") - (hpos + 0x10) # 0xB0
		file.seek(base + hpos + 4, SEEK_SET)    	# fp: 0    	   	   => base+hpos+4
		file.write(struct.pack("<I", size))			# fp: base+hpos+4  => base+hpos+8
		file.flush()
		file.seek(-8, SEEK_CUR)						# fp: base+hpos+8  => base+hpos
		sumh = sum(file.read(60)) & 0xFFFF			# fp: base+hpos    => base+hpos+60
		file.write(struct.pack("<H", sumh))			# fp: base+hpos+60 => base+hpos+62
		file.flush()
		file.seek(-hpos - 62, SEEK_CUR)				# fp: base+hpos+62 => base
		sumv = sum(file.read(hpos + 62)) & 0xFFFF	# fp: base         => base+hpos+62
		file.write(struct.pack("<H", sumv))			# fp: base+hpos+62 => base+hpos+64
		file.flush()
		print('patch: file=%s size=%d hpos=%d base=%d' % (path, size, hpos, base))

if __name__ == '__main__':
	for path in sys.argv[1:]:
		elfpatch(path=path, hpos=160)
		

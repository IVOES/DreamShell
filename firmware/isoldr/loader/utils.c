/**
 * DreamShell ISO Loader
 * Utils
 * (c)2011-2023 SWAT <http://www.dc-swat.ru>
 */

#include <main.h>
#include <limits.h>
#include <dc/sq.h>
#include <dcload.h>
#include <asic.h>
#include <reader.h>
#include <syscalls.h>
#include <exception.h>

#ifndef HAVE_EXPT
// Just for decreasing ifdef's with HAVE_EXPT
int exception_inited(void) {
	return 0;
}
int exception_inside_int(void) {
	return 0;
}
#endif

void setup_machine(void) {

	const uint32 val = 0xffffffff;
	uint32 addr = 0xffd80000;
	*(vuint8 *)(addr + 4) = 0;
	*(vuint8 *)(addr + 0) = 0;
	*(vuint32 *)(addr + 8) = val;
	*(vuint32 *)(addr + 12) = val;
	*(vuint32 *)(addr + 20) = val;
	*(vuint32 *)(addr + 24) = val;
	*(vuint16 *)(addr + 28) = 0;
	*(vuint32 *)(addr + 32) = val;
	*(vuint32 *)(addr + 36) = val;
	*(vuint16 *)(addr + 40) = 0;

	if (IsoInfo->boot_mode == BOOT_MODE_DIRECT) {
		*(vuint8 *)(addr + 4) |= 1;
	}

	/* Clear IRQ stuff */
	*((vuint32 *) 0xa05f6910) = 0;
	*((vuint32 *) 0xa05f6914) = 0;
	*((vuint32 *) 0xa05f6918) = 0;

	*((vuint32 *) 0xa05f6920) = 0;
	*((vuint32 *) 0xa05f6924) = 0;
	*((vuint32 *) 0xa05f6928) = 0;

	*((vuint32 *) 0xa05f6930) = 0;
	*((vuint32 *) 0xa05f6934) = 0;
	*((vuint32 *) 0xa05f6938) = 0;

	*((vuint32 *) 0xa05f6940) = 0;
	*((vuint32 *) 0xa05f6944) = 0;

	*((vuint32 *) 0xa05f6950) = 0;
	*((vuint32 *) 0xa05f6954) = 0;

	addr = *((vuint8 *)0xa05f709c);
	addr++; // Prevent gcc optimization on register reading.

	*((vuint32 *) 0xA05F6900) = 0x4038;

	*((vuint32 *)0xa05f6904) = 0xf;
	*((vuint32 *)0xa05f6908) = 0x9fffff;
	*((vuint32 *)0xa05f74a0) = 0x2001;
}

void shutdown_machine(void) {

	const uint32 reset_regs[] = {
		0xa05f6808, 0xa05f6820, 0xa05f6c14, 0xa05f7414, 
		0xa05f7814, 0xa05f7834, 0xa05f7854, 0xa05f7874, 
		0xa05f7c14, 0xffa0001c, 0xffa0002c, 0xffa0003c
	};

	*(vuint32 *)(0xff000010) = 0;
	*(vuint32 *)(0xff00001c) = 0x929;

	uint32 addr1 = 0xa05f6938;
	uint32 addr2 = 0xffd0000c;

	for (int i = 3; i; --i) {
		*(vuint32 *)(addr1) = 0;
		addr1 -= 4;
		*(vuint32 *)(addr1) = 0;
		addr1 -= 4;
		*(vuint32 *)(addr1) = 0;
		addr1 -= 8;
		*(vuint32 *)(addr2) = 0;
		addr2 -= 4;
	}

	*(vuint32 *)(addr2) = 0;
	addr2 = *(vuint32 *)(addr1);
	addr1 -= 8;
	addr2 += *(vuint32 *)(addr1);

	*(vuint32 *)(0xa05f8044) = (*(vuint32 *)(0xa05f8044) & 0xfffffffe);

	for (uint32 i = 0; i < ((sizeof(reset_regs)) >> 2); ++i) {
		*(vuint32 *)(reset_regs[i]) = (*(vuint32 *)(reset_regs[i]) & 0xfffffffe);
		for (uint32 j = 0; j < 127; ++j) {
			if (!(*(vuint32 *)(reset_regs[i]) & 0xfffffffe)) {
				break;
			}
		}
	}
}

uint Load_BootBin() {
	
	int rv, bsec;
	uint32 bs = 0xfff000; /* FIXME: switch stack pointer for use all memory */
	uint32 exec_addr = CACHED_ADDR(IsoInfo->exec.addr);
	const uint32 sec_size = 2048;
	uint8 *buff = (uint8*)(NONCACHED_ADDR(IsoInfo->exec.addr));

	if(IsoInfo->exec.size < bs) {
		bs = IsoInfo->exec.size;
	}
	
	bsec = (bs / sec_size) + ( bs % sec_size ? 1 : 0);
	
	if(loader_addr > exec_addr && loader_addr < (exec_addr + bs)) {

		int count = (loader_addr - exec_addr) / sec_size;
		int part = (ISOLDR_MAX_MEM_USAGE / sec_size) + count;
		
		rv = ReadSectors(buff, IsoInfo->exec.lba, count, NULL);
		
		if(rv == COMPLETED) {
			rv = ReadSectors(buff + (part * sec_size), IsoInfo->exec.lba + part, bsec - part, NULL);
		}
		
	} else {
		rv = ReadSectors(buff, IsoInfo->exec.lba, bsec, NULL);
	}

	return rv == COMPLETED ? 1 : 0;
}


static void set_region() {
	char region_str[3][6] = {
		{"00000"},
		{"00110"},
		{"00211"}
	};
	uint8 *src = (uint8 *)0xa021a000;
	uint8 *dst = (uint8 *)0x8c000070;
	uint8 *reg = (uint8 *)0x8c000020;

	if (*((uint32 *)0xac008030) == 0x2045554a) {
		*reg = 0;
		memcpy(dst, src, 5);
	} else if(*((char *)0x8c008032) == 'E') {
		*reg = 3;
		memcpy(dst, region_str[2], 5);
	} else if(*((char *)0x8c008031) == 'U') {
		*reg = 2;
		memcpy(dst, region_str[1], 5);
	} else {
		*reg = 1;
		memcpy(dst, region_str[0], 5);
	}
}

uint Load_IPBin(int header_only) {

	uint32 ipbin_addr = NONCACHED_ADDR(IPBIN_ADDR);
	uint32 lba = IsoInfo->track_lba[0];
	uint32 cnt = 16;
	uint8 *buff = (uint8*)ipbin_addr;

	if(header_only) {
		cnt = 1;
	} else if(IsoInfo->boot_mode == BOOT_MODE_IPBIN_TRUNC) {
		lba += 12;
		cnt -= 12;
		buff += (12 * 2048);
	}

	if(ReadSectors(buff, lba, cnt, NULL) == COMPLETED) {
		if(IsoInfo->boot_mode != BOOT_MODE_IPBIN_TRUNC && header_only == 0) {
			*((uint32 *)ipbin_addr + 0x032c) = 0x8c00e000;
			*((uint16 *)ipbin_addr + 0x10d8) = 0x5113;
			*((uint16 *)ipbin_addr + 0x140a) = 0x000b;
			*((uint16 *)ipbin_addr + 0x140c) = 0x0009;
			set_region();
		} else if(header_only) {
			set_region();
		}
		return 1;
	}
	return 0;
}

static int get_ds_fd() {
	char *fn = "/DS/DS_CORE.BIN";
	int fd = open(fn, O_RDONLY);

	if (fd < 0) {
		fd = open(fn + 3, O_RDONLY);
	}
	return fd;
}

int Load_DS() {
	LOGFF(NULL);

	if (iso_fd > FILEHND_INVALID) {
		close(iso_fd);
	}

	int fd = get_ds_fd();

	if (fd < 0) {
		LOGFF("FAILED\n");
		return -1;
	}
	restore_syscalls();

	int rs = read(fd, (uint8 *)NONCACHED_ADDR(APP_ADDR), total(fd));
	close(fd);
	return rs;
}

#ifdef HAVE_EXT_SYSCALLS
void Load_Syscalls() {

	uint8_t *dst = (uint8_t *) NONCACHED_ADDR(RAM_START_ADDR);
	uint8_t *src = (uint8_t *) IsoInfo->syscalls;
	uint32 lba = 0;
	int fd;

	memcpy(dst, src, 0x4000);
	memcpy(dst + 0x128C, &IsoInfo->toc, 408);

	if (IsoInfo->image_type == ISOFS_IMAGE_TYPE_GDI) {

		*((uint32 *)(dst + 0x1248)) = 0x80; // GD

		switch_gdi_data_track(150, get_GDS());
		ioctl(iso_fd, FS_IOCTL_GET_LBA, &lba);
		*((uint32 *)(dst + 0x1424)) = lba;

		switch_gdi_data_track(45150, get_GDS());
		ioctl(iso_fd, FS_IOCTL_GET_LBA, &lba);
		*((uint32 *)(dst + 0x1428)) = lba;

		*((uint32 *)(dst + 0xD0)) = *((uint32 *)(dst + 0x1428));

		if (IsoInfo->track_lba[0] != IsoInfo->track_lba[1]) {
			switch_gdi_data_track(IsoInfo->track_lba[1], get_GDS());
			ioctl(iso_fd, FS_IOCTL_GET_LBA, &lba);
			*((uint32 *)(dst + 0x142C)) = lba;
		}

		char ch_name[sizeof(IsoInfo->image_file)];
		memcpy(ch_name, IsoInfo->image_file, sizeof(IsoInfo->image_file));
		memcpy(&ch_name[strlen(IsoInfo->image_file) - 6], "103.iso", 7);

		close(iso_fd);
		fd = open(ch_name, O_RDONLY);

		if (fd > FILEHND_INVALID) {

			ioctl(fd, FS_IOCTL_GET_LBA, &lba);
			*((uint32 *)(dst + 0xD4)) = lba;
			close(fd);

			memcpy(&ch_name[strlen(IsoInfo->image_file)-6], "203.iso", 7);
			fd = open(ch_name, O_RDONLY);

			if (fd > FILEHND_INVALID) {

				ioctl(fd, FS_IOCTL_GET_LBA, &lba);
				*((uint32 *)(dst + 0xD8)) = lba;
				close(fd);

				memcpy(&ch_name[strlen(IsoInfo->image_file)-6], "303.iso", 7);
				fd = open(ch_name, O_RDONLY);

				if (fd > FILEHND_INVALID) {

					ioctl(fd, FS_IOCTL_GET_LBA, &lba);
					*((uint32 *)(dst + 0xDC)) = lba;
					close(fd);
				}
			}
		}

	} else { // ISO
		*((uint32 *)(dst + 0x1248)) = 0x20; // CD
		ioctl(iso_fd, FS_IOCTL_GET_LBA, &lba);
		*((uint32 *)(dst + 0x1424)) = lba;
		*((uint32 *)(dst + 0x1428)) = 0;
		*((uint32 *)(dst + 0x142C)) = 0;
	}

	// IGR
	fd = get_ds_fd();
	ioctl(fd, FS_IOCTL_GET_LBA, &lba);
	*((uint32 *)(dst + 0x1430)) = lba;
	*((uint32 *)(dst + 0x1434)) = total(fd);
	close(fd);

	if(IsoInfo->boot_mode == BOOT_MODE_DIRECT) {
		sys_misc_init();
	} else {
		*((uint32 *)0xa05f8040) = 0x00c0c0c0;
	}
}
#endif

void *search_memory(const uint8 *key, uint32 key_size) {

	uint32 start_loc = CACHED_ADDR(IsoInfo->exec.addr);
	uint32 end_loc = start_loc + IsoInfo->exec.size;

	for(uint8 *cur_loc = (uint8 *)start_loc; (uint32)cur_loc <= end_loc; cur_loc++) {

		if(*cur_loc == key[0]) {
			if(!memcmp((const uint8 *)cur_loc, key, key_size)) {
				return (void *)cur_loc;
			}
		}
	}
	return NULL;
}


int patch_memory(const uint32 key, const uint32 val) {

	uint32 exec_addr = NONCACHED_ADDR(IsoInfo->exec.addr);
	uint32 end_loc = exec_addr + IsoInfo->exec.size;
	uint8 *k = (uint8 *)&key;
	uint8 *v = (uint8 *)&val;
	int count = 0;

	for(uint8 *cur_loc = (uint8 *)exec_addr; (uint32)cur_loc <= end_loc; cur_loc++) {

		if(*cur_loc == k[0]) {

			if(!memcmp((const uint8 *)cur_loc, k, sizeof(val))) {
				memcpy(cur_loc, v, sizeof(val));
				count++;
			}
		}
	}

	return count;
}


void apply_patch_list() {

	if(!IsoInfo->patch_addr[0]) {
		return;
	}

	for(uint i = 0; i < sizeof(IsoInfo->patch_addr) >> 2; ++i) {

		if(*(uint32 *)IsoInfo->patch_addr[i] != IsoInfo->patch_value[i]) {
			*(uint32 *)IsoInfo->patch_addr[i] = IsoInfo->patch_value[i];
		}
	}
}

int printf(const char *fmt, ...) {

	if(IsoInfo != NULL && IsoInfo->fast_boot) {
		return 0;
	}

	static int print_y = 1;
	int i = 0;
	uint16 *vram = (uint16*)(VIDEO_VRAM_START);

	if(fmt == NULL) {
		print_y = 1;
		return 0;
	}

#if defined(LOG)
	char buff[64];
	va_list args;

	va_start(args, fmt);
	i = vsnprintf(buff, sizeof(buff), fmt, args);
	va_end(args);

# ifndef LOG_SCREEN
	LOGF(buff);
# endif
#else
	char *buff = (char *)fmt;
#endif

	bfont_draw_str(vram + ((print_y * 24 + 4) * 640) + 12, 0xffff, 0x0000, buff);

	if(buff[strlen(buff)-1] == '\n') {
		if (print_y++ > 15) {
			print_y = 0;
			memset((uint32 *)vram, 0, 2 * 1024 * 1024);
		}
	}

	return i;
}

void vid_waitvbl() {
	vuint32 *vbl = ((vuint32 *)0xa05f8000) + 0x010c / 4;

	while(!(*vbl & 0x01ff))
		;

	while(*vbl & 0x01ff)
		;
}

void draw_gdtex(uint8 *src) {

	int xPos = 640 - 280;
	int yPos = 480 - 280;
	int r, g, b, a, pos;
	uint16 *vram = (uint16*)(VIDEO_VRAM_START);

	for (uint x = 0; x < 256; ++x) {
		for (uint y = 0; y < 256; ++y) {

			pos = (yPos + x) * 640 + y + xPos;
			r = src[0];
			g = src[1];
			b = src[2];
			a = src[3];

			if(a) {
				r >>= 3;
				g >>= 2;
				b >>= 3;

				if(a < 255) {
					uint16 p = vram[pos];
					r = ((r * a) + (((p & 0xf800) >> 11) * (255 - a))) >> 8;
					g = ((g * a) + (((p & 0x07e0) >> 5) * (255 - a))) >> 8;
					b = ((b * a) + (( p & 0x001f) * (255 - a))) >> 8;	
				}
				vram[pos] = (r << 11) | (g << 5) | b;
			}
		   src += 4;
		}
	}
}

void set_file_number(char *filename, int num) {
    int len = strlen(filename);

    if (num < 10) {
        filename[len - 5] = '0' + num;
    } else if(num < 100) {
        filename[len - 5] = '0' + (num % 10);
        filename[len - 6] = '0' + (num / 10);
    } else if(num < 1000) {
        filename[len - 5] = '0' + (num % 10);
        filename[len - 6] = '0' + ((num % 100) / 10);
        filename[len - 7] = '0' + (num / 100);
    }
}

static char *replace_filename(char *filepath, char *filename) {
	uint8 *val = (uint8 *)filepath, *tmp = NULL;
	int len = strlen(filepath);

	do {
		tmp = tmp ? val + 1 : val;
		val = memchr(tmp, '/', len);
	} while(val != NULL);

	len = strlen(filename);
	memcpy(tmp, filename, len);
	tmp[len] = '\0';

	return filepath;
}

char *relative_filename(char *filename) {
	char *result = malloc(sizeof(IsoInfo->image_file));

    if (result == NULL) {
        LOGFF("malloc failed\n");
        return NULL;
    }

    memcpy(result, IsoInfo->image_file, sizeof(IsoInfo->image_file));
    return replace_filename(result, filename);
}

#ifdef HAVE_SCREENSHOT
void video_screenshot() {

	static int num = 0;
	static uint32 req = 0;

	if (exception_inside_int()) {
		req = 1;
		return;
	} else if(req == 0) {
		return;
	} else {
		req = 0;
	}

	char *filename = "/DS/screenshot/game_scr_001.ppm";
	const char *header = "# DreamShell ISO Loader\n640 480\n255\n";
	const size_t header_len = strlen(header);

	const size_t fs_sector_size = 512;
	const size_t buffer_size = fs_sector_size * 3;
	uint8 *buffer = (uint8 *)malloc(buffer_size);
	uint8 *buffer_end = buffer + buffer_size;
	uint8 *pbuffer = buffer;
	uint32 try_cnt = 30;

	if (buffer == NULL) {
		LOGFF("Can't allocate memory\n");
		return;
	}

	set_file_number(filename, ++num);
	file_t fd = open(filename, O_WRONLY | O_PIO);

	while (fd < 0) {
		if (--try_cnt == 0) {
			break;
		} else if(fd == FS_ERR_NO_PATH) {

			fd = open(filename + 3, O_WRONLY | O_PIO);

			if(fd == FS_ERR_NO_PATH) {
				break;
			}
		} else if (fd == FS_ERR_EXISTS) {

			num += 10;
			set_file_number(filename, num);
			fd = open(filename, O_WRONLY | O_PIO);
		}
	}
	if (fd < 0) {
		LOGFF("Can't create file: %s\n", filename);
		free(buffer);
		return;
	}

	memset(buffer, '#', buffer_size);
	buffer[0] = 'P';
	buffer[1] = '6';
	buffer[2] = '\n';
	memcpy(buffer + (fs_sector_size - header_len), header, header_len);

	for (uint32 i = 64; i < (fs_sector_size - header_len); ++i) {
		if (i % 64 == 0) {
			buffer[i] = '\n';
		}
	}

	write(fd, buffer, fs_sector_size);

	uint32 pixel;
	uint32 *vraml = (uint32 *)(VIDEO_VRAM_START);
	uint16 *vrams = (uint16 *)(vraml);
	uint8 *vramb = (uint8 *)(vraml);
	const uint32 display_cfg = *(vuint32 *)0xa05f8044;
	const uint32 pixel_mode = (display_cfg >> 2) & 0x0f;
	const uint32 display_size = *(vuint32 *)0xa05f805c;
	uint32 width = 640;
	uint32 height = ((display_size >> 10) & 0x3ff) + 1;
	if(height <= 240) {
		width = 320;
		height = 240;
	} else {
		height = 480;
	}
	const uint32 pix_num = width * height;
	LOGFF("%s %dx%d %d\n", filename, width, height, pixel_mode);

	for(uint32 i = 0; i < pix_num; ++i) {

		switch(pixel_mode) {
			case PM_RGB888P:
				pbuffer[0] = vramb[i * 3 + 2];
				pbuffer[1] = vramb[i * 3 + 1];
				pbuffer[2] = vramb[i * 3];
				break;
			case PM_RGB0888:
				pixel = vraml[i];
				pbuffer[0] = ((pixel >> 16) & 0xff);
				pbuffer[1] = ((pixel >> 8) & 0xff);
				pbuffer[2] = (pixel & 0xff);
				break;
			case PM_RGB555:
				pixel = vrams[i];
				pbuffer[0] = (((pixel >> 10) & 0x1f) << 3);
				pbuffer[1] = (((pixel >> 5) & 0x1f) << 3);
				pbuffer[2] = ((pixel & 0x1f) << 3);
				break;
			case PM_RGB565:
			default:
				pixel = vrams[i];
				pbuffer[0] = (((pixel >> 11) & 0x1f) << 3);
				pbuffer[1] = (((pixel >> 5) & 0x3f) << 2);
				pbuffer[2] = ((pixel & 0x1f) << 3);
				break;
		}
		pbuffer += 3;

		if(pbuffer >= buffer_end) {
			write(fd, buffer, buffer_size);
			pbuffer = buffer;
		}
	}

	if ((pbuffer - buffer) > 0) {
		write(fd, buffer, pbuffer - buffer);
	}

	close(fd);
	free(buffer);
}
#endif


#ifdef LOG

size_t strnlen(const char *s, size_t maxlen) {
	const char *e;
	size_t n;

	for (e = s, n = 0; *e && n < maxlen; e++, n++)
		;
	return n;
}

int check_digit (char c) {
	if((c>='0') && (c<='9')) {
		return 1;
	}
	return 0;
}

static char log_buff[128];

#if _FS_READONLY == 0 && defined(LOG_FILE)
static int log_fd = FS_ERR_SYSERR;
static int open_log_file() {
	int fd = open(LOG_FILE, O_WRONLY | O_APPEND);
	if (fd > FS_ERR_SYSERR) {
		write(fd, "--- Start log ---\n", 18);
		ioctl(fd, FS_IOCTL_SYNC, NULL);
	}
	return fd;
}
#endif /* _FS_READONLY */

int OpenLog() {

	memset(log_buff, 0, sizeof(log_buff));

#if _FS_READONLY == 0 && defined(LOG_FILE)
	log_fd = open_log_file();
#endif

#if defined(DEV_TYPE_DCL) || defined(LOG_DCL)
	dcload_init();
#elif !defined(LOG_SCREEN)
	scif_init();
#endif
	return 1;
}

static int PutLog(char *buff) {

	int len = strlen(buff);

#if _FS_READONLY == 0 && defined(LOG_FILE)
	if (log_fd == FS_ERR_SYSERR) {
		log_fd = open_log_file();
	}
	if(log_fd > FS_ERR_SYSERR) {
		write(log_fd, buff, len);
		ioctl(fd, FS_IOCTL_SYNC, NULL);
	}
#endif

#if defined(LOG_SCREEN)
	printf(buff);
#elif defined(DEV_TYPE_DCL) || defined(LOG_DCL)
	dcload_write_buffer((uint8 *)buff, len);
#else
	scif_write_buffer((uint8 *)buff, len, 1);
#endif
	return len;
}

int WriteLog(const char *fmt, ...) {
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(log_buff, sizeof(log_buff), fmt, args);
	va_end(args);

	PutLog(log_buff);
	return i;
}

int WriteLogFunc(const char *func, const char *fmt, ...) {

	PutLog((char *)func);

	if(fmt == NULL) {
		PutLog("\n");
		return 0;
	}

	PutLog(": ");

	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(log_buff, sizeof(log_buff), fmt, args);
	va_end(args);

	PutLog(log_buff);
	return i;
}

#endif /* LOG */

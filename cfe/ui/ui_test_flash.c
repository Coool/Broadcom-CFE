/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  Flash Test commands			File: ui_test_flash.c
    *  
    *  Some commands to test the flash device interface.
    *  
    *  Author:  Mitch Lichtenberg
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#include "cfe.h"
#include "ui_command.h"

int ui_init_flashtestcmds(void);

static int ui_cmd_flashtest(ui_cmdline_t *cmd,int argc,char *argv[]);
//static int ui_cmd_readnvram(ui_cmdline_t *cmd,int argc,char *argv[]);
//static int ui_cmd_erasenvram(ui_cmdline_t *cmd,int argc,char *argv[]);

int ui_init_flashtestcmds(void)
{
    cmd_addcmd("show flash",
	       ui_cmd_flashtest,
	       NULL,
	       "Display information about a flash device.",
	       "show flash [-sectors]",
	       "-sectors;Display sector information");

#if 0
    cmd_addcmd("nvram read",
	       ui_cmd_readnvram,
	       NULL,
	       "read the NVRAM",
	       "test nvram devname offset",
	       "");

    cmd_addcmd("nvram erase",
	       ui_cmd_erasenvram,
	       NULL,
	       "erase the NVRAM",
	       "erasenvram devname",
	       "-pattern");
#endif


    return 0;
}


static char *flashtypes[] = {
    "Unknown","SRAM","ROM","Flash"
};


static int ui_cmd_flashtest(ui_cmdline_t *cmd,int argc,char *argv[])
{
    flash_info_t info;
    int fd;
    int retlen;
    int res = 0;
    int idx;
    flash_sector_t sector;
    nvram_info_t nvraminfo;
    char *devname;
    int showsectors;

    devname = cmd_getarg(cmd,0);
    if (!devname) return ui_showusage(cmd);

    showsectors = cmd_sw_isset(cmd,"-sectors");

    fd = cfe_open(devname);
    if (fd < 0) {
	ui_showerror(fd,"Could not open flash device %s",devname);
	return fd;
	}

    res = cfe_ioctl(fd,IOCTL_FLASH_GETINFO,(uint8_t *) &info,sizeof(flash_info_t),&retlen,0);
    if (res == 0) {
	printf("FLASH: Base %016llX size %08X type %02X(%s) flags %08X\n",
	   info.flash_base,info.flash_size,info.flash_type,flashtypes[info.flash_type],
	       info.flash_flags);
	}
    else {
	printf("FLASH: Could not determine flash information\n");
	}

    res = cfe_ioctl(fd,IOCTL_NVRAM_GETINFO,(uint8_t *) &nvraminfo,sizeof(nvram_info_t),&retlen,0);
    if (res == 0) {
	printf("NVRAM: Offset %08X Size %08X EraseFlg %d\n",
	       nvraminfo.nvram_offset,nvraminfo.nvram_size,nvraminfo.nvram_eraseflg);
	}
    else {
	printf("NVRAM: Not supported by this flash\n");
	}

    if (showsectors && (info.flash_type == FLASH_TYPE_FLASH)) {
	printf("Flash sector information:\n");

	idx = 0;
	for (;;) {
	    sector.flash_sector_idx = idx;
	    res = cfe_ioctl(fd,IOCTL_FLASH_GETSECTORS,(uint8_t *) &sector,sizeof(flash_sector_t),&retlen,0);
	    if (res != 0) {
		printf("ioctl error\n");
		break;
		}
	    if (sector.flash_sector_status == FLASH_SECTOR_INVALID) break;
	    printf("  Sector %d offset %08X size %d\n",
		   sector.flash_sector_idx,
		   sector.flash_sector_offset,
		   sector.flash_sector_size);     
	    idx++;
	    }
	}

    cfe_close(fd);
    return 0;

}


#if 0
static int ui_cmd_readnvram(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *dev;
    char *tok;
    int fd;
    int offset = 0;
    int res;
    uint8_t buf[512];
    int idx;

    dev = cmd_getarg(cmd,0);
    if (!dev) return ui_showusage(cmd);

    tok = cmd_getarg(cmd,1);
    if (tok) offset = xtoi(tok);
    else offset = 0;

    fd = cfe_open(dev);
    if (fd < 0) {
	ui_showerror(fd,"could not open NVRAM");
	return fd;
	}

    res = cfe_readblk(fd,offset,buf,512);
    printf("Offset %d Result %d\n",offset,res);
    for (idx = 0; idx < 512; idx++) {
	if ((idx % 16) == 0) printf("\n");
	printf("%02X ",buf[idx]);
	}
    printf("\n");	

    cfe_close(fd);
    return 0;
	
}

static int ui_cmd_erasenvram(ui_cmdline_t *cmd,int argc,char *argv[])
{
    char *dev;
    int fd;
    uint8_t buffer[2048];
    int res;
    char *tok;
    int offset;
    int length;
    uint8_t data;

    dev = cmd_getarg(cmd,0);
    if (!dev) return ui_showusage(cmd);

    offset = 0;
    if ((tok = cmd_getarg(cmd,1))) offset = xtoi(tok);
    length = 512; 

    if ((tok = cmd_getarg(cmd,2))) length = xtoi(tok);
    if (length > 2048) length = 2048;

    data = 0xFF;
    if ((tok = cmd_getarg(cmd,3))) data = xtoi(tok);

    fd = cfe_open(dev);
    if (fd < 0) {
	ui_showerror(fd,"could not open NVRAM");
	return fd;
	}

    if (cmd_sw_isset(cmd,"-pattern")) {
	memset(buffer,0,sizeof(buffer));
	for (res = 0; res < 2048; res++) {
	    buffer[res] = res & 0xFF;
	    }
	}
    else memset(buffer,data,sizeof(buffer));

    printf("Fill offset %04X length %04X\n",offset,length);

    res = cfe_writeblk(fd,offset,buffer,length);

    printf("write returned %d\n",res);

    cfe_close(fd);
    return 0;
	
}
#endif


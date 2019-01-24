/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  ATMEL AT24C02 EEPROM driver 	File: dev_smbus_at24c02.c
    *  
    *  This module contains a CFE driver for a ATMEL AT24C02 EEPROM
    *  
    *  Author:  Binh Vo
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
#include "cfe_smbus.h"


/*  *********************************************************************
    *  Forward Declarations
    ********************************************************************* */

static void sb1250_at24c02eeprom_probe(cfe_driver_t *drv,
				     unsigned long probe_a, unsigned long probe_b, 
				     void *probe_ptr);


static int sb1250_at24c02eeprom_open(cfe_devctx_t *ctx);
static int sb1250_at24c02eeprom_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int sb1250_at24c02eeprom_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int sb1250_at24c02eeprom_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int sb1250_at24c02eeprom_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int sb1250_at24c02eeprom_close(cfe_devctx_t *ctx);

/*  *********************************************************************
    *  Dispatch tables
    ********************************************************************* */

#define AT24C02_EEPROM_SIZE	256

const static cfe_devdisp_t sb1250_at24c02eeprom_dispatch = {
    sb1250_at24c02eeprom_open,
    sb1250_at24c02eeprom_read,
    sb1250_at24c02eeprom_inpstat,
    sb1250_at24c02eeprom_write,
    sb1250_at24c02eeprom_ioctl,
    sb1250_at24c02eeprom_close,
    NULL,
    NULL
};

const cfe_driver_t smbus_at24c02 = {
    "ATMEL AT24C02 SPD EEPROM",
    "eeprom",
    CFE_DEV_NVRAM,
    &sb1250_at24c02eeprom_dispatch,
    sb1250_at24c02eeprom_probe
};

typedef struct sb1250_at24c02eeprom_s {
    cfe_smbus_channel_t *smbus_channel;
    int smbus_address;
    int env_offset;
    int env_size;
} sb1250_at24c02eeprom_t;


/*  *********************************************************************
    *  smbus_readbyte(chan,slaveaddr,devaddr)
    *  
    *  Read a byte from the chip.
    *
    *  Input parameters: 
    *  	   chan - SMBus channel
    *  	   slaveaddr -  SMBus slave address
    *  	   devaddr - byte within the at24c02 device to read
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else -1
    ********************************************************************* */

static int smbus_readbyte(cfe_smbus_channel_t *chan,int slaveaddr,int devaddr)
{
    uint8_t buf[1];
    int err;

    /*
     * Start the command
     */

    buf[0] = devaddr;
    err = SMBUS_WRITE(chan,slaveaddr,buf,1);
    if (err < 0) return err;

    /*
     * Read the data byte
     */

    err = SMBUS_READ(chan,slaveaddr,buf,1);
    if (err < 0) return err;

    return buf[0];
}

/*  *********************************************************************
    *  smbus_writebyte(chan,slaveaddr,devaddr,b)
    *  
    *  write a byte from the chip.
    *  
    *  Input parameters: 
    *  	   chan - SMBus channel
    *  	   slaveaddr -  SMBus slave address
    *  	   devaddr - byte within the at24c02 device to read
    *      b - byte to write
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else -1
    ********************************************************************* */

static int smbus_writebyte(cfe_smbus_channel_t *chan,int slaveaddr,int devaddr,int b)
{
    uint8_t buf[2];
    int err;
    int64_t timer;

    /*
     * Write the data to the controller
     */

    buf[0] = devaddr;
    buf[1] = b;

    err = SMBUS_WRITE(chan,slaveaddr,buf,2);

    /*
     * Pound on the device with a current address read
     * to poll for the write complete
     */

    TIMER_SET(timer,50);
    err = -1; 

    while (!TIMER_EXPIRED(timer)) {
	POLL();

	err = SMBUS_READ(chan,slaveaddr,buf,1);
	if (err == 0) break;
	}

    return err;
}

/*  *********************************************************************
    *  sb1250_at24c02eeprom_probe(drv,a,b,ptr)
    *  
    *  Probe routine for this driver.  This routine creates the 
    *  local device context and attaches it to the driver list
    *  within CFE.
    *  
    *  Input parameters: 
    *  	   drv - driver handle
    *  	   a,b - probe hints (longs)
    *  	   ptr - probe hint (pointer)
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

static void sb1250_at24c02eeprom_probe(cfe_driver_t *drv,
				     unsigned long probe_a, unsigned long probe_b, 
				     void *probe_ptr)
{
    sb1250_at24c02eeprom_t *softc;
    char descr[80];

    softc = (sb1250_at24c02eeprom_t *) KMALLOC(sizeof(sb1250_at24c02eeprom_t),0);

    /*
     * Probe_a is the SMBus channel number
     * Probe_b is the SMBus device offset
     * Probe_ptr is unused.
     */

    softc->smbus_channel = SMBUS_CHANNEL((int)probe_a);
    softc->smbus_address = (int)probe_b;
    softc->env_offset  = 0;
    softc->env_size = AT24C02_EEPROM_SIZE;

    xsprintf(descr,"%s on SMBus channel %d dev 0x%02X",
	     drv->drv_description,(int)probe_a,(int)probe_b);
    cfe_attach(drv,softc,NULL,descr);
}



/*  *********************************************************************
    *  sb1250_at24c02eeprom_open(ctx)
    *  
    *  Open this device. 
    *  
    *  Input parameters: 
    *  	   ctx - device context (can obtain our softc here)
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   else error code
    ********************************************************************* */

static int sb1250_at24c02eeprom_open(cfe_devctx_t *ctx)
{
    sb1250_at24c02eeprom_t *softc = ctx->dev_softc;
    int b;

    b = smbus_readbyte(softc->smbus_channel,
		       softc->smbus_address,
		       0);

    return (b < 0) ? -1 : 0;
}

/*  *********************************************************************
    *  sb1250_at24c02eeprom_read(ctx,buffer)
    *  
    *  Read bytes from the device.
    *  
    *  Input parameters: 
    *  	   ctx - device context (can obtain our softc here)
    *  	   buffer - buffer descriptor (target buffer, length, offset)
    *  	   
    *  Return value:
    *  	   number of bytes read
    *  	   -1 if an error occured
    ********************************************************************* */

static int sb1250_at24c02eeprom_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
    sb1250_at24c02eeprom_t *softc = ctx->dev_softc;
    hsaddr_t bptr;
    int blen;
    int idx;
    int b = 0;

    bptr = buffer->buf_ptr;
    blen = buffer->buf_length;

    if ((buffer->buf_offset + blen) > AT24C02_EEPROM_SIZE) return -1;

    idx = (int) buffer->buf_offset;

    while (blen > 0) {
	b = smbus_readbyte(softc->smbus_channel,
			  softc->smbus_address,
			  idx);
	if (b < 0) break;
	hs_write8(bptr,(unsigned char)b);
	bptr++;
	blen--;
	idx++;
	}

    buffer->buf_retlen = bptr - buffer->buf_ptr;
    return (b < 0) ? -1 : 0;
}

/*  *********************************************************************
    *  sb1250_at24c02eeprom_inpstat(ctx,inpstat)
    *  
    *  Test input (read) status for the device
    *  
    *  Input parameters: 
    *  	   ctx - device context (can obtain our softc here)
    *  	   inpstat - input status descriptor to receive value
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   -1 if an error occured
    ********************************************************************* */

static int sb1250_at24c02eeprom_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat)
{
    inpstat->inp_status = 1;

    return 0;
}

/*  *********************************************************************
    *  sb1250_at24c02eeprom_write(ctx,buffer)
    *  
    *  Write bytes from the device.
    *  
    *  Input parameters: 
    *  	   ctx - device context (can obtain our softc here)
    *  	   buffer - buffer descriptor (target buffer, length, offset)
    *  	   
    *  Return value:
    *  	   number of bytes read
    *  	   -1 if an error occured
    ********************************************************************* */

static int sb1250_at24c02eeprom_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
    sb1250_at24c02eeprom_t *softc = ctx->dev_softc;
    hsaddr_t bptr;
    int blen;
    int idx;
    int b = 0;

    bptr = buffer->buf_ptr;
    blen = buffer->buf_length;

    if ((buffer->buf_offset + blen) > AT24C02_EEPROM_SIZE) return -1;

    idx = (int) buffer->buf_offset;

    while (blen > 0) {
	b = hs_read8(bptr);
	bptr++;
	b = smbus_writebyte(softc->smbus_channel,
			   softc->smbus_address,
			   idx,
			   b);
	if (b < 0) break;
	blen--;
	idx++;
	}

    buffer->buf_retlen = bptr - buffer->buf_ptr;
    return (b < 0) ? -1 : 0;
}

/*  *********************************************************************
    *  sb1250_at24c02eeprom_ioctl(ctx,buffer)
    *  
    *  Perform miscellaneous I/O control operations on the device.
    *  
    *  Input parameters: 
    *  	   ctx - device context (can obtain our softc here)
    *  	   buffer - buffer descriptor (target buffer, length, offset)
    *  	   
    *  Return value:
    *  	   number of bytes read
    *  	   -1 if an error occured
    ********************************************************************* */

static int sb1250_at24c02eeprom_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer) 
{
    sb1250_at24c02eeprom_t *softc = ctx->dev_softc;
    nvram_info_t info;

    switch ((int)buffer->buf_ioctlcmd) {
	case IOCTL_NVRAM_GETINFO:
	    if (buffer->buf_length != sizeof(nvram_info_t)) return -1;
	    info.nvram_offset = softc->env_offset;
	    info.nvram_size =   softc->env_size;
	    info.nvram_eraseflg = FALSE;
	    buffer->buf_retlen = sizeof(nvram_info_t);
	    hs_memcpy_to_hs(buffer->buf_ptr,&info,sizeof(info));
	    return 0;
	default:
	    return -1;
	}
}

/*  *********************************************************************
    *  sb1250_at24c02eeprom_close(ctx,buffer)
    *  
    *  Close the device.
    *  
    *  Input parameters: 
    *  	   ctx - device context (can obtain our softc here)
    *  	   
    *  Return value:
    *  	   0 if ok
    *  	   -1 if an error occured
    ********************************************************************* */

static int sb1250_at24c02eeprom_close(cfe_devctx_t *ctx)
{
    return 0;
}



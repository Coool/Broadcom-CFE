/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  BCM5700/Tigon3 (10/100/1000 EthernetMAC) driver	File: dev_bcm5700.c
    *  
    *  Author:  Ed Satterthwaite
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
#include "lib_physio.h"
#ifdef CPUCFG_MEMCPY
#error "memcpy has been replaced by hs_memcpy_{to,from}_hs."
extern void *CPUCFG_MEMCPY(void *dest, const void *src, size_t cnt);
#define blockcopy CPUCFG_MEMCPY
#else
#define blockcopy memcpy
#endif
#include "cfe_irq.h"

#include "net_enet.h"

#include "pcivar.h"
#include "pcireg.h"

#include "bcm5700.h"
#include "mii.h"




static uint32_t l_phys_read32(uint32_t addr );
static void l_phys_write32(uint32_t addr, uint32_t val);


/* This is a driver for the Broadcom 570x ("Tigon 3") 10/100/1000 MAC.
   Currently, the 5700, 5701, 5703C, 5704C and 5705 have been tested.
   Only 10/100/1000 BASE-T PHYs are supported; variants with SerDes
   PHYs are not supported.

   Reference:
     Host Programmer Interface Specification for the BCM570X Family
       of Highly-Integrated Media Access Controllers, 570X-PG106-R.
     Broadcom Corp., 16215 Alton Parkway, Irvine CA, 09/27/02

   This driver takes advantage of DMA coherence in systems that
   support it (e.g., SB1250).  For systems without coherent DMA (e.g.,
   BCM47xx SOCs), descriptor and packet buffer memory is explicitly
   flushed, while status memory is always referenced uncached.

   The driver prefers "preserve bit lanes" mode for big-endian
   systems that provide the option, but it can use "preserve byte
   lanes" as well.

   Note that the 5705 does not fully map all address ranges.  Per
   the manual, reads and writes of the unmapped regions are permitted
   and do not fault; however, it apparently has some poisoned registers,
   at least in early revs, that should not be touched.  See the
   conditionals in the code. */

/* PIOSWAP controls whether word-swapping takes place for transactions
   in which the 570x is the target device.  In theory, either value
   should work (with access macros adjusted as below) and it should be
   set to be consistent with the settings for 570x as initiator.
   Empirically, however, some combinations only work with no swap.
   For big-endian systems:

                          SWAP=0    SWAP=1
   5700     32 PCI          OK        OK
   5700     64 Sturgeon     OK        OK
   5701-32  32 PCI          OK        OK
   5701-32  64 Sturgeon     OK        OK
   5701-32  64 Golem        OK        OK
   5701-64  64 Sturgeon     OK        OK
   5701-64  64 Golem        OK       FAIL
   5705     32 PCI          OK        OK
   5705     64 Sturgeon    (OK)*     FAIL
   5705     64 Golem        OK        OK

   For little-endian systems, only SWAP=1 appears to work.

   * PCI status/interrupt ordering problem under load.  */

#ifndef T3_DEBUG
#define T3_DEBUG 0
#endif

#ifndef T3_BRINGUP
#define T3_BRINGUP 0
#endif

#if ((ENDIAN_BIG + ENDIAN_LITTLE) != 1)
#error "dev_bcm5700: system endian not set"
#endif

#if ENDIAN_LITTLE
#define PIOSWAP 1
#else
#define PIOSWAP 0
#endif

/* Temporary, until configs supply MATCH_BYTES */
#if defined(MPC824X)  /* any machine without preserve-bits for PIO */
#define MATCH_BYTES  1
#else
#define MATCH_BYTES  0
#endif

/* Broadcom recommends using PHY interrupts instead of autopolling,
   but I haven't made it work yet. */
#define T3_AUTOPOLL 1

/* Set IPOLL to drive processing through the interrupt dispatcher.
   Set XPOLL to drive processing by an external polling agent.  One
   must be set; setting both is ok. */

#ifndef IPOLL
#define IPOLL 0
#endif
#ifndef XPOLL
#define XPOLL 1
#endif

#define MIN_ETHER_PACK  (ENET_MIN_PKT+ENET_CRC_SIZE)  /* min packet size */
#define MAX_ETHER_PACK  (ENET_MAX_PKT+ENET_CRC_SIZE)  /* max packet size */
#define VLAN_TAG_LEN    4                             /* VLAN type plus tag */

/* Packet buffers.  For the Tigon 3, packet buffer alignment is
   arbitrary and can be to any byte boundary.  We would like it
   aligned to a cache line boundary for performance, although there is
   a trade-off with IP/TCP header alignment.  Jumbo frames are not
   currently supported.  */

#define ETH_PKTBUF_LEN      (((MAX_ETHER_PACK+31)/32)*32)

typedef struct eth_pkt_s {
    queue_t next;			/*  8 */
    uint8_t *buffer;			/*  4 */
    uint32_t flags;			/*  4 */
    int32_t length;			/*  4 */
    uint32_t unused[3];			/* 12 */
    uint8_t data[ETH_PKTBUF_LEN];
} eth_pkt_t;

#define CACHE_ALIGN       32
#define ALIGN(n,align)    (((n)+((align)-1)) & ~((align)-1))

#define ETH_PKTBUF_LINES  ((sizeof(eth_pkt_t) + (CACHE_ALIGN-1))/CACHE_ALIGN)
#define ETH_PKTBUF_SIZE   (ETH_PKTBUF_LINES*CACHE_ALIGN)
#define ETH_PKTBUF_OFFSET (offsetof(eth_pkt_t, data))

#define ETH_PKT_BASE(data) ((eth_pkt_t *)((data) - ETH_PKTBUF_OFFSET))

static void
show_packet(char c, eth_pkt_t *pkt)
{
    int i;
    int n = (pkt->length < 32 ? pkt->length : 32);

    xprintf("%c[%4d]:", c, pkt->length);
    for (i = 0; i < n; i++) {
	if (i % 4 == 0)
	    xprintf(" ");
	xprintf("%02x", pkt->buffer[i]);
	}
    xprintf("\n");
}


static void t3_ether_probe(cfe_driver_t *drv,
			   unsigned long probe_a, unsigned long probe_b, 
			   void *probe_ptr);


/* BCM570X Hardware Common Data Structures
   XXX Should they move to the header file? */

/* Chip documentation numbers the rings with 1-origin.  */

#define RI(n)                 ((n)-1)

/* BCM570x Ring Sizes (no external memory).  Pages 97-98 */

#define TXP_MAX_RINGS         16
#define TXP_INTERNAL_RINGS    4
#define TXP_RING_ENTRIES      512

#define RXP_STD_ENTRIES       512

#define RXR_MAX_RINGS         16
#define RXR_RING_ENTRIES      1024

#define RXR_MAX_RINGS_05      1
#define RXR_RING_ENTRIES_05   512
  
#define RXR_MAX_RINGS_BCM571X_FAMILY    1
#define RXR_RING_ENTRIES_BCM571X_FAMILY 512


#define BCM571X_FAMILY_DEVICE(dev )  ( (dev) == K_PCI_ID_BCM5780 )

/* BCM570x Send Buffer Descriptors as a struct.  Pages 100-101 */

typedef struct t3_snd_bd_s {
    uint32_t  bufptr_hi;
    uint32_t  bufptr_lo;
#if ENDIAN_BIG
    uint16_t  length;
    uint16_t  flags;
    uint16_t  pad;
    uint16_t  vlan_tag;
#elif ENDIAN_LITTLE
    uint16_t  flags;
    uint16_t  length;
    uint16_t  vlan_tag;
    uint16_t  pad;
#else
#error "bcm5700: endian not set"
#endif
} t3_snd_bd_t;

#define SND_BD_SIZE           16

#define TX_FLAG_TCP_CKSUM     0x0001
#define TX_FLAG_IP_CKSUM      0x0002
#define TX_FLAG_PACKET_END    0x0004
#define TX_FLAG_IP_FRAG       0x0008
#define TX_FLAG_IP_FRAG_END   0x0010
#define TX_FLAG_VLAN_TAG      0x0040
#define TX_FLAG_COAL_NOW      0x0080
#define TX_FLAG_CPU_PRE_DMA   0x0100
#define TX_FLAG_CPU_POST_DMA  0x0200
#define TX_FLAG_ADD_SRC       0x1000
#define TX_FLAG_SRC_ADDR_SEL  0x6000
#define TX_FLAG_NO_CRC        0x8000

/* BCM570x Receive Buffer Descriptors as a struct.  Pages 105-107 */

typedef struct t3_rcv_bd_s {
    uint32_t  bufptr_hi;
    uint32_t  bufptr_lo;
#if ENDIAN_BIG
    uint16_t  index;
    uint16_t  length;
    uint16_t  type;
    uint16_t  flags;
    uint16_t  ip_cksum;
    uint16_t  tcp_cksum;
    uint16_t  error_flag;
    uint16_t  vlan_tag;
#elif ENDIAN_LITTLE
    uint16_t  length;
    uint16_t  index;
    uint16_t  flags;
    uint16_t  type;
    uint16_t  tcp_cksum;
    uint16_t  ip_cksum;
    uint16_t  vlan_tag;
    uint16_t  error_flag;
#else
#error "bcm5700: endian not set"
#endif
    uint32_t  pad;
    uint32_t  opaque;
} t3_rcv_bd_t;

#define RCV_BD_SIZE           32

#define RX_FLAG_PACKET_END    0x0004
#define RX_FLAG_JUMBO_RING    0x0020
#define RX_FLAG_VLAN_TAG      0x0040
#define RX_FLAG_ERROR         0x0400
#define RX_FLAG_MINI_RING     0x0800
#define RX_FLAG_IP_CKSUM      0x1000
#define RX_FLAG_TCP_CKSUM     0x2000
#define RX_FLAG_IS_TCP        0x4000

#define RX_ERR_BAD_CRC        0x0001
#define RX_ERR_COLL_DETECT    0x0002
#define RX_ERR_LINK_LOST      0x0004
#define RX_ERR_PHY_DECODE     0x0008
#define RX_ERR_DRIBBLE        0x0010
#define RX_ERR_MAC_ABORT      0x0020
#define RX_ERR_SHORT_PKT      0x0040
#define RX_ERR_TRUNC_NO_RES   0x0080
#define RX_ERR_GIANT_PKT      0x0100

/* BCM570x Status Block format as a struct (not BCM5705).  Pages 110-111. */

typedef struct t3_status_s {
    uint32_t status;
    uint32_t tag;
#if ENDIAN_BIG
    uint16_t rxc_std_index;
    uint16_t rxc_jumbo_index;
    uint16_t reserved2;
    uint16_t rxc_mini_index;
    struct {
	uint16_t send_c;
	uint16_t return_p;
    } index [16];
#elif ENDIAN_LITTLE
    uint16_t rxc_jumbo_index;
    uint16_t rxc_std_index;
    uint16_t rxc_mini_index;
    uint16_t reserved2;
    struct {
	uint16_t return_p;
	uint16_t send_c;
    } index [16];
#else
#error "bcm5700: endian not set"
#endif
} t3_status_t;

#define M_STATUS_UPDATED        0x00000001
#define M_STATUS_LINKCHNG       0x00000002
#define M_STATUS_ERROR          0x00000004

/* BCM570x Statistics Block format as a struct.  Pages 112-120 */

typedef struct t3_stats_s {
    uint64_t stats[L_MAC_STATS/sizeof(uint64_t)];
} t3_stats_t;

/* A common memory area for supplying zeros to clear the on-chip stats. */
static t3_stats_t *zero_stats = NULL;

/* Encoded status transfer block size (32, 64 or 80 bytes.  Page 412 */

#define STATUS_BLOCK_SIZE(rings) \
         ((rings) <= 4  ? K_HCM_SBSIZE_32 : \
          (rings) <= 12 ? K_HCM_SBSIZE_64 : \
          K_HCM_SBSIZE_80) 

/* End of 570X defined data structures */

/* The maximum supported BD ring index (QOS) for transmit or receive. */

#define MAX_RI                 1


typedef enum {
    eth_state_uninit,
    eth_state_off,
    eth_state_on, 
} eth_state_t;

typedef struct t3_ether_s {
    /* status block */
    volatile t3_status_t *status;  /* should be cache-aligned */

    /* PCI access information */
    uint32_t  regbase;
    uint32_t  membase;
    uint8_t   irq;
    pcitag_t  tag;		   /* tag for configuration registers */

    uint8_t   hwaddr[6];
    uint16_t  device;              /* chip device code */
    uint8_t   revision;            /* chip revision */
    uint16_t  asic_revision;       /* mask revision */

    eth_state_t state;             /* current state */
    uint32_t intmask;              /* interrupt mask */

    int linkspeed;		   /* encodings from cfe_ioctl */

    /* packet lists */
    queue_t freelist;
    uint8_t *pktpool;
    queue_t rxqueue;

    /* rings */
    /* For now, support only the standard Rx Producer Ring */
    t3_rcv_bd_t *rxp_std;          /* Standard Rx Producer Ring */
    uint32_t  rxp_std_index;
    uint32_t  prev_rxp_std_index;

   /* For now, support only 1 priority */
    uint32_t  rxr_entries;
    t3_rcv_bd_t *rxr_1;            /* Rx Return Ring 1 */
    uint32_t  rxr_1_index;
    t3_snd_bd_t *txp_1;            /* Send Ring 1 */
    uint32_t  txp_1_index;
    uint32_t  txc_1_index;

    cfe_devctx_t *devctx;

    /* PHY access */
    int      phy_addr;
    uint16_t phy_status;
    uint16_t phy_ability;
    uint16_t phy_xability;
    uint32_t phy_vendor;
    uint16_t phy_device;

    /* MII polling control */
    int      phy_change;
    int      mii_polling;

    /* statistics block */
    volatile t3_stats_t *stats;    /* should be cache-aligned */

    /* additional driver statistics */
    uint32_t rx_interrupts;
    uint32_t tx_interrupts;
    uint32_t bogus_interrupts;
} t3_ether_t;


/* Address mapping macros */

#if CFG_L2_RAM   /* Temporarily here for SiByte SOCs running from L2 */
#define PTR_TO_PHYS(x) (((uintptr_t)(x)) + 0xD0000000)
#define PHYS_TO_PTR(a) ((uint8_t *)((a) - 0xD0000000))
#else
#define PTR_TO_PHYS(x) (PHYSADDR((uintptr_t)(x)))
#define PHYS_TO_PTR(a) ((uint8_t *)KERNADDR(a))
#endif

#define PCI_TO_PTR(a)  (PHYS_TO_PTR(PCI_TO_PHYS(a)))
#define PTR_TO_PCI(x)  (PHYS_TO_PCI(PTR_TO_PHYS(x)))

/* Chip access macros */

#if (ENDIAN_BIG && MATCH_BYTES)
/* The host cannot support match-bits mode when operating big-endian.
   The 5700 swapping control can deal with this, but for now, just
   use byte-swapped access to the CSRs.  */

#define CSR_MATCH_MODE        PCI_MATCH_BYTES

#define READCSR(sc,csr)       (phys_read32_swapped((sc)->regbase + (csr)))
#define WRITECSR(sc,csr,val)  (phys_write32_swapped((sc)->regbase + (csr), (val)))

#if PIOSWAP
#define READMBOX(sc,csr)      (phys_read32_swapped((sc)->regbase+((csr)^4)))
#define WRITEMBOX(sc,csr,val) (phys_write32_swapped((sc)->regbase+((csr)^4), (val)))

#define READMEM(sc,csr)       (phys_read32_swapped((sc)->membase+(csr)))
#define WRITEMEM(sc,csr,val)  (phys_write32_swapped((sc)->membase+(csr), (val)))

#else
#define READMBOX(sc,csr)      (phys_read32_swapped((sc)->regbase+(csr)))
#define WRITEMBOX(sc,csr,val) (phys_write32_swapped((sc)->regbase+(csr), (val)))

#define READMEM(sc,csr)       (phys_read32_swapped((sc)->membase+((csr)^4)))
#define WRITEMEM(sc,csr,val)  (phys_write32_swapped((sc)->membase+((csr)^4), (val)))

#endif
#else  /* !ENDIAN_BIG || !MATCH_BYTES */
/* These macros attempt to be compatible with match-bits mode,
   which may put the data and byte masks into the wrong 32-bit word
   for 64-bit accesses.  See the comment above on PIOSWAP.
   Externally mastered DMA (control and data) uses match-bits and does
   specify word-swaps when operating big endian.  */

/* Most registers are 32 bits wide and are accessed by 32-bit
   transactions.  The mailbox registers and on-chip RAM are 64-bits
   wide but are generally accessed by 32-bit transactions.
   Furthermore, the documentation is ambiguous about which 32-bits of
   the mailbox is significant.  To localize the potential confusions,
   we define macros for the 3 different cases.  */

#define CSR_MATCH_MODE        PCI_MATCH_BITS

#define READCSR(sc,csr)       (l_phys_read32((sc)->regbase + (csr)))
#define WRITECSR(sc,csr,val)  (l_phys_write32((sc)->regbase + (csr), (val)))

#if PIOSWAP
#define READMBOX(sc,csr)      (phys_read32((sc)->regbase+((csr)^4)))
#define WRITEMBOX(sc,csr,val) (phys_write32((sc)->regbase+((csr)^4), (val)))

#define READMEM(sc,csr)       (phys_read32((sc)->membase+(csr)))
#define WRITEMEM(sc,csr,val)  (phys_write32((sc)->membase+(csr), (val)))

#else
#define READMBOX(sc,csr)      (l_phys_read32((sc)->regbase+(csr)))
#define WRITEMBOX(sc,csr,val) (l_phys_write32((sc)->regbase+(csr), (val)))

#define READMEM(sc,csr)       (l_phys_read32((sc)->membase+((csr)^4)))
#define WRITEMEM(sc,csr,val)  (l_phys_write32((sc)->membase+((csr)^4), (val)))

#endif
#endif  /* MATCH_BYTES */


/* 64-bit swap macros. */

#if ENDIAN_LITTLE
/* For little-endian systems, we use PCI swap settings that preserve
   the offsets of 32-bit fields in control structs (e.g.,
   descriptors).  As a result, upper and lower halves of 64-bit
   control fields are swapped.  We deal with this explicitly for
   addresses but must normalize MIB counters.  This choice could be
   reconsidered. */

static uint64_t
ctoh64(uint64_t x)
{
    return ((x & 0xFFFFFFFF) << 32) | ((x >> 32) & 0xFFFFFFFF);
}

#else
#define ctoh64(x) ((uint64_t)(x))

#endif /* ENDIAN_LITTLE */


/* Entry to and exit from critical sections (currently relative to
   interrupts only, not SMP) */

#if CFG_INTERRUPTS
#define CS_ENTER(sc) cfe_disable_irq(sc->irq)
#define CS_EXIT(sc)  cfe_enable_irq(sc->irq)
#else
#define CS_ENTER(sc) ((void)0)
#define CS_EXIT(sc)  ((void)0)
#endif


static void
dumpseq(t3_ether_t *sc, int start, int next)
{
    int offset, i, j;
    int columns = 4;
    int lines = (((next - start)/4 + 1) + 3)/columns;
    int step = lines*4;

    offset = start;
    for (i = 0; i < lines; i++) {
	xprintf("\nCSR");
	for (j = 0; j < columns; j++) {
	    if (offset + j*step < next)
		xprintf(" %04X: %08X ",
			offset+j*step, READCSR(sc, offset+j*step));
	    }
	offset += 4;
	}
    xprintf("\n");
}

static void
dumpcsrs(t3_ether_t *sc, const char *legend)
{
    xprintf("%s:\n", legend);

    /* Some device-specific PCI configuration registers */
    xprintf("-----PCI-----");
    dumpseq(sc, 0x68, 0x78);

    /* Some general control registers */
    xprintf("---General---");
    dumpseq(sc, 0x6800, 0x6810);

    xprintf("-------------\n");
}


/* Packet management.  Note that MIN_RXP_STD_BDS must be at least as
   big as STD_RCV_BD_THRESH */

#define ETH_PKTPOOL_SIZE  16
#define MIN_RXP_STD_BDS   8


static eth_pkt_t *
eth_alloc_pkt(t3_ether_t *sc)
{
    eth_pkt_t *pkt;

    CS_ENTER(sc);
    pkt = (eth_pkt_t *) q_deqnext(&sc->freelist);
    CS_EXIT(sc);
    if (!pkt) return NULL;

    pkt->buffer = pkt->data;
    pkt->length = ETH_PKTBUF_LEN;
    pkt->flags = 0;

    return pkt;
}


static void
eth_free_pkt(t3_ether_t *sc, eth_pkt_t *pkt)
{
    CS_ENTER(sc);
    q_enqueue(&sc->freelist, &pkt->next);
    CS_EXIT(sc);
}

static void
eth_initfreelist(t3_ether_t *sc)
{
    int idx;
    uint8_t *ptr;
    eth_pkt_t *pkt;

    q_init(&sc->freelist);

    ptr = sc->pktpool;
    for (idx = 0; idx < ETH_PKTPOOL_SIZE; idx++) {
	pkt = (eth_pkt_t *) ptr;
	eth_free_pkt(sc, pkt);
	ptr += ETH_PKTBUF_SIZE;
	}
}


/* Utilities */

static const char *
t3_devname(t3_ether_t *sc)
{
    return (sc->devctx != NULL ? cfe_device_name(sc->devctx) : "eth?");
}


/* CRCs */

uint32_t eth_crc32(const uint8_t *databuf, unsigned int datalen);
/*static*/ uint32_t
eth_crc32(const uint8_t *databuf, unsigned int datalen) 
{       
    unsigned int idx, bit, data;
    uint32_t crc;

    crc = 0xFFFFFFFFUL;
    for (idx = 0; idx < datalen; idx++)
	for (data = *databuf++, bit = 0; bit < 8; bit++, data >>= 1)
	    crc = (crc >> 1) ^ (((crc ^ data) & 1) ? ENET_CRC32_POLY : 0);
    return crc;
}


/* Descriptor ring management */

/* Modular arithmetic is done by masking (assumes powers of 2) */
#define RXP_STD_MASK  (RXP_STD_ENTRIES-1)
#define TXP_RING_MASK (TXP_RING_ENTRIES-1)

static int
t3_add_rcvbuf(t3_ether_t *sc, eth_pkt_t *pkt)
{
    t3_rcv_bd_t *rxp;

    rxp = &(sc->rxp_std[sc->rxp_std_index]);
    rxp->bufptr_lo = PTR_TO_PCI(pkt->buffer);
    rxp->length = ETH_PKTBUF_LEN;
    CACHE_DMA_SYNC(rxp, sizeof(t3_rcv_bd_t));
    sc->rxp_std_index = (sc->rxp_std_index + 1) & RXP_STD_MASK;
    return 0;
}

static void
t3_fillrxring(t3_ether_t *sc)
{
    eth_pkt_t *pkt;
    unsigned rxp_ci, rxp_onring;

    rxp_ci = sc->status->rxc_std_index;  /* Get a snapshot */
    rxp_onring = (sc->rxp_std_index - rxp_ci) & RXP_STD_MASK;

    while (rxp_onring < MIN_RXP_STD_BDS) {
	pkt = eth_alloc_pkt(sc);
	if (pkt == NULL) {
	    /* could not allocate a buffer */
	    break;
	    }
	if (t3_add_rcvbuf(sc, pkt) != 0) {
	    /* could not add buffer to ring */
	    eth_free_pkt(sc, pkt);
	    break;
	    }
	rxp_onring++;
	}
}

static void
t3_rx_callback(t3_ether_t *sc, eth_pkt_t *pkt)
{
    if (T3_DEBUG) show_packet('>', pkt);   /* debug */

    CS_ENTER(sc);
    q_enqueue(&sc->rxqueue, &pkt->next);
    CS_EXIT(sc);
}

static void
t3_procrxring(t3_ether_t *sc)
{
    eth_pkt_t   *pkt;
    t3_rcv_bd_t *rxc;
    uint32_t     rxr_1_index;
    volatile t3_status_t *status = sc->status;

    rxr_1_index = sc->rxr_1_index;
    rxc = &(sc->rxr_1[rxr_1_index]);

#if T3_BRINGUP
	    xprintf("%s: rx error %04X\n", t3_devname(sc), rxc->error_flag);
#endif

    do {
	CACHE_DMA_INVAL(rxc, sizeof(t3_rcv_bd_t));
	pkt = ETH_PKT_BASE(PCI_TO_PTR(rxc->bufptr_lo));
	pkt->length = rxc->length - ENET_CRC_SIZE;
	if ((rxc->flags & RX_FLAG_ERROR) == 0)
	    t3_rx_callback(sc, pkt);
	else {
#if T3_BRINGUP
	    xprintf("%s: rx error %04X\n", t3_devname(sc), rxc->error_flag);
#endif
	    eth_free_pkt(sc, pkt);   /* Could optimize */
	    }
	rxr_1_index++;
	rxc++;
	if (rxr_1_index == sc->rxr_entries) {
	    rxr_1_index = 0;
	    rxc = sc->rxr_1;
	    }
	} while (status->index[RI(1)].return_p != rxr_1_index);

    /* Update the return ring */
    sc->rxr_1_index = rxr_1_index;
    WRITEMBOX(sc, R_RCV_BD_RTN_CI(1), rxr_1_index);

    /* Refill the producer ring */
    t3_fillrxring(sc);
}


static int
t3_transmit(t3_ether_t *sc, eth_pkt_t *pkt)
{
    t3_snd_bd_t *txp;
    uint32_t     txp_1_next;

    if (T3_DEBUG) show_packet('<', pkt);   /* debug */

    txp_1_next = (sc->txp_1_index + 1) & TXP_RING_MASK;
    if (txp_1_next == sc->txc_1_index)
	return -1;

    txp = &(sc->txp_1[sc->txp_1_index]);
    txp->bufptr_hi = 0;
    txp->bufptr_lo = PTR_TO_PCI(pkt->buffer);
    txp->length = pkt->length;
    txp->flags = TX_FLAG_PACKET_END;
    CACHE_DMA_SYNC(txp, sizeof(t3_snd_bd_t));

    sc->txp_1_index = txp_1_next;
    WRITEMBOX(sc, R_SND_BD_PI(1), txp_1_next);

    return 0;
}


static void
t3_proctxring(t3_ether_t *sc)
{
    eth_pkt_t   *pkt;
    t3_snd_bd_t *txc;
    uint32_t     txc_1_index;
    volatile t3_status_t *status = sc->status;

    txc_1_index = sc->txc_1_index;
    txc = &(sc->txp_1[txc_1_index]);
    do {
	CACHE_DMA_INVAL(txc, sizeof(t3_snd_bd_t));
	pkt = ETH_PKT_BASE(PCI_TO_PTR(txc->bufptr_lo));
	eth_free_pkt(sc, pkt);
	txc_1_index = (txc_1_index + 1) & TXP_RING_MASK;
	txc++;
	if (txc_1_index == 0) txc = sc->txp_1;
	} while (status->index[RI(1)].send_c != txc_1_index);

    sc->txc_1_index = txc_1_index;
}


static void
t3_initrings(t3_ether_t *sc)
{
    int  i;
    t3_rcv_bd_t *rxp;
    volatile t3_status_t *status = sc->status;

    /* Clear all Producer BDs */
    rxp = &(sc->rxp_std[0]);
    for (i = 0; i < RXP_STD_ENTRIES; i++) {
        rxp->bufptr_hi = rxp->bufptr_lo = 0;
	rxp->length = 0;
	rxp->index = i;
	rxp->flags = 0;
	rxp->type = 0;
	rxp->ip_cksum = rxp->tcp_cksum = 0;
	rxp++;
	}
    CACHE_DMA_SYNC(sc->rxp_std, sizeof(t3_rcv_bd_t)*RXP_STD_ENTRIES);
    CACHE_DMA_INVAL(sc->rxp_std, sizeof(t3_rcv_bd_t)*RXP_STD_ENTRIES);

    /* Init the ring pointers */

    sc->rxp_std_index = 0;  status->rxc_std_index = 0;
    sc->rxr_1_index = 0;    status->index[RI(1)].return_p = 0;
    sc->txp_1_index = 0;    status->index[RI(1)].send_c = 0;

    /* Allocate some initial buffers for the Producer BD ring */
    sc->prev_rxp_std_index = 0;
    t3_fillrxring(sc);

    /* Nothing consumed yet */
    sc->txc_1_index = 0;
}

/* Allocate an integral number of cache lines suitable for DMA access. */
static uint8_t *
dma_alloc(size_t size, unsigned int align)
{
    uint8_t *base;
    size_t len = ALIGN(size, CACHE_ALIGN);

    base = KMALLOC(len, ALIGN(align, CACHE_ALIGN));
    if (base != NULL)
	CACHE_DMA_INVAL(base, len);
    return base;
}

static void
t3_init(t3_ether_t *sc)
{
    /* Allocate buffer pool */
    sc->pktpool = dma_alloc(ETH_PKTPOOL_SIZE*ETH_PKTBUF_SIZE, CACHE_ALIGN);

    if (sc->pktpool != NULL) {
	eth_initfreelist(sc);
	q_init(&sc->rxqueue);

	t3_initrings(sc);
	}
}

static void
t3_reinit(t3_ether_t *sc)
{
    eth_initfreelist(sc);
    q_init(&sc->rxqueue);

    t3_initrings(sc);
}


#if ENDIAN_BIG
/* Byte swap utilities. */

#define SWAP4(x) \
    ((((x) & 0x00FF) << 24) | \
     (((x) & 0xFF00) << 8)  | \
     (((x) >> 8) & 0xFF00)  | \
     (((x) >> 24) & 0x00FF))

static uint32_t
swap4(uint32_t x)
{
    uint32_t t;

    t = ((x & 0xFF00FF00) >> 8) | ((x & 0x00FF00FF) << 8);
    return (t >> 16) | ((t & 0xFFFF) << 16);
}
#endif /* ENDIAN_BIG */


/* EEPROM access functions (BCM5700 and BCM5701 version) */

/* The 570x chips support multiple access methods.  We use "Auto Access",
   which requires that
     Miscellaneous_Local_Control.Auto_SEEPROM_Access be set,
     Serial_EEprom.Address.HalfClock be programmed for <= 400 Hz.
   (both done by initialization code) */

#define EP_MAX_RETRIES  500
#define EP_DEVICE_ID    0x00           /* default ATMEL device ID */

static void
eeprom_access_init(t3_ether_t *sc)
{
  uint32_t mlctl;

  WRITECSR(sc, R_EEPROM_ADDR, M_EPADDR_RESET | V_EPADDR_HPERIOD(0x60));

  mlctl = READCSR(sc, R_MISC_LOCAL_CTRL);
  mlctl |= M_MLCTL_EPAUTOACCESS;
  WRITECSR(sc, R_MISC_LOCAL_CTRL, mlctl);
}


static uint32_t
eeprom_read_word(t3_ether_t *sc, unsigned int offset)
{
    /* Assumes that SEEPROM is already set up for auto access. */
    uint32_t epaddr, epdata;
    volatile uint32_t temp;
    int i;

    epaddr = READCSR(sc, R_EEPROM_ADDR);
    epaddr &= M_EPADDR_HPERIOD;
    epaddr |= (V_EPADDR_ADDR(offset) | V_EPADDR_DEVID(EP_DEVICE_ID)
	       | M_EPADDR_RW | M_EPADDR_START | M_EPADDR_COMPLETE);
    WRITECSR(sc, R_EEPROM_ADDR, epaddr);
    temp = READCSR(sc, R_EEPROM_ADDR);   /* push */

    for (i = 0; i < EP_MAX_RETRIES; i++) {
        temp = READCSR(sc, R_EEPROM_ADDR);
	if ((temp & M_EPADDR_COMPLETE) != 0)
	    break;
	cfe_usleep(10);
    }
    if (i == EP_MAX_RETRIES)
	xprintf("%s: eeprom_read_word: no SEEPROM response @ %x\n",
		t3_devname(sc), offset);

    epdata = READCSR(sc, R_EEPROM_DATA);   /* little endian */
#if ENDIAN_BIG
    return swap4(epdata);
#else
    return epdata;
#endif
}

static int
eeprom_read_range(t3_ether_t *sc, unsigned int offset, unsigned int len,
		  uint32_t buf[])
{
    int index;

    offset &= ~3;  len &= ~3;     /* 4-byte words only */
    index = 0;
    
    while (len > 0) {
	buf[index++] = eeprom_read_word(sc, offset);
	offset += 4;  len -= 4;
	}

    return index;
}

static void
eeprom_dump_range(const char *label,
		  uint32_t buf[], unsigned int offset, unsigned int len)
{
    int index;

    xprintf("EEPROM: %s", label);

    offset &= ~3;  len &= ~3;     /* 4-byte words only */
    index = 0;

    for (index = 0; len > 0; index++) {
	if (index % 8 == 0)
	    xprintf("\n %04x: ", offset);
	xprintf(" %08x", buf[offset/4]);
	offset += 4;  len -= 4;
	}
    xprintf("\n");
}


/* MII access functions.  */

/* BCM5401 device specific registers */

#define MII_ISR         0x1A    /* Interrupt Status Register */
#define MII_IMR         0x1B    /* Interrupt Mask Register */

#define M_INT_LINKCHNG  0x0002


/* The 570x chips support multiple access methods.  We use "Auto
   Access", which requires that MDI_Control_Register.MDI_Select be
   clear (done by initialization code) */

#define MII_MAX_RETRIES 5000

static void
mii_access_init(t3_ether_t *sc)
{
    WRITECSR(sc, R_MDI_CTRL, 0);                    /* here for now */
#if !T3_AUTOPOLL
    WRITECSR(sc, R_MI_MODE, V_MIMODE_CLKCNT(0x1F));  /* max divider */
#endif
}

/* XXX Autopolling should be disabled during reads and writes per the
   manual, but doing so currently generates recurvise LINKCHNG
   attentions. */

static uint16_t
mii_read_register(t3_ether_t *sc, int phy, int index)
{
    uint32_t mode;
    uint32_t comm, val = 0;
    int   i;

    mode = READCSR(sc, R_MI_MODE);
#if 0 /* for now */
    if (mode & M_MIMODE_POLLING) {
	WRITECSR(sc, R_MI_MODE, mode & ~M_MIMODE_POLLING);
	cfe_usleep(40);
	}
#endif

    comm = (V_MICOMM_CMD_RD | V_MICOMM_PHY(phy) | V_MICOMM_REG(index)
	    | M_MICOMM_BUSY);
    WRITECSR(sc, R_MI_COMM, comm);

    for (i = 0; i < MII_MAX_RETRIES; i++) {
	val = READCSR(sc, R_MI_COMM);
	if ((val & M_MICOMM_BUSY) == 0)
	    break;
	}	
    if (i == MII_MAX_RETRIES)
	xprintf("%s: mii_read_register: MII always busy\n", t3_devname(sc));

#if 0
    if (mode & M_MIMODE_POLLING)
	WRITECSR(sc, R_MI_MODE, mode);
#endif

    return G_MICOMM_DATA(val);
}

/* Register reads occasionally return spurious 0's.  Verify a zero by
   doing a second read, or spinning when a zero is "impossible".  */
static uint16_t
mii_read_register_v(t3_ether_t *sc, int phy, int index, int spin)
{
    uint32_t val;

    val = mii_read_register(sc, phy, index);
    if (val == 0) {
	do {
	    val = mii_read_register(sc, phy, index);
	    } while (spin && val == 0);
	}
    return val;
}

static void
mii_write_register(t3_ether_t *sc, int phy, int index, uint16_t value)
{
    uint32_t mode;
    uint32_t comm, val;
    int   i;

    mode = READCSR(sc, R_MI_MODE);
#if 0 /* for now */
    if (mode & M_MIMODE_POLLING) {
	WRITECSR(sc, R_MI_MODE, mode & ~M_MIMODE_POLLING);
	cfe_usleep(40);
	}
#endif

    comm = (V_MICOMM_CMD_WR | V_MICOMM_PHY(phy) | V_MICOMM_REG(index)
	    | V_MICOMM_DATA(value) | M_MICOMM_BUSY);
    WRITECSR(sc, R_MI_COMM, comm);

    for (i = 0; i < MII_MAX_RETRIES; i++) {
	val = READCSR(sc, R_MI_COMM);
	if ((val & M_MICOMM_BUSY) == 0)
	    break;
	}	
    if (i == MII_MAX_RETRIES)
	xprintf("%s: mii_write_register: MII always busy\n", t3_devname(sc));

#if 0
    if (mode & M_MIMODE_POLLING)
	WRITECSR(sc, R_MI_MODE, mode);
#endif
}

static int
mii_probe(t3_ether_t *sc)
{
    uint16_t id1, id2;

#if T3_AUTOPOLL   /* With autopolling, the code below is not reliable.  */
    sc->phy_addr = 1;     /* Guaranteed for integrated PHYs */
    id1 = mii_read_register(sc, 1, MII_PHYIDR1);
    id2 = mii_read_register(sc, 1, MII_PHYIDR2);
    sc->phy_vendor = ((uint32_t)id1 << 6) | ((id2 >> 10) & 0x3F);
    sc->phy_device = (id2 >> 4) & 0x3F;
    return 0;
#else
    int i;

    for (i = 0; i < 32; i++) {
        id1 = mii_read_register(sc, i, MII_PHYIDR1);
	id2 = mii_read_register(sc, i, MII_PHYIDR2);
	if ((id1 != 0x0000 && id1 != 0xFFFF) ||
	    (id2 != 0x0000 && id2 != 0xFFFF)) {
	    if (id1 != id2) {
	        sc->phy_addr = i;
		sc->phy_vendor = ((uint32_t)id1 << 6) | ((id2 >> 10) & 0x3F);
		sc->phy_device = (id2 >> 4) & 0x3F;
		return 0;
		}
	    }
	}
    return -1;
#endif
}

#if T3_DEBUG
static void
mii_dump(t3_ether_t *sc, const char *label)
{
    int i;
    uint16_t  r;

    xprintf("%s, MII:\n", label);

    /* Required registers */
    for (i = 0x0; i <= 0x6; ++i) {
	r = mii_read_register(sc, sc->phy_addr, i);
	xprintf(" REG%02X: %04X", i, r);
	if (i == 3 || i == 6)
	    xprintf("\n");
	}

    /* GMII extensions */
    for (i = 0x9; i <= 0xA; ++i) {
	r = mii_read_register(sc, sc->phy_addr, i);
	xprintf(" REG%02X: %04X", i, r);
	}
    r = mii_read_register(sc, sc->phy_addr, 0xF);
    xprintf(" REG%02X: %04X\n", 0xF, r);

    /* Broadcom extensions (54xx family) */
    if (sc->phy_vendor == OUI_BCM) {
	for (i = 0x10; i <= 0x14; i++) {
	    r = mii_read_register(sc, sc->phy_addr, i);
	    xprintf(" REG%02X: %04X", i, r);
	    }
	xprintf("\n");
	for (i = 0x18; i <= 0x1A; i++) {
	    r = mii_read_register(sc, sc->phy_addr, i);
	    xprintf(" REG%02X: %04X", i, r);
	    }
	xprintf("\n");
	}
}
#else
#define mii_dump(sc,label)
#endif

static void
mii_enable_interrupts(t3_ether_t *sc)
{
  mii_write_register(sc, sc->phy_addr, MII_IMR, ~M_INT_LINKCHNG);
}


/* For 5700/5701, LINKCHNG is read-only in the status register and
   cleared by writing to CFGCHNG | SYNCCHNG.  For the 5705
   (empirically), LINKCHNG is cleared by writing a one, while CFGCHNG
   and SYNCCHNG are unimplemented.  Thus we can safely clear the
   interrupt by writing ones to all the above bits.  */

#define M_LINKCHNG_CLR \
    (M_EVT_LINKCHNG | M_MACSTAT_CFGCHNG | M_MACSTAT_SYNCCHNG)

static int
mii_poll(t3_ether_t *sc)
{
    uint32_t  macstat;
    uint16_t  status, ability, xability;
    uint16_t isr;

    macstat = READCSR(sc, R_MAC_STATUS);
    if ((macstat & (M_EVT_LINKCHNG | M_EVT_MIINT)) != 0)
	WRITECSR(sc, R_MAC_STATUS, M_LINKCHNG_CLR);

    /* BMSR has read-to-clear bits; read twice.  */
    
    status = mii_read_register(sc, sc->phy_addr, MII_BMSR);
    status = mii_read_register_v(sc, sc->phy_addr, MII_BMSR, 1);
    ability = mii_read_register_v(sc, sc->phy_addr, MII_ANLPAR, 0);
    if (status & BMSR_1000BT_XSR)
	xability = mii_read_register_v(sc, sc->phy_addr, MII_K1STSR, 0);
    else
	xability = 0;
    isr = mii_read_register(sc, sc->phy_addr, MII_ISR);

    if (status != sc->phy_status
	|| ability != sc->phy_ability || xability != sc->phy_xability) {
#if T3_DEBUG
	xprintf("[%04x]", isr);
	xprintf((macstat & (M_EVT_LINKCHNG | M_EVT_MIINT)) != 0 ? "+" : "-");
      
	if (status != sc->phy_status)
	    xprintf(" ST: %04x %04x", sc->phy_status, status);
	if (ability != sc->phy_ability)
	    xprintf(" AB: %04x %04x", sc->phy_ability, ability);
	if (xability != sc->phy_xability)
	    xprintf(" XA: %04x %04x", sc->phy_xability, xability);
	xprintf("\n");
#endif
        sc->phy_status = status;
	sc->phy_ability = ability;
	sc->phy_xability = xability;
	return 1;
	}
    else if ((macstat & (M_EVT_LINKCHNG | M_EVT_MIINT)) != 0) {
	isr = mii_read_register(sc, sc->phy_addr, MII_ISR);
	}
    return 0;
}

static void
mii_set_speed(t3_ether_t *sc, int speed)
{
    uint16_t  control;

    control = mii_read_register(sc, sc->phy_addr, MII_BMCR);

    control &= ~(BMCR_ANENABLE | BMCR_RESTARTAN);
    mii_write_register(sc, sc->phy_addr, MII_BMCR, control);
    control &= ~(BMCR_SPEED0 | BMCR_SPEED1 | BMCR_DUPLEX);

    switch (speed) {
	case ETHER_SPEED_10HDX:
	default:
	    break;
	case ETHER_SPEED_10FDX:
	    control |= BMCR_DUPLEX;
	    break;
	case ETHER_SPEED_100HDX:
	    control |= BMCR_SPEED100;
	    break;
	case ETHER_SPEED_100FDX:
	    control |= BMCR_SPEED100 | BMCR_DUPLEX ;
	    break;
	}

    mii_write_register(sc, sc->phy_addr, MII_BMCR, control);
}

static void
mii_autonegotiate(t3_ether_t *sc)
{
    uint16_t  control, status, remote, xremote;
    unsigned int  timeout;
    int linkspeed;
    uint32_t mode;

    linkspeed = ETHER_SPEED_UNKNOWN;

    /* Read twice to clear latching bits */
    status = mii_read_register(sc, sc->phy_addr, MII_BMSR);
    status = mii_read_register_v(sc, sc->phy_addr, MII_BMSR, 1);
    mii_dump(sc, "query PHY");

    if ((status & (BMSR_AUTONEG | BMSR_LINKSTAT)) ==
        (BMSR_AUTONEG | BMSR_LINKSTAT))
	control = mii_read_register(sc, sc->phy_addr, MII_BMCR);
    else {
	for (timeout = 4*CFE_HZ; timeout > 0; timeout -= CFE_HZ/2) {
	    status = mii_read_register(sc, sc->phy_addr, MII_BMSR);
	    if ((status & BMSR_ANCOMPLETE) != 0)
		break;
	    cfe_sleep(CFE_HZ/2);
	    }
	}

    remote = mii_read_register_v(sc, sc->phy_addr, MII_ANLPAR, 0);
    
    /* XXX Empirically, it appears best to set/keep PortMode non-null to
       get STATUS_LINKCHNG assertions. */
    mode = READCSR(sc, R_MAC_MODE);

#if T3_DEBUG
    xprintf("Mode0:%08X ", (int) mode);
#endif

    xprintf("%s: Link speed: ", t3_devname(sc));
    if ((status & BMSR_ANCOMPLETE) != 0) {
	/* A link partner was negogiated... */

	if (status & BMSR_1000BT_XSR)
	    xremote = mii_read_register_v(sc, sc->phy_addr, MII_K1STSR, 0);
	else
	    xremote = 0;

	mode &= ~(M_MACM_PORTMODE | M_MACM_HALFDUPLEX);

#if T3_DEBUG
	xprintf("Mode1:%08X ", (int) mode);
#endif

	if ((xremote & K1STSR_LP1KFD) != 0) {
	    xprintf("1000BaseT FDX\n");
	    linkspeed = ETHER_SPEED_1000FDX;
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_GMII);
	    }
	else if ((xremote & K1STSR_LP1KHD) != 0) {
	    xprintf("1000BaseT HDX\n");
	    linkspeed = ETHER_SPEED_1000HDX;
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_GMII) | M_MACM_HALFDUPLEX;
	    }
	else if ((remote & ANLPAR_TXFD) != 0) {
	    xprintf("100BaseT FDX\n");
	    linkspeed = ETHER_SPEED_100FDX;	 
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_MII);
	    }
	else if ((remote & ANLPAR_TXHD) != 0) {
	    xprintf("100BaseT HDX\n");
	    linkspeed = ETHER_SPEED_100HDX;	 
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_MII) | M_MACM_HALFDUPLEX;
	    }
	else if ((remote & ANLPAR_10FD) != 0) {
	    xprintf("10BaseT FDX\n");
	    linkspeed = ETHER_SPEED_10FDX;	 
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_MII);
	    }
	else if ((remote & ANLPAR_10HD) != 0) {
	    xprintf("10BaseT HDX\n");
	    linkspeed = ETHER_SPEED_10HDX;	 
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_MII) | M_MACM_HALFDUPLEX;
	    }

	WRITECSR(sc, R_MAC_MODE, mode);
#if T3_DEBUG
    	xprintf("Mode2:%08X ", (int) mode);
#endif
	}
    else {
	/* no link partner convergence */
	xprintf("Unknown\n");
	linkspeed = ETHER_SPEED_UNKNOWN;
	remote = xremote = 0;
	if (G_MACM_PORTMODE(mode) == K_MACM_PORTMODE_NONE) {
	    /* Keep any previous port mode as the one most likely to reappear.
	       Otherwise, choose one, and 10/100FDX is more likely. */
	    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_MII);
	    WRITECSR(sc, R_MAC_MODE, mode);
	    }
	}
    sc->linkspeed = linkspeed;

    /* clear latching bits, XXX fix flakey reads */
    status = mii_read_register_v(sc, sc->phy_addr, MII_BMSR, 1);
    (void)mii_read_register(sc, sc->phy_addr, MII_ISR);

    sc->phy_status = status;
    sc->phy_ability = remote;
    sc->phy_xability = xremote;

    mii_dump(sc, "final PHY");
}


static void
t3_clear(t3_ether_t *sc, unsigned reg, uint32_t mask)
{
    uint32_t val;
    int timeout;

    val = READCSR(sc, reg);
    val &= ~mask;
    WRITECSR(sc, reg, val);
    val = READCSR(sc, reg);

    for (timeout = 4000; (val & mask) != 0 && timeout > 0; timeout -= 100) {
	cfe_usleep(100);
	val = READCSR(sc, reg);
	}
    if (timeout <= 0)
	xprintf("%s: cannot clear %04X/%08X\n", t3_devname(sc), reg, mask);
}


/* The following functions collectively implement the recommended
   BCM5700 Initialization Procedure (Section 8: Device Control) */

static int
t3_coldreset(t3_ether_t *sc)
{
    pcireg_t cmd;
    pcireg_t bhlc, subsysid;
    pcireg_t bar0, bar1;
    pcireg_t cmdx;
    uint32_t mhc, mcr, mcfg;
    uint32_t mode;
    int timeout;

    /* Steps 1-18 */
    /* Enable memory, also clear R/WC status bits (1) */
    cmd = pci_conf_read(sc->tag, PCI_COMMAND_STATUS_REG);
    cmd |= PCI_COMMAND_MEM_ENABLE | PCI_COMMAND_MASTER_ENABLE;
    pci_conf_write(sc->tag, PCI_COMMAND_STATUS_REG, cmd);
    
    /* Clear and disable INTA output. (2) */
    mhc = READCSR(sc, R_MISC_HOST_CTRL);
    mhc |= M_MHC_MASKPCIINT | M_MHC_CLEARINTA | M_MHC_ENINDIRECT | M_MHC_ENCLKCTRLRW;
    WRITECSR(sc, R_MISC_HOST_CTRL, mhc);

    /* Save some config registers modified by core clock reset (3). */
    bhlc = pci_conf_read(sc->tag, PCI_BHLC_REG);
    subsysid = pci_conf_read(sc->tag, PCI_SUBSYS_ID_REG);
    /* Empirically, these are clobbered too. */
    bar0 = pci_conf_read(sc->tag, PCI_MAPREG(0));
    bar1 = pci_conf_read(sc->tag, PCI_MAPREG(1));

    /* Reset the core clocks (4, 5). */
    mcfg = READCSR(sc, R_MISC_CFG);
    mcfg |= M_MCFG_CORERESET;
    WRITECSR(sc, R_MISC_CFG, mcfg);
    cfe_usleep(100);    /* 100 usec delay */

    /* NB: Until the BARs are restored and reenabled, only PCI
       configuration reads and writes will succeed.  */

    /* Reenable MAC memory (7) */
    pci_conf_write(sc->tag, PCI_MAPREG(0), bar0);
    pci_conf_write(sc->tag, PCI_MAPREG(1), bar1);
    (void)pci_conf_read(sc->tag, PCI_MAPREG(1));  /* push */
    pci_conf_write(sc->tag, PCI_COMMAND_STATUS_REG, cmd);
    (void)pci_conf_read(sc->tag, PCI_COMMAND_STATUS_REG);  /* push */

    /* Undo some of the resets (6) */
    mhc = READCSR(sc, R_MISC_HOST_CTRL);
    mhc |= M_MHC_MASKPCIINT | M_MHC_ENINDIRECT | M_MHC_ENCLKCTRLRW;
    WRITECSR(sc, R_MISC_HOST_CTRL, mhc);

    /* Verify that core clock resets completed and autocleared. */
    mcfg = READCSR(sc, R_MISC_CFG);
    if ((mcfg & M_MCFG_CORERESET) != 0) {
	xprintf("bcm5700: core clocks stuck in reset\n");
	}

    /* Configure PCI-X (8) */
    if (sc->device != K_PCI_ID_BCM5705) {
	cmdx = pci_conf_read(sc->tag, PCI_PCIX_CMD_REG);
	cmdx &= ~PCIX_CMD_RLXORDER_ENABLE;
	pci_conf_write(sc->tag, PCI_PCIX_CMD_REG, cmdx);
	}

    /* Enable memory arbiter (9)  */
    mode = READCSR(sc, R_MEM_MODE);
    mode |= M_MAM_ENABLE;    /* enable memory arbiter */
    WRITECSR(sc, R_MEM_MODE, mode);

    /* Assume no external SRAM for now (10) */

    /* Set up MHC for endianness and write enables (11-15) */
    mhc = READCSR(sc, R_MISC_HOST_CTRL);
    /* Since we use match-bits for Direct PCI access, don't swap bytes. */
    mhc &= ~M_MHC_ENBYTESWAP;
#if ENDIAN_LITTLE
    mhc |= M_MHC_ENWORDSWAP;
#endif
#if ENDIAN_BIG
#if PIOSWAP
    mhc |= M_MHC_ENWORDSWAP;
#endif
#endif
    mhc |= M_MHC_ENINDIRECT | M_MHC_ENPCISTATERW | M_MHC_ENCLKCTRLRW;
    WRITECSR(sc, R_MISC_HOST_CTRL, mhc);

    /* Set byte swapping (16, 17) */
    mcr = READCSR(sc, R_MODE_CTRL);
#if ENDIAN_LITTLE
    mcr |= M_MCTL_BSWAPDATA | M_MCTL_WSWAPDATA;
    mcr |= M_MCTL_WSWAPCTRL;
#endif
#if ENDIAN_BIG
#if MATCH_BYTES
    mcr |= M_MCTL_BSWAPDATA | M_MCTL_WSWAPDATA;
    mcr |= M_MCTL_BSWAPCTRL | M_MCTL_WSWAPCTRL;
#else
    mcr &= ~(M_MCTL_BSWAPCTRL | M_MCTL_BSWAPDATA);
    mcr |= M_MCTL_WSWAPCTRL | M_MCTL_WSWAPDATA;
#endif
#endif
    WRITECSR(sc, R_MODE_CTRL, mcr);

    /* Disable PXE restart, wait for firmware (18, 19) */
    if (READMEM(sc, A_PXE_MAILBOX) != T3_MAGIC_NUMBER) {
	/* Apparently, if the magic number is already set, firmware
	   ignores this attempted handshake. */
	WRITEMEM(sc, A_PXE_MAILBOX, T3_MAGIC_NUMBER);
	for (timeout = CFE_HZ; timeout > 0; timeout -= CFE_HZ/10) {
	    if (READMEM(sc, A_PXE_MAILBOX) == ~T3_MAGIC_NUMBER)
		break;
	    cfe_sleep(CFE_HZ/10);
	    }
	if (READMEM(sc, A_PXE_MAILBOX) != ~T3_MAGIC_NUMBER)
	    xprintf("bcm5700: no firmware PXE rendevous\n");

#if T3_DEBUG
	uint32_t mag;
	mag = READMEM(sc, A_PXE_MAILBOX);
	xprintf("magic number: %X %X\n",mag,~T3_MAGIC_NUMBER);

//	WRITEMEM(sc, A_PXE_MAILBOX, T3_MAGIC_NUMBER);

	mag = READMEM(sc, A_PXE_MAILBOX);	
	xprintf("magic number: %X %X\n",mag,~T3_MAGIC_NUMBER);
#endif

	}
    else
    {

#if T3_DEBUG
	uint32_t mag;
	mag = READMEM(sc, A_PXE_MAILBOX);
	xprintf("magic number: %X %X\n",mag,~T3_MAGIC_NUMBER);
#endif

	xprintf("bcm5700: PXE magic number already set\n");
	}

    /* Clear Ethernet MAC Mode (20) */
    WRITECSR(sc, R_MAC_MODE, 0x00000000);

    /* Restore remaining config registers (21) */
    pci_conf_write(sc->tag, PCI_BHLC_REG, bhlc);
    pci_conf_write(sc->tag, PCI_SUBSYS_ID_REG, subsysid);

    return 0;
}

/* XXX Not clear that the following is useful. */
static int
t3_warmreset(t3_ether_t *sc)
{
    uint32_t mode;

    /* Enable memory arbiter (9)  */
    mode = READCSR(sc, R_MEM_MODE);
    mode |= M_MAM_ENABLE;    /* enable memory arbiter */
    WRITECSR(sc, R_MEM_MODE, mode);

    /* Clear Ethernet MAC Mode (20) */
    WRITECSR(sc, R_MAC_MODE, 0x00000000);

    return 0;
}


static int
t3_init_registers(t3_ether_t *sc)
{
    unsigned offset;
    uint32_t dmac, mcr, mcfg;

    /* Steps 22-29 */

    /* Clear MAC statistics block (22) */
    for (offset = A_MAC_STATS; offset < A_MAC_STATS+L_MAC_STATS; offset += 4) {
	WRITEMEM(sc, offset, 0);
	}

    /* Clear driver status memory region (23) */
    /* ASSERT (sizeof(t3_status_t) == L_MAC_STATUS) */
    memset((uint8_t *)sc->status, 0, sizeof(t3_status_t));

    /* Set up PCI DMA control (24) */
    dmac = READCSR(sc, R_DMA_RW_CTRL);
    dmac &= ~(M_DMAC_RDCMD | M_DMAC_WRCMD | M_DMAC_MINDMA);
    dmac |= V_DMAC_RDCMD(K_PCI_MEMRD) | V_DMAC_WRCMD(K_PCI_MEMWR);
    switch (sc->device) {
	case K_PCI_ID_BCM5700:
	case K_PCI_ID_BCM5701:
	case K_PCI_ID_BCM5702:
	    dmac |= V_DMAC_MINDMA(0xF);    /* "Recommended" */
	    break;

	case K_PCI_ID_BCM5780:
	    /* XXX magic values, Broadcom-supplied Linux driver */
	    dmac |= (1 << 20) | (1 << 18) | M_DMAC_ONEDMA;
#if T3_DEBUG
	    dmac |= 0x00144000;
#endif
	    break;

     /* case other 5714 family */
	    /* dmac |= (1 << 20) | (1 << 18) | (1 << 15); */

	default:
	    dmac |= V_DMAC_MINDMA(0x0);
	    break;
	}
    WRITECSR(sc, R_DMA_RW_CTRL, dmac);

    /* Set DMA byte swapping (25) - XXX repeat of (17) */
    mcr = READCSR(sc, R_MODE_CTRL);
#if ENDIAN_LITTLE
    mcr |= M_MCTL_BSWAPDATA | M_MCTL_WSWAPDATA;
    mcr |= M_MCTL_WSWAPCTRL;
#endif
#if ENDIAN_BIG
#if MATCH_BYTES
    mcr |= M_MCTL_BSWAPDATA | M_MCTL_WSWAPDATA;
    mcr |= M_MCTL_BSWAPCTRL | M_MCTL_WSWAPCTRL;
#else
    mcr &= ~(M_MCTL_BSWAPCTRL | M_MCTL_BSWAPDATA);
    mcr |= M_MCTL_WSWAPCTRL | M_MCTL_WSWAPDATA;
#endif
#endif
    WRITECSR(sc, R_MODE_CTRL, mcr);

    /* Configure host rings (26) */
    mcr |= M_MCTL_HOSTBDS;
    WRITECSR(sc, R_MODE_CTRL, mcr);

    /* Indicate driver ready, disable checksums (27, 28) */
    mcr |= M_MCTL_HOSTUP;
    mcr |= (M_MCTL_NOTXPHSUM | M_MCTL_NORXPHSUM | M_MCTL_HOSTBDS);
    WRITECSR(sc, R_MODE_CTRL, mcr);

    /* Configure timer (29) */
    mcfg = READCSR(sc, R_MISC_CFG);
    mcfg &= ~M_MCFG_PRESCALER;
    mcfg |= V_MCFG_PRESCALER(66-1);    /* 66 MHz */
    WRITECSR(sc, R_MISC_CFG, mcfg);

    return 0;
}

static int
t3_init_pools(t3_ether_t *sc)
{
    uint32_t mode;
    int timeout;

    /* Steps 30-36.  These use "recommended" settings (p 150) */

    /* Configure the MAC memory pool (30) */
    if ((sc->device != K_PCI_ID_BCM5705) && 
        !BCM571X_FAMILY_DEVICE(sc->device)) {

        WRITECSR(sc, R_BMGR_MBUF_BASE, A_BUFFER_POOL);
        WRITECSR(sc, R_BMGR_MBUF_LEN, L_BUFFER_POOL);
	
	} 
    else 
    {
        /* Note: manual appears to recommend not even writing these (?) */
        /* WRITECSR(sc, R_BMGR_MBUF_BASE, A_RXMBUF); */
        /* WRITECSR(sc, R_BMGR_MBUF_LEN, 0x8000); */
	}

    if (BCM571X_FAMILY_DEVICE(sc->device))
    {

       /* Configure the MAC memory watermarks for BCM571X family (32) */
        WRITECSR(sc, R_BMGR_MBUF_DMA_LOW, 0x0);
        WRITECSR(sc, R_BMGR_MBUF_RX_LOW,  0x10);
        WRITECSR(sc, R_BMGR_MBUF_HIGH,    0x60);
    }
    else
    {

       /* Configure the MAC DMA resource pool (31) */
        WRITECSR(sc, R_BMGR_DMA_BASE, A_DMA_DESCS);
        WRITECSR(sc, R_BMGR_DMA_LEN,  L_DMA_DESCS);

        /* Configure the MAC memory watermarks (32) */
        WRITECSR(sc, R_BMGR_MBUF_DMA_LOW, 0x50);
        WRITECSR(sc, R_BMGR_MBUF_RX_LOW,  0x20);
        WRITECSR(sc, R_BMGR_MBUF_HIGH,    0x60);

        /* Configure the DMA resource watermarks (33) */
        WRITECSR(sc, R_BMGR_DMA_LOW,   5);
        WRITECSR(sc, R_BMGR_DMA_HIGH, 10);

        /* Enable the buffer manager (34, 35) */
        mode = READCSR(sc, R_BMGR_MODE);
        mode |= (M_BMODE_ENABLE | M_BMODE_MBUFLOWATTN);
        WRITECSR(sc, R_BMGR_MODE, mode);
        for (timeout = CFE_HZ/2; timeout > 0; timeout -= CFE_HZ/10) {
            mode = READCSR(sc, R_BMGR_MODE);
            if ((mode & M_BMODE_ENABLE) != 0)
                break;
            cfe_sleep(CFE_HZ/10);
        }
        if ((mode & M_BMODE_ENABLE) == 0)
            xprintf("bcm5700: buffer manager not enabled\n");
    }

    /* Enable internal queues (36) */
    WRITECSR(sc, R_FTQ_RESET, 0xFFFFFFFF);
#ifndef BCM47XX  /* XXX bus error on 5703 */
    (void)READCSR(sc, R_FTQ_RESET);    /* push */
#endif
    cfe_sleep(1);
    WRITECSR(sc, R_FTQ_RESET, 0x00000000);

    return 0;
}

static int
t3_init_rings(t3_ether_t *sc)
{
    unsigned rcbp;
    int i;

    /* Steps 37-46 */

    /* Initialize RCBs for Standard Receive Buffer Ring (37) */
    WRITECSR(sc, R_STD_RCV_BD_RCB+RCB_HOST_ADDR_HIGH, 0);
    WRITECSR(sc, R_STD_RCV_BD_RCB+RCB_HOST_ADDR_LOW, PTR_TO_PCI(sc->rxp_std));
    /* 5714 family device removed JUMBO ring */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) )
    {
        WRITECSR(sc, R_STD_RCV_BD_RCB+RCB_CTRL, V_RCB_MAXLEN(ETH_PKTBUF_LEN));
    }
    else
    {
        WRITECSR(sc, R_STD_RCV_BD_RCB+RCB_CTRL, V_RCB_MAXLEN(RXP_STD_ENTRIES));
    }
    WRITECSR(sc, R_STD_RCV_BD_RCB+RCB_NIC_ADDR, A_STD_RCV_RINGS);

    /* 5714 family device removed JUMBO ring */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) )
    {
        /* Disable RCBs for Jumbo and Mini Receive Buffer Rings (38,39) */
        WRITECSR(sc, R_JUMBO_RCV_BD_RCB+RCB_CTRL,
                 RCB_FLAG_USE_EXT_RCV_BD | RCB_FLAG_RING_DISABLED);
        WRITECSR(sc, R_JUMBO_RCV_BD_RCB+RCB_NIC_ADDR, A_JUMBO_RCV_RINGS);
        WRITECSR(sc, R_MINI_RCV_BD_RCB+RCB_CTRL, RCB_FLAG_RING_DISABLED);
        WRITECSR(sc, R_MINI_RCV_BD_RCB+RCB_NIC_ADDR, 0xe000);
        
        /* Set BD ring replenish thresholds (40) */
        WRITECSR(sc, R_MINI_RCV_BD_THRESH, 128);
    }

#if T3_BRINGUP
    WRITECSR(sc, R_STD_RCV_BD_THRESH, 1);
#else
    /* Note that STD_RCV_BD_THRESH cannot exceed MIN_RXP_STD_BDS */
    WRITECSR(sc, R_STD_RCV_BD_THRESH, 6);
#endif

    /* 5714 family device removed JUMBO ring */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) )
    {
        WRITECSR(sc, R_JUMBO_RCV_BD_THRESH, 16);
    }

      /* 5714 family device removed send rings 2-16 */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) )
     {
        /* Disable unused send producer rings 2-16 (41) */
        for (rcbp = A_SND_RCB(1); rcbp <= A_SND_RCB(16); rcbp += RCB_SIZE)
            WRITEMEM(sc, rcbp+RCB_CTRL, RCB_FLAG_RING_DISABLED);

        /* Initialize send producer index registers (42) */
        for (i = 1; i <= TXP_MAX_RINGS; i++) {
            WRITEMBOX(sc, R_SND_BD_PI(i), 0);
            WRITEMBOX(sc, R_SND_BD_NIC_PI(i), 0);
        }
    }
    else
    {
        WRITEMEM(sc, A_SND_RCB(1) + RCB_CTRL, RCB_FLAG_RING_DISABLED);
        WRITEMBOX(sc, R_SND_BD_PI(1), 0);
        WRITEMBOX(sc, R_SND_BD_NIC_PI(1), 0);
    }


    /* Initialize send producer ring 1 (43) */
    WRITEMEM(sc, A_SND_RCB(1)+RCB_HOST_ADDR_HIGH, 0);
    WRITEMEM(sc, A_SND_RCB(1)+RCB_HOST_ADDR_LOW, PTR_TO_PCI(sc->txp_1));
    WRITEMEM(sc, A_SND_RCB(1)+RCB_CTRL, V_RCB_MAXLEN(TXP_RING_ENTRIES));
    /* Only program send ring address for early chips */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) && 
         (sc->device != K_PCI_ID_BCM5705) )
    {
        WRITEMEM(sc, A_SND_RCB(1)+RCB_NIC_ADDR, A_SND_RINGS);
    }

    /* 5714 family device removed recieve return rings 2-16 */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) )
    {
        /* Disable unused receive return rings (44) */
        for (rcbp = A_RTN_RCB(1); rcbp <= A_RTN_RCB(16); rcbp += RCB_SIZE)
            WRITEMEM(sc, rcbp+RCB_CTRL, RCB_FLAG_RING_DISABLED);
    }
    else
    {
        WRITEMEM(sc, A_RTN_RCB(1) + RCB_CTRL, RCB_FLAG_RING_DISABLED);
    }

    /* Initialize receive return ring 1 (45) */
    WRITEMEM(sc, A_RTN_RCB(1)+RCB_HOST_ADDR_HIGH, 0);
    WRITEMEM(sc, A_RTN_RCB(1)+RCB_HOST_ADDR_LOW, PTR_TO_PCI(sc->rxr_1));
    WRITEMEM(sc, A_RTN_RCB(1)+RCB_CTRL, V_RCB_MAXLEN(sc->rxr_entries));
    WRITEMEM(sc, A_RTN_RCB(1)+RCB_NIC_ADDR, 0x0000);

    /* Initialize receive producer ring mailboxes (46) */
    WRITEMBOX(sc, R_RCV_BD_STD_PI, 0);

    /* 5714 family device removed jumbo / mini rings */
    if ( !BCM571X_FAMILY_DEVICE(sc->device) )
    {
        WRITEMBOX(sc, R_RCV_BD_JUMBO_PI, 0);
        WRITEMBOX(sc, R_RCV_BD_MINI_PI, 0);
    }

    return 0;
}

static int
t3_configure_mac(t3_ether_t *sc)
{
    uint32_t low, high;
    uint32_t seed;
    int i;

    /* Steps 47-52 */

    /* Configure the MAC unicast address (47) */
    high = (sc->hwaddr[0] << 8) | (sc->hwaddr[1]);
    low = ((sc->hwaddr[2] << 24) | (sc->hwaddr[3] << 16)
	   | (sc->hwaddr[4] << 8) | sc->hwaddr[5]);
    /* For now, use a single MAC address */
    WRITECSR(sc, R_MAC_ADDR1_HIGH, high);  WRITECSR(sc, R_MAC_ADDR1_LOW, low);
    WRITECSR(sc, R_MAC_ADDR2_HIGH, high);  WRITECSR(sc, R_MAC_ADDR2_LOW, low);
    WRITECSR(sc, R_MAC_ADDR3_HIGH, high);  WRITECSR(sc, R_MAC_ADDR3_LOW, low);
    WRITECSR(sc, R_MAC_ADDR4_HIGH, high);  WRITECSR(sc, R_MAC_ADDR4_LOW, low);

    /* Configure the random backoff seed (48) */
    seed = 0;
    for (i = 0; i < 6; i++)
      seed += sc->hwaddr[i];
    seed &= 0x3FF;
    WRITECSR(sc, R_TX_BACKOFF, seed);

    /* Configure the MTU (49) */
    WRITECSR(sc, R_RX_MTU, MAX_ETHER_PACK+VLAN_TAG_LEN);

    /* Configure the tx IPG (50) */
    WRITECSR(sc, R_TX_LENS,
	     V_TXLEN_SLOT(0x20) | V_TXLEN_IPG(0x6) | V_TXLEN_IPGCRS(0x2));

    /* Configure the default rx return ring 1 (51) */
    WRITECSR(sc, R_RX_RULES_CFG, V_RULESCFG_DEFAULT(1));

    /* Configure the receive lists and enable statistics (52) */
    WRITECSR(sc, R_RCV_LIST_CFG,
	     V_LISTCFG_GROUP(1) | V_LISTCFG_ACTIVE(1) | V_LISTCFG_BAD(1));
    /* was V_LISTCFG_DEFAULT(1) | V_LISTCFG_ACTIVE(16) | V_LISTCFG_BAD(1) */

    return 0;
}

static int
t3_enable_stats(t3_ether_t *sc)
{
    uint32_t ctrl;

    /* Steps 53-56 */

    /* Enable rx stats (53,54) */
    WRITECSR(sc, R_RCV_LIST_STATS_ENB, 0xFFFFFF);
    ctrl = READCSR(sc, R_RCV_LIST_STATS_CTRL);
    ctrl |= M_STATS_ENABLE;
    WRITECSR(sc, R_RCV_LIST_STATS_CTRL, ctrl);

    /* Enable tx stats (55,56) */
    WRITECSR(sc, R_SND_DATA_STATS_ENB, 0xFFFFFF);
    ctrl = READCSR(sc, R_SND_DATA_STATS_CTRL);
    ctrl |= (M_STATS_ENABLE | M_STATS_FASTUPDATE);
    WRITECSR(sc, R_SND_DATA_STATS_CTRL, ctrl);

    return 0;
}

static int
t3_init_coalescing(t3_ether_t *sc)
{
    uint32_t mode = 0;
    int timeout;

    /* Steps 57-68 */

    /* Disable the host coalescing engine (57, 58) */
    WRITECSR(sc, R_HOST_COAL_MODE, 0);    
    for (timeout = CFE_HZ/2; timeout > 0; timeout -= CFE_HZ/10) {
	mode = READCSR(sc, R_HOST_COAL_MODE);
	if (mode == 0)
	    break;
	cfe_sleep(CFE_HZ/10);
	}
    if (mode != 0)
	xprintf("bcm5700: coalescing engine not disabled\n");

    /* Set coalescing parameters (59-62) */
#if T3_BRINGUP
    WRITECSR(sc, R_RCV_COAL_TICKS, 0);
    WRITECSR(sc, R_RCV_COAL_MAX_CNT, 1);
#else
    WRITECSR(sc, R_RCV_COAL_TICKS, 150);
    WRITECSR(sc, R_RCV_COAL_MAX_CNT, 10);
#endif

    WRITECSR(sc, R_RCV_COAL_INT_TICKS, 0);
    WRITECSR(sc, R_RCV_COAL_INT_CNT, 0);
#if T3_BRINGUP
    WRITECSR(sc, R_SND_COAL_TICKS, 0);
    WRITECSR(sc, R_SND_COAL_MAX_CNT, 1);
#else
    WRITECSR(sc, R_SND_COAL_TICKS, 150);
    WRITECSR(sc, R_SND_COAL_MAX_CNT, 10);
#endif

    WRITECSR(sc, R_SND_COAL_INT_TICKS, 0);
    WRITECSR(sc, R_SND_COAL_INT_CNT, 0);

    /* Initialize host status block address (63) */
    WRITECSR(sc, R_STATUS_HOST_ADDR, 0);
    WRITECSR(sc, R_STATUS_HOST_ADDR+4, PTR_TO_PCI(sc->status));

    /* Initialize host statistics block address (64) */
    WRITECSR(sc, R_STATS_HOST_ADDR, 0);
    WRITECSR(sc, R_STATS_HOST_ADDR+4, PTR_TO_PCI(sc->stats));

    /* Set statistics block NIC address and tick count (65, 66) */
    WRITECSR(sc, R_STATS_TICKS, 1000000);
    WRITECSR(sc, R_STATS_BASE_ADDR, A_MAC_STATS);

    /* Set status block NIC address (67) */
    WRITECSR(sc, R_STATUS_BASE_ADDR, A_MAC_STATUS);

    /* Select the status block transfer size. */
    if (sc->device == K_PCI_ID_BCM5700)
	mode = 0;          /* Truncated transfers not supported */
    else
	mode = V_HCM_SBSIZE(STATUS_BLOCK_SIZE(MAX_RI));
      
    /* Enable the host coalescing engine (68) */
    mode |= M_HCM_ENABLE;
    WRITECSR(sc, R_HOST_COAL_MODE, mode);    

    return 0;
}

static int
t3_init_dma(t3_ether_t *sc)
{
    uint32_t mode;

    /* Steps 69-87 */

    /* Enable receive BD completion, placement, and selector blocks (69-71) */
    WRITECSR(sc, R_RCV_BD_COMP_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);
    WRITECSR(sc, R_RCV_LIST_MODE, M_MODE_ENABLE);

    /* Turn on RX list selector state machine. */
    if ( (sc->device != K_PCI_ID_BCM5705) 
         && !BCM571X_FAMILY_DEVICE(sc->device) ) {

	    WRITECSR(sc, R_RCV_LIST_SEL_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);
	}

    /* Enable DMA engines, enable and clear statistics (72, 73) */
    mode = READCSR(sc, R_MAC_MODE);
    mode |= (M_MACM_FHDEENB | M_MACM_RDEENB | M_MACM_TDEENB |
	     M_MACM_RXSTATSENB | M_MACM_RXSTATSCLR |
	     M_MACM_TXSTATSENB | M_MACM_TXSTATSCLR);

#if T3_AUTOPOLL
    mode |= V_MACM_PORTMODE(K_MACM_PORTMODE_MII);
#endif

    WRITECSR(sc, R_MAC_MODE, mode);

#if T3_AUTOPOLL
    WRITECSR(sc, R_MISC_LOCAL_CTRL, M_MLCTL_INTATTN);
#endif

    /* Configure GPIOs (74) - skipped */

    /* Clear interrupt mailbox (75) */
    WRITEMBOX(sc, R_INT_MBOX(0), 0);

    /* Enable DMA completion block (76) */
    if ( (sc->device != K_PCI_ID_BCM5705) 
        && !BCM571X_FAMILY_DEVICE(sc->device) )
    {
        WRITECSR(sc, R_DMA_COMP_MODE, M_MODE_ENABLE);
    }

    /* Configure write and read DMA modes (77, 78) */
    WRITECSR(sc, R_WR_DMA_MODE, M_MODE_ENABLE | M_ATTN_ALL);
    WRITECSR(sc, R_RD_DMA_MODE, M_MODE_ENABLE | M_ATTN_ALL);

#if 0   
    mode = M_MODE_ENABLE | M_ATTN_ALL;
    if ( sc->device == K_PCI_ID_BCM5705) 
    { 
      	mode |= RD_DMA_MODE_FIFO_SIZE_128;
    }
    else if (  BCM571X_FAMILY_DEVICE(sc->device) )
    {
        /*
         * XXX: magic values.
         * From Broadcom-supplied Linux driver;  apparently
         * required to workaround a DMA bug affecting TSO
         * on bcm575x/bcm5721?
         */
        mode |= (1 << 27);
    }

    WRITECSR(sc, R_RD_DMA_MODE, mode );
#endif

    return 0;
}

static int
t3_init_enable(t3_ether_t *sc)
{
    uint32_t mhc;
    uint32_t pmcs;
#if T3_AUTOPOLL
    uint32_t mode, mask;
#else
    int  i;
#endif

    /* Steps 79-97 */

    /* Enable completion functional blocks (79-82) */
    WRITECSR(sc, R_RCV_COMP_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);
    if ( (sc->device != K_PCI_ID_BCM5705) &&
         !BCM571X_FAMILY_DEVICE(sc->device) ) {

        WRITECSR(sc, R_MBUF_FREE_MODE, M_MODE_ENABLE);

	}

    if ( BCM571X_FAMILY_DEVICE(sc->device) )
    {
        WRITECSR(sc, R_SND_DATA_COMP_MODE, M_MODE_ENABLE | 0x8);
    }
    else
    {
        WRITECSR(sc, R_SND_DATA_COMP_MODE, M_MODE_ENABLE);
    }

    WRITECSR(sc, R_SND_BD_COMP_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);

    /* Enable initiator functional blocks (83-86) */
    WRITECSR(sc, R_RCV_BD_INIT_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);
    WRITECSR(sc, R_RCV_DATA_INIT_MODE, M_MODE_ENABLE | M_RCVINITMODE_RTNSIZE);
    WRITECSR(sc, R_SND_DATA_MODE, M_MODE_ENABLE);
    WRITECSR(sc, R_SND_BD_INIT_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);

    /* Enable the send BD selector (87) */
    WRITECSR(sc, R_SND_BD_SEL_MODE, M_MODE_ENABLE | M_MODE_ATTNENABLE);

    /* Download firmware (88) - skipped */

    /* Enable the MAC (89,90) */
    WRITECSR(sc, R_TX_MODE, M_MODE_ENABLE);   /* optional flow control */
    WRITECSR(sc, R_RX_MODE, M_MODE_ENABLE);   /* other options */

    /* Disable auto-polling (91) */
    mii_access_init(sc);

    /* Configure power state (92) */
    pmcs = READCSR(sc, PCI_PMCSR_REG);
    pmcs &= ~PCI_PMCSR_STATE_MASK;
    pmcs |= PCI_PMCSR_STATE_D0;
    WRITECSR(sc, PCI_PMCSR_REG, pmcs);

#if T3_AUTOPOLL
    /* Program hardware LED control (93) */
    WRITECSR(sc, R_MAC_LED_CTRL, 0x00);   /* LEDs at PHY layer */
#endif

#if T3_AUTOPOLL
    /* Ack/clear link change events */
    WRITECSR(sc, R_MAC_STATUS, M_LINKCHNG_CLR);
    WRITECSR(sc, R_MI_STATUS, 0);

    /* Enable autopolling */
    mode = READCSR(sc, R_MI_MODE);
    mode |= M_MIMODE_POLLING | 0x000c000;
    WRITECSR(sc, R_MI_MODE, mode);

    /* Enable link state attentions */
    mask = READCSR(sc, R_MAC_EVENT_ENB);
    mask |= M_EVT_LINKCHNG;
    WRITECSR(sc, R_MAC_EVENT_ENB, mask);
#else
    /* Initialize link (94) */
    WRITECSR(sc, R_MI_STATUS, M_MISTAT_LINKED);

    /* Start autonegotiation (95) - see t3_initlink below */

    /* Setup multicast filters (96) */
    for (i = 0; i < 4; i++)
	WRITECSR(sc, R_MAC_HASH(i), 0);
#endif /* T3_AUTOPOLL */

    /* Enable interrupts (97) */
    mhc = READCSR(sc, R_MISC_HOST_CTRL);
    mhc &= ~M_MHC_MASKPCIINT;
    WRITECSR(sc, R_MISC_HOST_CTRL, mhc);

    return 0;
}


static void
t3_initlink(t3_ether_t *sc)
{
    uint32_t mcr;

    if (mii_probe(sc) != 0) {
	xprintf("%s: no PHY found\n", t3_devname(sc));
	return;  
	}
#if T3_DEBUG
    xprintf("%s: PHY addr %d\n", t3_devname(sc), sc->phy_addr);
#endif
    if (1)   /* XXX Support only autonegotiation for now */
	mii_autonegotiate(sc);
    else
	mii_set_speed(sc, ETHER_SPEED_100HDX);

    mii_enable_interrupts(sc);

    mcr = READCSR(sc, R_MODE_CTRL);
    mcr |= M_MCTL_MACINT;
    WRITECSR(sc, R_MODE_CTRL, mcr);

    sc->mii_polling = 0;
    sc->phy_change = 0;
}

static void
t3_shutdownlink(t3_ether_t *sc)
{
    uint32_t mcr;

    mcr = READCSR(sc, R_MODE_CTRL);
    mcr &= ~M_MCTL_MACINT;
    WRITECSR(sc, R_MODE_CTRL, mcr);

    WRITECSR(sc, R_MAC_EVENT_ENB, 0);

    /* The manual is fuzzy about what to do with the PHY at this
       point.  Empirically, resetting the 5705 PHY (but not others)
       will cause it to get stuck in 10/100 MII mode.  */
    if (sc->device != K_PCI_ID_BCM5705)
	mii_write_register(sc, sc->phy_addr, MII_BMCR, BMCR_RESET);

    sc->mii_polling = 0;
    sc->phy_change = 0;
}


static void
t3_hwinit(t3_ether_t *sc)
{
    if (sc->state != eth_state_on) {

	if (sc->state == eth_state_uninit) {
	    WRITECSR(sc, R_MEMWIN_BASE_ADDR, 0);   /* Default memory window */
	    t3_coldreset(sc);
	    }
	else
	    t3_warmreset(sc);

	t3_init_registers(sc);
	t3_init_pools(sc);
	t3_init_rings(sc);
	t3_configure_mac(sc);
	t3_enable_stats(sc);
	t3_init_coalescing(sc);
	t3_init_dma(sc);
	t3_init_enable(sc);
 
    if ( 1 )
    {


#if T3_DEBUG
	dumpcsrs(sc, "end init");
#else
	(void)dumpcsrs;
#endif

	eeprom_access_init(sc);
#if T3_DEBUG
	{
	    uint32_t eeprom[0x100/4];
	    int i;
	    
	    cfe_sleep(1);
	    /* XXX Apparently a few reads can be required to get the
               AutoAccess logic into a good state. ??? */
	    for (i = 0; i < 4; i++) {
		eeprom_read_range(sc, 0, 4, eeprom);
		}

	    eeprom_read_range(sc, 0, sizeof(eeprom), eeprom);
	    eeprom_dump_range("Boot Strap", eeprom, 0x00, 20);
	    eeprom_dump_range("Manufacturing Info", eeprom, 0x74, 140);
	}
#else
	(void)eeprom_read_range;
	(void)eeprom_dump_range;
#endif


	t3_initlink(sc);

    }

	sc->state = eth_state_off;
	}
}

static void
t3_hwshutdown(t3_ether_t *sc)
{
    /* Receive path shutdown */
    t3_clear(sc, R_RX_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_RCV_BD_INIT_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_RCV_LIST_MODE, M_MODE_ENABLE);
    if ( (sc->device != K_PCI_ID_BCM5705) &&
         !BCM571X_FAMILY_DEVICE(sc->device) )
    {
	t3_clear(sc, R_RCV_LIST_SEL_MODE, M_MODE_ENABLE);
	}
    t3_clear(sc, R_RCV_DATA_INIT_MODE, M_MODE_ENABLE);
#ifndef BCM47XX  /* XXX bus error on 5705 */
    t3_clear(sc, R_RCV_COMP_MODE, M_MODE_ENABLE);
#endif
    t3_clear(sc, R_RCV_BD_COMP_MODE, M_MODE_ENABLE);

    /* Transmit path shutdown */
    t3_clear(sc, R_SND_BD_SEL_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_SND_BD_INIT_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_SND_DATA_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_RD_DMA_MODE, M_MODE_ENABLE);
#ifndef BCM47XX  /* XXX bus error on 5703 */
    t3_clear(sc, R_SND_DATA_COMP_MODE, M_MODE_ENABLE);
#endif
    if ( (sc->device != K_PCI_ID_BCM5705) 
        && !BCM571X_FAMILY_DEVICE(sc->device))
    {
#ifndef BCM47XX  /* XXX bus error on 5703 */
        t3_clear(sc, R_DMA_COMP_MODE, M_MODE_ENABLE);
#endif
	}

    t3_clear(sc, R_SND_BD_COMP_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_TX_MODE, M_MODE_ENABLE);

    /* Memory shutdown */
    t3_clear(sc, R_HOST_COAL_MODE, M_HCM_ENABLE);
    t3_clear(sc, R_WR_DMA_MODE, M_MODE_ENABLE);
    if (sc->device != K_PCI_ID_BCM5705) {
	t3_clear(sc, R_MBUF_FREE_MODE, M_MODE_ENABLE);
	}
    WRITECSR(sc, R_FTQ_RESET, 0xFFFFFFFF);
    cfe_sleep(1);
    WRITECSR(sc, R_FTQ_RESET, 0x00000000);
    t3_clear(sc, R_BMGR_MODE, M_BMODE_ENABLE);
    t3_clear(sc, R_MEM_MODE, M_MAM_ENABLE);

    t3_shutdownlink(sc);

    WRITECSR(sc, R_MEMWIN_BASE_ADDR, 0);   /* Default memory window */
    t3_coldreset(sc);

    sc->state = eth_state_uninit;
}


static void
t3_isr(void *arg)
{
    t3_ether_t *sc = (t3_ether_t *)arg;
    volatile t3_status_t *status = sc->status;
    uint32_t mac_status;
    int handled;

    do { 
	WRITEMBOX(sc, R_INT_MBOX(0), 1);

	handled = 0;
	mac_status = READCSR(sc, R_MAC_STATUS);  /* force ordering */
	status->status &= ~M_STATUS_UPDATED;
    
	if (status->index[RI(1)].return_p != sc->rxr_1_index) {
	    handled = 1;
	    if (IPOLL) sc->rx_interrupts++;  
	    t3_procrxring(sc);
	    }

	if (status->index[RI(1)].send_c != sc->txc_1_index) {
	    handled = 1;
    if (IPOLL) sc->tx_interrupts++;  
	    t3_proctxring(sc);
	    }

	if ((mac_status & M_EVT_LINKCHNG) != 0) {
	    handled = 1;
#if T3_AUTOPOLL
	    WRITECSR(sc, R_MAC_STATUS, M_LINKCHNG_CLR);
#endif
	    WRITECSR(sc, R_MAC_STATUS, M_EVT_MICOMPLETE);

	    status->status &= ~M_STATUS_LINKCHNG;
	    sc->phy_change = 1;
	    }

	WRITEMBOX(sc, R_INT_MBOX(0), 0);
	(void)READMBOX(sc, R_INT_MBOX(0));  /* push */

#if (!XPOLL)
	if (!handled)
	    sc->bogus_interrupts++;
#endif

	} while ((status->status & M_STATUS_UPDATED) != 0);

    if (sc->rxp_std_index != sc->prev_rxp_std_index) {
	sc->prev_rxp_std_index = sc->rxp_std_index;
	WRITEMBOX(sc, R_RCV_BD_STD_PI, sc->rxp_std_index);
	}
}


static void
t3_setaddr(t3_ether_t *sc, uint8_t *addr)
{
    uint32_t rx_mode, tx_mode;
    uint32_t low, high;

    /* MAC must be disabled */
    rx_mode = READCSR(sc, R_RX_MODE);
    tx_mode = READCSR(sc, R_TX_MODE);
    t3_clear(sc, R_RX_MODE, M_MODE_ENABLE);
    t3_clear(sc, R_TX_MODE, M_MODE_ENABLE);

    high = (addr[0] << 8) | addr[1];
    low = ((addr[2] << 24) | (addr[3] << 16) | (addr[4] << 8) | addr[5]);
    WRITECSR(sc, R_MAC_ADDR1_HIGH, high);  WRITECSR(sc, R_MAC_ADDR1_LOW, low);
    WRITECSR(sc, R_MAC_ADDR2_HIGH, high);  WRITECSR(sc, R_MAC_ADDR2_LOW, low);
    WRITECSR(sc, R_MAC_ADDR3_HIGH, high);  WRITECSR(sc, R_MAC_ADDR3_LOW, low);
    WRITECSR(sc, R_MAC_ADDR4_HIGH, high);  WRITECSR(sc, R_MAC_ADDR4_LOW, low);

    WRITECSR(sc, R_TX_MODE, tx_mode);
    WRITECSR(sc, R_RX_MODE, rx_mode);
}


static void
t3_clear_stats(t3_ether_t *sc)
{
#ifndef BCM47XX
    WRITEMBOX(sc, R_RELOAD_STATS_MBOX + 4, 0);
    WRITEMBOX(sc, R_RELOAD_STATS_MBOX, PTR_TO_PCI(zero_stats));
#endif
}


static void
t3_start(t3_ether_t *sc)
{
    t3_hwinit(sc);

    sc->intmask = 0;

#if IPOLL
    cfe_request_irq(sc->irq, t3_isr, sc, CFE_IRQ_FLAGS_SHARED, 0);

#if T3_AUTOPOLL
    sc->intmask |= M_EVT_LINKCHNG;
#else
    sc->intmask |= M_EVT_LINKCHNG | M_EVT_MIINT; 
#endif
    WRITECSR(sc, R_MAC_EVENT_ENB, sc->intmask);
#endif

    /* Post some Rcv Producer buffers */
    sc->prev_rxp_std_index = sc->rxp_std_index;
    WRITEMBOX(sc, R_RCV_BD_STD_PI, sc->rxp_std_index);

    sc->state = eth_state_on;
}

static void
t3_stop(t3_ether_t *sc)
{
    WRITECSR(sc, R_MAC_EVENT_ENB, 0);
    sc->intmask = 0;
#if IPOLL
    cfe_free_irq(sc->irq, 0);
#endif

    if (sc->state == eth_state_on) {
	sc->state = eth_state_off;
	t3_hwshutdown(sc);
	t3_reinit(sc);
	}
}


static int t3_ether_open(cfe_devctx_t *ctx);
static int t3_ether_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int t3_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int t3_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int t3_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int t3_ether_close(cfe_devctx_t *ctx);
static void t3_ether_poll(cfe_devctx_t *ctx, int64_t ticks);
static void t3_ether_reset(void *softc);

const static cfe_devdisp_t t3_ether_dispatch = {
    t3_ether_open,
    t3_ether_read,
    t3_ether_inpstat,
    t3_ether_write,
    t3_ether_ioctl,
    t3_ether_close,	
    t3_ether_poll,
    t3_ether_reset
};

cfe_driver_t bcm5700drv = {
    "BCM570x Ethernet",
    "eth",
    CFE_DEV_NETWORK,
    &t3_ether_dispatch,
    t3_ether_probe
};


static void
t3_delete_sc(t3_ether_t *sc)
{
    xprintf("BCM570x attach: No memory to complete probe\n");
    if (sc != NULL) {
	if (sc->txp_1 != NULL)
	    KFREE(sc->txp_1);
	if (sc->rxr_1 != NULL)
	    KFREE(sc->rxr_1);
	if (sc->rxp_std != NULL)
	    KFREE(sc->rxp_std);
	if (sc->stats != NULL)
	    KFREE((t3_stats_t *)sc->stats);
	if (sc->status != NULL)
	    KFREE((t3_ether_t *)CACHE_DMA_CACHEABLE(sc->status));
	KFREE(sc);
	}
}

static const uint8_t null_addr[ENET_ADDR_LEN]  = {0x00,0x00,0x00,0x00,0x00};
static const uint8_t bcast_addr[ENET_ADDR_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF};

static int
t3_ether_attach(cfe_driver_t *drv, pcitag_t tag, uint8_t hwaddr[])
{
    t3_ether_t *sc;
    char descr[80];
    phys_addr_t pa;
    uint32_t base;
    uint32_t pcictrl;
    uint32_t addr;
    pcireg_t device, class;
    const char *devname;
    int i;

    if (zero_stats == NULL)
	zero_stats = (t3_stats_t *) dma_alloc(sizeof(t3_stats_t), CACHE_ALIGN);
    if (zero_stats == NULL)
	return 0;
    memset(zero_stats, 0, sizeof(t3_stats_t));
    CACHE_DMA_SYNC(zero_stats, sizeof(t3_stats_t));

    pci_map_mem(tag, PCI_MAPREG(0), CSR_MATCH_MODE, &pa);
    base = (uint32_t)pa;

    sc = (t3_ether_t *) KMALLOC(sizeof(t3_ether_t), 0);
    if (sc == NULL) {
	t3_delete_sc(sc);
	return 0;
	}

    memset(sc, 0, sizeof(*sc));
    
    sc->status = NULL;
    sc->stats = NULL;

    device = pci_conf_read(tag, PCI_ID_REG);
    class = pci_conf_read(tag, PCI_CLASS_REG);
    sc->tag = tag;
    sc->device = PCI_PRODUCT(device);
    sc->revision = PCI_REVISION(class);

    /* (Some?) 5700s report the 5701 device code */
    sc->asic_revision = G_MHC_ASICREV(pci_conf_read(tag, R_MISC_HOST_CTRL));
    if (( sc->device == K_PCI_ID_BCM5701) 
        && (sc->asic_revision & 0xF000) == 0x7000 )
	sc->device = K_PCI_ID_BCM5700;

    sc->status = 
	(t3_status_t *) CACHE_DMA_SHARED(dma_alloc(sizeof(t3_status_t), CACHE_ALIGN));
    if (sc->status == NULL) {
	t3_delete_sc(sc);
	return 0;
	}

    sc->stats = (t3_stats_t *) dma_alloc(sizeof(t3_stats_t), CACHE_ALIGN);
    if (sc->stats == NULL) {
	t3_delete_sc(sc);
	return 0;
	}

    if (sc->device == K_PCI_ID_BCM5705)
    {
        sc->rxr_entries = RXR_RING_ENTRIES_05;
    }
    else if ( BCM571X_FAMILY_DEVICE(sc->device) )
    {
        sc->rxr_entries = RXR_RING_ENTRIES_BCM571X_FAMILY;
    }
    else
    {
        sc->rxr_entries = RXR_RING_ENTRIES;
    }

    sc->rxp_std =
        (t3_rcv_bd_t *) dma_alloc(RXP_STD_ENTRIES*RCV_BD_SIZE, CACHE_ALIGN);
    sc->rxr_1 =
        (t3_rcv_bd_t *) dma_alloc(sc->rxr_entries*RCV_BD_SIZE, CACHE_ALIGN);
    sc->txp_1 =
        (t3_snd_bd_t *) dma_alloc(TXP_RING_ENTRIES*SND_BD_SIZE, CACHE_ALIGN);
    if (sc->rxp_std == NULL || sc->rxr_1 == NULL || sc->txp_1 == NULL) {
	t3_delete_sc(sc);
	return 0;
	}

    sc->regbase = base;

    /* NB: the relative base of memory depends on the access model */
    pcictrl = pci_conf_read(tag, R_PCI_STATE);
    sc->membase = base + 0x8000;       /* Normal mode: 32K window */

    sc->irq = pci_conf_read(tag, PCI_BPARAM_INTERRUPT_REG) & 0xFF;

    sc->devctx = NULL;

    /* Assume on-chip firmware has initialized the MAC address. */
    addr = READCSR(sc, R_MAC_ADDR1_HIGH);
    for (i = 0; i < 2; i++)
	sc->hwaddr[i] = (addr >> (8*(1-i))) & 0xff;
    addr = READCSR(sc, R_MAC_ADDR1_LOW);
    for (i = 0; i < 4; i++)
	sc->hwaddr[2+i] = (addr >> (8*(3-i))) & 0xff;
    if (memcmp(sc->hwaddr, null_addr, ENET_ADDR_LEN) == 0
	|| memcmp(sc->hwaddr, bcast_addr, ENET_ADDR_LEN) == 0)
	memcpy(sc->hwaddr, hwaddr, ENET_ADDR_LEN);

    t3_init(sc);

    sc->state = eth_state_uninit;

    switch (sc->device) {
    case K_PCI_ID_BCM5700:
	devname = "BCM5700"; break;
    case K_PCI_ID_BCM5701:
	devname = "BCM5701"; break;
    case K_PCI_ID_BCM5702:
	devname = "BCM5702"; break;
    case K_PCI_ID_BCM5703:
    case K_PCI_ID_BCM5703a:
    case K_PCI_ID_BCM5703b:
	devname = "BCM5703"; break;
    case K_PCI_ID_BCM5704C:
	devname = "BCM5704C"; break;
    case K_PCI_ID_BCM5705:
	devname = "BCM5705"; break;
    case K_PCI_ID_BCM5780:
	devname = "BCM5780"; break;
    default:
	devname = "BCM570x"; break;
	}
    xsprintf(descr, "%s Ethernet at 0x%X (%a)",
	     devname, sc->regbase, sc->hwaddr);

    cfe_attach(drv, sc, NULL, descr);
    return 1;
}

static void
t3_ether_probe(cfe_driver_t *drv,
	       unsigned long probe_a, unsigned long probe_b, 
	       void *probe_ptr)
{
    int index;
    uint8_t hwaddr[ENET_ADDR_LEN];

    if (probe_ptr)
	enet_parse_hwaddr((char *)probe_ptr, hwaddr);
    else {
	/* Use default address 02-10-18-11-22-33 */
	hwaddr[0] = 0x02;  hwaddr[1] = 0x10;  hwaddr[2] = 0x18;
	hwaddr[3] = 0x11;  hwaddr[4] = 0x22;  hwaddr[5] = 0x33;
	}

    index = 0;
    for (;;) {
	pcitag_t tag;
	pcireg_t device;

	if (pci_find_class(PCI_CLASS_NETWORK, index, &tag) != 0)
	   break;

	index++;

	device = pci_conf_read(tag, PCI_ID_REG);
	if (PCI_VENDOR(device) == K_PCI_VENDOR_BROADCOM) {
	    switch (PCI_PRODUCT(device)) {
		case K_PCI_ID_BCM5700:
		case K_PCI_ID_BCM5701:
		case K_PCI_ID_BCM5702:
		case K_PCI_ID_BCM5703:
		case K_PCI_ID_BCM5703a:
		case K_PCI_ID_BCM5703b:
		case K_PCI_ID_BCM5704C:
		case K_PCI_ID_BCM5705:
        case K_PCI_ID_BCM5780:
		    t3_ether_attach(drv, tag, hwaddr);
		    enet_incr_hwaddr(hwaddr, 1);
		    break;
		default:
		    break;
		}
	    }
	}
}


/* The functions below are called via the dispatch vector for the Tigon 3 */

static int
t3_ether_open(cfe_devctx_t *ctx)
{
    t3_ether_t *sc = ctx->dev_softc;
    volatile t3_stats_t *stats = sc->stats;
    int i;

    if (sc->state == eth_state_on)
	t3_stop(sc);

    sc->devctx = ctx;

    for (i = 0; i < L_MAC_STATS/sizeof(uint64_t); i++)
	stats->stats[i] = 0;
    CACHE_DMA_SYNC(stats, sizeof(t3_stats_t));

    t3_start(sc);

    sc->rx_interrupts = sc->tx_interrupts = sc->bogus_interrupts = 0;
    t3_clear_stats(sc);

    if (XPOLL) t3_isr(sc);
    return 0;
}

static int
t3_ether_read(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    t3_ether_t *sc = ctx->dev_softc;
    eth_pkt_t *pkt;
    int blen;

    if (XPOLL) t3_isr(sc);

    if (sc->state != eth_state_on) return -1;

    CS_ENTER(sc);
    pkt = (eth_pkt_t *) q_deqnext(&(sc->rxqueue));
    CS_EXIT(sc);

    if (pkt == NULL) {
	buffer->buf_retlen = 0;
	return 0;
	}

    blen = buffer->buf_length;
    if (blen > pkt->length) blen = pkt->length;

    CACHE_DMA_INVAL(pkt->buffer, blen);
    hs_memcpy_to_hs(buffer->buf_ptr, pkt->buffer, blen);
    buffer->buf_retlen = blen;

    eth_free_pkt(sc, pkt);

    if (XPOLL) t3_isr(sc);
    return 0;
}

static int
t3_ether_inpstat(cfe_devctx_t *ctx, iocb_inpstat_t *inpstat)
{
    t3_ether_t *sc = ctx->dev_softc;

    if (XPOLL) t3_isr(sc);

    if (sc->state != eth_state_on) return -1;

    /* We avoid an interlock here because the result is a hint and an
       interrupt cannot turn a non-empty queue into an empty one. */
    inpstat->inp_status = (q_isempty(&(sc->rxqueue))) ? 0 : 1;

    return 0;
}

static int
t3_ether_write(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    t3_ether_t *sc = ctx->dev_softc;
    eth_pkt_t *pkt;
    int blen;

    if (XPOLL) t3_isr(sc);

    if (sc->state != eth_state_on) return -1;

    pkt = eth_alloc_pkt(sc);
    if (!pkt) return CFE_ERR_NOMEM;

    blen = buffer->buf_length;
    if (blen > pkt->length) blen = pkt->length;

    hs_memcpy_from_hs(pkt->buffer, buffer->buf_ptr, blen);
    pkt->length = blen;
    CACHE_DMA_SYNC(pkt->buffer, blen);

    if (t3_transmit(sc, pkt) != 0) {
	eth_free_pkt(sc,pkt);
	return CFE_ERR_IOERR;
	}

    if (XPOLL) t3_isr(sc);
    return 0;
}

static int
t3_ether_ioctl(cfe_devctx_t *ctx, iocb_buffer_t *buffer) 
{
    t3_ether_t *sc = ctx->dev_softc;
    int speed;

    switch ((int)buffer->buf_ioctlcmd) {
	case IOCTL_ETHER_GETHWADDR:
	    hs_memcpy_to_hs(buffer->buf_ptr, sc->hwaddr, sizeof(sc->hwaddr));
	    return 0;

	case IOCTL_ETHER_SETHWADDR:
	    hs_memcpy_from_hs(sc->hwaddr, buffer->buf_ptr, sizeof(sc->hwaddr));
	    t3_setaddr(sc, sc->hwaddr);
	    return -1;

	case IOCTL_ETHER_GETSPEED:
	    speed = sc->linkspeed;
	    hs_memcpy_to_hs(buffer->buf_ptr,&speed,sizeof(int));
	    return 0;

	default:
	    return -1;
	}
}

static int
t3_ether_close(cfe_devctx_t *ctx)
{
    t3_ether_t *sc = ctx->dev_softc;
    volatile t3_stats_t *stats = sc->stats;
    uint32_t inpkts, outpkts, interrupts;
    int i;

    t3_stop(sc);

    CACHE_DMA_INVAL(stats, sizeof(t3_stats_t));
#if T3_BRINGUP
    for (i = 0; i < L_MAC_STATS/sizeof(uint64_t); i++) {
	uint64_t count = ctoh64(stats->stats[i]);

	if (count != 0)
	    xprintf(" stats[%d] = %8lld\n", i, count);
	}
#else
    (void) i;
#endif

    inpkts = ctoh64(stats->stats[ifHCInUcastPkts])
	      + ctoh64(stats->stats[ifHCInMulticastPkts])
	      + ctoh64(stats->stats[ifHCInBroadcastPkts]);
    outpkts = ctoh64(stats->stats[ifHCOutUcastPkts])
	      + ctoh64(stats->stats[ifHCOutMulticastPkts])
	      + ctoh64(stats->stats[ifHCOutBroadcastPkts]);
    interrupts = ctoh64(stats->stats[nicInterrupts]);

    /* Empirically, counters on the 5705 are always zero.  */
    if (sc->device != K_PCI_ID_BCM5705) {
	xprintf("%s: %d sent, %d received, %d interrupts\n",
		t3_devname(sc), outpkts, inpkts, interrupts);
	if (IPOLL) {
	    xprintf("  %d rx interrupts, %d tx interrupts",
		    sc->rx_interrupts, sc->tx_interrupts);
	    if (sc->bogus_interrupts != 0)
	        xprintf(", %d bogus interrupts", sc->bogus_interrupts);
	    xprintf("\n");
	    }
	}

    sc->devctx = NULL;
    return 0;
}

static void
t3_ether_poll(cfe_devctx_t *ctx, int64_t ticks)
{
    t3_ether_t *sc = ctx->dev_softc;
    int changed;

    if (sc->phy_change && sc->state != eth_state_uninit && !sc->mii_polling) {
	uint32_t mask;

	sc->mii_polling++;
	mask = READCSR(sc, R_MAC_EVENT_ENB);
	WRITECSR(sc, R_MAC_EVENT_ENB, 0);

	changed = mii_poll(sc);
	if (changed) {
	    mii_autonegotiate(sc);
	    }
	sc->phy_change = 0;
	sc->mii_polling--;

	WRITECSR(sc, R_MAC_EVENT_ENB, mask);
	}
}

static void
t3_ether_reset(void *softc)
{
    t3_ether_t *sc = (t3_ether_t *)softc;

    /* Turn off the Ethernet interface. */

    if (sc->state == eth_state_on)
	t3_stop(sc);

    sc->state = eth_state_uninit;
}


uint32_t l_phys_read32(uint32_t addr )
{
    //printf("rd:%08X\n", addr);
    return( phys_read32( addr ) );
} 

void l_phys_write32(uint32_t addr, uint32_t val)
{
    //printf("wr:%08X %08X\n ", addr, val);
    phys_write32( addr, val );
}


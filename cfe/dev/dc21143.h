/*
 * Register and bit definitions for the DEC/Intel 21143 Ethernet controller,
 * part of the Tulip family of 10 and 10/100 controllers.
 * Reference:
 *   21143 PCI/CardBus 10/100 Mb/s Ethernet LAN Controller,
 *     Hardware Reference Manual, Revision 1.0.
 *   Document No. 278074-001
 *   Intel Corp., October 1998
 * Includes extensions/alternatives for the DEC 21040, 21041 and 21140(A)
 * Ethernet controllers.
 */
#ifndef _DC21143_H_
#define _DC21143_H_

#define _DD_MAKEMASK1(n) (1 << (n))
#define _DD_MAKEMASK(v,n) ((((1)<<(v))-1) << (n))
#define _DD_MAKEVALUE(v,n) ((v) << (n))
#define _DD_GETVALUE(v,n,m) (((v) & (m)) >> (n))


/*  *********************************************************************
    *  PCI Configuration Register offsets (Tulip nomenclature)
    ********************************************************************* */

#define R_CFG_CFID       PCI_ID_REG
#define K_PCI_VENDOR_DEC 0x1011
#define K_PCI_ID_DC21040 0x0002
#define K_PCI_ID_DC21041 0x0014
#define K_PCI_ID_DC21140 0x0009
#define K_PCI_ID_DC21143 0x0019

#define R_CFG_CFRV       PCI_CLASS_REG

#define R_CFG_CBIO       PCI_MAPREG(0)
#define R_CFG_CBMA       PCI_MAPREG(1)

#define R_CFG_CFIT       PCI_BPARAM_INTERRUPT_REG

/* Tulip extensions */
#define R_CFG_CFDD       0x40

#define M_CFDD_SLEEP     __DD_MAKEMASK1(31)
#define M_CFDD_SNOOZE    __DD_MAKEMASK1(30)

#define R_CFG_CPMS       0xE0


/*  *********************************************************************
    *  CSRs: offsets
    ********************************************************************* */

#define R_CSR_BUSMODE	      0x00
#define R_CSR_TXPOLL          0x08
#define R_CSR_RXPOLL          0x10
#define R_CSR_RXRING          0x18
#define R_CSR_TXRING          0x20
#define R_CSR_STATUS          0x28
#define R_CSR_OPMODE          0x30
#define R_CSR_INTMASK         0x38  
#define R_CSR_MISSEDFRAME     0x40
#define R_CSR_ROM_MII         0x48
#define R_CSR_BOOTROM_ADDR    0x50
#define R_CSR_GENTIMER        0x58

/* The following registers are specific to the 21040 */

#define R_CSR_FDUPLEX         0x58
   
/* The following registers are specific to the 21040/21041 and 21142/21143 */

#define R_CSR_SIASTATUS       0x60
#define R_CSR_SIAMODE0        0x68
#define R_CSR_SIAMODE1        0x70
#define R_CSR_SIAMODE2        0x78

/* The following registers are specific to the 21140/21140A */

#define R_CSR_GENPORT         0x60
#define R_CSR_WATCHDOG_TIMER  0x78


/* CSR0:  Bus Mode register */

#define M_CSR0_SWRESET          _DD_MAKEMASK1(0)
#define M_CSR0_BUSARB         	_DD_MAKEMASK1(1)

#define S_CSR0_SKIPLEN		2
#define M_CSR0_SKIPLEN          _DD_MAKEMASK(5,S_CSR0_SKIPLEN)
#define V_CSR0_SKIPLEN(x)       _DD_MAKEVALUE(x,S_CSR0_SKIPLEN)
#define G_CSR0_SKIPLEN(x)       _DD_GETVALUE(x,S_CSR0_SKIPLEN,M_CSR0_SKIPLEN)

#define M_CSR0_BIGENDIAN        _DD_MAKEMASK1(7)

#define S_CSR0_BURSTLEN		8
#define M_CSR0_BURSTLEN         _DD_MAKEMASK(6,S_CSR0_BURSTLEN)
#define V_CSR0_BURSTLEN(x)      _DD_MAKEVALUE(x,S_CSR0_BURSTLEN)
#define G_CSR0_BURSTLEN(x)      _DD_GETVALUE(x,S_CSR0_BURSTLEN,M_CSR0_BURSTLEN)

#define S_CSR0_CACHEALIGN	14
#define M_CSR0_CACHEALIGN       _DD_MAKEMASK(2,S_CSR0_CACHEALIGN)
#define V_CSR0_CACHEALIGN(x)    _DD_MAKEVALUE(x,S_CSR0_CACHEALIGN)
#define G_CSR0_CACHEALIGN(x)    _DD_GETVALUE(x,S_CSR0_CACHEALIGN,M_CSR0_CACHEALIGN)

#define S_CSR0_TXAUTOPOLL	17
#define M_CSR0_TXAUTOPOLL	_DD_MAKEMASK(3,S_CSR0_AUTOPOLL)
#define V_CSR0_TXAUTOPOLL(x)    _DD_MAKEVALUE(x,S_CSR0_TXAUTOPOLL)
#define G_CSR0_TXAUTOPOLL(x)    _DD_GETVALUE(x,S_CSR0_TXAUTOPOLL,M_CSR0_TXAUTOPOLL)

#define M_CSR0_DESCBYTEORDER	_DD_MAKEMASK1(20)  /* not 21040 */
#define M_CSR0_READMULTENAB	_DD_MAKEMASK1(21)  /* not 2104{0,1} */
#define M_CSR0_READLINEENAB	_DD_MAKEMASK1(23)  /* not 2104{0,1} */
#define M_CSR0_WRITEINVALENAB	_DD_MAKEMASK1(24)  /* not 2104{0,1} */

#define K_CSR0_TAPDISABLED	0x00
#define K_CSR0_TAP200US		0x01
#define K_CSR0_TAP800US		0x02
#define K_CSR0_TAP1600US	0x03

#define K_CSR0_ALIGNNONE	0
#define K_CSR0_ALIGN32          1
#define K_CSR0_ALIGN64          2
#define K_CSR0_ALIGN128         3

#define K_CSR0_BURST32          32
#define K_CSR0_BURST16          16
#define K_CSR0_BURST8           8
#define K_CSR0_BURST4           4
#define K_CSR0_BURST2           2
#define K_CSR0_BURST1           1
#define K_CSR0_BURSTANY         0


#define M_CSR3_RXDSCRADDR	0xFFFFFFFC
#define M_CSR4_TXDSCRADDR	0xFFFFFFFC


/* CSR5:  Status register */

#define M_CSR5_TXINT            _DD_MAKEMASK1(0)
#define M_CSR5_TXSTOP           _DD_MAKEMASK1(1)
#define M_CSR5_TXBUFUNAVAIL   	_DD_MAKEMASK1(2)
#define M_CSR5_TXJABTIMEOUT     _DD_MAKEMASK1(3)
#define M_CSR5_LINKPASS		_DD_MAKEMASK1(4)  /* not 21040 */
#define M_CSR5_TXUNDERFLOW      _DD_MAKEMASK1(5)
#define M_CSR5_RXINT            _DD_MAKEMASK1(6)
#define M_CSR5_RXBUFUNAVAIL   	_DD_MAKEMASK1(7)
#define M_CSR5_RXSTOPPED        _DD_MAKEMASK1(8)
#define M_CSR5_RXWDOGTIMEOUT    _DD_MAKEMASK1(9)
#define M_CSR5_AUITPPIN         _DD_MAKEMASK1(10)  /* 21040 only */
#define M_CSR5_TXEARLYINT	_DD_MAKEMASK1(10)  /* not 2104{0,1} */
#define M_CSR5_FDSHORTFRAME     _DD_MAKEMASK1(11)  /* 21040 only */
#define M_CSR5_GPTIMEREXPIRE	_DD_MAKEMASK1(11)  /* not 21040 */
#define M_CSR5_LINKFAIL         _DD_MAKEMASK1(12)
#define M_CSR5_FATALBUSERROR    _DD_MAKEMASK1(13)
#define M_CSR5_RXEARLYINT	_DD_MAKEMASK1(14)  /* not 21040 */
#define M_CSR5_ABNORMALINT      _DD_MAKEMASK1(15)
#define M_CSR5_NORMALINT        _DD_MAKEMASK1(16)

#define S_CSR5_RXPROCSTATE	17
#define M_CSR5_RXPROCSTATE      _DD_MAKEMASK(3,S_CSR5_RXPROCSTATE)
#define V_CSR5_RXPROCSTATE(x)   _DD_MAKEVALUE(x,S_CSR5_RXPROCSTATE)
#define G_CSR5_RXPROCSTATE(x)   _DD_GETVALUE(x,S_CSR5_RXPROCSTATE,M_CSR5_RXPROCSTATE)

#define K_CSR5_RXSTOPPED	0x00	/* RESET or STOP command */
#define K_CSR5_RXFETCH		0x01	/* fetching rx desc */
#define K_CSR5_RXCHECK		0x02	/* checking end of rx pkt */
#define K_CSR5_RXWAIT		0x03	/* waiting for rx pkt */
#define K_CSR5_RXSUSPEND	0x04	/* unavailable rx buffer */
#define K_CSR5_RXCLOSE		0x05	/* closing rx desc */
#define K_CSR5_RXFLUSH		0x06	/* flushing rx frame */
#define K_CSR5_RXQUEUE		0x07	/* reading rx frame from FIFO */

#define S_CSR5_TXPROCSTATE	20
#define M_CSR5_TXPROCSTATE      _DD_MAKEMASK(3,S_CSR5_TXPROCSTATE)
#define V_CSR5_TXPROCSTATE(x)   _DD_MAKEVALUE(x,S_CSR5_TXPROCSTATE)
#define G_CSR5_TXPROCSTATE(x)   _DD_GETVALUE(x,S_CSR5_TXPROCSTATE,M_CSR5_TXPROCSTATE)

#define K_CSR5_TXSTOPPED	0x00	/* RESET or STOP command */
#define K_CSR5_TXFETCH		0x01	/* fetching tx desc */
#define K_CSR5_TXWAIT		0x02	/* waiting for end of tx */
#define K_CSR5_TXREAD		0x03	/* reading buffer into FIFO */
#define K_CSR5_TXSETUP		0x05	/* setup packet */
#define K_CSR5_TXSUSPEND	0x06	/* tx underflow or no tx desc */
#define K_CSR5_TXCLOSE		0x07	/* closing tx desc */

#define S_CSR5_ERRORBITS	23
#define M_CSR5_ERRORBITS	_DD_MAKEMASK(3,S_CSR5_ERRORBITS)
#define V_CSR5_ERRORBITS(x)     _DD_MAKEVALUE(x,S_CSR5_ERRORBITS)
#define G_CSR5_ERRORBITS(x)     _DD_GETVALUE(x,S_CSR5_ERRORBITS,M_CSR5_ERRORBITS)

#define K_CSR5_FBE_PARITY	0x00
#define K_CSR5_FBE_MABORT	0x01
#define K_CSR5_FBE_TABORT	0x02

#define M_CSR5_GPPORTINT	_DD_MAKEMASK1(26)  /* not 2104{0,1} */
#define M_CSR5_LINKCHANGED	_DD_MAKEMASK1(27)  /* not 2104{0,1} */


/* CSR6:  Operating Mode register */

#define M_CSR6_RXHASHFILT     	_DD_MAKEMASK1(0)
#define M_CSR6_RXSTART          _DD_MAKEMASK1(1)
#define M_CSR6_HASHONLY	     	_DD_MAKEMASK1(2)
#define M_CSR6_PASSBADFRAMES    _DD_MAKEMASK1(3)
#define M_CSR6_INVERSEFILT      _DD_MAKEMASK1(4)
#define M_CSR6_STOPBACKOFF    	_DD_MAKEMASK1(5)
#define M_CSR6_PROMISCUOUS      _DD_MAKEMASK1(6)
#define M_CSR6_PASSALLMULTI     _DD_MAKEMASK1(7)
#define M_CSR6_FULLDUPLEX       _DD_MAKEMASK1(9)

#define M_CSR6_INTLOOPBACK  	_DD_MAKEMASK1(10)
#define M_CSR6_EXTLOOPBACK	_DD_MAKEMASK1(11)

#define S_CSR6_OPMODE		10
#define M_CSR6_OPMODE           _DD_MAKEMASK(2,S_CSR6_OPMODE)
#define V_CSR6_OPMODE(x)        _DD_MAKEVALUE(x,S_CSR6_OPMODE)
#define G_CSR6_OPMODE(x)        _DD_GETVALUE(x,S_CSR6_OPMODE,M_CSR6_OPMODE)

#define M_CSR6_FORCECOLL    	_DD_MAKEMASK1(12)
#define M_CSR6_TXSTART          _DD_MAKEMASK1(13)

#define S_CSR6_THRESHCONTROL	14
#define M_CSR6_THRESHCONTROL	_DD_MAKEMASK(2,S_CSR6_THRESHCONTROL)
#define V_CSR6_THRESHCONTROL(x) _DD_MAKEVALUE(x,S_CSR6_THRESHCONTROL)
#define G_CSR6_THRESHCONTROL(x) _DD_GETVALUE(x,S_CSR6_THRESHCONTROL,M_CSR6_THRESHCONTROL)

#define M_CSR6_BACKPRESSURE     _DD_MAKEMASK1(16)  /* 21040 only */
#define M_CSR6_CAPTUREEFFECT    _DD_MAKEMASK1(17)

#define M_CSR6_PORTSEL       	_DD_MAKEMASK1(18)  /* not 2104{0,1} */
#define M_CSR6_HBDISABLE 	_DD_MAKEMASK1(19)  /* not 2104{0,1} */
#define M_CSR6_STOREFWD         _DD_MAKEMASK1(21)  /* not 2104{0,1} */
#define M_CSR6_TXTHRESH	  	_DD_MAKEMASK1(22)  /* not 2104{0,1} */
#define M_CSR6_PCSFUNC		_DD_MAKEMASK1(23)  /* not 2104{0,1} */
#define M_CSR6_SCRAMMODE 	_DD_MAKEMASK1(24)  /* not 2104{0,1} */
#define	M_CSR6_MBO		_DD_MAKEMASK1(25)  /* not 2104{0,1} */
#define M_CSR6_RXALL		_DD_MAKEMASK1(30)  /* not 2104{0,1} */

#define M_CSR6_SPECCAP		_DD_MAKEMASK1(31)  /* not 21040 */

#define K_CSR6_TXTHRES_128_72   0x00
#define K_CSR6_TXTHRES_256_96   0x01
#define K_CSR6_TXTHRES_512_128  0x02
#define K_CSR6_TXTHRES_1024_160 0x03


#define M_CSR6_SPEED_10		(M_CSR6_TXTHRESH)

#define M_CSR6_SPEED_100	(M_CSR6_HBDISABLE | \
				 M_CSR6_SCRAMMODE | \
				 M_CSR6_PCSFUNC | \
				 M_CSR6_PORTSEL)

#define M_CSR6_SPEED_10_MII	(M_CSR6_TXTHRESH | \
                                 M_CSR6_PORTSEL)

#define M_CSR6_SPEED_100_MII	(M_CSR6_HBDISABLE | \
				 M_CSR6_PORTSEL)


/* CSR7:  Interrupt mask register */

#define M_CSR7_TXINT		_DD_MAKEMASK1(0)
#define M_CSR7_TXSTOP           _DD_MAKEMASK1(1)
#define M_CSR7_TXBUFUNAVAIL 	_DD_MAKEMASK1(2)
#define M_CSR7_TXJABTIMEOUT     _DD_MAKEMASK1(3)
#define M_CSR7_LINKPASS		_DD_MAKEMASK1(4)  /* not 21040 */
#define M_CSR7_TXUNDERFLOW      _DD_MAKEMASK1(5)
#define M_CSR7_RXINT          	_DD_MAKEMASK1(6)
#define M_CSR7_RXBUFUNAVAIL 	_DD_MAKEMASK1(7)
#define M_CSR7_RXSTOPPED        _DD_MAKEMASK1(8)
#define M_CSR7_RXWDOGTIMEOUT   	_DD_MAKEMASK1(9)
#define M_CSR7_AUITPSW          _DD_MAKEMASK1(10)  /* 21040 only */
#define M_CSR7_TXEARLY		_DD_MAKEMASK1(10)  /* not 2104{0,1} */
#define M_CSR7_FD               _DD_MAKEMASK1(11)  /* 21040 only */
#define M_CSR7_GPTIMER		_DD_MAKEMASK1(11)  /* not 21040 */
#define M_CSR7_LINKFAIL         _DD_MAKEMASK1(12)
#define M_CSR7_FATALBUSERROR    _DD_MAKEMASK1(13)
#define M_CSR7_RXEARLY		_DD_MAKEMASK1(14)  /* not 21040 */
#define M_CSR7_ABNORMALINT    	_DD_MAKEMASK1(15)
#define M_CSR7_NORMALINT      	_DD_MAKEMASK1(16)
#define M_CSR7_GPPORT		_DD_MAKEMASK1(26)  /* not 2104{0,1} */
#define M_CSR7_LINKCHANGED	_DD_MAKEMASK1(27)  /* not 2104{0,1} */


/* CSR8: Missed Frame register */

#define M_CSR8_RXOVER_WRAP	_DD_MAKEMASK1(28)  /* not 2104{0,1} */
#define S_CSR8_RXOVER		17
#define M_CSR8_RXOVER		_DD_MAKEMASK(11,S_CSR8_RXOVER)  /* not 2104{0,1} */
#define V_CSR8_RXOVER(x)        _DD_MAKEVALUE(x,S_CSR8_RXOVER)
#define G_CSR8_RXOVER(x)        _DD_GETVALUE(x,S_CSR8_RXOVER,M_CSR8_RXOVER)

#define M_CSR8_MISSEDWRAP	_DD_MAKEMASK1(16)
#define S_CSR8_MISSED		0
#define M_CSR8_MISSED		_DD_MAKEMASK(16,S_CSR8_MISSED)
#define V_CSR8_MISSED(x)        _DD_MAKEVALUE(x,S_CSR8_MISSED)
#define G_CSR8_MISSED(x)        _DD_GETVALUE(x,S_CSR8_MISSED,M_CSR8_MISSED)


/* CSR9: ROM and MII register */

#define S_CSR9_ROMDATA		0
#define M_CSR9_ROMDATA		_DD_MAKEMASK(8,S_CSR9_ROMDATA)
#define V_CSR9_ROMDATA(x)       _DD_MAKEVALUE(x,S_CSR9_ROMDATA)
#define G_CSR9_ROMDATA(x)       _DD_GETVALUE(x,S_CSR9_ROMDATA,M_CSR9_ROMDATA)

#define M_CSR9_SROMCHIPSEL	_DD_MAKEMASK1(0)   /* not 21040 */
#define M_CSR9_SROMCLOCK	_DD_MAKEMASK1(1)   /* not 21040 */
#define M_CSR9_SROMDATAIN	_DD_MAKEMASK1(2)   /* not 21040 */
#define M_CSR9_SROMDATAOUT	_DD_MAKEMASK1(3)   /* not 21040 */

#define M_CSR9_REGSELECT	_DD_MAKEMASK1(10)  /* not 21040 */
#define M_CSR9_SERROMSEL	_DD_MAKEMASK1(11)  /* not 21040 */
#define M_CSR9_ROMSEL		_DD_MAKEMASK1(12)  /* not 21040 */
#define M_CSR9_ROMWRITE		_DD_MAKEMASK1(13)  /* not 21040 */
#define M_CSR9_ROMREAD		_DD_MAKEMASK1(14)  /* not 21040 */
#define M_CSR9_MODESEL		_DD_MAKEMASK1(15)  /* 21041 only */
#define M_CSR9_MDC		_DD_MAKEMASK1(16)  /* not 2104{0,1} */
#define M_CSR9_MDO		_DD_MAKEMASK1(17)  /* not 2104{0,1} */
#define M_CSR9_MIIMODE		_DD_MAKEMASK1(18)  /* not 2104{0,1} */
#define M_CSR9_MDI		_DD_MAKEMASK1(19)  /* not 2104{0,1} */

#define M_CSR9_DATANOTVALID	_DD_MAKEMASK1(31)  /* 21040 only */

#define M_CSR10_BOOTROMADDR	_DD_MAKEMASK(18,0) /* not 21040 */


/* CSR11 General Purpose Timer register */

#define S_CSR11_GPTIMER		0                  /* not 21040 */
#define M_CSR11_GPTIMER 	_DD_MAKEMASK(16,S_CSR11_GPTIMER)
#define V_CSR11_GPTIMER(x)      _DD_MAKEVALUE(x,S_CSR11_GPTIMER)
#define G_CSR11_GPTIMER(x)      _DD_GETVALUE(x,S_CSR11_GPTIMER,M_CSR11_GPTIMER)


#define M_CSR11_GPTIMERCONT     _DD_MAKEMASK1(16)  /* not 21040 */

#define S_CSR11_FDAUTOCONF      0                  /* 21040 only */
#define M_CSR11_FDAUTOCONF      _DD_MAKEMASK(16,S_CSR11_FDAUTOCONF)
#define V_CSR11_FDAUTOCONF(x)   _DD_MAKEVALUE(x,S_CSR11_FDAUTOCONF)
#define G_CSR11_FRAUTOCONF(x)   _DD_GETVALUE(x,S_CSR11_FDAUTOCONF,M_CSR11_AUTOCONF)


/* CSR12:  SIA Status register (21143) */

#define M_CSR12_MIIRPA		_DD_MAKEMASK1(0)
#define M_CSR12_100MBLINK	_DD_MAKEMASK1(1)
#define M_CSR12_10MBLINK	_DD_MAKEMASK1(2)
#define M_CSR12_AUTOPOLSTATE	_DD_MAKEMASK1(3)

#define M_CSR12_RXAUIACT	_DD_MAKEMASK1(8)
#define M_CSR12_RX10BASETACT	_DD_MAKEMASK1(9)
#define M_CSR12_NLPDETECT	_DD_MAKEMASK1(10)
#define M_CSR12_TXREMFAULT	_DD_MAKEMASK1(11)

#define S_CSR12_AUTONEGARBIT	12
#define M_CSR12_AUTONEGARBIT	_DD_MAKEMASK(3,S_CSR12_AUTONEGARBIT)
#define V_CSR12_AUTONEGARBIT(x) _DD_MAKEVALUE(x,S_CSR12_AUTONEGARBIT)
#define G_CSR12_AUTONEGARBIT(x) _DD_GETVALUE(x,S_CSR12_AUTONEGARBIT,M_CSR12_AUTONEGARBIT)

#define M_CSR12_LINKPARTNEG	_DD_MAKEMASK1(15)

#define S_CSR12_LINKPARTCODE	16
#define M_CSR12_LINKPARTCODE	_DD_MAKEMASK(16,S_CSR12_LINKPARTCODE)
#define V_CSR12_LINKPARTCODE(x) _DD_MAKEVALUE(x,S_CSR12_LINKPARTCODE)
#define G_CSR12_LINKPARTCODE(x) _DD_GETVALUE(x,S_CSR12_LINKPARTCODE,M_CSR12_LINKPARTCODE)


/* CSR12:  SIA Status register (21041, also 31:12, 3:3 as for 21143) */

#define M_CSR12_NETCONNERR	_DD_MAKEMASK1(1)
#define M_CSR12_LINKFAIL	_DD_MAKEMASK1(2)
#define M_CSR12_SELPORTACT	_DD_MAKEMASK1(8)
#define M_CSR12_NONSELPORTACT	_DD_MAKEMASK1(9)
#define M_CSR12_AUTONEGRESTART	_DD_MAKEMASK1(10)
#define M_CSR12_UNSTABLENLP	_DD_MAKEMASK1(11)


/* CSR12: General Purpose Port register (21140) */

#define S_CSR12_DATA		0
#define M_CSR12_DATA		_DD_MAKEMASK(8,S_CSR12_DATA)
#define V_CSR12_DATA		_DD_MAKEVALUE(x,S_CSR12_DATA,M_CSR12_DATA)
#define G_CSR12_DATA(x)		_DD_GETVALUE(x,S_CSR12_DATA,M_CSR12_DATA)

#define M_CSR12_CONTROL		_DD_MAKEMASK1(8)


/* CSR13:  SIA Mode 0 register (21143 and 21041) */

#define M_CSR13_CONN_NOT_RESET	_DD_MAKEMASK1(0)
#define M_CSR13_CONN_CSR_AUTO	_DD_MAKEMAKS1(2)   /* 21041 only */
#define M_CSR13_CONN_AUI_10BT	_DD_MAKEMASK1(3)


/* CSR14:  SIA Mode 1 register (21143 and 21041) */

#define M_CSR14_ENCODER		_DD_MAKEMASK1(0)
#define M_CSR14_LOOPBACK	_DD_MAKEMASK1(1)
#define M_CSR14_DRIVER		_DD_MAKEMASK1(2)
#define M_CSR14_LINKPULSE	_DD_MAKEMASK1(3)

#define S_CSR14_COMPENSATE	4
#define M_CSR14_COMPENSATE	_DD_MAKEMASK(2,S_CSR14_COMPENSATE)
#define V_CSR14_COMPENSATE(x)   _DD_MAKEVALUE(x,S_CSR14_COMPENSATE)
#define G_CSR14_COMPENSATE(x)   _DD_GETVALUE(x,S_CSR14_COMPENSATE,M_CSR14_COMPENSATE)

#define M_CSR14_HALFDUPLEX10BASET _DD_MAKEMASK1(6)
#define M_CSR14_AUTONEGOTIATE	_DD_MAKEMASK1(7)
#define M_CSR14_RXSQUELCH	_DD_MAKEMASK1(8)
#define M_CSR14_COLLSQUELCH	_DD_MAKEMASK1(9)
#define M_CSR14_COLLDETECT	_DD_MAKEMASK1(10)
#define M_CSR14_SIGQUALGEN	_DD_MAKEMASK1(11)
#define M_CSR14_LINKTEST	_DD_MAKEMASK1(12)
#define M_CSR14_AUTOPOLARITY	_DD_MAKEMASK1(13)
#define M_CSR14_SETPOLARITY	_DD_MAKEMASK1(14)
#define M_CSR14_10BASETAUIAUTO	_DD_MAKEMASK1(15)
#define M_CSR14_100BASETHALFDUP	_DD_MAKEMASK1(16)  /* not 21041 */
#define M_CSR14_100BASETFULLDUP	_DD_MAKEMASK1(17)  /* not 21041 */
#define M_CSR14_100BASET4	_DD_MAKEMASK1(18)  /* not 21041 */

#define M_CSR14_10BT_HD		0x7F3F
#define M_CSR14_10BT_FD		0x7F3D	


/* CSR15:  SIA Mode 2 register (21143 and 21041) */

#define M_CSR15_GP_JABBERDIS	_DD_MAKEMASK1(0)   /* 21041 only */
#define M_CSR15_GP_HOSTUNJAB	_DD_MAKEMASK1(1)
#define M_CSR15_GP_JABBERCLK	_DD_MAKEMASK1(2)
#define M_CSR15_GP_AUIBNC	_DD_MAKEMASK1(3)
#define M_CSR15_GP_RXWATCHDIS	_DD_MAKEMASK1(4)
#define M_CSR15_GP_RXWATCHREL	_DD_MAKEMASK1(5)

/* (CSR15: 21143 only) */

#define S_CSR15_GP_GPDATA	16
#define M_CSR15_GP_GPDATA	_DD_MAKEMASK(4,S_CSR15_GP_GPDATA)
#define V_CSR15_GP_GPDATA(x)    _DD_MAKEVALUE(x,S_CSR15_GP_GPDATA)
#define G_CSR15_GP_GPDATA(x)    _DD_GETVALUE(x,S_CSR15_GP_GPDATA,M_CSR15_GP_GPDATA)

#define M_CSR15_GP_LED0		_DD_MAKEMASK1(20)
#define M_CSR15_GP_LED1		_DD_MAKEMASK1(21)
#define M_CSR15_GP_LED2		_DD_MAKEMASK1(22)
#define M_CSR15_GP_LED3		_DD_MAKEMASK1(23)
#define M_CSR15_GP_INTPORT0	_DD_MAKEMASK1(24)
#define M_CSR15_GP_INTPORT1	_DD_MAKEMASK1(25)
#define M_CSR15_GP_RXMATCH	_DD_MAKEMASK1(26)
#define M_CSR15_GP_CONTROLWRITE	_DD_MAKEMASK1(27)
#define M_CSR15_GP_GPINT0	_DD_MAKEMASK1(28)
#define M_CSR15_GP_GPINT1	_DD_MAKEMASK1(29)
#define M_CSR15_GP_RXMATCHINT	_DD_MAKEMASK1(30)

#define M_CSR15_DEFAULT_VALUE		0x00050008	
#define M_CSR15_CONFIG_GEPS_LEDS	0x08af0000

/* (CSR15: 21041 only) */

#define M_CSR15_GP_LED1ENB	_DD_MAKEMASK1(6)
#define M_CSR15_GP_LED1VALUE	_DD_MAKEMASK1(7)
#define M_CSR15_GP_TSTCLK	_DD_MAKEMASK1(8)
#define M_CSR15_GP_FORCEUNSQ	_DD_MAKEMASK1(9)
#define M_CSR15_GP_FORCEFAIL	_DD_MAKEMASK1(10)
#define M_CSR15_GP_LEDSTRDIS	_DD_MAKEMASK1(11)
#define M_CSR15_GP_PLLTEST	_DD_MAKEMASK1(12)
#define M_CSR15_GP_FORCERXLOW	_DD_MAKEMASK1(13)
#define M_CSR15_GP_LED2ENB	_DD_MAKEMASK1(14)
#define M_CSR15_GP_LED2VALUE	_DD_MAKEMASK1(15)


/* CSR15: Watchdog Timer register (21140) */

#define M_CSR15_WT_JABBER	_DD_MAKEMASK1(0)
#define M_CSR15_WT_HOSTUNJAB	_DD_MAKEMASK1(1)
#define M_CSR15_WT_JABBERCLK	_DD_MAKEMASK1(2)
#define M_CSR15_WT_RXWATCHDIS	_DD_MAKEMASK1(4)
#define M_CSR15_WT_RXWATCHREL	_DD_MAKEMASK1(5)


/*  *********************************************************************
    *  Receive Descriptors
    ********************************************************************* */

#define M_RDES0_OWNSYS          0
#define M_RDES0_OWNADAP         _DD_MAKEMASK1(31)

#define S_RDES0_FRAMELEN	16
#define M_RDES0_FRAMELEN        _DD_MAKEMASK(14,S_RDES0_FRAMELEN)
#define V_RDES0_FRAMELEN(x)     _DD_MAKEVALUE(x,S_RDES0_FRAMELEN)
#define G_RDES0_FRAMELEN(x)     _DD_GETVALUE(x,S_RDES0_FRAMELEN,M_RDES0_FRAMELEN)

#define M_RDES0_ZERO		_DD_MAKEMASK1(0)
#define M_RDES0_OVFL		_DD_MAKEMAKS1(0)   /* 21041 only */
#define M_RDES0_CRCERR          _DD_MAKEMASK1(1)
#define M_RDES0_DRIBBLE         _DD_MAKEMASK1(2)
#define M_RDES0_MIIERROR	_DD_MAKEMASK1(3)   /* not 21041 */
#define M_RDES0_WDOGTIMER       _DD_MAKEMASK1(4)
#define M_RDES0_FRAMETYPE       _DD_MAKEMASK1(5)
#define M_RDES0_COLLSEEN        _DD_MAKEMASK1(6)
#define M_RDES0_FRAMETOOLONG    _DD_MAKEMASK1(7)
#define M_RDES0_LASTDES         _DD_MAKEMASK1(8)
#define M_RDES0_FIRSTDES        _DD_MAKEMASK1(9)
#define M_RDES0_MCASTFRAME      _DD_MAKEMASK1(10)
#define M_RDES0_RUNTFRAME       _DD_MAKEMASK1(11)

#define S_RDES0_DATATYPE	12
#define M_RDES0_DATATYPE        _DD_MAKEMASK(2,S_RDES0_DATATYPE)
#define V_RDES0_DATATYPE(x)     _DD_MAKEVALUE(x,S_RDES0_DATATYPE)
#define G_RDES0_DATATYPE(x)     _DD_GETVALUE(x,S_RDES0_DATATYPE,M_RDES0_DATATYPE)

#define M_RDES0_ERROR		_DD_MAKEMASK1(14)
#define M_RDES0_ERRORSUM        _DD_MAKEMASK1(15)
#define M_RDES0_FILTFAIL	_DD_MAKEMASK1(30)  /* not 21041 */

#define S_RDES1_BUF1SIZE	0
#define M_RDES1_BUF1SIZE        _DD_MAKEMASK(11,S_TDES1_BUF1SIZE)
#define V_RDES1_BUF1SIZE(x)     _DD_MAKEVALUE(x,S_RDES1_BUF1SIZE)
#define G_RDES1_BUF1SIZE(x)     _DD_GETVALUE(x,S_RDES1_BUF1SIZE,M_RDES1_BUF1SIZE)

#define S_RDES1_BUF2SIZE	11
#define M_RDES1_BUF2SIZE        _DD_MAKEMASK(11,S_TDES2_BUF2SIZE)
#define V_RDES1_BUF2SIZE(x)     _DD_MAKEVALUE(x,S_RDES1_BUF2SIZE)
#define G_RDES1_BUF2SIZE(x)     _DD_GETVALUE(x,S_RDES1_BUF2SIZE,M_RDES1_BUF2SIZE)

#define M_RDES1_CHAINED         _DD_MAKEMASK1(24)
#define M_RDES1_ENDOFRING       _DD_MAKEMASK1(25)

#define M_RDES2_BUFADDR		0xFFFFFFFF
#define M_RDES3_BUFADDR		0xFFFFFFFF

/*  *********************************************************************
    *  Transmit Descriptors
    ********************************************************************* */

#define M_TDES0_OWNSYS          0
#define M_TDES0_OWNADAP         _DD_MAKEMASK1(31)

#define M_TDES0_DEFERRED        _DD_MAKEMASK1(0)
#define M_TDES0_UNDERFLOW       _DD_MAKEMASK1(1)
#define M_TDES0_LINK_FAIL       _DD_MAKEMASK1(2)

#define S_TDES0_COLLCOUNT	3
#define M_TDES0_COLLCOUNT	_DD_MAKEMASK(4,S_TDES0_COLLCOUNT)
#define V_TDES0_COLLCOUNT(x)    _DD_MAKEVALUE(x,S_TDES0_COLLCOUNT)
#define G_TDES0_COLLCOUNT(x)    _DD_GETVALUE(x,S_TDES0_COLLCOUNT,M_TDES0_COLLCOUNT)

#define M_TDES0_HEARTBEAT_FAIL           _DD_MAKEMASK1(7)
#define M_TDES0_EXCESSIVE_COLLISIONS     _DD_MAKEMASK1(8)
#define M_TDES0_LATE_COLLISION           _DD_MAKEMASK1(9)
#define M_TDES0_NO_CARRIER               _DD_MAKEMASK1(10)
#define M_TDES0_LOSS_OF_CARRIER          _DD_MAKEMASK1(11)
#define M_TDES0_TX_JABBER_TIMEOUT        _DD_MAKEMASK1(14)
#define M_TDES0_ERROR_SUMMARY            _DD_MAKEMASK1(15)
#define M_TDES0_OWN_BIT                  _DD_MAKEMASK1(31)

#define S_TDES1_BUF1SIZE	0
#define M_TDES1_BUF1SIZE        _DD_MAKEMASK(11,S_TDES1_BUF1SIZE)
#define V_TDES1_BUF1SIZE(x)     _DD_MAKEVALUE(x,S_TDES1_BUF1SIZE)
#define G_TDES1_BUF1SIZE(x)     _DD_GETVALUE(x,S_TDES1_BUF1SIZE,M_TDES1_BUF1SIZE)

#define S_TDES1_BUF2SIZE	11
#define M_TDES1_BUF2SIZE        _DD_MAKEMASK(11,S_TDES2_BUF2SIZE)
#define V_TDES1_BUF2SIZE(x)     _DD_MAKEVALUE(x,S_TDES1_BUF2SIZE)
#define G_TDES1_BUF2SIZE(x)     _DD_GETVALUE(x,S_TDES1_BUF2SIZE,M_TDES1_BUF2SIZE)

#define M_TDES1_FT0             _DD_MAKEMASK1(22)
#define M_TDES1_NOPADDING       _DD_MAKEMASK1(23)
#define M_TDES1_CHAINED      	_DD_MAKEMASK1(24)
#define M_TDES1_ENDOFRING       _DD_MAKEMASK1(25)
#define M_TDES1_NOADDCRC        _DD_MAKEMASK1(26)
#define M_TDES1_SETUP           _DD_MAKEMASK1(27)
#define M_TDES1_FT1             _DD_MAKEMASK1(28)
#define M_TDES1_FIRSTSEG        _DD_MAKEMASK1(29)
#define M_TDES1_LASTSEG         _DD_MAKEMASK1(30)
#define M_TDES1_INTERRUPT  	_DD_MAKEMASK1(31)

#define M_TDES2_BUFADDR		0xFFFFFFFF
#define M_TDES3_BUFADDR		0xFFFFFFFF


/* CAM  */

#define CAM_HASH_THRESHOLD          14
#define CAM_PERFECT_ENTRIES         16

#define CAM_SETUP_BUFFER_SIZE       192

#endif /* _DC21143_H_ */

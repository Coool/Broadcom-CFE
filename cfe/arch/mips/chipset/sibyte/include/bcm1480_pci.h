/*  *********************************************************************
    *  BCM1280/BCM1480 Board Support Package
    *  
    *  PCI constants				File: bcm1480_pci.h
    *  
    *  This module contains constants and macros to describe 
    *  the PCI-X interface on the BCM1255/BCM1280/BCM1455/BCM1480.  
    *  
    *  BCM1480 specification level:  1X55_1X80_UM100-R (12/18/03)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003,2004
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


#ifndef _BCM1480_PCI_H
#define _BCM1480_PCI_H

#include "sb1250_defs.h"


/*
 * PCI Reset Register (Table 108)
 */

#define M_BCM1480_PCI_RESET_PIN             _SB_MAKEMASK1(0)
#define M_BCM1480_PCI_INTERNAL_RESET        _SB_MAKEMASK1(1)
#define M_BCM1480_PCI_TIMEOUT_RESET         _SB_MAKEMASK1(2)
#define M_BCM1480_PCI_RESET_INTR            _SB_MAKEMASK1(4)
#define M_BCM1480_PCI_M66EN_STATUS          _SB_MAKEMASK1(8)
#define M_BCM1480_PCI_M66EN_DRIVE_LOW       _SB_MAKEMASK1(11)
#define M_BCM1480_PCI_PCIXCAP_STATUS        _SB_MAKEMASK1(12)
#define M_BCM1480_PCI_PCIXCAP_PULLUP        _SB_MAKEMASK1(15)
#define M_BCM1480_PCI_PERR_RST_ASSERT       _SB_MAKEMASK1(16)
#define M_BCM1480_PCI_DEVSEL_RST_ASSERT     _SB_MAKEMASK1(17)
#define M_BCM1480_PCI_STOP_RST_ASSERT       _SB_MAKEMASK1(18)
#define M_BCM1480_PCI_TRDY_RST_ASSERT       _SB_MAKEMASK1(19)
#define M_BCM1480_PCI_PERR_RST_STATUS       _SB_MAKEMASK1(20)
#define M_BCM1480_PCI_DEVSEL_RST_STATUS     _SB_MAKEMASK1(21)
#define M_BCM1480_PCI_STOP_RST_STATUS       _SB_MAKEMASK1(22)
#define M_BCM1480_PCI_TRDY_RST_STATUS       _SB_MAKEMASK1(23)

/*
 * PCI DLL Register (Table 110)
 */

#define S_BCM1480_PCI_DLL_BYPASS_MODE        0
#define M_BCM1480_PCI_DLL_BYPASS_MODE        _SB_MAKEMASK(2,S_BCM1480_PCI_DLL_BYPASS_MODE)
#define V_BCM1480_PCI_DLL_BYPASS_MODE(x)     _SB_MAKEVALUE(x,S_BCM1480_PCI_DLL_BYPASS_MODE)
#define G_BCM1480_PCI_DLL_BYPASS_MODE(x)     _SB_GETVALUE(x,S_BCM1480_PCI_DLL_BYPASS_MODE,M_BCM1480_PCI_DLL_BYPASS_MODE)
#define K_BCM1480_PCI_DLL_AUTO               0x0
#define K_BCM1480_PCI_DLL_FORCE_BYPASS       0x1
#define K_BCM1480_PCI_DLL_FORCE_USE          0x2

#define M_BCM1480_PCI_DLL_FIXED_VALUE_EN     _SB_MAKEMASK1(3)

#define S_BCM1480_PCI_DLL_FIXED_VALUE        4
#define M_BCM1480_PCI_DLL_FIXED_VALUE        _SB_MAKEMASK(6,S_BCM1480_PCI_DLL_FIXED_VALUE)
#define V_BCM1480_PCI_DLL_FIXED_VALUE(x)     _SB_MAKEVALUE(x,S_BCM1480_PCI_DLL_FIXED_VALUE)
#define G_BCM1480_PCI_DLL_FIXED_VALUE(x)     _SB_GETVALUE(x,S_BCM1480_PCI_DLL_FIXED_VALUE,M_BCM1480_PCI_DLL_FIXED_VALUE)

#define S_BCM1480_PCI_DLL_DELAY              12
#define M_BCM1480_PCI_DLL_DELAY              _SB_MAKEMASK(4,S_BCM1480_PCI_DLL_DELAY)
#define V_BCM1480_PCI_DLL_DELAY(x)           _SB_MAKEVALUE(x,S_BCM1480_PCI_DLL_DELAY)
#define G_BCM1480_PCI_DLL_DELAY(x)           _SB_GETVALUE(x,S_BCM1480_PCI_DLL_DELAY,M_BCM1480_PCI_DLL_DELAY)

#define S_BCM1480_PCI_DLL_STEP_SIZE          16
#define M_BCM1480_PCI_DLL_STEP_SIZE          _SB_MAKEMASK(4,S_BCM1480_PCI_DLL_STEP_SIZE)
#define V_BCM1480_PCI_DLL_STEP_SIZE(x)       _SB_MAKEVALUE(x,S_BCM1480_PCI_DLL_STEP_SIZE)
#define G_BCM1480_PCI_DLL_STEP_SIZE(x)       _SB_GETVALUE(x,S_BCM1480_PCI_DLL_STEP_SIZE,M_BCM1480_PCI_DLL_STEP_SIZE)


/*
 * The following definitions refer to PCI Configuration Space of the
 * PCI-X Host Bridge (PHB).  All registers are 32 bits.
 */

#define K_BCM1480_PHB_VENDOR_SIBYTE     0x166D
#define K_BCM1480_PHB_DEVICE_BCM1480    0x0012

/*
 * PHB Interface Configuration Header (Table 111).
 * The first 64 bytes are a standard Type 0 header.  The bridge also
 * implements the standard PCIX and MSI capabilities.  Only
 * device-specific extensions are defined here.
 */

#define R_BCM1480_PHB_FCTRL             0x0040
#define R_BCM1480_PHB_MAPBASE           0x0044	/* 0x44 through 0x80 - map table */
#define BCM1480_PHB_MAPENTRIES          16	/* 64 bytes, 16 entries */
#define R_BCM1480_PHB_MAP(n)            (R_BCM1480_PHB_MAPBASE + (n)*4)
#define R_BCM1480_PHB_ERRORADDR         0x0084  /* lower, upper */
#define R_BCM1480_PHB_ADDSTATCMD        0x008C
#define R_BCM1480_PHB_SUBSYSSET         0x0090
#define R_BCM1480_PHB_SIGNALINTA        0x0094
#define R_BCM1480_PHB_EXTCONFIGDIS      0x0098
#define R_BCM1480_PHB_VENDORIDSET       0x009C
#define R_BCM1480_PHB_CLASSREVSET       0x00A0
#define R_BCM1480_PHB_TIMEOUT           0x00A4
#define R_BCM1480_PHB_XACTCTRL          0x00A8
#define R_BCM1480_PHB_TESTDEBUG         0x00AC
#define R_BCM1480_PHB_OMAPBASE          0x00B0	/* 0xB0 through 0xCC - omap table */
#define BCM1480_PHB_OMAPENTRIES         4	/* 32 bytes, 4 entries */
#define R_BCM1480_PHB_OMAP(n)           (R_BCM1480_PHB_OMAPBASE + (n)*8)
#define R_BCM1480_PHB_MSICAP            0x00D0
#define R_BCM1480_PHB_PCIXCAP           0x00E0
#define R_BCM1480_PHB_TGTDONE           0x00E8


/*
 * PHB Feature Control Register (Table 116)
 */

#define M_BCM1480_PHB_FCTRL_FULL_BAR_EN      _SB_MAKEMASK1_32(0)
#define M_BCM1480_PHB_FCTRL_FULL_BAR_SPLIT   _SB_MAKEMASK1_32(1)
#define M_BCM1480_PHB_FCTRL_LOW_MEM_EN       _SB_MAKEMASK1_32(2)
#define M_BCM1480_PHB_FCTRL_UPPER_MEM_EN     _SB_MAKEMASK1_32(3)
#define M_BCM1480_PHB_FCTRL_EXP_MEM_EN       _SB_MAKEMASK1_32(4)
#define M_BCM1480_PHB_FCTRL_EXP_MEM_SPLIT    _SB_MAKEMASK1_32(5)
#define M_BCM1480_PHB_FCTRL_TOP_ACC_EN       _SB_MAKEMASK1_32(6)
#define M_BCM1480_PHB_FCTRL_TOP_ACC_SPLIT    _SB_MAKEMASK1_32(7)
#define M_BCM1480_PHB_FCTRL_USE_NODE_ID      _SB_MAKEMASK1_32(8)
#define M_BCM1480_PHB_FCTRL_UPPER_MEM_TR     _SB_MAKEMASK1_32(12)
#define V_BCM1480_PHB_FCTRL_DEFAULT          0

/*
 * PHB BAR0/1 Map Table Entry (Offsets 0x44-0x80) (Table 117)
 */

#define M_BCM1480_PHB_MAP_ENABLE             _SB_MAKEMASK1_32(0)
#define M_BCM1480_PHB_MAP_L2CA               _SB_MAKEMASK1_32(2)
#define M_BCM1480_PHB_MAP_ENDIAN             _SB_MAKEMASK1_32(3)

#define S_BCM1480_PHB_MAP_ADDR               12
#define M_BCM1480_PHB_MAP_ADDR               _SB_MAKEMASK_32(20,S_BCM1480_PHB_MAP_ADDR)
#define V_BCM1480_PHB_MAP_ADDR(x)            _SB_MAKEVALUE_32(x,S_BCM1480_PHB_MAP_ADDR)
#define G_BCM1480_PHB_MAP_ADDR(x)            _SB_GETVALUE_32(x,S_BCM1480_PHB_MAP_ADDR,M_BCM1480_PHB_MAP_ADDR)

/*
 * PHB Additional Status and Command Register (Table 118)
 */

#define M_BCM1480_PHB_ASTCMD_HOTPLUG_EN      _SB_MAKEMASK1_32(0)
#define M_BCM1480_PHB_ASTCMD_SERR_DET        _SB_MAKEMASK1_32(1)
#define M_BCM1480_PHB_ASTCMD_TRDY_ERR        _SB_MAKEMASK1_32(2)
#define M_BCM1480_PHB_ASTCMD_RETRY_ERR       _SB_MAKEMASK1_32(3)
#define M_BCM1480_PHB_ASTCMD_TRDY_INT_EN     _SB_MAKEMASK1_32(4)
#define M_BCM1480_PHB_ASTCMD_RETRY_INT_EN    _SB_MAKEMASK1_32(5)
#define M_BCM1480_PHB_ASTCMD_COMPL_TO_ERR    _SB_MAKEMASK1_32(6)
#define M_BCM1480_PHB_ASTCMD_COMPL_TO_INT_EN _SB_MAKEMASK1_32(7)
#define M_BCM1480_PHB_ASTCMD_64B_DEVICE_SET  _SB_MAKEMASK1_32(16)
#define M_BCM1480_PHB_ASTCMD_133MHZ_CAP_SET  _SB_MAKEMASK1_32(17)
#define V_BCM1480_PHB_ASTCMD_DEFAULT         (M_BCM1480_PHB_ASTCMD_64B_DEVICE_SET | \
                                       M_BCM1480_PHB_ASTCMD_133MHZ_CAP_SET)

/*
 * PHB INTA Control Register (Table 119)
 */

#define M_BCM1480_PHB_SIGNAL_INTA            _SB_MAKEMASK1_32(0)

/*
 * PHB External Configuratation Disable Register (Table 120)
 */

#define M_BCM1480_PHB_EXT_CONFIG_DIS         _SB_MAKEMASK1_32(0)

/*
 * PHB Timeout Register (Table 121)
 */

#define S_BCM1480_PHB_TIMEOUT_TRDY           0
#define M_BCM1480_PHB_TIMEOUT_TRDY           _SB_MAKEMASK_32(8,S_BCM1480_PHB_TIMEOUT_TRDY)
#define V_BCM1480_PHB_TIMEOUT_TRDY(x)        _SB_MAKEVALUE_32(x,S_BCM1480_PHB_TIMEOUT_TRDY)
#define G_BCM1480_PHB_TIMEOUT_TRDY(x)        _SB_GETVALUE_32(x,S_BCM1480_PHB_TIMEOUT_TRDY,M_BCM1480_PHB_TIMEOUT_TRDY)

#define S_BCM1480_PHB_TIMEOUT_RETRY          8
#define M_BCM1480_PHB_TIMEOUT_RETRY          _SB_MAKEMASK_32(8,S_BCM1480_PHB_TIMEOUT_RETRY)
#define V_BCM1480_PHB_TIMEOUT_RETRY(x)       _SB_MAKEVALUE_32(x,S_BCM1480_PHB_TIMEOUT_RETRY)
#define G_BCM1480_PHB_TIMEOUT_RETRY(x)       _SB_GETVALUE_32(x,S_BCM1480_PHB_TIMEOUT_RETRY,M_BCM1480_PHB_TIMEOUT_RETRY)

#define S_BCM1480_PHB_TIMEOUT_COMPL          16
#define M_BCM1480_PHB_TIMEOUT_COMPL          _SB_MAKEMASK_32(4,S_BCM1480_PHB_TIMEOUT_COMPL)
#define V_BCM1480_PHB_TIMEOUT_COMPL(x)       _SB_MAKEVALUE_32(x,S_BCM1480_PHB_TIMEOUT_COMPL)
#define G_BCM1480_PHB_TIMEOUT_COMPL(x)       _SB_GETVALUE_32(x,S_BCM1480_PHB_TIMEOUT_COMPL,M_BCM1480_PHB_TIMEOUT_COMPL)

#define S_BCM1480_PHB_TIMEOUT_INB_RD_PREF    20
#define M_BCM1480_PHB_TIMEOUT_INB_RD_PREF    _SB_MAKEMASK_32(4,S_BCM1480_PHB_TIMEOUT_INB_RD_PREF)
#define V_BCM1480_PHB_TIMEOUT_INB_RD_PREF(x) _SB_MAKEVALUE_32(x,S_BCM1480_PHB_TIMEOUT_INB_RD_PREF)
#define G_BCM1480_PHB_TIMEOUT_INB_RD_PREF(x) _SB_GETVALUE_32(x,S_BCM1480_PHB_TIMEOUT_INB_RD_PREF,M_BCM1480_PHB_TIMEOUT_INB_RD_PREF)

#define M_BCM1480_PHB_TIMEOUT_INB_RD_OUTB_WR _SB_MAKEMASK1_32(24)
#define M_BCM1480_PHB_TIMEOUT_INB_RD_OUTB_RD _SB_MAKEMASK1_32(25)
#define M_BCM1480_PHB_TIMEOUT_INB_RD_INB_WR  _SB_MAKEMASK1_32(25)

#define S_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG    28
#define M_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG    _SB_MAKEMASK_32(4,S_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG)
#define V_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG(x) _SB_MAKEVALUE_32(x,S_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG)
#define G_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG(x) _SB_GETVALUE_32(x,S_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG,M_BCM1480_PHB_TIMEOUT_OUTB_FWD_PROG)

#define V_BCM1480_PHB_TIMEOUT_DEFAULT        (V_BCM1480_PHB_TIMEOUT_TRDY(0x80) |  \
                                      V_BCM1480_PHB_TIMEOUT_RETRY(0x80) | \
                                      V_BCM1480_PHB_TIMEOUT_COMPL(0xA))

/*
 * PHB Transaction Control Register (Table 122)
 */

#define S_BCM1480_PHB_XACT_WR_COMBINE_TMR          0
#define M_BCM1480_PHB_XACT_WR_COMBINE_TMR          _SB_MAKEMASK_32(8,S_BCM1480_PHB_XACT_WR_COMBINE_TMR)
#define V_BCM1480_PHB_XACT_WR_COMBINE_TMR(x)       _SB_MAKEVALUE_32(x,S_BCM1480_PHB_XACT_WR_COMBINE_TMR)
#define G_BCM1480_PHB_XACT_WR_COMBINE_TMR(x)       _SB_GETVALUE_32(x,S_BCM1480_PHB_XACT_WR_COMBINE_TMR,M_BCM1480_PHB_XACT_WR_COMBINE_TMR)

#define S_BCM1480_PHB_XACT_OUTB_NP_ORDER           8
#define M_BCM1480_PHB_XACT_OUTB_NP_ORDER           _SB_MAKEMASK_32(2,S_BCM1480_PHB_XACT_OUTB_NP_ORDER)
#define V_BCM1480_PHB_XACT_OUTB_NP_ORDER(x)        _SB_MAKEVALUE_32(x,S_BCM1480_PHB_XACT_OUTB_NP_ORDER)
#define G_BCM1480_PHB_XACT_OUTB_NP_ORDER(x)        _SB_GETVALUE_32(x,S_BCM1480_PHB_XACT_OUTB_NP_ORDER,M_BCM1480_PHB_XACT_OUTB_NP_ORDER)

#define M_BCM1480_PHB_XACT_SET_WR_RLX_ORDER        _SB_MAKEMASK1_32(10)
#define M_BCM1480_PHB_XACT_SET_RSP_RLX_ORDER       _SB_MAKEMASK1_32(11)
#define M_BCM1480_PHB_XACT_SET_WR_NO_SNOOP         _SB_MAKEMASK1_32(12)
#define M_BCM1480_PHB_XACT_SET_RD_NO_SNOOP         _SB_MAKEMASK1_32(13)
#define M_BCM1480_PHB_XACT_SET_OUTB_RD_PREF_DIS    _SB_MAKEMASK1_32(14)
#define M_BCM1480_PHB_XACT_SET_OUTB_RSP_WR_ORD_DIS _SB_MAKEMASK1_32(15)

#define S_BCM1480_PHB_XACT_INB_RD_MAX_PREF         20
#define M_BCM1480_PHB_XACT_INB_RD_MAX_PREF         _SB_MAKEMASK_32(3,S_BCM1480_PHB_XACT_INB_RD_MAX_PREF)
#define V_BCM1480_PHB_XACT_INB_RD_MAX_PREF(x)      _SB_MAKEVALUE_32(x,S_BCM1480_PHB_XACT_INB_RD_MAX_PREF)
#define G_BCM1480_PHB_XACT_INB_RD_MAX_PREF(x)      _SB_GETVALUE_32(x,S_BCM1480_PHB_XACT_INB_RD_MAX_PREF,M_BCM1480_PHB_XACT_INB_RD_MAX_PREF)

#define S_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF      24
#define M_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF      _SB_MAKEMASK_32(3,S_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF)
#define V_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF(x)   _SB_MAKEVALUE_32(x,S_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF)
#define G_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF(x)   _SB_GETVALUE_32(x,S_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF,M_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF)

#define S_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF     28
#define M_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF     _SB_MAKEMASK_32(3,S_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF)
#define V_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF(x)  _SB_MAKEVALUE_32(x,S_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF)
#define G_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF(x)  _SB_GETVALUE_32(x,S_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF,M_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF)

#define K_BCM1480_PHB_PREF_256B              0x0
#define K_BCM1480_PHB_PREF_32B               0x1
#define K_BCM1480_PHB_PREF_64B               0x2
#define K_BCM1480_PHB_PREF_96B               0x3
#define K_BCM1480_PHB_PREF_128B              0x4   /* also 0x5 */
#define K_BCM1480_PHB_PREF_192B              0x6   /* also 0x7 */

#define V_BCM1480_PHB_XACT_DEFAULT           (V_BCM1480_PHB_XACT_WR_COMBINE_TMR(0x20)     | \
                                      V_BCM1480_PHB_XACT_OUTB_NP_ORDER(0x1)       | \
                                      V_BCM1480_PHB_XACT_INB_RD_MAX_PREF(0x1)     | \
                                      V_BCM1480_PHB_XACT_INB_RD_LN_MAX_PREF(0x1)  | \
                                      V_BCM1480_PHB_XACT_INB_RD_MUL_MAX_PREF(0x4))

/*
 * PHB Test and Debug Register (Table 123)
 */

#define M_BCM1480_PHB_TEST_LOOPBACK          _SB_MAKEMASK1_32(0)
#define M_BCM1480_PHB_TEST_32BIT_MODE        _SB_MAKEMASK1_32(1)
#define M_BCM1480_PHB_TEST_QUICK_TEST        _SB_MAKEMASK1_32(2)

/*
 * PHB Outbound Map Table Entries (Lower, Upper) (Tables 124 and 125)
 */

#define M_BCM1480_PHB_OMAP_L_ENABLE          _SB_MAKEMASK1_32(0)

#define S_BCM1480_PHB_OMAP_L_ADDR            20
#define M_BCM1480_PHB_OMAP_L_ADDR            _SB_MAKEMASK_32(12,S_BCM1480_PHB_OMAP_L_ADDR)
#define V_BCM1480_PHB_OMAP_L_ADDR(x)         _SB_MAKEVALUE_32(x,S_BCM1480_PHB_OMAP_L_ADDR)
#define G_BCM1480_PHB_OMAP_L_ADDR(x)         _SB_GETVALUE_32(x,S_BCM1480_PHB_OMAP_L_ADDR,M_BCM1480_PHB_OMAP_L_ADDR)

#define S_BCM1480_PHB_OMAP_U_ADDR            0
#define M_BCM1480_PHB_OMAP_U_ADDR            _SB_MAKEMASK_32(32,S_BCM1480_PHB_OMAP_U_ADDR)
#define V_BCM1480_PHB_OMAP_U_ADDR(x)         _SB_MAKEVALUE_32(x,S_BCM1480_PHB_OMAP_U_ADDR)
#define G_BCM1480_PHB_OMAP_U_ADDR(x)         _SB_GETVALUE_32(x,S_BCM1480_PHB_OMAP_U_ADDR,M_BCM1480_PHB_OMAP_U_ADDR)

/*
 * PHB Target Done Register (Table 129)
 */

#define S_BCM1480_PHB_TGT_DONE_COUNTER       0
#define M_BCM1480_PHB_TGT_DONE_COUNTER       _SB_MAKEMASK_32(8,S_BCM1480_PHB_TGT_DONE_COUNTER)
#define V_BCM1480_PHB_TGT_DONE_COUNTER(x)    _SB_MAKEVALUE_32(x,S_BCM1480_PHB_TGT_DONE_COUNTER)
#define G_BCM1480_PHB_TGT_DONE_COUNTER(x)    _SB_GETVALUE_32(x,S_BCM1480_PHB_TGT_DONE_COUNTER,M_BCM1480_PHB_TGT_DONE_COUNTER)


struct bcm1480_inbw_conf  {

   unsigned long long   pa;       /* Base address(Physical) of the memory region to be mapped at BAR0 */
 
   unsigned int  offset;   /* Offset from the Base address - Start of the region */

   unsigned int  len;      /* Length of the region */

   int           l2ca;     /* L2CA flag */

   int           endian;   /* Endian flag */
};

#endif /* _BCM1480_PCI_H */


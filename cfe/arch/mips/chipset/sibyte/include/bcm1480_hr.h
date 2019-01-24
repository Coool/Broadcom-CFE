/*  *********************************************************************
    *  BCM1280/BCM1480 Board Support Package
    *  
    *  Hash and Route Block constants             File: bcm1480_hr.h       
    *  
    *  This module contains constants and macros useful for
    *  programming the hash and route block of each rx port.
    *  
    *  BCM1400 specification level:  1X55_1X80-UM100-D4 (11/24/03)
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

#ifndef _BCM1480_HR_H
#define _BCM1480_HR_H

#include "sb1250_defs.h"
/*
 * H&R to PMI mapping register
 */

#define S_BCM1480_HR_RX2PMI_MAP_LO             0
#define M_BCM1480_HR_RX2PMI_MAP_LO             _SB_MAKEMASK(4,S_BCM1480_HR_RX2PMI_MAP_LO)
#define V_BCM1480_HR_RX2PMI_MAP_LO(x)          _SB_MAKEVALUE(x,S_BCM1480_HR_RX2PMI_MAP_LO)
#define G_BCM1480_HR_RX2PMI_MAP_LO(x)          _SB_GETVALUE(x,S_BCM1480_HR_RX2PMI_MAP_LO,M_BCM1480_HR_RX2PMI_MAP_LO)

#define K_BCM1480_HR_RX2PMI_MAP_7_0		0
#define K_BCM1480_HR_RX2PMI_MAP_15_8		1
#define K_BCM1480_HR_RX2PMI_MAP_23_16		2
#define K_BCM1480_HR_RX2PMI_MAP_31_24		3

#define S_BCM1480_HR_RX2PMI_MAP_HI             4
#define M_BCM1480_HR_RX2PMI_MAP_HI             _SB_MAKEMASK(4,S_BCM1480_HR_RX2PMI_MAP_HI)
#define V_BCM1480_HR_RX2PMI_MAP_HI(x)          _SB_MAKEVALUE(x,S_BCM1480_HR_RX2PMI_MAP_HI)
#define G_BCM1480_HR_RX2PMI_MAP_HI(x)          _SB_GETVALUE(x,S_BCM1480_HR_RX2PMI_MAP_HI,M_BCM1480_HR_RX2PMI_MAP_HI)


/*
 * H&R Configuration Register (Table 283)
 */

#define S_BCM1480_HR_HEADER_PTR             0
#define M_BCM1480_HR_HEADER_PTR             _SB_MAKEMASK(8,S_BCM1480_HR_HEADER_PTR)
#define V_BCM1480_HR_HEADER_PTR(x)          _SB_MAKEVALUE(x,S_BCM1480_HR_HEADER_PTR)
#define G_BCM1480_HR_HEADER_PTR(x)          _SB_GETVALUE(x,S_BCM1480_HR_HEADER_PTR,M_BCM1480_HR_HEADER_PTR)

#define M_BCM1480_HR_HDR_PTR_IMMD           _SB_MAKEMASK1(8)
#define M_BCM1480_HR_SELECT_PTNUM_TO_TAG    _SB_MAKEMASK1(9)
#define M_BCM1480_HR_PT_UNMATCH_ENABLE      _SB_MAKEMASK1(10)
#define M_BCM1480_HR_PT_MULTIMATCH_ENABLE   _SB_MAKEMASK1(11)


/* XXX The following have field name clashes.  Resolution deferred. */

/*
 * Rule Operand Configuration Entry (Table 284)
 */

#define S_BCM1480_HR_RULE_OP_OPERAND_3              0
#define M_BCM1480_HR_RULE_OP_OPERAND_3              _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_OPERAND_3)
#define V_BCM1480_HR_RULE_OP_OPERAND_3(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_3)
#define G_BCM1480_HR_RULE_OP_OPERAND_3(x)           _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_3,M_BCM1480_HR_RULE_OP_OPERAND_3)

#define S_BCM1480_HR_RULE_OP_OPERAND_2              8
#define M_BCM1480_HR_RULE_OP_OPERAND_2              _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_OPERAND_2)
#define V_BCM1480_HR_RULE_OP_OPERAND_2(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_2)
#define G_BCM1480_HR_RULE_OP_OPERAND_2(x)           _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_2,M_BCM1480_HR_RULE_OP_OPERAND_2)

#define S_BCM1480_HR_RULE_OP_OPERAND_1              16
#define M_BCM1480_HR_RULE_OP_OPERAND_1              _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_OPERAND_1)
#define V_BCM1480_HR_RULE_OP_OPERAND_1(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_1)
#define G_BCM1480_HR_RULE_OP_OPERAND_1(x)           _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_1,M_BCM1480_HR_RULE_OP_OPERAND_1)

#define S_BCM1480_HR_RULE_OP_OPERAND_0              24
#define M_BCM1480_HR_RULE_OP_OPERAND_0              _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_OPERAND_0)
#define V_BCM1480_HR_RULE_OP_OPERAND_0(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_0)
#define G_BCM1480_HR_RULE_OP_OPERAND_0(x)           _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_OPERAND_0,M_BCM1480_HR_RULE_OP_OPERAND_0)

#define S_BCM1480_HR_RULE_OP_ENABLE_3               32
#define M_BCM1480_HR_RULE_OP_ENABLE_3               _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_ENABLE_3)
#define V_BCM1480_HR_RULE_OP_ENABLE_3(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_3)
#define G_BCM1480_HR_RULE_OP_ENABLE_3(x)            _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_3,M_BCM1480_HR_RULE_OP_ENABLE_3)

#define S_BCM1480_HR_RULE_OP_ENABLE_2               40
#define M_BCM1480_HR_RULE_OP_ENABLE_2               _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_ENABLE_2)
#define V_BCM1480_HR_RULE_OP_ENABLE_2(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_2)
#define G_BCM1480_HR_RULE_OP_ENABLE_2(x)            _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_2,M_BCM1480_HR_RULE_OP_ENABLE_2)

#define S_BCM1480_HR_RULE_OP_ENABLE_1               48
#define M_BCM1480_HR_RULE_OP_ENABLE_1               _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_ENABLE_1)
#define V_BCM1480_HR_RULE_OP_ENABLE_1(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_1)
#define G_BCM1480_HR_RULE_OP_ENABLE_1(x)            _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_1,M_BCM1480_HR_RULE_OP_ENABLE_1)

#define S_BCM1480_HR_RULE_OP_ENABLE_0               56
#define M_BCM1480_HR_RULE_OP_ENABLE_0               _SB_MAKEMASK(8,S_BCM1480_HR_RULE_OP_ENABLE_0)
#define V_BCM1480_HR_RULE_OP_ENABLE_0(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_0)
#define G_BCM1480_HR_RULE_OP_ENABLE_0(x)            _SB_GETVALUE(x,S_BCM1480_HR_RULE_OP_ENABLE_0,M_BCM1480_HR_RULE_OP_ENABLE_0)


/*
 * Rule Type Configuration Entry (Table 285)
 */

/* XXX WORD_OFFSET - clashes */
#define S_BCM1480_HR_RULE_TYPE_WORD_OFST_0       0
#define M_BCM1480_HR_RULE_TYPE_WORD_OFST_0       _SB_MAKEMASK(6,S_BCM1480_HR_RULE_TYPE_WORD_OFST_0)
#define V_BCM1480_HR_RULE_TYPE_WORD_OFST_0(x)    _SB_MAKEVALUE(x,S_BCM1480_HR_RULE_TYPE_WORD_OFST_0)
#define G_BCM1480_HR_RULE_TYPE_WORD_OFST_0(x)    _SB_GETVALUE(x,S_BCM1480_HR_RULE_TYPE_WORD_OFST_0,M_BCM1480_HR_RULE_TYPE_WORD_OFST_0)

/* XXX SELECT      - clashes */
#define M_BCM1480_HR_RULE_TYPE_SEL_0       _SB_MAKEMASK1(8)

/*
 * Path Definition Entry (Table 286)
 */


#define S_BCM1480_HR_ENABLE                 0
#define M_BCM1480_HR_ENABLE                 _SB_MAKEMASK(16,S_BCM1480_HR_ENABLE)
#define V_BCM1480_HR_ENABLE(x)              _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE)
#define G_BCM1480_HR_ENABLE(x)              _SB_GETVALUE(x,S_BCM1480_HR_ENABLE,M_BCM1480_HR_ENABLE)

#define S_BCM1480_HR_TEST                   16
#define M_BCM1480_HR_TEST                   _SB_MAKEMASK(16,S_BCM1480_HR_TEST)
#define V_BCM1480_HR_TEST(x)                _SB_MAKEVALUE(x,S_BCM1480_HR_TEST)
#define G_BCM1480_HR_TEST(x)                _SB_GETVALUE(x,S_BCM1480_HR_TEST,M_BCM1480_HR_TEST)

#define S_BCM1480_HR_PATH_DATA              32
#define M_BCM1480_HR_PATH_DATA              _SB_MAKEMASK(16,S_BCM1480_HR_PATH_DATA)
#define V_BCM1480_HR_PATH_DATA(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_PATH_DATA)
#define G_BCM1480_HR_PATH_DATA(x)           _SB_GETVALUE(x,S_BCM1480_HR_PATH_DATA,M_BCM1480_HR_PATH_DATA)

#define S_BCM1480_HR_PATH_DATA_DEST         44
#define M_BCM1480_HR_PATH_DATA_DEST         _SB_MAKEMASK(2,S_BCM1480_HR_PATH_DATA_DEST)
#define V_BCM1480_HR_PATH_DATA_DEST(x)      _SB_MAKEVALUE(x,S_BCM1480_HR_PATH_DATA_DEST)
#define G_BCM1480_HR_PATH_DATA_DEST(x)      _SB_GETVALUE(x,S_BCM1480_HR_PATH_DATA_DEST,M_BCM1480_HR_PATH_DATA_DEST)

#define S_BCM1480_HR_PATH_DATA_VC           36
#define M_BCM1480_HR_PATH_DATA_VC           _SB_MAKEMASK(4,S_BCM1480_HR_PATH_DATA_VC)
#define V_BCM1480_HR_PATH_DATA_VC(x)        _SB_MAKEVALUE(x,S_BCM1480_HR_PATH_DATA_VC)
#define G_BCM1480_HR_PATH_DATA_VC(x)        _SB_GETVALUE(x,S_BCM1480_HR_PATH_DATA_VC,M_BCM1480_HR_PATH_DATA_VC)

#define S_BCM1480_HR_PATH_DATA_NDEST        32
#define M_BCM1480_HR_PATH_DATA_NDEST        _SB_MAKEMASK(4,S_BCM1480_HR_PATH_DATA_NDEST)
#define V_BCM1480_HR_PATH_DATA_NDEST(x)     _SB_MAKEVALUE(x,S_BCM1480_HR_PATH_DATA_NDEST)
#define G_BCM1480_HR_PATH_DATA_NDEST(x)     _SB_GETVALUE(x,S_BCM1480_HR_PATH_DATA_NDEST,M_BCM1480_HR_PATH_DATA_NDEST)

#define S_BCM1480_HR_PATH_TYPE              48
#define M_BCM1480_HR_PATH_TYPE              _SB_MAKEMASK(3,S_BCM1480_HR_PATH_TYPE)
#define V_BCM1480_HR_PATH_TYPE(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_PATH_TYPE)
#define G_BCM1480_HR_PATH_TYPE(x)           _SB_GETVALUE(x,S_BCM1480_HR_PATH_TYPE,M_BCM1480_HR_PATH_TYPE)
#define K_BCM1480_HR_PATH_TYPE_OVC          0x0
#define K_BCM1480_HR_PATH_TYPE_RTI          0x1
#define K_BCM1480_HR_PATH_TYPE_HA_LEAF0     0x2
#define K_BCM1480_HR_PATH_TYPE_EX_LEAF0     0x4

/* end of clashes */


/*
 * Hash leaf0 Data Word Definition Entry (Table 287)
 */

#define S_BCM1480_HR_ENABLE_3               0
#define M_BCM1480_HR_ENABLE_3               _SB_MAKEMASK(8,S_BCM1480_HR_ENABLE_3)
#define V_BCM1480_HR_ENABLE_3(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE_3)
#define G_BCM1480_HR_ENABLE_3(x)            _SB_GETVALUE(x,S_BCM1480_HR_ENABLE_3,M_BCM1480_HR_ENABLE_3)

#define S_BCM1480_HR_ENABLE_2               8
#define M_BCM1480_HR_ENABLE_2               _SB_MAKEMASK(8,S_BCM1480_HR_ENABLE_2)
#define V_BCM1480_HR_ENABLE_2(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE_2)
#define G_BCM1480_HR_ENABLE_2(x)            _SB_GETVALUE(x,S_BCM1480_HR_ENABLE_2,M_BCM1480_HR_ENABLE_2)

#define S_BCM1480_HR_ENABLE_1               16
#define M_BCM1480_HR_ENABLE_1               _SB_MAKEMASK(8,S_BCM1480_HR_ENABLE_1)
#define V_BCM1480_HR_ENABLE_1(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE_1)
#define G_BCM1480_HR_ENABLE_1(x)            _SB_GETVALUE(x,S_BCM1480_HR_ENABLE_1,M_BCM1480_HR_ENABLE_1)

#define S_BCM1480_HR_ENABLE_0               24
#define M_BCM1480_HR_ENABLE_0               _SB_MAKEMASK(8,S_BCM1480_HR_ENABLE_0)
#define V_BCM1480_HR_ENABLE_0(x)            _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE_0)
#define G_BCM1480_HR_ENABLE_0(x)            _SB_GETVALUE(x,S_BCM1480_HR_ENABLE_0,M_BCM1480_HR_ENABLE_0)

#define S_BCM1480_HR_WORD_OFFSET            32
#define M_BCM1480_HR_WORD_OFFSET            _SB_MAKEMASK(6,S_BCM1480_HR_WORD_OFFSET)
#define V_BCM1480_HR_WORD_OFFSET(x)         _SB_MAKEVALUE(x,S_BCM1480_HR_WORD_OFFSET)
#define G_BCM1480_HR_WORD_OFFSET(x)         _SB_GETVALUE(x,S_BCM1480_HR_WORD_OFFSET,M_BCM1480_HR_WORD_OFFSET)

#define M_BCM1480_HR_SELECT                 _SB_MAKEMASK1(40)

/*
 * Extract leaf0 Extract Definition Register (Table 288)
 */

#define S_BCM1480_HR_WORD_OFFSET_LOW        0
#define M_BCM1480_HR_WORD_OFFSET_LOW        _SB_MAKEMASK(6,S_BCM1480_HR_WORD_OFFSET_LOW)
#define V_BCM1480_HR_WORD_OFFSET_LOW(x)     _SB_MAKEVALUE(x,S_BCM1480_HR_WORD_OFFSET_LOW)
#define G_BCM1480_HR_WORD_OFFSET_LOW(x)     _SB_GETVALUE(x,S_BCM1480_HR_WORD_OFFSET_LOW,M_BCM1480_HR_WORD_OFFSET_LOW)

#define M_BCM1480_HR_SELECT_LOW             _SB_MAKEMASK1(8)

#define S_BCM1480_HR_ENABLE_LOW             16
#define M_BCM1480_HR_ENABLE_LOW             _SB_MAKEMASK(4,S_BCM1480_HR_ENABLE_LOW)
#define V_BCM1480_HR_ENABLE_LOW(x)          _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE_LOW)
#define G_BCM1480_HR_ENABLE_LOW(x)          _SB_GETVALUE(x,S_BCM1480_HR_ENABLE_LOW,M_BCM1480_HR_ENABLE_LOW)

#define S_BCM1480_HR_NIB_OFFSET_LOW         20
#define M_BCM1480_HR_NIB_OFFSET_LOW         _SB_MAKEMASK(3,S_BCM1480_HR_NIB_OFFSET_LOW)
#define V_BCM1480_HR_NIB_OFFSET_LOW(x)      _SB_MAKEVALUE(x,S_BCM1480_HR_NIB_OFFSET_LOW)
#define G_BCM1480_HR_NIB_OFFSET_LOW(x)      _SB_GETVALUE(x,S_BCM1480_HR_NIB_OFFSET_LOW,M_BCM1480_HR_NIB_OFFSET_LOW)

#define S_BCM1480_HR_WORD_OFFSET_HIGH       32
#define M_BCM1480_HR_WORD_OFFSET_HIGH       _SB_MAKEMASK(6,S_BCM1480_HR_WORD_OFFSET_HIGH)
#define V_BCM1480_HR_WORD_OFFSET_HIGH(x)    _SB_MAKEVALUE(x,S_BCM1480_HR_WORD_OFFSET_HIGH)
#define G_BCM1480_HR_WORD_OFFSET_HIGH(x)    _SB_GETVALUE(x,S_BCM1480_HR_WORD_OFFSET_HIGH,M_BCM1480_HR_WORD_OFFSET_HIGH)

#define M_BCM1480_HR_SELECT_HIGH            _SB_MAKEMASK1(40)

#define S_BCM1480_HR_ENABLE_HIGH            48
#define M_BCM1480_HR_ENABLE_HIGH            _SB_MAKEMASK(4,S_BCM1480_HR_ENABLE_HIGH)
#define V_BCM1480_HR_ENABLE_HIGH(x)         _SB_MAKEVALUE(x,S_BCM1480_HR_ENABLE_HIGH)
#define G_BCM1480_HR_ENABLE_HIGH(x)         _SB_GETVALUE(x,S_BCM1480_HR_ENABLE_HIGH,M_BCM1480_HR_ENABLE_HIGH)

#define S_BCM1480_HR_NIB_OFFSET_HIGH        52
#define M_BCM1480_HR_NIB_OFFSET_HIGH        _SB_MAKEMASK(3,S_BCM1480_HR_NIB_OFFSET_HIGH)
#define V_BCM1480_HR_NIB_OFFSET_HIGH(x)     _SB_MAKEVALUE(x,S_BCM1480_HR_NIB_OFFSET_HIGH)
#define G_BCM1480_HR_NIB_OFFSET_HIGH(x)     _SB_GETVALUE(x,S_BCM1480_HR_NIB_OFFSET_HIGH,M_BCM1480_HR_NIB_OFFSET_HIGH)

/*
 * RX0 Route Table Definition Entry (Table 289)
 */

#define S_BCM1480_HR_NEXT_DEST              0
#define M_BCM1480_HR_NEXT_DEST              _SB_MAKEMASK(4,S_BCM1480_HR_NEXT_DEST)
#define V_BCM1480_HR_NEXT_DEST(x)           _SB_MAKEVALUE(x,S_BCM1480_HR_NEXT_DEST)
#define G_BCM1480_HR_NEXT_DEST(x)           _SB_GETVALUE(x,S_BCM1480_HR_NEXT_DEST,M_BCM1480_HR_NEXT_DEST)

#define S_BCM1480_HR_OVC_IQ                 4
#define M_BCM1480_HR_OVC_IQ                 _SB_MAKEMASK(4,S_BCM1480_HR_OVC_IQ)
#define V_BCM1480_HR_OVC_IQ(x)              _SB_MAKEVALUE(x,S_BCM1480_HR_OVC_IQ)
#define G_BCM1480_HR_OVC_IQ(x)              _SB_GETVALUE(x,S_BCM1480_HR_OVC_IQ,M_BCM1480_HR_OVC_IQ)

#define S_BCM1480_HR_DEST                   12
#define M_BCM1480_HR_DEST                   _SB_MAKEMASK(2,S_BCM1480_HR_DEST)
#define V_BCM1480_HR_DEST(x)                _SB_MAKEVALUE(x,S_BCM1480_HR_DEST)
#define G_BCM1480_HR_DEST(x)                _SB_GETVALUE(x,S_BCM1480_HR_DEST,M_BCM1480_HR_DEST)
#define K_BCM1480_HR_DEST_TX0               0x0
#define K_BCM1480_HR_DEST_TX1               0x1
#define K_BCM1480_HR_DEST_TX2               0x2
#define K_BCM1480_HR_DEST_PMI               0x3

/*
 * Default Path Register (Table 290)
 */

/* Uses PATH_DATA and PATH_TYPE fields from Path Definition (Table 286) */

#endif /* _BCM1480_HR_H */

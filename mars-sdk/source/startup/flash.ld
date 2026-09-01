/*
 * Copyright(c)2019 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0(the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************
 * @file     flash.ld
 * @brief    GNU Linker Script for Nuclei N/NX based device in flash Download Mode
 * @version  V1.0.0
 * @date     17. Dec 2019
 ******************************************************************************/
OUTPUT_ARCH("riscv")

ENTRY(_start)

/*
 * R   - Read-only sections.
 * W   - Read/write sections.
 * X   - Sections containing executable code.
 * A   - Allocated sections.
 * I/L - Initialized sections.
 * !   - Invert the sense of any of the following attributes.
 */
MEMORY
{
    FLASH(rx!aw): ORIGIN = 0x30000000, LENGTH = 2M
    SRAM (warx) : ORIGIN = 0x20000000, LENGTH = 224K
}

REGION_ALIAS("ROM", FLASH)
REGION_ALIAS("RAM", SRAM)

#define SCATCOPY(LEN,VMA,LMA)   LONG(LEN) LONG(VMA) LONG(LMA) 
#define SCATLOAD(SCT)           LONG(SIZEOF(SCT)) LONG(ADDR(SCT)) LONG(LOADADDR(SCT)) 
#define SCATZERO(SCT)           LONG(SIZEOF(SCT)) LONG(ADDR(SCT))

#include "romelf.ls"

SECTIONS
{
    PROVIDE(__VTAB_SIZE = 144); /* remap 36 entries, and skip next 4 enties + 64 bytes header */
    PROVIDE(__STACK_SIZE = 1K);
    PROVIDE(__HEAP_SIZE = 256);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // STARTUP
    .init           :
    {
        /* Default Vector Table */
        * (.vtable)

        /* Starup Initialition Region */
        . = ALIGN(4);
        KEEP(* (SORT_NONE(.init)))
    } >ROM AT>ROM

    .scatab         : ALIGN(4)
    {
        /* Scatter Copy/Load Table */
        . = ALIGN(4);
        PROVIDE(__scat_copy_base = .);
            SCATCOPY(__VTAB_SIZE, ADDR(.vtab), LOADADDR(.init))
            SCATLOAD(.text);
            SCATLOAD(.rodata);
            SCATLOAD(.data);
            SCATLOAD(.fastro);
            SCATLOAD(.fastrw);
            SCATLOAD(.mapi);
        PROVIDE(__scat_copy_last = .);

        /* Scatter Zero-Fill Table */
        . = ALIGN(4);
        PROVIDE(__scat_zero_base = .);
            SCATZERO(.fastzi);
            SCATZERO(.bss);
        PROVIDE(__scat_zero_last = .);
    } >ROM AT>ROM

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // DYNLOAD
    /* remapped vtable: align 4 byte for RV32, 8 Byte for RV64 */
    .vtab (NOLOAD)  : ALIGN(8)
    {
        . = ALIGN(8);
        PROVIDE(__vmaof_vtab = .);
        . += __VTAB_SIZE;
    } >RAM AT>RAM

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // ROM/RAM => RAM
    .fastro         : ALIGN(4)
    {
        . = ALIGN(4);
        * (.fast_rodata .fast_rodata.*)
        *drv.a:uart.* (.rodata.*)
        . = ALIGN(4);
    } >RAM AT>ROM
    .fastrw         : ALIGN(4)
    {
        . = ALIGN(4);
        * (.fast_data .fast_data.*)
        . = ALIGN(4);
    } >RAM AT>ROM
    .fastzi(NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        * (.fast_bss .fast_bss.*)
        . = ALIGN(4); 
    } >RAM AT>RAM

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // ROM => ROM
    .cache          : ALIGN(32)
    {
        . = ALIGN(32);
        * (.text.irq .text.xPortTaskSwitch .text.xPortSysTickHandler)
        * (.fast_text .fast_text.*)
        *drv.a:uart.* (.text.*)
        . = ALIGN(4);
    } >ROM AT>ROM
    PROVIDE(__lmaof_cache = LOADADDR(.cache));
    PROVIDE(__vmaof_cache = ADDR(.cache));
    PROVIDE(__lenof_cache = SIZEOF(.cache));

    .shell          : ALIGN(4)
    {
        . = ALIGN(4);
        _shell_items_start = .;
        KEEP(* (.shell.user))
        KEEP(* (.shell.priv))
        . = ALIGN(4);
    } >ROM AT>ROM

    .text           : ALIGN(4)
    {
        . = ALIGN(4);
        *luna.a: (.text .text.*)
        * (.stext .stext.*)
        * (.text .text.*)
        * (.gnu.linkonce.t.*)
        . = ALIGN(4);
    } >ROM AT>ROM

    .rodata         : ALIGN(4)
    {
        . = ALIGN(4);
        *luna.a: (.rodata .rodata.*)
        * (.rodata .rodata.*)
        * (.strdat)
        . = ALIGN(4);
    } >ROM AT>ROM

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // ROM => RAM
    .mapi           : ALIGN(4)
    {
        . = ALIGN(4);
        PROVIDE_HIDDEN(__mod_stdc = .);
        KEEP(* (SORT(.mod.0.?)))
        PROVIDE_HIDDEN(__mod_rtos = .);
        KEEP(* (SORT(.mod.?.?)))
        PROVIDE_HIDDEN(__mod_ends = .);
        . = ALIGN(4);
    } >RAM AT>ROM

    .data           : ALIGN(8)
    {
        . = ALIGN(8);
        *luna.a: (.data.*)
        * (.data .data.*)
        * (.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE(__global_pointer$ = . + 0x800);
        * (.sdata .sdata.*)
        * (.gnu.linkonce.s.*)
        . = ALIGN(8);
        * (.rdata .rdata.*)
        * (.gnu.linkonce.r.*)
        * (.srodata .srodata.*)
    } >RAM AT>ROM
    PROVIDE(__tls_base = .);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // ROM => ROM
    .stub           :
    {
        KEEP(* (.stub))
    } >ROM AT>ROM

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // ZERO => RAM
    .bss(NOLOAD)    : ALIGN(8)
    {
        . = ALIGN(8);
        *luna.a: (.bss .bss.*)
        * (.sbss .sbss.*)
        * (.gnu.linkonce.sb.*)
        * (.bss .bss.*)
        * (.gnu.linkonce.b.*)
        * (COMMON)
        . = ALIGN(4);
    } >RAM AT>RAM

	/* RTOS Heap Region (8 bytes aligned at least) */
    .osheap(NOLOAD) : ALIGN(16)
    {
        . = ALIGN(ORIGIN(RAM) + LENGTH(RAM) - __HEAP_SIZE - __STACK_SIZE, 16);
    } >RAM AT>RAM
    PROVIDE(__vmaof_osheap = ADDR(.osheap));
    PROVIDE(__lenof_osheap = SIZEOF(.osheap));

    /* Nuclei C Runtime Library requirements:
     * 1. heap need to be align at 16 bytes
     * 2. __heap_start and __heap_end symbol need to be defined
     * 3. reserved at least __HEAP_SIZE space for heap
     * 4. reserved at least __STACK_SIZE space for stack
     */
    .heapstk(NOLOAD) : ALIGN(16)
    {
        . = ALIGN(16);
        PROVIDE(__heap_start = .);
        . += __HEAP_SIZE;
        . = ALIGN(16);
        PROVIDE(__heap_end = .);
        PROVIDE(__StackLimit = .);
        . += __STACK_SIZE;
        . = ALIGN(16);
        PROVIDE(__StackTop = .);
    } >RAM AT>RAM
}

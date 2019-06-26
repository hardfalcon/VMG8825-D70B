#include <asm/asm.h>
#include <asm/asmmacro.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>


/* cache operations */
#define CACHE_OPCODE( code, type )              ( ((code) << 2) | (type) )

#define ICACHE_INDEX_INVALIDATE                 CACHE_OPCODE(0x0, 0)
#define ICACHE_INDEX_LOAD_TAG                   CACHE_OPCODE(0x1, 0)
#define ICACHE_INDEX_STORE_TAG                  CACHE_OPCODE(0x2, 0)
#define DCACHE_INDEX_WRITEBACK_INVALIDATE       CACHE_OPCODE(0x0, 1)
#define DCACHE_INDEX_LOAD_TAG                   CACHE_OPCODE(0x1, 1)
#define DCACHE_INDEX_STORE_TAG                  CACHE_OPCODE(0x2, 1)
#define SCACHE_INDEX_WRITEBACK_INVALIDATE       CACHE_OPCODE(0x0, 3)
#define SCACHE_INDEX_STORE_TAG                  CACHE_OPCODE(0x2, 3)

#define ICACHE_ADDR_HIT_INVALIDATE              CACHE_OPCODE(0x4, 0)
#define ICACHE_ADDR_FILL                        CACHE_OPCODE(0x5, 0)
#define ICACHE_ADDR_FETCH_LOCK                  CACHE_OPCODE(0x7, 0)
#define DCACHE_ADDR_HIT_INVALIDATE              CACHE_OPCODE(0x4, 1)
#define DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE    CACHE_OPCODE(0x5, 1)
#define DCACHE_ADDR_HIT_WRITEBACK               CACHE_OPCODE(0x6, 1)
#define DCACHE_ADDR_FETCH_LOCK                  CACHE_OPCODE(0x7, 1)


#define L2CACHE_ADDR_HIT_WRITEBACK_INVALIDATE   CACHE_OPCODE(0x5, 3)
#define L2CACHE_ADDR_HIT_INVALIDATE             CACHE_OPCODE(0x4, 3)
#define SCACHE_ADDR_HIT_WRITEBACK               CACHE_OPCODE(0x6, 3)


.set noreorder   // no optimisation by re ordering assembly code
.option pic0

	/************************************************************************
     *  Invalidate L$2 cache -> writeback & Invalidate base on address
     ************************************************************************/
    /* Routine for invalidating L2 cache */
    /* Usage
       mips_l2cache_invalidate(u32 lock_size, u32 cache_line_size,  u32 start_address)
       a0 : lock size
       a1 : cache line size
       a2 : start address
     */
.globl   mips_l2cache_invalidate
.ent mips_l2cache_invalidate
.align 4
    mips_l2cache_invalidate:
    /* Calc an address that will correspond to the last cache line  */
    addu    a3, a2, a0
    subu    a3, a1

    /* Loop through all lines, invalidating each of them */
1:
    cache   L2CACHE_ADDR_HIT_INVALIDATE, 0(a2)
    bne     a2, a3, 1b
    addu    a2, a1

9:
    jr  ra
    nop
.end   mips_l2cache_invalidate

	/************************************************************************
     *  flush l2 cache -> writeback & Invalidate base on index
     ************************************************************************/
    /* Routine for flushing L2 cache */
    /* Usage
       mips_l2cache_flush(u32 lock_size, u32 cache_line_size,  u32 start_address)
       a0 : lock size
       a1 : cache line size
       a2 : start address
     */
.globl   mips_l2cache_flush
.ent mips_l2cache_flush
.align 4
mips_l2cache_flush:
    /* Calc an address that will correspond to the last cache line  */
    addu    a3, a2, a0
    subu    a3, a1

    /* Loop through all lines, invalidating each of them */
1:
    cache   L2CACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0(a2)
    bne     a2, a3, 1b
    addu    a2, a1

9:
    jr  ra
    nop
.end   mips_l2cache_flush


    /************************************************************************
     *  flush dcache -> writeback & Invalidate base on index
     ************************************************************************/
    /* Routine for flushing Dcache */
    /* Usage
       mips_dcache_flush(u32 lock_size, u32 cache_line_size,  u32 start_address)
       a0 : lock size
       a1 : cache line size
       a2 : start address
     */
.globl   mips_dcache_flush
.ent mips_dcache_flush
.align 4
mips_dcache_flush:
    /* Calc an address that will correspond to the last cache line  */
    addu    a3, a2, a0
    subu    a3, a1

    /* Loop through all lines, invalidating each of them */
1:
    cache   DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE, 0(a2)
    bne     a2, a3, 1b
    addu    a2, a1

9:
    jr  ra
    nop
.end   mips_dcache_flush

    /************************************************************************
     *  Invalidate dcache -> writeback & Invalidate base on address
     ************************************************************************/
    /* Routine for flushing Dcache */
    /* Usage
       mips_dcache_flush(u32 lock_size, u32 cache_line_size,  u32 start_address)
       a0 : lock size
       a1 : cache line size
       a2 : start address
     */
.globl   mips_dcache_invalidate
.ent mips_dcache_invalidate
.align 4
mips_dcache_invalidate:
    /* Calc an address that will correspond to the last cache line  */
    addu    a3, a2, a0
    subu    a3, a1

    /* Loop through all lines, invalidating each of them */
1:
    cache   DCACHE_ADDR_HIT_INVALIDATE, 0(a2)
    bne     a2, a3, 1b
    addu    a2, a1

9:
    jr  ra
    nop
.end   mips_dcache_invalidate

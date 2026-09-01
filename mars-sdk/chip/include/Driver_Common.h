/*
 * Project:      Common Driver definitions
 */

#ifndef __DRIVER_COMMON_H
#define __DRIVER_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CSK_DRIVER_VERSION_MAJOR_MINOR(major,minor) (((major) << 8) | (minor))

/**
 \brief Driver Version
 */
typedef struct _CSK_DRIVER_VERSION
{
    uint16_t api;                         ///< API version
    uint16_t drv;                         ///< Driver version
} CSK_DRIVER_VERSION;

/* General return codes */
#define CSK_DRIVER_OK                 0 ///< Operation succeeded
#define CSK_DRIVER_ERROR             -1 ///< Unspecified error
#define CSK_DRIVER_ERROR_BUSY        -2 ///< Driver is busy
#define CSK_DRIVER_ERROR_TIMEOUT     -3 ///< Timeout occurred
#define CSK_DRIVER_ERROR_UNSUPPORTED -4 ///< Operation not supported
#define CSK_DRIVER_ERROR_PARAMETER   -5 ///< Parameter error
#define CSK_DRIVER_ERROR_SPECIFIC    -6 ///< Start of driver specific errors

/**
 \brief General power states
 */
typedef enum _CSK_POWER_STATE
{
    CSK_POWER_OFF,                        ///< Power off: no operation possible
    CSK_POWER_LOW,                        ///< Low Power mode: retain state, detect and signal wake-up events
    CSK_POWER_FULL                        ///< Power on: full operation at maximum performance
} CSK_POWER_STATE;


/** @addtogroup Exported_macro
  * @{
  */
//#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
//
//#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

#define SWAP_16BIT(x) (((x) & 0x00FF) << 8) | (((x) & 0xFF00) >> 8)

#define SWAP_32BIT(x) (((x) & 0x0000FFFF) << 16) | (((x) & 0xFFFF0000) >> 16)

#ifndef NULL
    #define NULL              (void *)0
#endif

/* =========================================  End of section using anonymous unions  ========================================= */
#if defined (__GNUC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif

/* Macros for memory access operations */
#define _REG8P(p, i)                        ((volatile uint8_t *) ((uintptr_t)((p) + (i))))
#define _REG16P(p, i)                       ((volatile uint16_t *) ((uintptr_t)((p) + (i))))
#define _REG32P(p, i)                       ((volatile uint32_t *) ((uintptr_t)((p) + (i))))
#define _REG64P(p, i)                       ((volatile uint64_t *) ((uintptr_t)((p) + (i))))
#define _REG8(p, i)                         (*(_REG8P(p, i)))
#define _REG16(p, i)                        (*(_REG16P(p, i)))
#define _REG32(p, i)                        (*(_REG32P(p, i)))
#define _REG64(p, i)                        (*(_REG64P(p, i)))
#define REG8(addr)                          _REG8((addr), 0)
#define REG16(addr)                         _REG16((addr), 0)
#define REG32(addr)                         _REG32((addr), 0)
#define REG64(addr)                         _REG64((addr), 0)

/* Macros for address type convert and access operations */
#define ADDR16(addr)                        ((uint16_t)(uintptr_t)(addr))
#define ADDR32(addr)                        ((uint32_t)(uintptr_t)(addr))
#define ADDR64(addr)                        ((uint64_t)(uintptr_t)(addr))
#define ADDR8P(addr)                        ((uint8_t *)(uintptr_t)(addr))
#define ADDR16P(addr)                       ((uint16_t *)(uintptr_t)(addr))
#define ADDR32P(addr)                       ((uint32_t *)(uintptr_t)(addr))
#define ADDR64P(addr)                       ((uint64_t *)(uintptr_t)(addr))

/* Macros for Bit Operations */
#if __riscv_xlen == 32
#define BITMASK_MAX                         0xFFFFFFFFUL
#define BITOFS_MAX                          31
#else
#define BITMASK_MAX                         0xFFFFFFFFFFFFFFFFULL
#define BITOFS_MAX                          63
#endif

// BIT/BITS only support bit mask for __riscv_xlen
// For RISC-V 32 bit, it support mask 32 bit wide
// For RISC-V 64 bit, it support mask 64 bit wide
#define BIT(ofs)                            (0x1UL << (ofs))
#define BITS(start, end)                    ((BITMASK_MAX) << (start) & (BITMASK_MAX) >> (BITOFS_MAX - (end)))
#define GET_BIT(regval, bitofs)             (((regval) >> (bitofs)) & 0x1)
#define SET_BIT(regval, bitofs)             ((regval) |= BIT(bitofs))
#define CLR_BIT(regval, bitofs)             ((regval) &= (~BIT(bitofs)))
#define FLIP_BIT(regval, bitofs)            ((regval) ^= BIT(bitofs))
#define WRITE_BIT(regval, bitofs, val)      CLR_BIT(regval, bitofs); ((regval) |= ((val) << bitofs) & BIT(bitofs))
#define CHECK_BIT(regval, bitofs)           (!!((regval) & (0x1UL<<(bitofs))))
#define GET_BITS(regval, start, end)        (((regval) & BITS((start), (end))) >> (start))
#define SET_BITS(regval, start, end)        ((regval) |= BITS((start), (end)))
#define CLR_BITS(regval, start, end)        ((regval) &= (~BITS((start), (end))))
#define FLIP_BITS(regval, start, end)       ((regval) ^= BITS((start), (end)))
#define WRITE_BITS(regval, start, end, val) CLR_BITS(regval, start, end); ((regval) |= ((val) << start) & BITS((start), (end)))
#define CHECK_BITS_ALL(regval, start, end)  (!((~(regval)) & BITS((start), (end))))
#define CHECK_BITS_ANY(regval, start, end)  ((regval) & BITS((start), (end)))

#define BITMASK_SET(regval, mask)           ((regval) |= (mask))
#define BITMASK_CLR(regval, mask)           ((regval) &= (~(mask)))
#define BITMASK_FLIP(regval, mask)          ((regval) ^= (mask))
#define BITMASK_CHECK_ALL(regval, mask)     (!((~(regval)) & (mask)))
#define BITMASK_CHECK_ANY(regval, mask)     ((regval) & (mask))

/**
  * @}
  */


#endif /* __DRIVER_COMMON_H */

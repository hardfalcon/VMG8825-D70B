#ifndef __MT_CHADR_H__
#define __MT_CHADR_H__

#ifdef  MT_GLOBAL
#define MT_EXTERN
#define MT_I(x) x
#else
#define MT_EXTERN extern
#define MT_I(x)
#endif

/* unit base addresses */
#define MT_LOCAL_MIPS_BASE_ADDRESS           0xb4900000 //0xa7000000 //HUANX
#define GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET       0x001c0000
#define GRX500_BOOTCORE_SYS_RESET_REG_OFFSET         0x00000008
#define GRX500_BOOTCORE_SPI_MODE_ADDR                (MT_LOCAL_MIPS_BASE_ADDRESS+0x130)
#define GRX500_BOOTCORE_SPI_MODE_SW_BIT              (0x400) //When this bit is set, FLASH is accessed in SW mode

#define GRX500_BOOTCORE_RESET_OFFSET                 8
#define GRX500_BOOTCORE_RESET_ADDR                   (MT_LOCAL_MIPS_BASE_ADDRESS+GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET+GRX500_BOOTCORE_RESET_OFFSET)
#define GRX500_BOOTCORE_REBOOT_DATA                  0
#define GRX500_BOOTCORE_REBOOT_BIT_MASK_NOT          0xfffeffff

#define GRX500_BOOTCORE_SHARED_GMAC_BASE_ADDR        (MT_LOCAL_MIPS_BASE_ADDRESS+GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET)
#define GRX500_BOOTCORE_GMAC_MODE_REG_OFFSET         0x68
#define GRX500_BOOTCORE_GMAC_MODE_REG_ADDR           (MT_LOCAL_MIPS_BASE_ADDRESS+GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET+GRX500_BOOTCORE_GMAC_MODE_REG_OFFSET)
#define GRX500_BOOTCORE_GMAC_MODE_2_REG_OFFSET       0x80
#define GRX500_BOOTCORE_GMAC_MODE_2_REG_ADDR         (MT_LOCAL_MIPS_BASE_ADDRESS+GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET+GRX500_BOOTCORE_GMAC_MODE_2_REG_OFFSET)
#define GRX500_BOOTCORE_DLY_PGM_REG_OFFSET           0x64
#define GRX500_BOOTCORE_DLY_PGM_REG_ADDR             (MT_LOCAL_MIPS_BASE_ADDRESS+GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET+GRX500_BOOTCORE_DLY_PGM_REG_OFFSET)
#define GRX500_BOOTCORE_HCYCLE_CALIB_IND_REG_OFFSET  0x90
#define GRX500_BOOTCORE_HCYCLE_CALIB_IND_REG_ADDR    (MT_LOCAL_MIPS_BASE_ADDRESS+GRX500_BOOTCORE_SYS_SHARED_REGS_OFFSET+GRX500_BOOTCORE_HCYCLE_CALIB_IND_REG_OFFSET)


#undef MT_EXTERN
#undef MT_I

#endif /* __MT_CHADR_H__ */

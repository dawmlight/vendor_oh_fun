/*
 * Copyright (c) 2020 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _HI_MCI_REG_H_
#define _HI_MCI_REG_H_


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
 * Controller registers
 */

#define SDHC_DMA_ADDRESS    0x00
#define SDHC_ARGUMENT2        SDHC_DMA_ADDRESS

#define SDHC_BLOCK_SIZE    0x04
#define  SDHC_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))

#define SDHC_BLOCK_COUNT    0x06

#define SDHC_ARGUMENT        0x08

#define SDHC_TRANSFER_MODE    0x0C
#define  SDHC_TRNS_DMA        0x01
#define  SDHC_TRNS_BLK_CNT_EN    0x02
#define  SDHC_TRNS_AUTO_CMD12    0x04
#define  SDHC_TRNS_AUTO_CMD23    0x08
#define  SDHC_TRNS_READ    0x10
#define  SDHC_TRNS_MULTI    0x20

#define SDHC_COMMAND        0x0E
#define  SDHC_CMD_RESP_MASK    0x03
#define  SDHC_CMD_CRC        0x08
#define  SDHC_CMD_INDEX    0x10
#define  SDHC_CMD_DATA        0x20
#define  SDHC_CMD_ABORTCMD    0xC0

#define  SDHC_CMD_RESP_NONE    0x00
#define  SDHC_CMD_RESP_LONG    0x01
#define  SDHC_CMD_RESP_SHORT    0x02
#define  SDHC_CMD_RESP_SHORT_BUSY 0x03

#define SDHC_MAKE_CMD(c, f) (((c & 0xff) << 8) | (f & 0xff))
#define SDHC_GET_CMD(c) ((c>>8) & 0x3f)

#define SDHC_RESPONSE        0x10

#define SDHC_BUFFER        0x20

#define SDHC_PRESENT_STATE    0x24
#define  SDHC_CMD_INHIBIT    0x00000001
#define  SDHC_DATA_INHIBIT    0x00000002
#define  SDHC_DOING_WRITE    0x00000100
#define  SDHC_DOING_READ    0x00000200
#define  SDHC_SPACE_AVAILABLE    0x00000400
#define  SDHC_DATA_AVAILABLE    0x00000800
#define  SDHC_CARD_PRESENT    0x00010000
#define  SDHC_WRITE_PROTECT    0x00080000
#define  SDHC_DATA_LVL_MASK    0x00F00000
#define   SDHC_DATA_LVL_SHIFT    20
#define   SDHC_DATA_0_LVL_MASK    0x00100000

#define SDHC_HOST_CONTROL    0x28
#define  SDHC_CTRL_LED        0x01
#define  SDHC_CTRL_4BITBUS    0x02
#define  SDHC_CTRL_HISPD    0x04
#define  SDHC_CTRL_DMA_MASK    0x18
#define   SDHC_CTRL_SDMA    0x00
#define   SDHC_CTRL_ADMA1    0x08
#define   SDHC_CTRL_ADMA32    0x10
#define   SDHC_CTRL_ADMA64    0x18
#define   SDHC_CTRL_8BITBUS    0x20

#define SDHC_POWER_CONTROL    0x29
#define  SDHC_POWER_ON        0x01
#define  SDHC_POWER_180    0x0A
#define  SDHC_POWER_300    0x0C
#define  SDHC_POWER_330    0x0E

#define SDHC_BLOCK_GAP_CONTROL    0x2A

#define SDHC_WAKE_UP_CONTROL    0x2B
#define  SDHC_WAKE_ON_INT    0x01
#define  SDHC_WAKE_ON_INSERT    0x02
#define  SDHC_WAKE_ON_REMOVE    0x04

#define SDHC_CLOCK_CONTROL    0x2C
#define  SDHC_DIVIDER_SHIFT    8
#define  SDHC_DIVIDER_HI_SHIFT    6
#define  SDHC_DIV_MASK    0xFF
#define  SDHC_DIV_MASK_LEN    8
#define  SDHC_DIV_HI_MASK    0x300
#define  SDHC_PROG_CLOCK_MODE    0x0020
#define  SDHC_CLOCK_PLL_EN     0x0008
#define  SDHC_CLOCK_CARD_EN    0x0004
#define  SDHC_CLOCK_INT_STABLE    0x0002
#define  SDHC_CLOCK_INT_EN    0x0001

#define SDHC_TIMEOUT_CONTROL    0x2E
#define SDHC_DEFINE_TIMEOUT    0xE

#define SDHC_SOFTWARE_RESET    0x2F
#define  SDHC_RESET_ALL    0x01
#define  SDHC_RESET_CMD    0x02
#define  SDHC_RESET_DATA    0x04

// NORMAL_INT_STAT_R
#define SDHC_NOR_INT_STAT 0x30
#define SDHC_NOR_INT_STAT_MASK   0xFFFF

// NORMAL_INT_SIGNAL_EN_R
#define SDHC_NOR_INT_SIGNAL 0x38
#define SDHC_NOR_INT_SIGNAL_DIS 0xFFFF

// ERROR_INT_SIGNAL_EN_R
#define SDHC_ERR_INT_SIGNAL 0x3a
#define SDHC_ERR_INT_SIGNAL_DIS 0xFFFF

#define SDHC_TOUT_CTRL 0x2e

#define SDHC_EMMC_CTRL 0x52c
#define SDHC_EMMC_CTRL_EMMC 0x1
#define SDHC_EMMC_CTRL_ENH_STROBE_EN   (1 << 8)

#define SDHC_PWR_CTRL 0x29
#define SDHC_PWR_CTRL_VDD2 0x1

#define SDHC_CLK_CTRL 0x2c
#define SDHC_CLK_CTRL_PLL (1<<3)
#define SDHC_CLK_CTRL_CLK_EN (1<<2)
#define SDHC_CLK_CTRL_INT_CLK (1<<0)

#define SDHC_INT_STAT_EN 0x34
#define SDHC_INT_STAT_EN_ALL 0xffff

#define SDHC_ERR_INT_STAT 0x32
#define SDHC_ERR_INT_STAT_MASK 0xFFFF

#define SDHC_INT_STATUS    0x30
#define SDHC_INT_ENABLE    0x34
#define SDHC_SIGNAL_ENABLE    0x38
#define  SDHC_INT_RESPONSE    0x00000001
#define  SDHC_INT_DATA_END    0x00000002
#define  SDHC_INT_BLK_GAP    0x00000004
#define  SDHC_INT_DMA_END    0x00000008
#define  SDHC_INT_SPACE_AVAIL    0x00000010
#define  SDHC_INT_DATA_AVAIL    0x00000020
#define  SDHC_INT_CARD_INSERT    0x00000040
#define  SDHC_INT_CARD_REMOVE    0x00000080
#define  SDHC_INT_CARD_INT    0x00000100
#define  SDHC_INT_ERROR    0x00008000
#define  SDHC_INT_TIMEOUT    0x00010000
#define  SDHC_INT_CRC        0x00020000
#define  SDHC_INT_END_BIT    0x00040000
#define  SDHC_INT_INDEX    0x00080000
#define  SDHC_INT_DATA_TIMEOUT    0x00100000
#define  SDHC_INT_DATA_CRC    0x00200000
#define  SDHC_INT_DATA_END_BIT    0x00400000
#define  SDHC_INT_BUS_POWER    0x00800000
#define  SDHC_INT_AUTO_CMD_ERR    0x01000000
#define  SDHC_INT_ADMA_ERROR    0x02000000
// do not set it to the HW register
#define  SDHC_INT_RESCAN_SW    0x80000000

#define  SDHC_INT_NORMAL_MASK    0x00007FFF
#define  SDHC_INT_ERROR_MASK    0xFFFF8000

#define  SDHC_INT_CMD_MASK    (SDHC_INT_RESPONSE | SDHC_INT_TIMEOUT | \
        SDHC_INT_CRC | SDHC_INT_END_BIT | SDHC_INT_INDEX | \
                 SDHC_INT_AUTO_CMD_ERR)

#define  SDHC_INT_DATA_MASK    (SDHC_INT_DATA_END | SDHC_INT_DMA_END | \
        SDHC_INT_DATA_AVAIL | SDHC_INT_SPACE_AVAIL | \
        SDHC_INT_DATA_TIMEOUT | SDHC_INT_DATA_CRC | \
        SDHC_INT_DATA_END_BIT | SDHC_INT_ADMA_ERROR | \
        SDHC_INT_BLK_GAP)

#define SDHC_INT_CMDQ_EN    (0x1 << 14)
#define SDHC_INT_ALL_MASK    ((unsigned int)-1)

#define SDHC_AUTO_CMD_HOSTCTRL2  0x3c
#define SDHC_AUTO_CMD_ERR        0x3C
#define SDHC_AUTO_CMD12_NOT_EXEC    0x0001
#define SDHC_AUTO_CMD_TIMEOUT_ERR    0x0002
#define SDHC_AUTO_CMD_CRC_ERR        0x0004
#define SDHC_AUTO_CMD_ENDBIT_ERR    0x0008
#define SDHC_AUTO_CMD_INDEX_ERR    0x0010
#define SDHC_AUTO_CMD12_NOT_ISSUED    0x0080
#define SDHC_AUTO_CMD_SEL 0x10000000

#define SDHC_HOST_CONTROL2        0x3E
#define  SDHC_CTRL_UHS_MASK        0x0007
#define   SDHC_CTRL_UHS_SDR12        0x0000
#define   SDHC_CTRL_UHS_SDR25        0x0001
#define   SDHC_CTRL_UHS_SDR50        0x0002
#define   SDHC_CTRL_UHS_SDR104        0x0003
#define   SDHC_CTRL_UHS_DDR50        0x0004
#define   SDHC_CTRL_HS_SDR200        0x0005 /* reserved value in SDIO spec */
#define   SDHC_CTRL_HS400        0x0007
#define  SDHC_CTRL_VDD_180        0x0008
#define  SDHC_CTRL_DRV_TYPE_MASK    0x0030
#define   SDHC_CTRL_DRV_TYPE_B        0x0000
#define   SDHC_CTRL_DRV_TYPE_A        0x0010
#define   SDHC_CTRL_DRV_TYPE_C        0x0020
#define   SDHC_CTRL_DRV_TYPE_D        0x0030
#define  SDHC_CTRL_EXEC_TUNING        0x0040
#define  SDHC_CTRL_TUNED_CLK        0x0080
#define  SDHC_CTRL_ASYNC_INT_ENABLE    0x4000
#define  SDHC_CTRL_PRESET_VAL_ENABLE    0x8000

#define SDHC_CAPABILITIES    0x40
#define  SDHC_TIMEOUT_CLK_MASK    0x0000003F
#define  SDHC_TIMEOUT_CLK_SHIFT 0
#define  SDHC_TIMEOUT_CLK_UNIT    0x00000080
#define  SDHC_CLOCK_BASE_MASK    0x00003F00
#define  SDHC_CLOCK_V3_BASE_MASK    0x0000FF00
#define  SDHC_CLOCK_BASE_SHIFT    8
#define  SDHC_MAX_BLOCK_MASK    0x00030000
#define  SDHC_MAX_BLOCK_SHIFT  16
#define  SDHC_CAN_DO_8BIT    0x00040000
#define  SDHC_CAN_DO_ADMA2    0x00080000
#define  SDHC_CAN_DO_ADMA1    0x00100000
#define  SDHC_CAN_DO_HISPD    0x00200000
#define  SDHC_CAN_DO_SDMA    0x00400000
#define  SDHC_CAN_VDD_330    0x01000000
#define  SDHC_CAN_VDD_300    0x02000000
#define  SDHC_CAN_VDD_180    0x04000000
#define  SDHC_CAN_64BIT    0x10000000
#define  SDHC_CAN_ASYNC_INT    0x20000000

#define  SDHC_SUPPORT_SDR50    0x00000001
#define  SDHC_SUPPORT_SDR104    0x00000002
#define  SDHC_SUPPORT_DDR50    0x00000004
#define  SDHC_DRIVER_TYPE_A    0x00000010
#define  SDHC_DRIVER_TYPE_C    0x00000020
#define  SDHC_DRIVER_TYPE_D    0x00000040
#define  SDHC_RETUNING_TIMER_COUNT_MASK    0x00000F00
#define  SDHC_RETUNING_TIMER_COUNT_SHIFT    8
#define  SDHC_USE_SDR50_TUNING            0x00002000
#define  SDHC_RETUNING_MODE_MASK        0x0000C000
#define  SDHC_RETUNING_MODE_SHIFT        14
#define  SDHC_CLOCK_MUL_MASK    0x00FF0000
#define  SDHC_CLOCK_MUL_SHIFT    16
#define  SDHC_CAN_DO_ADMA3     0x8000000
#define  SDHC_SUPPORT_HS400    0x80000000 /* Non-standard */

#define SDHC_CAPABILITIES_1    0x44

#define SDHC_MAX_CURRENT        0x48
#define  SDHC_MAX_CURRENT_LIMIT    0xFF
#define  SDHC_MAX_CURRENT_330_MASK    0x0000FF
#define  SDHC_MAX_CURRENT_330_SHIFT    0
#define  SDHC_MAX_CURRENT_300_MASK    0x00FF00
#define  SDHC_MAX_CURRENT_300_SHIFT    8
#define  SDHC_MAX_CURRENT_180_MASK    0xFF0000
#define  SDHC_MAX_CURRENT_180_SHIFT    16
#define   SDHC_MAX_CURRENT_MULTIPLIER    4

/* 4C-4F reserved for more max current */

#define SDHC_SET_ACMD12_ERROR    0x50
#define SDHC_SET_INT_ERROR    0x52

#define SDHC_ADMA_ERROR    0x54

/* 55-57 reserved */

#define SDHC_HI_SHIFT 32
#define SDHC_ADMA_ADDRESS_LOW    0x58 /* addr[0:31] */
#define SDHC_ADMA_ADDRESS_HIGH    0x5C /* addr[32:63] */

/* 60-FB reserved */
#define SDHC_PRESET_FOR_SDR12 0x66
#define SDHC_PRESET_FOR_SDR25 0x68
#define SDHC_PRESET_FOR_SDR50 0x6A
#define SDHC_PRESET_FOR_SDR104        0x6C
#define SDHC_PRESET_FOR_DDR50 0x6E
#define SDHC_PRESET_FOR_HS400 0x74 /* Non-standard */
#define SDHC_PRESET_DRV_MASK  0xC000
#define SDHC_PRESET_DRV_SHIFT  14
#define SDHC_PRESET_CLKGEN_SEL_MASK   0x400
#define SDHC_PRESET_CLKGEN_SEL_SHIFT    10
#define SDHC_PRESET_SDCLK_FREQ_MASK   0x3FF
#define SDHC_PRESET_SDCLK_FREQ_SHIFT    0

#define SDHC_SLOT_INT_STATUS    0xFC

#define SDHC_HOST_VERSION    0xFE
#define  SDHC_VENDOR_VER_MASK    0xFF00
#define  SDHC_VENDOR_VER_SHIFT    8
#define  SDHC_SPEC_VER_MASK    0x00FF
#define  SDHC_SPEC_VER_SHIFT    0
#define   SDHC_SPEC_100    0
#define   SDHC_SPEC_200    1
#define   SDHC_SPEC_300    2
#define   SDHC_SPEC_400    3
#define   SDHC_SPEC_410    4
#define   SDHC_SPEC_420    5

#define SDHC_MSHC_CTRL             0x508
#define SDHC_CMD_CONFLIT_CHECK     0x01

#define SDHC_CARD_IS_EMMC          0x00000001


#define SDHC_AXI_MBIIU_CTRL        0x510
#define SDHC_GM_WR_OSRC_LMT_MASK   (0x7 << 24)
#define SDHC_GM_WR_OSRC_LMT_SEL(x) ((x) << 24)
#define SDHC_GM_RD_OSRC_LMT_MASK   (0x7 << 16)
#define SDHC_GM_RD_OSRC_LMT_SEL(x) ((x) << 16)
#define SDHC_UNDEFL_INCR_EN        0x1

#define SDHC_GM_WR_OSRC_LMT_VAL ((7) << 24)
#define SDHC_GM_RD_OSRC_LMT_VAL ((7) << 16)

#define SDHC_EMMC_HW_RESET         0x534
#define SDHC_AUTO_TUNING_CTRL      0x540
#define SDHC_SW_TUNING_EN          0x00000010
#define SDHC_AUTO_TUNING_STATUS    0x544

#define SDHC_CENTER_PH_CODE_MASK   0x000000ff
#define SDHC_PHASE_SEL_MASK   0xff


#define SDHC_MULTI_CYCLE       0x54c
#define SDHC_FOUND_EDGE        (0x1 << 11)
#define SDHC_EDGE_DETECT_EN    (0x1 << 8)
#define SDHC_DOUT_EN_F_EDGE    (0x1 << 6)
#define SDHC_DATA_DLY_EN       (0x1 << 3)
#define SDHC_CMD_DLY_EN        (0x1 << 2)

/*
 * End of controller registers.
 */

#define REG_MISC_CTRL14         0x14
#define SDIO0_PWRSW_CTRL_1V8    (0x1 << 8)
#define SDIO0_PWRSW_EN          (0x1 << 9)
#define SDIO0_PWRSW_SEL_1V8     (0x1 << 10)

#define ADMA2_END       0x2

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

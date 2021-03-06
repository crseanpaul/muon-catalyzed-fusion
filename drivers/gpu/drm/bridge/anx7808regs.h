/*
 * Copyright(c) 2012, Analogix Semiconductor. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _SP_TX_REG_DEF_H
#define _SP_TX_REG_DEF_H

#define TX_P0 0x70
#define TX_P1 0x7A
#define TX_P2 0x72

#define RX_P0 0x7E
#define RX_P1 0x80

/***************************************************************/
/* DEV_ADDR = 0x7e , HDMI port register*/
#define HDMI_RX_PORT_SEL_REG 0x7E10
#define DDC_EN 0x10
#define TMDS_EN 0x01

#define HDMI_RX_SRST_REG 0x7E11
#define VIDEO_RST 0x10
#define HDCP_MAN_RST 0x04
#define TMDS_RST 0x02
#define SW_MAN_RST 0x01

#define HDMI_RX_SYS_STATUS_REG 0x7E14
#define PWR5V 0x08
#define TMDS_VSYNC_DET 0x04
#define TMDS_CLOCK_DET 0x02
#define TMDS_DE_DET 0x01

#define HDMI_RX_HDMI_STATUS_REG 0x7E15
#define DEEP_COLOR_MODE 0x40
#define HDMI_AUD_LAYOUT 0x08
#define MUTE_STAT 0x04
#define HDMI_MODE 0x01

#define HDMI_RX_HDMI_MUTE_CTRL_REG 0x7E16
#define MUTE_POL 0x04
#define AUD_MUTE 0x02
#define VID_MUTE 0x01

#define HDMI_RX_SYS_CTRL1_REG 0x7E17

#define HDMI_RX_SYS_PWDN1_REG 0x7E18
#define PWDN_CTRL 0x01

#define HDMI_RX_AEC_CTRL_REG 0x7E20
#define AVC_OE 0x80
#define AAC_OE 0x40
#define AVC_EN 0x02
#define AAC_EN 0x01

#define HDMI_RX_AEC_EN0_REG 0x7E24
#define AEC_EN07 0x80
#define AEC_EN06 0x40
#define AEC_EN05 0x20
#define AEC_EN04 0x10
#define AEC_EN03 0x08
#define AEC_EN02 0x04
#define AEC_EN01 0x02
#define AEC_EN00 0x01

#define HDMI_RX_AEC_EN1_REG 0x7E25
#define AEC_EN15 0x80
#define AEC_EN14 0x40
#define AEC_EN13 0x20
#define AEC_EN12 0x10
#define AEC_EN11 0x08
#define AEC_EN10 0x04
#define AEC_EN09 0x02
#define AEC_EN08 0x01

#define HDMI_RX_AEC_EN2_REG 0x7E26
#define AEC_EN23 0x80
#define AEC_EN22 0x40
#define AEC_EN21 0x20
#define AEC_EN20 0x10
#define AEC_EN19 0x08
#define AEC_EN18 0x04
#define AEC_EN17 0x02
#define AEC_EN16 0x01

#define HDMI_RX_INT_STATUS1_REG 0x7E31
#define HDMI_DVI 0x80
#define CKDT_CHANGE 0x40
#define SCDT_CHANGE 0x20
#define PCLK_CHANGE 0x10
#define PLL_UNLOCK 0x08
#define CABLE_UNPLUG 0x04
#define SET_MUTE 0x02
#define SW_INTR 0x01

#define HDMI_RX_INT_STATUS2_REG 0x7E32
#define AUTH_START 0x80
#define AUTH_DONE 0x40
#define HDCP_ERR 0x20
#define ECC_ERR 0x10

#define HDMI_RX_INT_STATUS3_REG 0x7E33
#define AUD_MODE_CHANGE 0x01

#define HDMI_RX_INT_STATUS4_REG 0x7E34
#define VSYNC_DET 0x80
#define SYNC_POL_CHANGE 0x40
#define V_RES_CHANGE 0x20
#define H_RES_CHANGE 0x10
#define I_P_CHANGE 0x08
#define DP_CHANGE 0x04
#define COLOR_DEPTH_CHANGE 0x02
#define COLOR_MODE_CHANGE 0x01

#define HDMI_RX_INT_STATUS5_REG 0x7E35
#define VFIFO_OVERFLOW 0x80
#define VFIFO_UNDERFLOW 0x40
#define CTS_N_ERR 0x08
#define NO_AVI 0x02
#define AUDIO_RCV 0x01

#define HDMI_RX_INT_STATUS6_REG 0x7E36
#define CTS_RCV 0x80
#define NEW_UNR_PKT 0x40
#define NEW_MPEG 0x20
#define NEW_AUD 0x10
#define NEW_SPD 0x08
#define NEW_ACP 0x04
#define NEW_AVI 0x02
#define NEW_CP 0x01

#define HDMI_RX_INT_STATUS7_REG 0x7E37
#define NO_VSI 0x80
#define HSYNC_DET 0x20
#define NEW_VS 0x10
#define NO_ACP 0x08
#define REF_CLK_CHG 0x04
#define CEC_RX_READY 0x02
#define CEC_TX_DONE 0x01

#define HDMI_RX_PKT_RX_INDU_INT_CTRL 0x7E3F
#define NEW_VS_CTRL 0x80
#define NEW_UNR 0x40
#define NEW_MPEG 0x20
#define NEW_AUD 0x10
#define NEW_SPD 0x08
#define NEW_ACP 0x04
#define NEW_AVI 0x02
#define HDMI_RX_INT_MASK1_REG 0x7E41
#define HDMI_RX_INT_MASK2_REG 0x7E42
#define HDMI_RX_INT_MASK3_REG 0x7E43
#define HDMI_RX_INT_MASK4_REG 0x7E44
#define HDMI_RX_INT_MASK5_REG 0x7E45
#define HDMI_RX_INT_MASK6_REG 0x7E46
#define HDMI_RX_INT_MASK7_REG 0x7E47

#define HDMI_RX_TMDS_CTRL_REG2 0x7E51
#define HDMI_RX_TMDS_CTRL_REG4 0x7E53
#define HDMI_RX_TMDS_CTRL_REG5 0x7E54
#define HDMI_RX_TMDS_CTRL_REG6 0x7E55
#define TERM_PD 0x01
#define HDMI_RX_TMDS_CTRL_REG7 0x7E56
#define HDMI_RX_TMDS_CTRL_REG18 0x7E61
#define PLL_RESET 0x10

#define HDMI_RX_TMDS_CTRL_REG19 0x7E62
#define HDMI_RX_TMDS_CTRL_REG21 0x7E64
#define HDMI_RX_TMDS_CTRL_REG22 0x7E65

#define HDMI_RX_VIDEO_STATUS_REG1 0x7E70
#define COLOR_DEPTH 0xF0
#define DEFAULT_PHASE 0x08
#define VIDEO_TYPE 0x04

#define HDMI_RX_HTOTAL_LOW 0x7E71
#define HDMI_RX_HTOTAL_HIGH 0x7E72
#define HDMI_RX_VTOTAL_LOW 0x7E73
#define HDMI_RX_VTOTAL_HIGH 0x7E74

#define HDMI_RX_HACT_LOW 0x7E75
#define HDMI_RX_HACT_HIGH 0x7E76
#define HDMI_RX_VACT_LOW 0x7E77
#define HDMI_RX_VACT_HIGH 0x7E78

#define HDMI_RX_V_SYNC_WIDTH 0x7E79
#define HDMI_RX_V_BACK_PORCH 0x7E7A
#define HDMI_RX_H_FRONT_PORCH_LOW 0x7E7B
#define HDMI_RX_H_FRONT_PORCH_HIGH 0x7E7C

#define HDMI_RX_H_SYNC_WIDTH_LOW 0x7E7D
#define HDMI_RX_H_SYNC_WIDTH_HIGH 0x7E7E

#define HDMI_RX_VID_DATA_RNG_CTRL_REG 0x7E83
#define YC_LIMT 0x10
#define OUTPUT_LIMIT_EN 0x08
#define OUTPUT_LIMIT_RANGE 0x04
#define R2Y_INPUT_LIMIT 0x02
#define XVYCC_LIMIT 0x01

#define HDMI_RX_VID_OUTPUT_CTRL3_REG 0x7E86

#define HDMI_RX_VID_PCLK_CNTR_REG 0x7E8B

#define HDMI_RX_AUD_IN_CH_STATUS1_REG 0x7EC7
#define HDMI_RX_AUD_IN_CH_STATUS4_REG 0x7ECA

#define HDMI_RX_CEC_CTRL_REG 0x7ED0
#define CEC_RX_EN 0x08
#define CEC_TX_ST 0x04
#define CEC_PIN_SEL 0x02
#define CEC_RST 0x01

#define HDMI_RX_CEC_SPEED_CTRL_REG 0x7ED4
#define CEC_SPEED_27M 0x40

#define HDMI_RX_HDMI_CRITERIA_REG 0x7EE1

#define HDMI_RX_HDCP_EN_CRITERIA_REG 0x7EE2
#define ENC_EN_MODE 0x20

#define HDMI_RX_CHIP_CTRL_REG 0x7EE3
#define MAN_HDMI5V_DET 0x08
#define PLLLOCK_CKDT_EN 0x04
#define ANALOG_CKDT_EN 0x02
#define DIGITAL_CKDT_EN 0x01

/***************************************************************/
/*Register definition of device address 0x80*/

#define HDMI_RX_HDCP_BCAPS 0x802A
#define REPEATER 0x40

#define HDMI_RX_HDCP_BSTATUS_0 0x802B
#define HDMI_RX_HDCP_BSTATUS_1 0x802C

#define HDMI_RX_HDCP_STATUS_REG 0x803F
#define ADV_CIPHER 0x80
#define LOAD_KEY_DONE 0x40
#define DECRYPT_EN 0x20
#define AUTH_EN 0x10
#define BKSV_DISABLE 0x02
#define CLEAR_RI 0x01

#define HDMI_RX_SPD_TYPE_REG 0x8040
#define HDMI_RX_SPD_VER_REG 0x8041
#define HDMI_RX_SPD_LEN_REG 0x8042
#define HDMI_RX_SPD_CHKSUM_REG 0x8043
#define HDMI_RX_SPD_DATA00_REG 0x8044

#define HDMI_RX_ACP_HB0_REG 0x8060
#define HDMI_RX_ACP_HB1_REG 0x8061
#define HDMI_RX_ACP_HB2_REG 0x8062
#define HDMI_RX_ACP_DATA00_REG 0x8063

#define HDMI_RX_AVI_TYPE_REG 0x80A0
#define HDMI_RX_AVI_VER_REG 0x80A1
#define HDMI_RX_AVI_LEN_REG 0x80A2
#define HDMI_RX_AVI_CHKSUM_REG 0x80A3
#define HDMI_RX_AVI_DATA00_REG 0x80A4

#define HDMI_RX_AUDIO_TYPE_REG 0x80C0
#define HDMI_RX_AUDIO_VER_REG 0x80C1
#define HDMI_RX_AUDIO_LEN_REG 0x80C2
#define HDMI_RX_AUDIO_CHKSUM_REG 0x80C3
#define HDMI_RX_AUDIO_DATA00_REG 0x80C4

#define HDMI_RX_MPEG_TYPE_REG 0x80E0
#define HDMI_RX_MPEG_VER_REG 0x80E1
#define HDMI_RX_MPEG_LEN_REG 0x80E2
#define HDMI_RX_MPEG_CHKSUM_REG 0x80E3
#define HDMI_RX_MPEG_DATA00_REG 0x80E4
#define HDMI_RX_MPEG_DATA03_REG 0x80E7
#define HDMI_RX_MPEG_DATA05_REG 0x80E9

#define HDMI_RX_SPD_INFO_CTRL 0x805F
#define HDMI_RX_ACP_INFO_CTRL 0x807F

#define HDMI_RX_GENERAL_CTRL 0x809F
#define SET_AVMUTE 0x01
#define CLEAR_AVMUTE 0x10

#define HDMI_RX_MPEG_VS_CTRL 0x80DF
#define HDMI_RX_MPEG_VS_INFO_CTRL 0x80FF

/***************************************************************/
/*Register definition of device address 0x70*/
#define SP_TX_HDCP_STATUS 0x7000
#define SP_TX_HDCP_AUTH_PASS 0x02

#define SP_TX_HDCP_CTRL0_REG 0x7001
#define STORE_AN 0x80
#define RX_REPEATER 0x40
#define RE_AUTH 0x20
#define SW_AUTH_OK 0x10
#define HARD_AUTH_EN 0x08
#define ENC_EN 0x04
#define BKSV_SRM_PASS 0x02
#define KSVLIST_VLD 0x01

#define SP_TX_HDCP_CTRL1_REG 0x7002
#define AINFO_EN 0x04
#define RCV_11_EN 0x02
#define HDCP_11_EN 0x01

#define SP_TX_HDCP_LINK_CHK_FRAME_NUM 0x7003
#define SP_TX_HDCP_CTRL2_REG 0x7004
#define SP_TX_HDCP_RX_BCAPS 0x701A
#define SP_TX_HDCP_RX_BSTATUS_0 0x701B
#define SP_TX_HDCP_RX_BSTATUS_1 0x701C
#define SP_TX_HDCP_KEY_STATUS 0x701E

#define SP_TX_VID_BLANK_SET1 0x702C
#define SP_TX_VID_BLANK_SET2 0x702D
#define SP_TX_VID_BLANK_SET3 0x702E

#define SP_TX_WAIT_R0_TIME 0x7040
#define SP_TX_LINK_CHK_TIMER 0x7041
#define SP_TX_WAIT_KSVR_TIME 0x7042

#define M_VID_0 0x70C0
#define M_VID_1 0x70C1
#define M_VID_2 0x70C2
#define N_VID_0 0x70C3
#define N_VID_1 0x70C4
#define N_VID_2 0x70C5

#define SP_TX_SYS_CTRL1_REG 0x7080
#define CHIP_AUTH_RESET 0x80
#define PD_BYPASS_CHIP_AUTH 0x40
#define DET_STA 0x04
#define FORCE_DET 0x02
#define DET_CTRL 0x01

#define SP_TX_SYS_CTRL2_REG 0x7081
#define CHA_STA 0x04
#define FORCE_CHA 0x02
#define CHA_CTRL 0x01

#define SP_TX_SYS_CTRL3_REG 0x7082
#define HPD_STATUS 0x40
#define F_HPD 0x20
#define HPD_CTRL 0x10
#define STRM_VALID 0x04
#define F_VALID 0x02
#define VALID_CTRL 0x01

#define SP_TX_SYS_CTRL4_REG 0x7083
#define ENHANCED_MODE 0x08

#define SP_TX_VID_CTRL 0x7084

#define SP_TX_AUD_CTRL 0x7087
#define AUD_EN 0x01

#define I2C_GEN_10US_TIMER0 0x7088
#define I2C_GEN_10US_TIMER1 0x7089

#define SP_TX_PKT_EN_REG 0x7090
#define AUD_IF_UP 0x80
#define AVI_IF_UD 0x40
#define MPEG_IF_UD 0x20
#define SPD_IF_UD 0x10
#define AUD_IF_EN 0x08
#define AVI_IF_EN 0x04
#define MPEG_IF_EN 0x02
#define SPD_IF_EN 0x01

#define SP_TX_HDCP_CTRL 0x7092
#define AUTO_EN 0x80
#define AUTO_START 0x20
#define LINK_POLLING 0x02

#define SP_TX_LINK_BW_SET_REG 0x70A0
#define LINK_5P4G 0x14
#define LINK_2P7G 0x0A
#define LINK_1P62G 0x06

#define SP_TX_TRAINING_PTN_SET_REG 0x70A2
#define SCRAMBLE_DISABLE 0x20

#define SP_TX_LT_SET_REG 0x70A3
#define MAX_PRE_REACH 0x20
#define MAX_DRIVE_REACH 0x04
#define DRVIE_CURRENT_LEVEL1 0x01
#define PRE_EMP_LEVEL1 0x08

#define SSC_CTRL_REG1 0x70A7
#define SPREAD_AMP 0x10
#define MODULATION_FREQ 0x01

#define SP_TX_LT_CTRL_REG 0x70A8
#define SP_TX_LT_EN 0x01

#define SP_TX_DEBUG_REG1 0x70B0
#define FORCE_HPD 0x80
#define HPD_POLLING_DET 0x40
#define HPD_POLLING_EN 0x20
#define DEBUG_PLL_LOCK 0x10
#define FORCE_PLL_LOCK 0x08
#define POLLING_EN 0x02

#define SP_TX_DP_POLLING_PERIOD 0x70B3

#define SP_TX_DP_POLLING_CTRL_REG 0x70B4
#define AUTO_POLLING_DISABLE 0x01

#define SP_TX_LINK_DEBUG_REG 0x70B8
#define M_VID_DEBUG 0x20
#define NEW_PRBS7 0x10
#define INSERT_ER 0x02
#define PRBS31_EN 0x01

#define SP_TX_SINK_STATUS 0x70BE
#define SINK_STATUS_1 0x02
#define SINK_STATUS_0 0x01

#define SP_TX_PLL_CTRL_REG 0x70C7
#define PLL_RST 0x40

#define SP_TX_ANALOG_PD_REG 0x70C8
#define MACRO_PD 0x20
#define AUX_PD 0x10
#define CH0_PD 0x01

#define SP_TX_MISC_CTRL_REG 0x70CD
#define EQ_TRAINING_LOOP 0x40
#define SP_TX_DOWN_SPREADING_CTRL1 0x70D0
#define SP_TX_DOWN_SPREADING_CTRL2 0x70D1
#define SP_TX_DOWN_SPREADING_CTRL3 0x70D2
#define SSC_D_CTRL 0x40
#define FS_CTRL_TH_CTRL 0x20

#define SP_TX_M_CALCU_CTRL 0x70D9
#define M_GEN_CLK_SEL 0x01

#define SP_TX_EXTRA_ADDR_REG 0x70CE
#define I2C_STRETCH_DISABLE 0x80
#define I2C_EXTRA_ADDR 0x50

#define SP_TX_AUX_STATUS 0x70E0
#define AUX_BUSY 0x10

#define SP_TX_BUF_DATA_COUNT_REG 0x70E4
#define BUF_CLR 0x80

#define SP_TX_AUX_CTRL_REG 0x70E5
#define SP_TX_AUX_ADDR_7_0_REG 0x70E6
#define SP_TX_AUX_ADDR_15_8_REG 0x70E7
#define SP_TX_AUX_ADDR_19_16_REG 0x70E8

#define SP_TX_AUX_CTRL_REG2 0x70E9
#define ADDR_ONLY_BIT 0x02
#define AUX_OP_EN 0x01

#define SP_TX_3D_VSC_CTRL 0x70EA
#define INFO_FRAME_VSC_EN 0x01

#define SP_TX_VSC_DB1 0x70EB

#define SP_TX_BUF_DATA_0_REG 0x70F0

/***************************************************************/
/*Register definition of device address 0x72*/
#define SP_TX_VND_IDL_REG 0x7200
#define SP_TX_VND_IDH_REG 0x7201
#define SP_TX_DEV_IDL_REG 0x7202
#define SP_TX_DEV_IDH_REG 0x7203
#define SP_TX_DEV_REV_REG 0x7204

#define SP_POWERD_CTRL_REG 0x7205
#define REGISTER_PD 0x80
#define HDCP_PD 0x20
#define AUDIO_PD 0x10
#define VIDEO_PD 0x08
#define LINK_PD 0x04
#define TOTAL_PD 0x02

#define SP_TX_RST_CTRL_REG 0x7206
#define MISC_RST 0x80
#define VIDCAP_RST 0x40
#define VIDFIF_RST 0x20
#define AUDFIF_RST 0x10
#define AUDCAP_RST 0x08
#define HDCP_RST 0x04
#define SW_RST 0x02
#define HW_RST 0x01

#define SP_TX_RST_CTRL2_REG 0x7207
#define SSC_RST 0x80
#define AUX_RST 0x04
#define SERDES_FIFO_RST 0x02
#define I2C_REG_RST 0x01

#define SP_TX_VID_CTRL1_REG 0x7208
#define VIDEO_EN 0x80
#define VIDEO_MUTE 0x40
#define IN_BIT_SEL 0x04
#define DDR_CTRL 0x02
#define EDGE_CTRL 0x01

#define SP_TX_VID_CTRL2_REG 0x7209
#define IN_BPC_12BIT 0x30
#define IN_BPC_10BIT 0x20
#define IN_BPC_8BIT 0x10

#define SP_TX_VID_CTRL3_REG 0x720A
#define HPD_OUT 0x40

#define SP_TX_VID_CTRL5_REG 0x720C
#define CSC_STD_SEL 0x80
#define RANGE_Y2R 0x20
#define CSPACE_Y2R 0x10

#define SP_TX_VID_CTRL6_REG 0x720D
#define VIDEO_PROCESS_EN 0x40
#define UP_SAMPLE 0x02
#define DOWN_SAMPLE 0x01

#define SP_TX_TOTAL_LINE_STA_L 0x7224
#define SP_TX_TOTAL_LINE_STA_H 0x7225
#define SP_TX_ACT_LINE_STA_L 0x7226
#define SP_TX_ACT_LINE_STA_H 0x7227
#define SP_TX_V_F_PORCH_STA 0x7228
#define SP_TX_V_SYNC_STA 0x7229
#define SP_TX_V_B_PORCH_STA 0x722A
#define SP_TX_TOTAL_PIXEL_STA_L 0x722B
#define SP_TX_TOTAL_PIXEL_STA_H 0x722C
#define SP_TX_ACT_PIXEL_STA_L 0x722D
#define SP_TX_ACT_PIXEL_STA_H 0x722E
#define SP_TX_H_F_PORCH_STA_L 0x722F
#define SP_TX_H_F_PORCH_STA_H 0x7230
#define SP_TX_H_SYNC_STA_L 0x7231
#define SP_TX_H_SYNC_STA_H 0x7232
#define SP_TX_H_B_PORCH_STA_L 0x7233
#define SP_TX_H_B_PORCH_STA_H 0x7234

#define SP_TX_DP_ADDR_REG1 0x723E
#define SP_TX_VID_BIT_CTRL0_REG 0x7240
#define SP_TX_VID_BIT_CTRL10_REG 0x724A
#define SP_TX_VID_BIT_CTRL20_REG 0x7254

#define SP_TX_AVI_TYPE 0x7270
#define SP_TX_AVI_KTYPE 0x82
#define SP_TX_AVI_VER 0x7271
#define SP_TX_AVI_KVER 0x02
#define SP_TX_AVI_LEN 0x7272
#define SP_TX_AVI_KLEN 0x0D
#define SP_TX_AVI_DB0 0x7273

#define SP_TX_AUD_TYPE 0x7283
#define SP_TX_AUD_KTYPE 0x84
#define SP_TX_AUD_VER 0x7284
#define SP_TX_AUD_KVER 0x01
#define SP_TX_AUD_LEN 0x7285
#define SP_TX_AUD_KLEN 0x0A
#define SP_TX_AUD_DB0 0x7286

#define SP_TX_SPD_TYPE 0x7291
#define SP_TX_SPD_VER 0x7292
#define SP_TX_SPD_LEN 0x7293
#define SP_TX_SPD_DB0 0x7294

#define SP_TX_MPEG_TYPE 0x72B0
#define SP_TX_MPEG_VER 0x72B1
#define SP_TX_MPEG_LEN 0x72B2
#define SP_TX_MPEG_DB0 0x72B3

#define SP_TX_AUD_CH_STATUS_REG1 0x72D0

#define SP_TX_AUD_CH_NUM_REG5 0x72D5
#define CH_NUM_8 0xE0
#define AUD_LAYOUT 0x01

#define SP_TX_ANALOG_DEBUG_REG2 0x72DD
#define POWERON_TIME_1P5MS 0x06

#define SP_TX_PLL_FILTER_CTRL11 0x72DF
#define PD_RING_OSC 0x40
#define AUX_TERM_50OHM 0x30
#define V33_SWITCH_ON 0x02

#define SP_TX_PLL_FILTER_CTRL6 0x72E4
#define P5V_PROTECT 0x80
#define SHORT_PROTECT 0x40
#define P5V_PROTECT_PD 0x20
#define SHORT_PROTECT_PD 0x10

#define SP_TX_ANALOG_CTRL 0x72E5

#define SP_COMMON_INT_STATUS1 0x72F1
#define PLL_LOCK_CHG 0x40
#define VIDEO_FORMAT_CHG 0x08
#define AUDIO_CLK_CHG 0x04
#define VIDEO_CLOCK_CHG 0x02

#define SP_COMMON_INT_STATUS2 0x72F2
#define HDCP_AUTH_CHG 0x02
#define HDCP_AUTH_DONE 0x01

#define SP_COMMON_INT_STATUS3 0x72F3
#define HDCP_LINK_CHK_FAIL 0x01

#define SP_COMMON_INT_STATUS4 0x72F4
#define PLUG 0x01
#define ESYNC_ERR 0x10
#define HPD_LOST 0x02
#define HPD_CHANGE 0x04

#define SP_TX_INT_STATUS1 0x72F7
#define DPCD_IRQ_REQUEST 0x80
#define HPD 0x40
#define TRAINING_FINISH 0x20
#define POLLING_ERR 0x10
#define LINK_CHANGE 0x04
#define SINK_CHG 0x08

#define SP_COMMON_INT_MASK1 0x72F8
#define SP_COMMON_INT_MASK2 0x72F9
#define SP_COMMON_INT_MASK3 0x72FA
#define SP_COMMON_INT_MASK4 0x72FB
#define SP_INT_MASK 0x72FE
#define SP_TX_INT_CTRL_REG 0x72FF

/***************************************************************/
/*Register definition of device address 0x7A*/

#define SP_TX_LT_CTRL_REG0 0x7A30
#define SP_TX_LT_CTRL_REG1 0x7A31
#define SP_TX_LT_CTRL_REG2 0x7A34
#define SP_TX_LT_CTRL_REG3 0x7A35
#define SP_TX_LT_CTRL_REG4 0x7A36
#define SP_TX_LT_CTRL_REG5 0x7A37
#define SP_TX_LT_CTRL_REG6 0x7A38
#define SP_TX_LT_CTRL_REG7 0x7A39
#define SP_TX_LT_CTRL_REG8 0x7A3A
#define SP_TX_LT_CTRL_REG9 0x7A3B
#define SP_TX_LT_CTRL_REG10 0x7A40
#define SP_TX_LT_CTRL_REG11 0x7A41
#define SP_TX_LT_CTRL_REG12 0x7A44
#define SP_TX_LT_CTRL_REG13 0x7A45
#define SP_TX_LT_CTRL_REG14 0x7A46
#define SP_TX_LT_CTRL_REG15 0x7A47
#define SP_TX_LT_CTRL_REG16 0x7A48
#define SP_TX_LT_CTRL_REG17 0x7A49
#define SP_TX_LT_CTRL_REG18 0x7A4A
#define SP_TX_LT_CTRL_REG19 0x7A4B

#define SP_TX_AUD_INTERFACE_CTRL0 0x7A5F
#define AUD_INTERFACE_DISABLE 0x80

#define SP_TX_AUD_INTERFACE_CTRL2 0x7A60
#define M_AUD_ADJUST_ST 0x04

#define SP_TX_AUD_INTERFACE_CTRL3 0x7A62
#define SP_TX_AUD_INTERFACE_CTRL4 0x7A67
#define SP_TX_AUD_INTERFACE_CTRL5 0x7A68
#define SP_TX_AUD_INTERFACE_CTRL6 0x7A69

/***************************************************************/
/*AUX channel*/

#define AUX_DPCD 0x08
#define AUX_DDC 0x00

/***************************************************************/
/*Definition of DPCD*/

#define DPCD_REV 0x00000
#define MAX_LINK_RATE 0x00001
#define MAX_LINE_COUNT 0x00002
#define MAX_DOWNSPREAD 0x00003
#define DP_PWR_VOLTAGE_CAP 0x00004
#define DOWNSTREAMPORT_PRESENT 0x00005
#define DOWNSTREAMPORT_TYPE 0x06
#define DOWNSTREAMPORT_DP 0x00
#define DOWNSTREAMPORT_VGA 0x02
#define DOWNSTREAMPORT_HDMI 0x04

#define MAIN_LINK_CHANNEL_CODING 0x00006
#define DOWN_STM_PORT 0x00007
#define RECEIVE_PORT0_CAP_0 0x00008
#define RECEIVE_PORT0_CAP_1 0x00009
#define RECEIVE_PORT1_CAP_0 0x0000a
#define RECEIVE_PORT1_CAP_1 0x0000b
#define I2C_RATE_CONTROL 0x0000c
#define DWN_STRM_PORT0_CAP_BYTE0 0x00080
#define DWN_STRM_PORT0_CAP_BYTE1 0x00081
#define DWN_STRM_PORT0_CAP_BYTE2 0x00082
#define LINK_BW_SET 0x00100
#define LANE_COUNT_SET 0x00101
#define TRAINING_PATTERN_SET 0x00102
#define TRAINING_LANE0_SET 0x00103
#define DOWNSPREAD_CTRL 0x00107
#define MAIN_LINK_CHANNEL_CODING_SET 0x00108
#define SINK_COUNT 0x00200
#define DEVICE_SERVICE_IRQ_VECTOR 0x00201
#define CP_IRQ 0x04
#define SINK_SPECIFIC_IRQ 0x40
#define LANE0_1_STATUS 0x00202
#define LANE0_1_STATUS_SUCCESS 0x07

#define SINK_COUNT 0x00200
#define SINK_COUNT_MASK 0x3F
#define LANE_ALIGH_STUTUS_UPDATED 0x00204
#define SINK_STATUS 0x00205
#define AUDIO_STABLE 0x02
#define VIDEO_STABLE 0x01
#define ADJUST_REQUEST_LANE0_1 0x00206
#define SYMBOL_ERROR_COUNT_LANE0_0 0x00210
#define SYMBOL_ERROR_COUNT_LANE0_1 0x00211
#define SOURCE_IEEE_OUI_1 0x00300
#define SOURCE_IEEE_OUI_2 0x00301
#define SOURCE_IEEE_OUI_3 0x00302
#define SINK_IEEE_OUI_1 0x00400
#define SINK_IEEE_OUI_2 0x00401
#define SINK_IEEE_OUI_3 0x00402
#define BRANCH_IEEE_OUI_1 0x00500
#define BRANCH_IEEE_OUI_2 0x00501
#define BRANCH_IEEE_OUI_3 0x00502
#define SPECIFIC_INTERRUPT_1 0x00510
#define DWN_STREAM_HDCP_LINK_INTEGRITY_FAIL 0x20
#define DWN_STREAM_HDCP_FAIL 0x10
#define DWN_STREAM_HDCP_DONE 0x08
#define DWN_STREAM_DISCONNECTED 0x02
#define DWN_STREAM_CONNECTED 0x01
#define SPECIFIC_INTERRUPT_2 0x00511
#define DOWN_STREAM_STATUS_1 0x00518
#define DOWN_R_TERM_DET 0x40
#define SRAM_EEPROM_LOAD_DONE 0x20
#define SRAM_CRC_CHK_DONE 0x10
#define SRAM_CRC_CHK_PASS 0x08
#define DOWN_STRM_ENC 0x04
#define DOWN_STRM_AUTH 0x02
#define DOWN_STRM_HPD 0x01

#define DOWN_STREAM_STATUS_2 0x00519
#define US_COMM_TX_INT 0x00520
#define US_COMM_RX_INT 0x00521
#define US_COMM_1 0x00522
#define US_COMM_2 0x00523
#define US_COMM_3 0x00524
#define US_COMM_4 0x00525
#define US_COMM_5 0x00526
#define US_COMM_6 0x00527
#define US_COMM_7 0x00528
#define US_COMM_8 0x00529
#define CHIP_AUTHENTICATION 0x00533
#define ENCRYPTED_DATA_1 0x00540
#define ENCRYPTED_DATA_2 0x00541
#define ENCRYPTED_DATA_3 0x00542
#define ENCRYPTED_DATA_4 0x00543
#define ENCRYPTED_DATA_5 0x00544
#define ENCRYPTED_DATA_6 0x00545
#define ENCRYPTED_DATA_7 0x00546
#define ENCRYPTED_DATA_8 0x00547
#define ENCRYPTED_DATA_9 0x00548
#define ENCRYPTED_DATA_10 0x00549
#define ENCRYPTED_DATA_11 0x0054A
#define ENCRYPTED_DATA_12 0x0054B
#define ENCRYPTED_DATA_13 0x0054C
#define ENCRYPTED_DATA_14 0x0054D
#define ENCRYPTED_DATA_15 0x0054E
#define ENCRYPTED_DATA_16 0x0054F
#define BRANCH_CHECKSUM_L 0x00560
#define BRANCH_CHECKSUM_H 0x00561
#define CEC_CONTROL 0x00570
#define CEC_RX_STATUS 0x00571
#define CEC_TX_STATUS 0x00572
#define CEC_SPEED_CONTROL 0x00573
#define CEC_FIFO 0x00580
#define SINK_DEV_SEL 0x005f0
#define SINK_ACCESS_OFFSET 0x005f1
#define SINK_ACCESS_REG 0x005f2
#define GPIO_I2C_SEL 0x005f3
#define SET_DP_PWR_VOLTAGE 0x00600
/*0x68000-0x68FFF used for HDCP*/
#define BKSV_1 0x68000
#define BKSV_2 0x68001
#define BKSV_3 0x68002
#define BKSV_4 0x68003
#define BKSV_5 0x68004
#define HDCP_R0_1 0x68005
#define HDCP_R0_2 0x68006
#define SOURCE_AKSV_1 0x68007
#define SOURCE_AKSV_2 0x68008
#define SOURCE_AKSV_3 0x68009
#define SOURCE_AKSV_4 0x6800a
#define SOURCE_AKSV_5 0x6800b
#define SOURCE_AN_1 0x6800c
#define SOURCE_AN_2 0x6800d
#define SOURCE_AN_3 0x6800e
#define SOURCE_AN_4 0x6800f
#define SOURCE_AN_5 0x68010
#define SOURCE_AN_6 0x68011
#define SOURCE_AN_7 0x68012
#define SOURCE_AN_8 0x68013
#define H0_1 0x68014
#define H0_2 0x68015
#define H0_3 0x68016
#define H0_4 0x68017
#define H1_1 0x68018
#define H1_2 0x68019
#define H1_3 0x6801a
#define H1_4 0x6801b
#define H2_1 0x6801c
#define H2_2 0x6801d
#define H2_3 0x6801e
#define H2_4 0x6801f
#define H3_1 0x68020
#define H3_2 0x68021
#define H3_3 0x68022
#define H3_4 0x68023
#define H4_1 0x68024
#define H4_2 0x68025
#define H4_3 0x68026
#define H4_4 0x68027
#define BCAPS 0x68028
#define HDCP_CAPABLE 0x01
#define BSTATUS 0x68029
#define LINK_INTEGRITY_FAILURE 0x40
#define BINFO_L 0x6802a
#define MAX_DEVS_EXCEEDED 0x80
#define BINFO_H 0x6802b
#define MAX_CASCADE_EXCEEDED 0x08
#define KSV_FIFO_1 0x6802c
#define KSV_FIFO_2 0x6802d
#define KSV_FIFO_3 0x6802e
#define KSV_FIFO_4 0x6802f
#define KSV_FIFO_5 0x68030
#define KSV_FIFO_6 0x68031
#define KSV_FIFO_7 0x68032
#define KSV_FIFO_8 0x68033
#define KSV_FIFO_9 0x68034
#define KSV_FIFO_10 0x68035
#define KSV_FIFO_11 0x68036
#define KSV_FIFO_12 0x68037
#define KSV_FIFO_13 0x68038
#define KSV_FIFO_14 0x68039
#define KSV_FIFO_15 0x6803a
#define HPD_DP_PWR_STATUS 0xfff00
#define AUX_HPD_BRIDGE_POWER_CONTROL 0xfff01

#endif

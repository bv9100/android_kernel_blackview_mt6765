/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __MT65XX_LCM_LIST_H__
#define __MT65XX_LCM_LIST_H__


extern struct LCM_DRIVER hct_rm67120_dsi_vdo_hd_gvo;
extern struct LCM_DRIVER hct_otm1282a_dsi_vdo_hd_auo_50_ll;
extern struct LCM_DRIVER hct_jd9366d_dsi_vdo_hdp_panda_57_dzx;
extern struct LCM_DRIVER hct_ili9881d_dsi_vdo_hdp_ivo_57_hlt;
extern struct LCM_DRIVER hct_nt36672a_dsi_vdo_fhp_hx_60_xld;
extern struct LCM_DRIVER hct_jd9365z_dsi_vdo_hdp_sc_57_hz;
extern struct LCM_DRIVER hct_nt35596_dsi_vdo_fhd_auo_60_rx;
extern struct LCM_DRIVER hct_ft8006p_dsi_vdo_hdp_tm_62_hl;	//add by micheal 20181226
extern struct LCM_DRIVER hct_ili7807d_dsi_vdo_fhdplus_auto_72_hz;	//add by wkp 20190104
extern struct LCM_DRIVER hct_ft8006p_dsi_vdo_hdp_hx_62_wj;
extern struct LCM_DRIVER hct_ft8719p_dsi_vdo_fhdplus_jdi_63_hlt;	//add by wkp 20190110
extern struct LCM_DRIVER hct_td4330_dsi_vdo_hdp_jdi_628_xhwy;
extern struct LCM_DRIVER hct_ft8719p_dsi_vdo_fhdplus_jdi_63_baoxu;
extern struct LCM_DRIVER hct_nt36672_dsi_vdo_fhd_auo_60_ykl;
extern struct LCM_DRIVER hct_ft8719_dsi_vdo_fhdplus_auo_63_by;
extern struct LCM_DRIVER hct_ft8719p_dsi_vdo_fhdplus_auo_63_wcl;
extern struct LCM_DRIVER hct_jd9365z_dsi_vdo_hdp_ctc_57_dzx;
extern struct LCM_DRIVER hct_ft8006u_dsi_vdo_hdp_hjc_55_hf;
extern struct LCM_DRIVER hct_ft8009a_dsi_vdo_hdp_tm_641_wcl;
extern struct LCM_DRIVER hct_ft8009a_dsi_vdo_hdp_tm_641_dzx;
extern struct LCM_DRIVER hct_nt36526h_dsi_vdo_hdp_tm_641_wcl;
extern struct LCM_DRIVER hct_ft8009a_dsi_vdo_hdp_tm_641_ykl;
extern struct LCM_DRIVER hct_nt36672_dsi_vdo_fhd_tm_63_xld;
extern struct LCM_DRIVER hct_ili9881d_dsi_vdo_hdp_hjc_57_hlt;
extern struct LCM_DRIVER hct_jd9365z_dsi_vdo_hdp_sc_57_bh;
extern struct LCM_DRIVER hct_st7703_dsi_vdo_hdp_sc_57_gz;
extern struct LCM_DRIVER hct_ili9881d_dsi_vdo_hdp_ivo_57_dzx;
extern struct LCM_DRIVER hct_icnl991_dsi_vdo_hdp_ivo_62_kl;
extern struct LCM_DRIVER hct_jd9365z_dsi_vdo_hdp_sc_57_kl;
extern struct LCM_DRIVER hct_icnl9911_dsi_vdo_hdp_hjc_62_hz;
struct LCM_DRIVER *lcm_driver_list[] = {


#if defined(HCT_RM67120_DSI_VDO_HD_GVO)
	&hct_rm67120_dsi_vdo_hd_gvo,
#endif
#if defined(HCT_OTM1282A_DSI_VDO_HD_AUO_50_LL)
	&hct_otm1282a_dsi_vdo_hd_auo_50_ll,
#endif
#if defined(HCT_JD9366D_DSI_VDO_HDP_PANDA_57_DZX)
	&hct_jd9366d_dsi_vdo_hdp_panda_57_dzx,
#endif
#if defined(HCT_ILI9881D_DSI_VDO_HDP_IVO_57_HLT)
	&hct_ili9881d_dsi_vdo_hdp_ivo_57_hlt,
#endif
#if defined(HCT_NT36672A_DSI_VDO_FHP_HX_60_XLD)
	&hct_nt36672a_dsi_vdo_fhp_hx_60_xld,
#endif
#if defined(HCT_JD9365Z_DSI_VDO_HDP_SC_57_HZ)
	&hct_jd9365z_dsi_vdo_hdp_sc_57_hz,
#endif
#if defined(HCT_NT35596_DSI_VDO_FHD_AUO_60_RX)
	&hct_nt35596_dsi_vdo_fhd_auo_60_rx,
#endif
#if defined(HCT_FT8006P_DSI_VDO_HDP_TM_62_HL)
	&hct_ft8006p_dsi_vdo_hdp_tm_62_hl,
#endif
#if defined(HCT_ILI7807D_DSI_VDO_FHDPLUS_AUTO_72_HZ)
	&hct_ili7807d_dsi_vdo_fhdplus_auto_72_hz,
#endif
#if defined(HCT_FT8006P_DSI_VDO_HDP_HX_62_WJ)
	&hct_ft8006p_dsi_vdo_hdp_hx_62_wj,
#endif
#if defined(HCT_FT8719P_DSI_VDO_FHDPLUS_JDI_63_HLT)
	&hct_ft8719p_dsi_vdo_fhdplus_jdi_63_hlt,
#endif
#if defined(HCT_TD4330_DSI_VDO_HDP_JDI_628_XHWY)
	&hct_td4330_dsi_vdo_hdp_jdi_628_xhwy,
#endif
#if defined(HCT_FT8719P_DSI_VDO_FHDPLUS_JDI_63_BAOXU)
	&hct_ft8719p_dsi_vdo_fhdplus_jdi_63_baoxu,
#endif
#if defined(HCT_NT36672_DSI_VDO_FHD_AUO_60_YKL)
	&hct_nt36672_dsi_vdo_fhd_auo_60_ykl,
#endif
#if defined(HCT_FT8719_DSI_VDO_FHDPLUS_AUO_63_BY)
	&hct_ft8719_dsi_vdo_fhdplus_auo_63_by,
#endif
#if defined(HCT_FT8719P_DSI_VDO_FHDPLUS_AUO_63_WCL)
	&hct_ft8719p_dsi_vdo_fhdplus_auo_63_wcl,
#endif
#if defined(HCT_JD9365Z_DSI_VDO_HDP_CTC_57_DZX)
	&hct_jd9365z_dsi_vdo_hdp_ctc_57_dzx,
#endif
#if defined(HCT_FT8006U_DSI_VDO_HDP_HJC_55_HF)
	&hct_ft8006u_dsi_vdo_hdp_hjc_55_hf,
#endif
#if defined(HCT_FT8009A_DSI_VDO_HDP_TM_641_WCL)
	&hct_ft8009a_dsi_vdo_hdp_tm_641_wcl,
#endif
#if defined(HCT_FT8009A_DSI_VDO_HDP_TM_641_DZX)
	&hct_ft8009a_dsi_vdo_hdp_tm_641_dzx,
#endif
#if defined(HCT_NT36526H_DSI_VDO_HDP_TM_641_WCL)
	&hct_nt36526h_dsi_vdo_hdp_tm_641_wcl,
#endif
#if defined(HCT_FT8009A_DSI_VDO_HDP_TM_641_YKL)
	&hct_ft8009a_dsi_vdo_hdp_tm_641_ykl,
#endif
#if defined(HCT_NT36672_DSI_VDO_FHD_TM_63_XLD)
	&hct_nt36672_dsi_vdo_fhd_tm_63_xld,
#endif
#if defined(HCT_ILI9881D_DSI_VDO_HDP_HJC_57_HLT)
	&hct_ili9881d_dsi_vdo_hdp_hjc_57_hlt,
#endif
#if defined(HCT_JD9365Z_DSI_VDO_HDP_SC_57_BH)
	&hct_jd9365z_dsi_vdo_hdp_sc_57_bh,
#endif
#if defined(HCT_ST7703_DSI_VDO_HDP_SC_57_GZ)
	&hct_st7703_dsi_vdo_hdp_sc_57_gz,
#endif
#if defined(HCT_ILI9881D_DSI_VDO_HDP_IVO_57_DZX)
	&hct_ili9881d_dsi_vdo_hdp_ivo_57_dzx,
#endif
#if defined(HCT_ICNL991_DSI_VDO_HDP_IVO_62_KL)
	&hct_icnl991_dsi_vdo_hdp_ivo_62_kl,
#endif
#if defined(HCT_JD9365Z_DSI_VDO_HDP_SC_57_KL)
	&hct_jd9365z_dsi_vdo_hdp_sc_57_kl,
#endif
#if defined(HCT_ICNL9911_DSI_VDO_HDP_HJC_62_HZ)
	&hct_icnl9911_dsi_vdo_hdp_hjc_62_hz,
#endif
};

#ifdef BUILD_LK
extern void mdelay(unsigned long msec);
#endif

#endif

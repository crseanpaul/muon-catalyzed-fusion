/* exynos_drm_fimd.c
 *
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Authors:
 *	Joonyoung Shim <jy0922.shim@samsung.com>
 *	Inki Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <drm/drmP.h>
#include <drm/drm_atomic.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/pm_runtime.h>

#include <video/samsung_fimd.h>
#include <drm/exynos_drm.h>

#include "exynos_dp_core.h"
#include "exynos_drm_drv.h"
#include "exynos_drm_fb.h"
#include "exynos_drm_fbdev.h"
#include "exynos_drm_crtc.h"
#include "exynos_drm_gem.h"
#include "exynos_drm_iommu.h"

/*
 * FIMD is stand for Fully Interactive Mobile Display and
 * as a display controller, it transfers contents drawn on memory
 * to a LCD Panel through Display Interfaces such as RGB or
 * CPU Interface.
 */

/* position control register for hardware window 0, 2 ~ 4.*/
#define VIDOSD_A(win)		(VIDOSD_BASE + 0x00 + (win) * 16)
#define VIDOSD_B(win)		(VIDOSD_BASE + 0x04 + (win) * 16)
/* size control register for hardware window 0. */
#define VIDOSD_C_SIZE_W0	(VIDOSD_BASE + 0x08)
/* alpha control register for hardware window 1 ~ 4. */
#define VIDOSD_C(win)		(VIDOSD_BASE + 0x18 + (win) * 16)
/* size control register for hardware window 1 ~ 4. */
#define VIDOSD_D(win)		(VIDOSD_BASE + 0x0C + (win) * 16)

#define VIDWx_BUF_START(win, buf)	(VIDW_BUF_START(buf) + (win) * 8)
#define VIDWx_BUF_START_S(win, buf)	(VIDW_BUF_START_S(buf) + (win) * 8)
#define VIDWx_BUF_END(win, buf)		(VIDW_BUF_END(buf) + (win) * 8)
#define VIDWx_BUF_SIZE(win, buf)	(VIDW_BUF_SIZE(buf) + (win) * 4)

/* color key control register for hardware window 1 ~ 4. */
#define WKEYCON0_BASE(x)		((WKEYCON0 + 0x140) + (x * 8))
/* color key value register for hardware window 1 ~ 4. */
#define WKEYCON1_BASE(x)		((WKEYCON1 + 0x140) + (x * 8))

/* FIMD has totally five hardware windows. */
#define FIMD_WIN_NR	5

static const uint32_t plane_formats[] = {
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_ARGB8888,
};

#define exynos_plane_to_win_idx(ctx, x) (unsigned int)(x - ctx->planes)
#define get_fimd_context(dev)	platform_get_drvdata(to_platform_device(dev))

struct fimd_driver_data {
	unsigned int timing_base;
};

static struct fimd_driver_data exynos4_fimd_driver_data = {
	.timing_base = 0x0,
};

static struct fimd_driver_data exynos5_fimd_driver_data = {
	.timing_base = 0x20000,
};

struct fimd_mode_data {
	unsigned		vtotal;
	unsigned		vdisplay;
	unsigned		vsync_len;
	unsigned		vbpd;
	unsigned		vfpd;
	unsigned		htotal;
	unsigned		hdisplay;
	unsigned		hsync_len;
	unsigned		hbpd;
	unsigned		hfpd;
	u32			clkdiv;
};

struct fimd_context {
	struct device			*dev;
	struct drm_device		*drm_dev;
	struct exynos_drm_plane		planes[FIMD_WIN_NR];
	struct clk			*sclk_mout_fimd;
	struct clk			*fimd_mux_clk;
	struct clk			*bus_clk;
	struct clk			*lcd_clk;
	unsigned long			lcd_clk_rate;
	void __iomem			*regs;
	void __iomem			*regs_mie;
	void __iomem			*regs_timing;
	struct fimd_mode_data		mode;
	unsigned int			default_win;
	u32				vidcon0;
	u32				vidcon1;
	bool				suspended;
	int				pipe;
	wait_queue_head_t		wait_vsync_queue;
	atomic_t			wait_vsync_event;
	enum dither_mode		dither_mode;
	u32				dither_rgb;
	unsigned int			irq;
};

static struct device *dp_dev;

static void fimd_wait_for_vblank(struct fimd_context *ctx);

void exynos_fimd_dp_attach(struct device *dev)
{
	DRM_DEBUG_KMS("%s\n", dev_name(dev));
	dp_dev = dev;
}

static const struct of_device_id fimd_driver_dt_match[] = {
	{ .compatible = "samsung,exynos4-fimd",
	  .data = &exynos4_fimd_driver_data },
	{ .compatible = "samsung,exynos5-fimd",
	  .data = &exynos5_fimd_driver_data },
	{},
};
MODULE_DEVICE_TABLE(of, fimd_driver_dt_match);

static inline struct fimd_driver_data *drm_fimd_get_driver_data(
	struct platform_device *pdev)
{
	const struct of_device_id *of_id =
			of_match_device(fimd_driver_dt_match, &pdev->dev);

	DRM_DEBUG_KMS("[PDEV:%s]\n", pdev->name);

	if (of_id)
		return (struct fimd_driver_data *)of_id->data;

	return (struct fimd_driver_data *)
		platform_get_device_id(pdev)->driver_data;
}

static void fimd_win_set_colkey(struct fimd_context *ctx, unsigned int win)
{
	unsigned int keycon0 = 0, keycon1 = 0;

	DRM_DEBUG_KMS("[WIN:%u]\n", win);

	keycon0 = ~(WxKEYCON0_KEYBL_EN | WxKEYCON0_KEYEN_F |
			WxKEYCON0_DIRCON) | WxKEYCON0_COMPKEY(0);

	keycon1 = WxKEYCON1_COLVAL(0xffffffff);

	writel(keycon0, ctx->regs + WKEYCON0_BASE(win));
	writel(keycon1, ctx->regs + WKEYCON1_BASE(win));
}

static int fimd_parse_dither_rgb(struct fimd_context *ctx,
		const u32 rgb_bpc[3])
{
	u32 val, reg = 0;
	int i;

	for (i = 0; i < 3; i++) {
		val = 0;
		switch (rgb_bpc[i]) {
		case 5:
			val |= 2;
			break;
		case 6:
			val |= 1;
			break;
		case 8:
			break;
		default:
			DRM_ERROR("FIMD: Unsupported bpc value, %d\n",
					rgb_bpc[i]);
			return -EINVAL;
		}
		val <<= (2 * (2 - i));
		reg |= val;
	}

	ctx->dither_rgb = reg << 1;

	return 0;
}

static void fimd_set_dithering(struct fimd_context *ctx)
{
	writel(ctx->dither_rgb | DITHMODE_DITH_EN, ctx->regs + DITHMODE);
}

static void mie_set_6bit_dithering(struct fimd_context *ctx)
{
	struct fimd_mode_data *mode = &ctx->mode;
	int i;

	writel(MIE_HRESOL(mode->hdisplay) | MIE_VRESOL(mode->vdisplay) |
			MIE_MODE_UI, ctx->regs_mie + MIE_CTRL1);

	writel(MIE_WINHADDR0(0) | MIE_WINHADDR1(mode->hdisplay),
			ctx->regs_mie + MIE_WINHADDR);

	writel(MIE_WINVADDR0(0) | MIE_WINVADDR1(mode->vdisplay),
		ctx->regs_mie + MIE_WINVADDR);

	writel(PWMCLKCNT(mode->vtotal * mode->htotal /
		(MIE_PWMCLKVAL + 1)), ctx->regs_mie + MIE_PWMCLKCNT);

	writel(MIE_VBPD(mode->vbpd) | MIE_VFPD(mode->vfpd) |
		MIE_VSPW(mode->vsync_len), ctx->regs_mie + MIE_PWMVIDTCON1);

	writel(MIE_HBPD(mode->hbpd) | MIE_HFPD(mode->hbpd) |
		MIE_HSPW(mode->hsync_len), ctx->regs_mie + MIE_PWMVIDTCON2);

	writel(MIE_DITHCON_EN | MIE_RGB6MODE, ctx->regs_mie + MIE_AUXCON);

	/* Bypass MIE image brightness enhancement */
	for (i = 0; i <= 0x30; i += 4) {
		writel(0, ctx->regs_mie + 0x100 + i);
		writel(0, ctx->regs_mie + 0x200 + i);
	}
}

static void exynos_set_dithering(struct fimd_context *ctx)
{
	if (ctx->dither_mode == USE_FIMD_DITHERING)
		fimd_set_dithering(ctx);
	else if (ctx->dither_mode == USE_MIE_DITHERING)
		mie_set_6bit_dithering(ctx);
}

static void fimd_dither_enable(struct fimd_context *ctx)
{
	if (ctx->dither_mode == USE_MIE_DITHERING)
		writel(MIE_CLK_ENABLE, ctx->regs + DPCLKCON);
}

static void fimd_dither_disable(struct fimd_context *ctx)
{
	if (ctx->dither_mode == USE_MIE_DITHERING)
		writel(0, ctx->regs + DPCLKCON);
}

/*
 * Schedule a plane (hardware overlay) to be disabled at the next vblank.
 * This is useful when disabling multiple windows, for example during suspend.
 */
static void fimd_plane_disable_nowait(struct drm_plane *plane)
{
	struct exynos_drm_plane *exynos_plane = to_exynos_plane(plane);
	struct fimd_context *ctx = exynos_plane->ctx;
	int win = exynos_plane_to_win_idx(ctx, exynos_plane);
	u32 val;

	DRM_DEBUG_KMS("[WIN:%d]\n", win);

	if (ctx->suspended)
		return;

	/* protect windows */
	val = readl(ctx->regs + SHADOWCON);
	val |= SHADOWCON_WINx_PROTECT(win);
	writel(val, ctx->regs + SHADOWCON);

	/* wincon */
	val = readl(ctx->regs + WINCON(win));
	val &= ~WINCONx_ENWIN;
	writel(val, ctx->regs + WINCON(win));

	/* unprotect windows */
	val = readl(ctx->regs + SHADOWCON);
	val &= ~SHADOWCON_CHx_ENABLE(win);
	val &= ~SHADOWCON_WINx_PROTECT(win);
	writel(val, ctx->regs + SHADOWCON);
}

static void fimd_plane_commit(struct drm_plane *plane,
		struct drm_framebuffer *fb)
{
	struct exynos_drm_plane *exynos_plane = to_exynos_plane(plane);
	struct fimd_context *ctx = exynos_plane->ctx;
	int win = exynos_plane_to_win_idx(ctx, exynos_plane);
	struct exynos_drm_fb *exynos_fb = to_exynos_fb(fb);
	struct exynos_drm_gem_buf *buffer;
	dma_addr_t dma_addr;
	unsigned long val, alpha, size;
	unsigned int last_x;
	unsigned int last_y;
	uint32_t offset, line_size;

	DRM_DEBUG_KMS("[WIN:%d]\n", win);

	if (ctx->suspended)
		goto done;

	if (exynos_plane->src_w == 0 || exynos_plane->src_h == 0)
		goto done;

	buffer = exynos_drm_fb_buffer(exynos_fb, 0);
	dma_addr = buffer->dma_addr + exynos_plane->src_x
			* (fb->bits_per_pixel >> 3) + exynos_plane->src_y
			* fb->pitches[0];
	DRM_DEBUG_KMS("buffer: dma_addr = %p\n", (void *)dma_addr);

	/*
	 * SHADOWCON register is used for enabling timing.
	 *
	 * for example, once only width value of a register is set,
	 * if the dma is started then fimd hardware could malfunction so
	 * with protect window setting, the register fields with prefix '_F'
	 * wouldn't be updated at vsync also but updated once unprotect window
	 * is set.
	 */

	/* protect windows */
	val = readl(ctx->regs + SHADOWCON);
	val |= SHADOWCON_WINx_PROTECT(win);
	writel(val, ctx->regs + SHADOWCON);

	/* buffer start address */
	val = (unsigned long)dma_addr;
	writel(val, ctx->regs + VIDWx_BUF_START(win, 0));

	/* buffer end address */
	size = exynos_plane->src_h * fb->pitches[0];
	val = (unsigned long)(dma_addr + size);
	writel(val, ctx->regs + VIDWx_BUF_END(win, 0));

	DRM_DEBUG_KMS("start addr = 0x%lx, end addr = 0x%lx, size = 0x%lx\n",
			(unsigned long)dma_addr, val, size);
	DRM_DEBUG_KMS("crtc_width = %d, crtc_height = %d\n",
			exynos_plane->crtc_w, exynos_plane->crtc_h);

	/* buffer size */
	line_size = exynos_plane->src_w * (fb->bits_per_pixel >> 3);
	offset = fb->pitches[0] - line_size;
	val = VIDW_BUF_SIZE_OFFSET(offset) |
		VIDW_BUF_SIZE_PAGEWIDTH(line_size) |
		VIDW_BUF_SIZE_OFFSET_E(offset) |
		VIDW_BUF_SIZE_PAGEWIDTH_E(line_size);
	writel(val, ctx->regs + VIDWx_BUF_SIZE(win, 0));

	/* OSD position */
	val = VIDOSDxA_TOPLEFT_X(exynos_plane->crtc_x) |
		VIDOSDxA_TOPLEFT_Y(exynos_plane->crtc_y) |
		VIDOSDxA_TOPLEFT_X_E(exynos_plane->crtc_x) |
		VIDOSDxA_TOPLEFT_Y_E(exynos_plane->crtc_y);
	writel(val, ctx->regs + VIDOSD_A(win));

	last_x = exynos_plane->crtc_x + exynos_plane->crtc_w;
	if (last_x)
		last_x--;
	last_y = exynos_plane->crtc_y + exynos_plane->crtc_h;
	if (last_y)
		last_y--;

	val = VIDOSDxB_BOTRIGHT_X(last_x) | VIDOSDxB_BOTRIGHT_Y(last_y) |
		VIDOSDxB_BOTRIGHT_X_E(last_x) | VIDOSDxB_BOTRIGHT_Y_E(last_y);

	writel(val, ctx->regs + VIDOSD_B(win));

	DRM_DEBUG_KMS("osd pos: tx = %d, ty = %d, bx = %d, by = %d\n",
		exynos_plane->crtc_x, exynos_plane->crtc_y, last_x, last_y);

	/* hardware window 0 doesn't support alpha channel. */
	if (win != 0) {
		/* OSD alpha */
		alpha = VIDISD14C_ALPHA1_R(0xf) |
			VIDISD14C_ALPHA1_G(0xf) |
			VIDISD14C_ALPHA1_B(0xf);

		writel(alpha, ctx->regs + VIDOSD_C(win));
	}

	/* OSD size */
	if (win != 3 && win != 4) {
		u32 offset = VIDOSD_D(win);
		if (win == 0)
			offset = VIDOSD_C_SIZE_W0;
		val = exynos_plane->crtc_w * exynos_plane->crtc_h;
		writel(val, ctx->regs + offset);

		DRM_DEBUG_KMS("osd size = 0x%x\n", (unsigned int)val);
	}

	val = WINCONx_ENWIN | WINCON1_BPPMODE_28BPP_A4888 | WINCON1_BLD_PIX |
		WINCON1_ALPHA_SEL | WINCONx_WSWP;
	/* Restrict the burst length to 4WORD for cursor */
	if (exynos_plane->src_w <= 96)
		val |= WINCONx_BURSTLEN_4WORD;
	else
		val |= WINCONx_BURSTLEN_16WORD;

	writel(val, ctx->regs + WINCON(win));

	/* hardware window 0 doesn't support color key. */
	if (win != 0)
		fimd_win_set_colkey(ctx, win);

	/* wincon */
	val = readl(ctx->regs + WINCON(win));
	val |= WINCONx_ENWIN;
	writel(val, ctx->regs + WINCON(win));

	/* Enable DMA channel and unprotect windows */
	val = readl(ctx->regs + SHADOWCON);
	val |= SHADOWCON_CHx_ENABLE(win);
	val &= ~SHADOWCON_WINx_PROTECT(win);
	writel(val, ctx->regs + SHADOWCON);


done:
	plane->enabled = true;
}

static int fimd_plane_update(struct drm_plane *plane,
		struct drm_crtc *crtc, struct drm_framebuffer *fb, int crtc_x,
		int crtc_y, unsigned int crtc_w, unsigned int crtc_h,
		uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h)
{
	struct exynos_drm_plane *exynos_plane = to_exynos_plane(plane);

	/* Copy the plane parameters so we can restore it later */
	exynos_plane->crtc_x = crtc_x;
	exynos_plane->crtc_y = crtc_y;
	exynos_plane->crtc_w = crtc_w;
	exynos_plane->crtc_h = crtc_h;
	exynos_plane->src_x = src_x >> 16;
	exynos_plane->src_y = src_y >> 16;
	exynos_plane->src_w = src_w >> 16;
	exynos_plane->src_h = src_h >> 16;

	exynos_sanitize_plane_coords(plane, crtc);

	fimd_plane_commit(plane, fb);

	return 0;
}

static int fimd_plane_disable(struct drm_plane *plane)
{
	struct exynos_drm_plane *exynos_plane = to_exynos_plane(plane);
	struct fimd_context *ctx = exynos_plane->ctx;

	DRM_DEBUG_KMS("[WIN:%d]\n", exynos_plane_to_win_idx(ctx, exynos_plane));

	plane->enabled = false;
	fimd_plane_disable_nowait(plane);

	/* Synchronously wait for window to be disabled */
	fimd_wait_for_vblank(ctx);

	return 0;
}

static const struct drm_plane_funcs fimd_plane_funcs = {
	.update_plane = fimd_plane_update,
	.disable_plane = fimd_plane_disable,
	.set_property = drm_atomic_plane_set_property,
	.destroy = drm_plane_cleanup,
};

static int fimd_mgr_initialize(void *in_ctx, struct drm_crtc *crtc, int pipe)
{
	struct fimd_context *ctx = in_ctx;
	int i, ret;

	ctx->drm_dev = crtc->dev;
	ctx->pipe = pipe;

	if (is_drm_iommu_supported(ctx->drm_dev))
		drm_iommu_attach_device(ctx->drm_dev, ctx->dev);

	for (i = 0; i < FIMD_WIN_NR; i++) {
		struct exynos_drm_plane *exynos_plane = &ctx->planes[i];

		/*
		 * TODO: There's a small hack here which sets possible_crtcs to
		 *	 0 for the default win. This will prevent userspace from
		 *	 choosing it for display. It's necessary until we
		 *	 properly implement it as the primary plane. For now,
		 *	 we'll let drm treat it as an overlay plane so it's
		 *	 disabled at the right times (notably when we restore
		 *	 fbdev mode).
		 */
		ret = drm_plane_init(ctx->drm_dev, &exynos_plane->base,
			i == ctx->default_win ?	0 :1 << ctx->pipe,
			&fimd_plane_funcs, plane_formats,
			ARRAY_SIZE(plane_formats), false);
		if (ret) {
			DRM_ERROR("Init plane %d failed (ret=%d)\n", i, ret);
			goto err;
		}

		exynos_plane->ctx = ctx;
		exynos_plane->base.enabled = false;
	}

	return 0;
err:
	for (i--; i >= 0; i--)
		drm_plane_cleanup(&ctx->planes[i].base);

	return ret;
}

static void fimd_mgr_remove(void *in_ctx)
{
	struct fimd_context *ctx = in_ctx;

	if (is_drm_iommu_supported(ctx->drm_dev))
		drm_iommu_detach_device(ctx->drm_dev, ctx->dev);
}

static u32 fimd_calc_clkdiv(struct fimd_context *ctx,
		const struct drm_display_mode *mode)
{
	unsigned long ideal_clk = mode->htotal * mode->vtotal * mode->vrefresh;
	u32 clkdiv;

	/* Find the clock divider value that gets us closest to ideal_clk */
	clkdiv = DIV_ROUND_CLOSEST(ctx->lcd_clk_rate, ideal_clk);

	return (clkdiv < 0x100) ? clkdiv : 0xff;
}

static bool fimd_mode_fixup(void *in_ctx, const struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode)
{
	struct fimd_context *ctx = in_ctx;

	if (adjusted_mode->vrefresh == 0)
		adjusted_mode->vrefresh = 60;

	adjusted_mode->clock = ctx->lcd_clk_rate /
			fimd_calc_clkdiv(ctx, adjusted_mode);
	return true;
}

static void fimd_mode_set(void *in_ctx, const struct drm_display_mode *in_mode)
{
	struct fimd_context *ctx = in_ctx;
	struct fimd_mode_data *mode = &ctx->mode;

	mode->vtotal = in_mode->crtc_vtotal;
	mode->vdisplay = in_mode->crtc_vdisplay;
	mode->vsync_len = in_mode->crtc_vsync_end - in_mode->crtc_vsync_start;
	mode->vbpd = in_mode->crtc_vtotal - in_mode->crtc_vsync_end;
	mode->vfpd = in_mode->crtc_vsync_start - in_mode->crtc_vdisplay;

	mode->htotal = in_mode->crtc_htotal;
	mode->hdisplay = in_mode->crtc_hdisplay;
	mode->hsync_len = in_mode->crtc_hsync_end - in_mode->crtc_hsync_start;
	mode->hbpd = in_mode->crtc_htotal - in_mode->crtc_hsync_end;
	mode->hfpd = in_mode->crtc_hsync_start - in_mode->crtc_hdisplay;

	mode->clkdiv = fimd_calc_clkdiv(ctx, in_mode);
}

static int fimd_update(void *in_ctx, struct drm_crtc *crtc,
		struct drm_framebuffer *fb)
{
	struct fimd_context *ctx = in_ctx;
	struct exynos_drm_plane *exynos_plane = &ctx->planes[ctx->default_win];
	struct drm_plane *plane = &exynos_plane->base;

	/* Copy the plane parameters so we can restore it later */
	exynos_plane->crtc_x = 0;
	exynos_plane->crtc_y = 0;
	exynos_plane->crtc_w = fb->width - crtc->x;
	exynos_plane->crtc_h = fb->height - crtc->y;
	exynos_plane->src_x = crtc->x;
	exynos_plane->src_y = crtc->y;
	exynos_plane->src_w = exynos_plane->crtc_w;
	exynos_plane->src_h = exynos_plane->crtc_h;

	exynos_sanitize_plane_coords(plane, crtc);

	/* Grab a reference, just as setplane would */
	drm_framebuffer_reference(fb);

	fimd_plane_commit(plane, fb);

	if (plane->fb)
		drm_framebuffer_unreference(plane->fb);

	plane->fb = fb;
	plane->crtc = crtc;

	return 0;
}

static void fimd_commit(void *in_ctx)
{
	struct fimd_context *ctx = in_ctx;
	struct fimd_mode_data *mode = &ctx->mode;
	u32 val;

	if (ctx->suspended)
		return;

	/* nothing to do if we haven't set the mode yet */
	if (mode->htotal == 0 || mode->vtotal == 0)
		return;

	DRM_DEBUG_KMS("%ux%u\n", mode->htotal, mode->vtotal);

	/* setup polarity values from machine code. */
	writel(ctx->vidcon1, ctx->regs_timing + VIDCON1);

	/* setup vertical timing values. */
	val = VIDTCON0_VBPD(mode->vbpd - 1) |
		VIDTCON0_VFPD(mode->vfpd - 1) |
		VIDTCON0_VSPW(mode->vsync_len - 1);
	writel(val, ctx->regs_timing + VIDTCON0);

	/* setup horizontal timing values.  */
	val = VIDTCON1_HBPD(mode->hbpd - 1) |
		VIDTCON1_HFPD(mode->hfpd - 1) |
		VIDTCON1_HSPW(mode->hsync_len - 1);
	writel(val, ctx->regs_timing + VIDTCON1);

	/* setup horizontal and vertical display size. */
	val = VIDTCON2_LINEVAL(mode->vdisplay - 1) |
	       VIDTCON2_HOZVAL(mode->hdisplay - 1) |
	       VIDTCON2_LINEVAL_E(mode->vdisplay - 1) |
	       VIDTCON2_HOZVAL_E(mode->hdisplay - 1);
	writel(val, ctx->regs_timing + VIDTCON2);

	/* setup clock source, clock divider, enable dma. */
	val = ctx->vidcon0;
	val &= ~(VIDCON0_CLKVAL_F_MASK | VIDCON0_CLKDIR);

	if (mode->clkdiv > 1)
		val |= VIDCON0_CLKVAL_F(mode->clkdiv - 1) | VIDCON0_CLKDIR;
	else
		val &= ~VIDCON0_CLKDIR;	/* 1:1 clock */

	/*
	 * fields of register with prefix '_F' would be updated
	 * at vsync(same as dma start)
	 */
	val |= VIDCON0_ENVID | VIDCON0_ENVID_F;
	writel(val, ctx->regs + VIDCON0);

	exynos_set_dithering(ctx);
}


/*
 * FIMD has three interrupt sources ("FIFO level", "Video frame sync", and "i80
 * done interface".
 * However, this driver only uses one: the "Video frame sync", which is enabled
 * by VIDINTCON0_INT_FRAME.
 * The Video frame sync interrupt itself has two interrupt sources, both of
 * which are configurable.
 * This driver only uses one source (FRAMESEL0), and configures it to interrupt
 * at the start of VSYNC.
 */
static void fimd_irq_mask(struct fimd_context *ctx, bool enable)
{
	u32 val;

	val = VIDINTCON0_INT_FRAME | VIDINTCON0_FRAMESEL0_VSYNC |
			VIDINTCON0_FRAMESEL1_NONE;
	val |= enable ? VIDINTCON0_INT_ENABLE : 0;
	writel(val, ctx->regs + VIDINTCON0);
}

static int fimd_enable_vblank(void *in_ctx)
{
	struct fimd_context *ctx = in_ctx;

	if (ctx->suspended)
		return -EPERM;

	DRM_DEBUG_KMS("\n");

	fimd_irq_mask(ctx, true);

	return 0;
}

static void fimd_disable_vblank(void *in_ctx)
{
	struct fimd_context *ctx = in_ctx;

	if (ctx->suspended)
		return;

	DRM_DEBUG_KMS("\n");

	fimd_irq_mask(ctx, false);
}

static void fimd_wait_for_vblank(struct fimd_context *ctx)
{
	if (ctx->suspended)
		return;

	DRM_DEBUG_KMS("\n");

	drm_vblank_get(ctx->drm_dev, ctx->pipe);

	atomic_set(&ctx->wait_vsync_event, 1);

	/*
	 * wait for FIMD to signal VSYNC interrupt or return after
	 * timeout which is set to 50ms (refresh rate of 20).
	 */
	if (!wait_event_timeout(ctx->wait_vsync_queue,
				!atomic_read(&ctx->wait_vsync_event),
				DRM_HZ/20))
		DRM_ERROR("vblank wait timed out.\n");

	drm_vblank_put(ctx->drm_dev, ctx->pipe);
}

static void fimd_disable_planes(struct fimd_context *ctx)
{
	int i;
	bool wait_for_vblank = false;

	DRM_DEBUG_KMS("\n");

	for (i = 0; i < FIMD_WIN_NR; i++) {
		struct drm_plane *plane = &ctx->planes[i].base;

		if (!plane->enabled)
			continue;

		fimd_plane_disable_nowait(plane);
		wait_for_vblank = true;
	}

	/* Synchronously wait for any window disables to complete */
	if (wait_for_vblank)
		fimd_wait_for_vblank(ctx);
}

static void fimd_enable_planes(struct fimd_context *ctx)
{
	int i;

	DRM_DEBUG_KMS("\n");

	for (i = 0; i < FIMD_WIN_NR; i++) {
		struct exynos_drm_plane *exynos_plane = &ctx->planes[i];
		struct drm_plane *plane = &exynos_plane->base;

		if (!plane->enabled)
			continue;

		fimd_plane_commit(plane, plane->fb);
	}
}

static int fimd_poweron(struct fimd_context *ctx)
{
	int ret;

	if (!ctx->suspended)
		return 0;

	DRM_DEBUG_KMS("\n");

	ctx->suspended = false;

	pm_runtime_get_sync(ctx->dev);

	ret = clk_prepare_enable(ctx->bus_clk);
	if (ret < 0) {
		DRM_ERROR("Failed to prepare_enable the bus clk [%d]\n", ret);
		goto bus_clk_err;
	}

	ret = clk_prepare_enable(ctx->lcd_clk);
	if  (ret < 0) {
		DRM_ERROR("Failed to prepare_enable the lcd clk [%d]\n", ret);
		goto lcd_clk_err;
	}

	fimd_dither_enable(ctx);

	/* Update irq mask to current state of this crtc's vblank. */
	fimd_irq_mask(ctx, drm_is_vblank_enabled(ctx->drm_dev, ctx->pipe));

	enable_irq(ctx->irq);

	fimd_enable_planes(ctx);

	/*
	 * Restore the lost configuration of fimd
	 * as in case flips are not called reqularly
	 * it may cause display failure
	 */
	fimd_commit(ctx);

	return 0;

lcd_clk_err:
	clk_disable_unprepare(ctx->bus_clk);
bus_clk_err:
	ctx->suspended = true;
	return ret;
}

static int fimd_poweroff(struct fimd_context *ctx)
{
	if (ctx->suspended)
		return 0;

	DRM_DEBUG_KMS("\n");

	/*
	 * We need to make sure that all windows are disabled before we
	 * suspend that connector. Otherwise we might try to scan from
	 * a destroyed buffer later.
	 */
	fimd_disable_planes(ctx);

	/*
	 * There is tiny race here if a FIMD irq vblank irq arrives
	 * between fimd_disable_planes() and fimd_disable_vblank().
	 * However, since we've just synchronized to vblank in
	 * fimd_disable_planes(), it is very very unlikely that we would
	 * immediately get another vblank irq (we'd need to be processing a
	 * backlog of vblank irqs that were ~16.7 ms delayed).
	*/

	disable_irq(ctx->irq);

	fimd_dither_disable(ctx);

	clk_disable_unprepare(ctx->lcd_clk);
	clk_disable_unprepare(ctx->bus_clk);

	pm_runtime_put_sync(ctx->dev);

	ctx->suspended = true;
	return 0;
}

static void fimd_dpms(void *in_ctx, int mode)
{
	struct fimd_context *ctx = in_ctx;

	DRM_DEBUG_KMS("[DPMS:%s]\n", drm_get_dpms_name(mode));

	switch (mode) {
	case DRM_MODE_DPMS_ON:
		fimd_poweron(ctx);
		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		fimd_poweroff(ctx);
		break;
	default:
		DRM_DEBUG_KMS("unspecified mode %d\n", mode);
		break;
	}
}

static const struct exynos_drm_manager_ops fimd_manager_ops = {
	.initialize = fimd_mgr_initialize,
	.remove = fimd_mgr_remove,
	.dpms = fimd_dpms,
	.mode_fixup = fimd_mode_fixup,
	.mode_set = fimd_mode_set,
	.update = fimd_update,
	.commit = fimd_commit,
	.enable_vblank = fimd_enable_vblank,
	.disable_vblank = fimd_disable_vblank,
};

static struct exynos_drm_manager fimd_manager = {
	.type = EXYNOS_DISPLAY_TYPE_LCD,
	.ops = &fimd_manager_ops,
};

static irqreturn_t fimd_irq_handler(int irq, void *dev_id)
{
	struct fimd_context *ctx = (struct fimd_context *)dev_id;
	u32 val;
	u32 start, start_s;

	WARN_ON(ctx->suspended);
	val = readl(ctx->regs + VIDINTCON1);

	if (val & VIDINTCON1_INT_FRAME)
		/* VSYNC interrupt */
		writel(VIDINTCON1_INT_FRAME, ctx->regs + VIDINTCON1);

	/* check the crtc is detached already from encoder */
	if (ctx->pipe < 0 || !ctx->drm_dev)
		goto out;

	drm_handle_vblank(ctx->drm_dev, ctx->pipe);

	/*
	 * Ensure finish_pageflip is called iff a pending flip has completed.
	 * This works around a race between a page_flip request and the latency
	 * between vblank interrupt and this irq_handler:
	 *   => FIMD vblank: BUF_START_S[0] := BUF_START[0], and asserts irq
	 *   | => fimd_win_commit(0) writes new BUF_START[0]
	 *   |    exynos_drm_crtc_try_do_flip() marks exynos_fb as prepared
	 *   => fimd_irq_handler()
	 *       exynos_drm_crtc_finish_pageflip() sees prepared exynos_fb,
	 *           and unmaps "old" fb
	 *   ==> but, since BUF_START_S[0] still points to that "old" fb...
	 *   ==> FIMD iommu fault
	 */
	start = readl(ctx->regs + VIDWx_BUF_START(0, 0));
	start_s = readl(ctx->regs + VIDWx_BUF_START_S(0, 0));
	if (start == start_s)
		exynos_drm_crtc_finish_pageflip(ctx->drm_dev, ctx->pipe);

	/* set wait vsync event to zero and wake up queue. */
	if (atomic_read(&ctx->wait_vsync_event)) {
		atomic_set(&ctx->wait_vsync_event, 0);
		DRM_WAKEUP(&ctx->wait_vsync_queue);
	}
out:
	return IRQ_HANDLED;
}

static void fimd_clear_win(struct fimd_context *ctx, int win)
{
	u32 val;

	DRM_DEBUG_KMS("[WIN:%d]\n", win);

	writel(0, ctx->regs + WINCON(win));
	writel(0, ctx->regs + VIDOSD_A(win));
	writel(0, ctx->regs + VIDOSD_B(win));
	writel(0, ctx->regs + VIDOSD_C(win));

	if (win == 1 || win == 2)
		writel(0, ctx->regs + VIDOSD_D(win));

	val = readl(ctx->regs + SHADOWCON);
	val &= ~SHADOWCON_WINx_PROTECT(win);
	writel(val, ctx->regs + SHADOWCON);
}

static const char *exynos_drm_fimd_dithering_name(enum dither_mode dither_mode)
{
	switch (dither_mode) {
	case USE_MIE_DITHERING:
		return "MIE";
	case USE_FIMD_DITHERING:
		return "FIMD";
	case USE_NO_DITHERING:
	default:
		return "NO";
	}
}
static struct exynos_drm_fimd_pdata *drm_fimd_dt_parse_pdata(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct exynos_drm_fimd_pdata *pd;

	pd = devm_kzalloc(dev, sizeof(*pd), GFP_KERNEL);
	if (!pd) {
		dev_err(dev, "memory allocation for pdata failed\n");
		return ERR_PTR(-ENOMEM);
	}

	if (of_get_property(np, "samsung,fimd-vidout-rgb", NULL))
		pd->vidcon0 |= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB;
	if (of_get_property(np, "samsung,fimd-inv-hsync", NULL))
		pd->vidcon1 |= VIDCON1_INV_HSYNC;
	if (of_get_property(np, "samsung,fimd-inv-vsync", NULL))
		pd->vidcon1 |= VIDCON1_INV_VSYNC;
	if (of_get_property(np, "samsung,fimd-inv-vclk", NULL))
		pd->vidcon1 |= VIDCON1_INV_VCLK;
	if (of_get_property(np, "samsung,fimd-inv-vden", NULL))
		pd->vidcon1 |= VIDCON1_INV_VDEN;

	if (of_get_property(np, "samsung,use-mie-dithering", NULL))
		pd->dither_mode = USE_MIE_DITHERING;
	else if (of_get_property(np, "samsung,use-fimd-dithering", NULL)) {
		pd->dither_mode = USE_FIMD_DITHERING;
		if (of_property_read_u32_array(np,
				"samsung-rgb-dithpos", pd->dither_rgb_bpc, 3)) {
			dev_err(dev, "bpc values needed for dithering\n");
			pd->dither_mode = USE_NO_DITHERING;
		}
	} else
		pd->dither_mode = USE_NO_DITHERING;

	of_property_read_u32(np, "samsung,default-window", &pd->default_win);

	of_property_read_u32(np, "samsung,fimd-win-bpp", &pd->bpp);

	of_property_read_u32(np, "samsung,fimd-src-clk-rate",
							&pd->src_clk_rate);

	return pd;
}

static int fimd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fimd_context *ctx;
	struct exynos_drm_fimd_pdata *pdata = pdev->dev.platform_data;
	struct exynos_drm_panel_info *panel;
	struct fimd_driver_data *fimd_driver_data;
	struct resource *res;
	int win;
	int ret = -EINVAL;

	DRM_DEBUG_KMS("[PDEV:%s]\n", pdev->name);

	if (pdev->dev.of_node) {
		pdata = drm_fimd_dt_parse_pdata(&pdev->dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
	}
	if (!pdata) {
		dev_err(dev, "no platform data specified\n");
		return -EINVAL;
	}

	panel = &pdata->panel;
	if (!panel) {
		dev_err(dev, "panel is null.\n");
		return -EINVAL;
	}

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->bus_clk = devm_clk_get(dev, "fimd");
	if (IS_ERR(ctx->bus_clk)) {
		dev_err(dev, "failed to get bus clock\n");
		return PTR_ERR(ctx->bus_clk);
	}

	ctx->lcd_clk = devm_clk_get(dev, "sclk_fimd");
	if (IS_ERR(ctx->lcd_clk)) {
		dev_err(dev, "failed to get lcd clock\n");
		return PTR_ERR(ctx->lcd_clk);
	}

	/* Set the parent for FIMD pixel clock */
	ctx->fimd_mux_clk = devm_clk_get(dev, "mout_fimd");
	if (IS_ERR(ctx->fimd_mux_clk)) {
		dev_err(dev, "failed to get fimd mux clk\n");
		return PTR_ERR(ctx->fimd_mux_clk);
	}

	ctx->sclk_mout_fimd = devm_clk_get(dev, "sclk_mout_fimd");
	if (IS_ERR(ctx->sclk_mout_fimd)) {
		dev_err(dev, "failed to get mout_fimd parent\n");
		return PTR_ERR(ctx->sclk_mout_fimd);
	}
	clk_set_parent(ctx->fimd_mux_clk, ctx->sclk_mout_fimd);

	/* Set the FIMD pixel clock to desired value */
	clk_set_rate(ctx->lcd_clk, pdata->src_clk_rate);
	ctx->lcd_clk_rate = pdata->src_clk_rate;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	ctx->regs = devm_request_and_ioremap(&pdev->dev, res);
	if (!ctx->regs) {
		dev_err(dev, "failed to map registers\n");
		return -ENXIO;
	}

	/* timing registers are at a platform specific offset */
	fimd_driver_data = drm_fimd_get_driver_data(pdev);
	ctx->regs_timing = ctx->regs + fimd_driver_data->timing_base;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(dev, "irq request failed.\n");
		return -ENXIO;
	}

	ctx->irq = res->start;
	ret = devm_request_irq(&pdev->dev, ctx->irq, fimd_irq_handler,
							0, "drm_fimd", ctx);
	if (ret) {
		dev_err(dev, "irq request failed.\n");
		return ret;
	}
	disable_irq(ctx->irq);

	ctx->vidcon0 = pdata->vidcon0;
	ctx->vidcon1 = pdata->vidcon1;
	ctx->default_win = pdata->default_win;
	ctx->dev = dev;

	ctx->dither_mode = pdata->dither_mode;
	if (ctx->dither_mode == USE_FIMD_DITHERING) {
		if (fimd_parse_dither_rgb(ctx, pdata->dither_rgb_bpc))
			ctx->dither_mode = USE_NO_DITHERING;
	} else if (ctx->dither_mode == USE_MIE_DITHERING) {
		ctx->regs_mie = devm_ioremap(dev, MIE_BASE_ADDRESS, 0x400);
		if (!ctx->regs_mie) {
			dev_err(dev, "failed to map registers\n");
			ctx->dither_mode = USE_NO_DITHERING;
		}
	}
	DRM_INFO("FIMD: using %s dithering\n",
			exynos_drm_fimd_dithering_name(ctx->dither_mode));

	ctx->suspended = true;
	DRM_INIT_WAITQUEUE(&ctx->wait_vsync_queue);
	atomic_set(&ctx->wait_vsync_event, 0);

	platform_set_drvdata(pdev, ctx);

	/*
	 * We need to runtime pm to enable/disable sysmmu since it is a child of
	 * this driver. Ideally, this would hang off the drm driver's runtime
	 * operations, but we're not quite there yet.
	 *
	 * Tracked in crbug.com/264312
	 */
	pm_runtime_enable(dev);

	for (win = 0; win < FIMD_WIN_NR; win++)
		fimd_clear_win(ctx, win);

	fimd_manager.ctx = ctx;
	exynos_drm_manager_register(&fimd_manager);

	return 0;
}

static int fimd_remove(struct platform_device *pdev)
{
	struct fimd_context *ctx = platform_get_drvdata(pdev);

	DRM_DEBUG_KMS("[PDEV:%s]\n", pdev->name);

	exynos_drm_manager_unregister(&fimd_manager);

	fimd_dpms(ctx, DRM_MODE_DPMS_OFF);

	pm_runtime_disable(&pdev->dev);

	return 0;
}

static struct platform_device_id fimd_driver_ids[] = {
	{
		.name		= "exynos4-fb",
		.driver_data	= (unsigned long)&exynos4_fimd_driver_data,
	}, {
		.name		= "exynos5-fb",
		.driver_data	= (unsigned long)&exynos5_fimd_driver_data,
	},
	{},
};
MODULE_DEVICE_TABLE(platform, fimd_driver_ids);

struct platform_driver fimd_driver = {
	.probe		= fimd_probe,
	.remove		= fimd_remove,
	.id_table       = fimd_driver_ids,
	.driver		= {
		.name	= "exynos4-fb",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(fimd_driver_dt_match),
	},
};

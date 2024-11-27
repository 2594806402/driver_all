#include <linux/busfreq-imx.h>
#include <linux/console.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/pm_qos.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/pinctrl/consumer.h>
#include <linux/fb.h>
#include <linux/mxcfb.h>
#include <linux/regulator/consumer.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <video/of_display_timing.h>
#include <video/videomode.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include "mxc/mxc_dispdrv.h"

struct lcd_regs {
	volatile unsigned int fb_base_phys;	//物理基地址
	volatile unsigned int fb_xres;		//x方向分辨率
	volatile unsigned int fb_yres;		//y方向分辨率
	volatile unsigned int fb_bpp;		//颜色位数
};

static struct lcd_regs *myLCD_regs;

struct fb_info *fb_info;

static unsigned int pseudo_palette[16];

/* 位域转换 */
static inline u_int chan_to_field(u_int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}
/* 设置调色板 */
static int myLCD_setcolreg(u_int regno, u_int red, u_int green, 
							u_int blue,u_int trans, struct fb_info *info)
{
	unsigned int val;
	int ret = 1;
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR://真彩色
		/*
		 * 16位真彩色。我们根据RGB位域信息对RGB值进行编码。
		 */
		if (regno < 16) {
			u32 *pal = info->pseudo_palette;

			val  = chan_to_field(red, &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue, &info->var.blue);

			pal[regno] = val;
			ret = 0;
		}
		break;
	}

	return ret;
}
		   
static struct fb_ops myLCD_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= myLCD_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};



int __init myLCD_init(void)
{
	dma_addr_t phy_addr;

	/* 分配fb_info结构体 */
	fb_info = framebuffer_alloc(0, NULL);
	/* 1.2 设置fb_info */
	fb_info->var.xres = fb_info->var.xres_virtual = 500;//x方向分辨率
	fb_info->var.yres = fb_info->var.yres_virtual = 300;//y方向分辨率

	fb_info->var.bits_per_pixel = 16;//RGB565
	/* 红色位域 */
	fb_info->var.red.offset = 11;
	fb_info->var.red.length = 5;
	/* 绿色位域 */
	fb_info->var.green.offset = 5;
	fb_info->var.green.length = 6;
	/* 蓝色位域 */
	fb_info->var.blue.offset = 0;
	fb_info->var.blue.length = 5;

	


	strcpy(fb_info->fix.id, "my_lcd");
	/* 计算显存范围 */
	if(fb_info->var.bits_per_pixel == 16){//RGB565
		fb_info->fix.smem_len = fb_info->var.xres * fb_info->var.yres * fb_info->var.bits_per_pixel / 8;
	}
	else if(fb_info->var.bits_per_pixel == 24){//RGB888
		fb_info->fix.smem_len = fb_info->var.xres * fb_info->var.yres * 4;
	}

	/* fb的虚拟地址 */
	fb_info->screen_base = dma_alloc_wc(NULL, fb_info->fix.smem_len, &phy_addr, GFP_KERNEL);
	fb_info->fix.smem_start = phy_addr; /* fb的物理地址 */
	fb_info->fix.type = FB_TYPE_PACKED_PIXELS;
	fb_info->fix.visual = FB_VISUAL_TRUECOLOR;//真彩色
	
	if(fb_info->var.bits_per_pixel == 16){//RGB565
		fb_info->fix.line_length = fb_info->var.xres * fb_info->var.bits_per_pixel / 8;
	}
	else if(fb_info->var.bits_per_pixel == 24){//RGB888
		fb_info->fix.line_length = fb_info->var.xres * 4;
	}
	
	fb_info->fbops = &myLCD_ops;
	fb_info->pseudo_palette = pseudo_palette;

	/* 1.3 注册fb_info */
	register_framebuffer(fb_info);
	/* 1.4 硬件操作 */
	myLCD_regs = ioremap(0x021c8000, sizeof(struct lcd_regs));
	myLCD_regs->fb_base_phys = phy_addr;
	myLCD_regs->fb_xres = 500;
	myLCD_regs->fb_yres = 300;
	myLCD_regs->fb_bpp = 16;
	return 0;
}

static void __exit myLCD_cleanup(void)
{
	/* 反过来操作 */
	/* 2.1 反注册fb_info */
	unregister_framebuffer(fb_info);
	
	/* 2.2 释放fb_info */
	framebuffer_release(fb_info);

	iounmap(myLCD_regs);
}




module_init(myLCD_init);
module_exit(myLCD_cleanup);

MODULE_AUTHOR("CHEN CHENG YANG");
MODULE_AUTHOR("2594806402@qq.com");
MODULE_DESCRIPTION("Framebuffer driver for the 7'RGBLCD");
MODULE_LICENSE("GPL");
















































































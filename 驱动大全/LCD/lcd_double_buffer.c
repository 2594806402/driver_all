#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

static int fd_fb;
static struct fb_var_screeninfo fb_var;	/* Current var */
static struct fb_fix_screeninfo fb_fix;

static int screen_size;
static unsigned char *fb_base;
static unsigned int line_width;
static unsigned int pixel_width;

/**********************************************************************
 * 函数名称： lcd_put_pixel
 * 功能描述： 在LCD指定位置上输出指定颜色（描点）
 * 输入参数： x坐标，y坐标，颜色
 * 输出参数： 无
 * 返 回 值： 无
 ***********************************************************************/ 
void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char *pen_8 = fb_base+y*line_width+x*pixel_width;
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (fb_var.bits_per_pixel)
	{
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			printf("can't surport %dbpp\n", fb_var.bits_per_pixel);
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int i;
	/* 打开 framebuffer 设备 */
	fd_fb = open("/dev/fb0", O_RDWR);
	if (fd_fb < 0)
	{
		printf("can't open /dev/fb0\n");
		return -1;
	}
	/* 获取屏幕参数 */
	if (ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix))
	{
		printf("can't get var\n");
		return -1;
	}
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var))
	{
		printf("can't get var\n");
		return -1;
	}

	line_width  = fb_fix.line_length;//每行字节数
	pixel_width = fb_var.bits_per_pixel / 8;//每个像素多少字节
	screen_size = fb_fix.line_length * fb_var.yres;//屏幕总需内存大小
	/* 申请多buffer的内存空间 */
	fb_base = (unsigned char *)mmap(NULL , fb_fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);
	if (fb_base == (unsigned char *)-1)
	{
		printf("can't mmap\n");
		return -1;
	}

	/* 退出 */
	munmap(fb_base , screen_size);
	close(fd_fb);
	
	return 0;	
}




































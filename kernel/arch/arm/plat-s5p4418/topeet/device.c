/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/irq.h>
#include <linux/amba/pl022.h>

/* nexell soc headers */
#include <mach/platform.h>
#include <mach/devices.h>
#include <mach/soc.h>

#include <linux/gpio.h>

#include <linux/combo_mt66xx.h>
#if defined(CONFIG_NXP_HDMI_CEC)
#include <mach/nxp-hdmi-cec.h>
#endif

/*------------------------------------------------------------------------------
 * BUS Configure
 */
#if (CFG_BUS_RECONFIG_ENB == 1)
#include <mach/s5p4418_bus.h>

const u16 g_DrexQoS[2] = {
	0x100,		// S0
	0xFFF		// S1, Default value
};

const u8 g_TopBusSI[8] = {
	TOPBUS_SI_SLOT_DMAC0,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST0,
	TOPBUS_SI_SLOT_DMAC1,
	TOPBUS_SI_SLOT_SDMMC,
	TOPBUS_SI_SLOT_USBOTG,
	TOPBUS_SI_SLOT_USBHOST1,
	TOPBUS_SI_SLOT_USBOTG
};

const u8 g_BottomBusSI[8] = {
	BOTBUS_SI_SLOT_1ST_ARM,
	BOTBUS_SI_SLOT_MALI,
	BOTBUS_SI_SLOT_DEINTERLACE,
	BOTBUS_SI_SLOT_1ST_CODA,
	BOTBUS_SI_SLOT_2ND_ARM,
	BOTBUS_SI_SLOT_SCALER,
	BOTBUS_SI_SLOT_TOP,
	BOTBUS_SI_SLOT_2ND_CODA
};

const u8 g_BottomQoSSI[2] = {
	1,	// Tidemark
	(1<<BOTBUS_SI_SLOT_1ST_ARM) |	// Control
	(1<<BOTBUS_SI_SLOT_2ND_ARM) |
	(1<<BOTBUS_SI_SLOT_MALI) |
	(1<<BOTBUS_SI_SLOT_TOP) |
	(1<<BOTBUS_SI_SLOT_DEINTERLACE) |
	(1<<BOTBUS_SI_SLOT_1ST_CODA)
};

const u8 g_DispBusSI[3] = {
	DISBUS_SI_SLOT_1ST_DISPLAY,
	DISBUS_SI_SLOT_2ND_DISPLAY,
/* modify by cym 20150911 */
#if 0
	DISBUS_SI_SLOT_2ND_DISPLAY  //DISBUS_SI_SLOT_GMAC
#else
	DISBUS_SI_SLOT_GMAC  // DISBUS_SI_SLOT_2ND_DISPLAY
#endif
/* end modify */
};
#endif	/* #if (CFG_BUS_RECONFIG_ENB == 1) */

/*------------------------------------------------------------------------------
 * CPU Frequence
 */
#if defined(CONFIG_ARM_NXP_CPUFREQ)

static unsigned long dfs_freq_table[][2] = {
/* modify by cym 20160525 */
#if 0
	{ 1400000, },
//	{ 1300000, },
	{ 1200000, },
//	{ 1100000, },
	{ 1000000, },
//	{  900000, },
	{  800000, },
	{  700000, },
	{  600000, },
	{  500000, },
	{  400000, },
#else
	{ 1400000, 1280000, },
	{ 1200000, 1200000, },
	{ 1000000, 1140000, },
	{  800000, 1100000, },
	{  700000, 1080000, },
	{  600000, 1040000, },
	{  500000, 1040000, },
	{  400000, 1040000, },
#endif
/* end modify */
};

struct nxp_cpufreq_plat_data dfs_plat_data = {
	.pll_dev	   	= CONFIG_NXP_CPUFREQ_PLLDEV,
	.supply_name	= "vdd_arm_1.3V",	//refer to CONFIG_REGULATOR_NXE2000
	.freq_table	   	= dfs_freq_table,
	.table_size	   	= ARRAY_SIZE(dfs_freq_table),
	.max_cpufreq	= 1400*1000,
#if 0
	.max_retention  =   20*1000,
	.rest_cpufreq   =  400*1000,
	.rest_retention =    1*1000,
#else
	.max_retention  =   10*1000,
        .rest_cpufreq   =  800*1000,
        .rest_retention =    2*1000,
        .fixed_voltage  = 1280*1000,
#endif
};

static struct platform_device dfs_plat_device = {
	.name			= DEV_NAME_CPUFREQ,
	.dev			= {
		.platform_data	= &dfs_plat_data,
	}
};
#endif

/* add by cym 20150525 */
#if defined(CONFIG_SENSORS_NXP_ADC_TEMP)
struct nxp_adc_tmp_trigger adc_tmp_event[] = {
        {
                .temp   = 50,
                .freq   = 1200000,
                .period = 1000,
        } , {
                .temp   = 55,
                .freq   = 1000000,
                .period = 1000,
        } , {
                .temp   = 60,
                .freq   = 800000,
                .period = 1000,
        } , {
                .temp   = 65,
                .freq   = 400000,       /* freq = 0: Set critical temp. Power off! */
                .period = 1000,         /* Ms */
        },
};

struct nxp_adc_tmp_platdata adc_tmp_plat_data ={
        .channel        = 2,
        .tmp_offset     = 0,
        .duration       = 1000,
        .step_up        = 1,
        .event          = adc_tmp_event,
        .eventsize = ARRAY_SIZE(adc_tmp_event),
};

static struct platform_device adc_temp_plat_device = {
        .name           = "nxp-adc-tmp",
        .dev            = {
                .platform_data  = &adc_tmp_plat_data,
        }
};
#endif /* CONFIG_SENSORS_NXP_ADC_TEMP */
/* end add */

//#define CPU_LIMIT_CONTROL
/*------------------------------------------------------------------------------
 * CPUFREQ Limit
 */
#if defined(CPU_LIMIT_CONTROL)
#if 0
static char *freq_proct_list[] = { "com.into.stability", };

static struct nxp_cpufreq_limit_data freq_limit_data = {
	.limit_name	 	= freq_proct_list,
	.limit_num 		= ARRAY_SIZE(freq_proct_list),
	.aval_max_freq 	= 1200000,
	.op_max_freq	= 1600000,
	.sched_duration	= 1000,
	.sched_timeout	= 3000,
};
#else
static char *freq_proct_list[] = { "com.antutu", };

static struct nxp_cpufreq_limit_data freq_limit_data = {
	.limit_name		= freq_proct_list,
	.limit_num 		= ARRAY_SIZE(freq_proct_list),
	.aval_max_freq	= 1400000,
	.op_max_freq	= 1200000,
#if defined(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)
	/* modify by cym 20151130 */
#if 0
	.limit_level0_freq	= 1200000,
	.limit_level1_freq	= 1000000,
	.min_max_freq	= 800000,
#else
	.limit_level0_freq      = 1400000,
        .limit_level1_freq      = 1200000,
        .min_max_freq   = 1200000,
#endif
	/* end modify */
	.prev_max_freq = 1400000,
#endif
};
#endif

static struct platform_device freq_limit_device = {
	.name			= "cpufreq-limit",
	.dev			= {
		.platform_data	= &freq_limit_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * Network DM9000
 */
#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
#include <linux/dm9000.h>

static struct resource dm9000_resource[] = {
	[0] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR,
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 1,		// 1 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[1] = {
		.start	= CFG_ETHER_EXT_PHY_BASEADDR + 4,		// + 4 (8/16 BIT)
		.end	= CFG_ETHER_EXT_PHY_BASEADDR + 5,		// + 5 (8/16 BIT)
		.flags	= IORESOURCE_MEM
	},
	[2] = {
		.start	= CFG_ETHER_EXT_IRQ_NUM,
		.end	= CFG_ETHER_EXT_IRQ_NUM,
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	}
};

static struct dm9000_plat_data eth_plat_data = {
	.flags		= DM9000_PLATF_8BITONLY,	// DM9000_PLATF_16BITONLY
};

static struct platform_device dm9000_plat_device = {
	.name			= "dm9000",
	.id				= 0,
	.num_resources	= ARRAY_SIZE(dm9000_resource),
	.resource		= dm9000_resource,
	.dev			= {
		.platform_data	= &eth_plat_data,
	}
};
#endif	/* CONFIG_DM9000 || CONFIG_DM9000_MODULE */

/* add by cym 20150909 */
/*------------------------------------------------------------------------------
 * DW GMAC board config
 */
#if defined(CONFIG_NXPMAC_ETH)
#include <linux/phy.h>
#include <linux/nxpmac.h>
#include <linux/delay.h>
#include <linux/gpio.h>
int  nxpmac_init(struct platform_device *pdev)
{
    u32 addr;

	nxp_soc_gpio_set_io_drv((PAD_GPIO_E +  7), 3);     // PAD_GPIOE7,     GMAC0_PHY_TXD[0]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E +  8), 3);     // PAD_GPIOE8,     GMAC0_PHY_TXD[1]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E +  9), 3);     // PAD_GPIOE9,     GMAC0_PHY_TXD[2]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 10), 3);     // PAD_GPIOE10,    GMAC0_PHY_TXD[3]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 11), 3);     // PAD_GPIOE11,    GMAC0_PHY_TXEN
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 14), 3);     // PAD_GPIOE14,    GMAC0_PHY_RXD[0]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 15), 3);     // PAD_GPIOE15,    GMAC0_PHY_RXD[1]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 16), 3);     // PAD_GPIOE16,    GMAC0_PHY_RXD[2]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 17), 3);     // PAD_GPIOE17,    GMAC0_PHY_RXD[3]
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 18), 3);     // PAD_GPIOE18,    GMAC0_RX_CLK
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 19), 3);     // PAD_GPIOE19,    GMAC0_PHY_RX_DV
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 20), 3);     // PAD_GPIOE20,    GMAC0_GMII_MDC
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 21), 3);     // PAD_GPIOE21,    GMAC0_GMII_MDI
        nxp_soc_gpio_set_io_drv((PAD_GPIO_E + 24), 3);     // PAD_GPIOE24,    GMAC0_GTX_CLK

        printk("NXP mac 1000Base-T gpio init\n");

	// Clock control
        NX_CLKGEN_Initialize();
        addr = NX_CLKGEN_GetPhysicalAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE);
        NX_CLKGEN_SetBaseAddress(CLOCKINDEX_OF_DWC_GMAC_MODULE, (u32)IO_ADDRESS(addr));

        NX_CLKGEN_SetClockSource(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 4);     // Sync mode for 100 & 10Base-T : External RX_clk
        NX_CLKGEN_SetClockDivisor(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, 1);    // Sync mode for 100 & 10Base-T

        NX_CLKGEN_SetClockOutInv(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CFALSE);    // TX Clk invert off : 100 & 10Base-T
//      NX_CLKGEN_SetClockOutInv(CLOCKINDEX_OF_DWC_GMAC_MODULE, 0, CTRUE);     // TX clk invert on : 100 & 10Base-T

        NX_CLKGEN_SetClockDivisorEnable(CLOCKINDEX_OF_DWC_GMAC_MODULE, CTRUE);

	// Reset control
        NX_RSTCON_Initialize();
        addr = NX_RSTCON_GetPhysicalAddress();
        NX_RSTCON_SetBaseAddress((u32)IO_ADDRESS(addr));
        NX_RSTCON_SetnRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ENABLE);
        udelay(100);
        NX_RSTCON_SetnRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_DISABLE);
        udelay(100);
        NX_RSTCON_SetnRST(RESETINDEX_OF_DWC_GMAC_MODULE_aresetn_i, RSTCON_ENABLE);
        udelay(100);

	gpio_request(CFG_ETHER_GMAC_PHY_RST_NUM,"Ethernet Rst pin");
        gpio_direction_output(CFG_ETHER_GMAC_PHY_RST_NUM, 1);
        udelay(100);
        gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0);
        udelay(100);
        gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1);

	gpio_free(CFG_ETHER_GMAC_PHY_RST_NUM);

	printk("NXP mac init\n");

        return 0;
}

int gmac_phy_reset(void *priv)
{
        // Set GPIO nReset
        gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1);
        udelay(100);
        gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 0);
        udelay(100);
        gpio_set_value(CFG_ETHER_GMAC_PHY_RST_NUM, 1);
        msleep(30);

        return 0;
}

static struct stmmac_mdio_bus_data nxpmac0_mdio_bus = {
        .phy_reset = gmac_phy_reset,
        .phy_mask = 0,
        .probed_phy_irq = CFG_ETHER_GMAC_PHY_IRQ_NUM,
};

static struct plat_stmmacenet_data nxpmac_plat_data = {
/* modify by cym 20160408, RTL---4;8031---1 */
#ifdef CONFIG_AR8031_PHY
        .phy_addr = 1,
#else
        .phy_addr = 4,
#endif
/* end modify */
    	.clk_csr = 0x4, // PCLK 150~250 Mhz
    	.speed = SPEED_1000,        // SPEED_1000
    	.interface = PHY_INTERFACE_MODE_RGMII,
        .autoneg = AUTONEG_ENABLE, //AUTONEG_ENABLE or AUTONEG_DISABLE
        .duplex = DUPLEX_FULL,
        .pbl = 16,          /* burst 16 */
        .has_gmac = 1,      /* GMAC ethernet    */
        .enh_desc = 0,
        .mdio_bus_data = &nxpmac0_mdio_bus,
        .init = &nxpmac_init,
};

/* DWC GMAC Controller registration */
static struct resource nxpmac_resource[] = {
    	[0] = DEFINE_RES_MEM(PHY_BASEADDR_GMAC, SZ_8K),
    	[1] = DEFINE_RES_IRQ_NAMED(IRQ_PHY_GMAC, "macirq"),
};

static u64 nxpmac_dmamask = DMA_BIT_MASK(32);

struct platform_device nxp_gmac_dev = {
    	.name           = "stmmaceth",  //"s5p4418-gmac",
    	.id             = -1,
    	.num_resources  = ARRAY_SIZE(nxpmac_resource),
    	.resource       = nxpmac_resource,
    	.dev            = {
        	.dma_mask           = &nxpmac_dmamask,
        	.coherent_dma_mask  = DMA_BIT_MASK(32),
        	.platform_data      = &nxpmac_plat_data,
    	}
};
#endif
/* end add */
/*------------------------------------------------------------------------------
 * DISPLAY (LVDS) / FB
 */
#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
static struct nxp_fb_plat_data fb0_plat_data = {
	.module			= CONFIG_FB0_NXP_DISPOUT,
	.layer			= CFG_DISP_PRI_SCREEN_LAYER,
	.format			= CFG_DISP_PRI_SCREEN_RGB_FORMAT,
	.bgcolor		= CFG_DISP_PRI_BACK_GROUND_COLOR,
	.bitperpixel	= CFG_DISP_PRI_SCREEN_PIXEL_BYTE * 8,
	.x_resol		= CFG_DISP_PRI_RESOL_WIDTH,
	.y_resol		= CFG_DISP_PRI_RESOL_HEIGHT,
	#ifdef CONFIG_ANDROID
	.buffers		= 3,
	.skip_pan_vsync	= 1,
	#else
	.buffers		= 2,
	#endif
	.lcd_with_mm	= CFG_DISP_PRI_LCD_WIDTH_MM,	/* 152.4 */
	.lcd_height_mm	= CFG_DISP_PRI_LCD_HEIGHT_MM,	/* 91.44 */
};

static struct platform_device fb0_device = {
	.name	= DEV_NAME_FB,
	.id		= 0,	/* FB device node num */
	.dev    = {
		.coherent_dma_mask 	= 0xffffffffUL,	/* for DMA allocate */
		.platform_data		= &fb0_plat_data
	},
};
#endif

static struct platform_device *fb_devices[] = {
	#if defined (CONFIG_FB0_NXP)
	&fb0_device,
	#endif
};
#endif /* CONFIG_FB_NXP */

/*------------------------------------------------------------------------------
 * backlight : generic pwm device
 */
#if defined(CONFIG_BACKLIGHT_PWM)
#include <linux/pwm_backlight.h>

static struct platform_pwm_backlight_data bl_plat_data = {
	.pwm_id			= CFG_LCD_PRI_PWM_CH,
	.max_brightness = 255,//	/* 255 is 100%, set over 100% */
	.dft_brightness = 100,//	/* 99% */
	.pwm_period_ns	= 1000000000/CFG_LCD_PRI_PWM_FREQ,
};

static struct platform_device bl_plat_device = {
	.name	= "pwm-backlight",
	.id		= -1,
	.dev	= {
		.platform_data	= &bl_plat_data,
	},
};

#endif

/* add by cym 20150911 */
#if defined(CONFIG_PPM_NXP)
#include <mach/ppm.h>
struct nxp_ppm_platform_data ppm_plat_data = {
    .input_polarity = NX_PPM_INPUTPOL_INVERT,//NX_PPM_INPUTPOL_INVERT  or  NX_PPM_INPUTPOL_BYPASS
};

static struct platform_device ppm_device = {
    .name           = DEV_NAME_PPM,
    .dev            = {
        .platform_data  = &ppm_plat_data,
        }
};
#endif
/* end add */

/* add by cym 20150921 */
#if defined(CONFIG_BUZZER_CTL)
struct platform_device buzzer_plat_device = {
        .name   = "buzzer_ctl",
        .id             = -1,
};
#endif

#if defined(CONFIG_LEDS_CTL)
struct platform_device leds_plat_device = {
        .name   = "leds_ctl",
        .id             = -1,
};
#endif

#if defined(CONFIG_MAX485_CTL)
struct platform_device max485_plat_device = {
        .name   = "max485_ctl",
        .id             = -1,
};
#endif
/* end add */
/*------------------------------------------------------------------------------
 * NAND device
 */
#if defined(CONFIG_MTD_NAND_NXP)
#include <linux/mtd/partitions.h>
#include <asm-generic/sizes.h>

static struct mtd_partition nxp_nand_parts[] = {
#if 0
	{
		.name           = "root",
		.offset         =   0 * SZ_1M,
	},
#else
	{
		.name		= "system",
		.offset		=  64 * SZ_1M,
		.size		= 512 * SZ_1M,
	}, {
		.name		= "cache",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 256 * SZ_1M,
	}, {
		.name		= "userdata",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	}
#endif
};

static struct nxp_nand_plat_data nand_plat_data = {
	.parts		= nxp_nand_parts,
	.nr_parts	= ARRAY_SIZE(nxp_nand_parts),
	.chip_delay = 10,
};

static struct platform_device nand_plat_device = {
	.name	= DEV_NAME_NAND,
	.id		= -1,
	.dev	= {
		.platform_data	= &nand_plat_data,
	},
};
#endif	/* CONFIG_MTD_NAND_NXP */

#if defined(CONFIG_TOUCHSCREEN_GSLX680)
#include <linux/i2c.h>
#define	GSLX680_I2C_BUS		(1)

static struct i2c_board_info __initdata gslX680_i2c_bdi = {
	.type	= "gslX680",
	.addr	= (0x40),
    	.irq    = PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),
};
#endif

/* add by cym 20150812 */
#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
#include <linux/i2c.h>
#include <ft5x0x_touch.h>

#define FT5X0X_I2C_BUS         (1)

static struct ft5x0x_i2c_platform_data ft5x0x_pdata = {
        .gpio_irq               = CFG_IO_TOUCH_PENDOWN_DETECT,
        //.irq_cfg                = S3C_GPIO_SFN(0xf),
        .screen_max_x   = 768,
        .screen_max_y   = 1024,
        .pressure_max   = 255,
};

static struct i2c_board_info __initdata ft5x0x_i2c_bdi = {
        .type   = "ft5x0x_ts",
        .addr   = (0x70>>1),
        .irq    = PB_PIO_IRQ(CFG_IO_TOUCH_PENDOWN_DETECT),

	//I2C_BOARD_INFO("ft5x0x_ts", 0x70>>1),
        //.irq = IRQ_EINT(4),
        .platform_data = &ft5x0x_pdata,
};
#endif
/* end add */

/* add by cym 20151128 */
#if 1
static int __init setup_width_height_param(char *str)
{
        if (!strncasecmp("9.7", str, 3)) {
		//printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
		//printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
                ft5x0x_pdata.screen_max_x = 768;
                ft5x0x_pdata.screen_max_y = 1024;
#endif

#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
		//printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
		fb0_plat_data.x_resol = 1024;
		fb0_plat_data.y_resol = 768;
#endif
#endif
        }
        else if(!strncasecmp("7.0", str, 3))
        {
#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
		//printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
                ft5x0x_pdata.screen_max_x = 800;
                ft5x0x_pdata.screen_max_y = 1280;
#endif

#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
		//printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
		fb0_plat_data.x_resol = 800;
                fb0_plat_data.y_resol = 1280;
#endif
#endif
        }

/* add by cym 20160222 */
#if 1
	else if(!strncasecmp("4.3", str, 3))
	{
		#if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
                //printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
                fb0_plat_data.x_resol = 480;
                fb0_plat_data.y_resol = 272;
#endif
#endif
	}
#endif
/* end add */

/* add by cym 20161104 */
#if 1
        else if(!strncasecmp("hdmi", str, 3))
        {
                #if defined (CONFIG_FB_NXP)
#if defined (CONFIG_FB0_NXP)
                //printk("fun:%s, line = %d(lcdtype:%s)\n", __FUNCTION__, __LINE__, str);
                fb0_plat_data.x_resol = 1920;
                fb0_plat_data.y_resol = 1080;
#endif
#endif
        }
#endif
/* end add */

	//printk("fun:%s, line = %d\n", __FUNCTION__, __LINE__);
}
early_param("lcdtype", setup_width_height_param);
#endif
/* end add */

/* add by cym 20160222 */
#ifdef CONFIG_TOUCHSCREEN_TSC2007
#include <linux/i2c/tsc2007.h>

#define GPIO_TSC_PORT ((PAD_GPIO_C + 26))
static int ts_get_pendown_state(void)
{
        int val;

        val = gpio_get_value(GPIO_TSC_PORT);

        return !val;
}

static int ts_init(void)
{
        int err;
#if 0
        err = gpio_request_one(GPIO_TSC_PORT, GPIOF_IN, "TSC2007_IRQ");
        if (err) {
                printk(KERN_ERR "failed to request TSC2007_IRQ pin\n");
                return -1;
        }

        s3c_gpio_cfgpin(GPIO_TSC_PORT, S3C_GPIO_SFN(0xF));
        s3c_gpio_setpull(GPIO_TSC_PORT, S3C_GPIO_PULL_NONE);
        gpio_free(GPIO_TSC_PORT);
#endif
	gpio_to_irq(GPIO_TSC_PORT);

        return 0;
}

static struct tsc2007_platform_data tsc2007_info = {
        .model                  = 2007,
        .x_plate_ohms           = 180,
        .get_pendown_state      = ts_get_pendown_state,
        .init_platform_hw       = ts_init,
};

static struct i2c_board_info __initdata tsc2007_i2c_bdi = {
        .type   = "tsc2007",
        .addr   = (0x48),
        .irq    = PB_PIO_IRQ(CFG_IO_TSC2007_TOUCH_PENDOWN_DETECT),

        .platform_data = &tsc2007_info,
};
#endif
/* end add */

/*------------------------------------------------------------------------------
 * Keypad platform device
 */
#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)

#include <linux/input.h>

static unsigned int  button_gpio[] = CFG_KEYPAD_KEY_BUTTON;
static unsigned int  button_code[] = CFG_KEYPAD_KEY_CODE;

struct nxp_key_plat_data key_plat_data = {
	.bt_count	= ARRAY_SIZE(button_gpio),
	.bt_io		= button_gpio,
	.bt_code	= button_code,
	.bt_repeat	= CFG_KEYPAD_REPEAT,
};

static struct platform_device key_plat_device = {
	.name	= DEV_NAME_KEYPAD,
	.id		= -1,
	.dev    = {
		.platform_data	= &key_plat_data
	},
};
#endif	/* CONFIG_KEYBOARD_NXP_KEY || CONFIG_KEYBOARD_NXP_KEY_MODULE */

/*------------------------------------------------------------------------------
 * ASoC Codec platform device
 */
#if defined(CONFIG_SND_CODEC_WM8976) || defined(CONFIG_SND_CODEC_WM8976_MODULE)
#include <linux/i2c.h>

#define	WM8976_I2C_BUS		(0)

/* CODEC */
static struct i2c_board_info __initdata wm8976_i2c_bdi = {
	.type	= "wm8978",			// compatilbe with wm8976
	.addr	= (0x34>>1),		// 0x1A (7BIT), 0x34(8BIT)
};

/* DAI */
struct nxp_snd_dai_plat_data i2s_dai_data = {
	.i2s_ch	= 0,
	.sample_rate	= 48000,
	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_E + 8,
		/* modify by cym 20161104*/
#if 0
		.detect_level	= 1,
#else
		.detect_level   = 0,
#endif
		/* end modify */
	},
};

static struct platform_device wm8976_dai = {
	.name			= "wm8976-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &i2s_dai_data,
	}
};
#endif

#if defined(CONFIG_SND_CODEC_ALC5623)
#include <linux/i2c.h>

#define	WM8976_I2C_BUS		(0)

/* CODEC */
static struct i2c_board_info __initdata alc5623_i2c_bdi = {
	.type	= "alc562x-codec",			// compatilbe with wm8976
	.addr	= (0x34>>1),		// 0x1A (7BIT), 0x34(8BIT)
};

/* DAI */
struct nxp_snd_dai_plat_data i2s_dai_data = {
	.i2s_ch	= 0,
	.sample_rate	= 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
	.hp_jack 		= {
		.support    	= 1,
		.detect_io		= PAD_GPIO_B + 27,
		/* modify by cym 20161104*/
#if 0           
		.detect_level	= 1,
#else
		.detect_level   = 0,
#endif
		/* end modify */
	},
};

static struct platform_device alc5623_dai = {
	.name			= "alc5623-audio",
	.id				= 0,
	.dev			= {
		.platform_data	= &i2s_dai_data,
	}
};
#endif


#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
static struct platform_device spdif_transciever = {
	.name	= "spdif-dit",
	.id		= -1,
};

struct nxp_snd_dai_plat_data spdif_trans_dai_data = {
	.sample_rate = 48000,
	.pcm_format	 = SNDRV_PCM_FMTBIT_S16_LE,
};

static struct platform_device spdif_trans_dai = {
	.name	= "spdif-transciever",
	.id		= -1,
	.dev	= {
		.platform_data	= &spdif_trans_dai_data,
	}
};
#endif

/*------------------------------------------------------------------------------
 * G-Sensor platform device
 */
#if defined(CONFIG_SENSORS_MMA865X) || defined(CONFIG_SENSORS_MMA865X_MODULE)
#include <linux/i2c.h>

#define	MMA865X_I2C_BUS		(2)

/* CODEC */
static struct i2c_board_info __initdata mma865x_i2c_bdi = {
	.type	= "mma8653",
	.addr	= 0x1D//(0x4c),
};

#endif

#if defined(CONFIG_SENSORS_STK831X) || defined(CONFIG_SENSORS_STK831X_MODULE)
#include <linux/i2c.h>

#define	STK831X_I2C_BUS		(2)

/* CODEC */
static struct i2c_board_info __initdata stk831x_i2c_bdi = {
#if   defined CONFIG_SENSORS_STK8312
	.type	= "stk8312",
	.addr	= (0x3d),
#elif defined CONFIG_SENSORS_STK8313
	.type	= "stk8313",
	.addr	= (0x22),
#endif
};

#endif

/* add by cym 20151214 */
#if defined(CONFIG_MXC_MMA8451) || defined(CONFIG_MXC_MMA8451_MODULE)
#include <linux/i2c.h>

static int mma8451_position = 1;

static struct i2c_board_info __initdata mma8451_i2c_bdi = {
        .type   = "mma8451",
        .addr   = 0x1c,
	.platform_data = (void *)&mma8451_position,
};
#endif
/* end add */

/*------------------------------------------------------------------------------
 *  * reserve mem
 *   */
#ifdef CONFIG_CMA
#include <linux/cma.h>
extern void nxp_cma_region_reserve(struct cma_region *, const char *);

void __init nxp_reserve_mem(void)
{
    static struct cma_region regions[] = {
        {
            .name = "ion",
#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
            .size = CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K,
#else
			.size = 0,
#endif
            {
                .alignment = PAGE_SIZE,
            }
        },
        {
            .size = 0
        }
    };

    static const char map[] __initconst =
        "ion-nxp=ion;"
        "nx_vpu=ion;";

#ifdef CONFIG_ION_NXP_CONTIGHEAP_SIZE
    printk("%s: reserve CMA: size %d\n", __func__, CONFIG_ION_NXP_CONTIGHEAP_SIZE * SZ_1K);
#endif
    nxp_cma_region_reserve(regions, map);
}
#endif

#if defined(CONFIG_I2C_NXP)
#define I2CUDELAY(x)	1000000/x
/* gpio i2c 3 */
/* modify by cym 20150800 */
#if 0
#define	I2C3_SCL	PAD_GPIO_E + 14
#define	I2C3_SDA	PAD_GPIO_E + 15
#else
#define I2C3_SCL        PAD_GPIO_C + 15
#define I2C3_SDA        PAD_GPIO_C + 16
#endif
/* end modify */

static struct i2c_gpio_platform_data nxp_i2c_gpio_port3 = {
	.sda_pin	= I2C3_SDA,
	.scl_pin	= I2C3_SCL,
	/* modify by cym 20150811 */
#if 0
	.udelay		= I2CUDELAY(CFG_I2C2_CLK),				/* Gpio_mode CLK Rate = 1/( udelay*2) * 1000000 */
#else
	.udelay         = I2CUDELAY(CFG_I2C3_CLK),
#endif

	.timeout	= 10,
};


static struct platform_device i2c_device_ch3 = {
	.name	= "i2c-gpio",
	.id		= 3,
	.dev    = {
		.platform_data	= &nxp_i2c_gpio_port3,
	},
};

static struct platform_device *i2c_devices[] = {
	&i2c_device_ch3,
};
#endif /* CONFIG_I2C_NXP */

/*------------------------------------------------------------------------------
 * PMIC platform device
 */
#if defined(CONFIG_REGULATOR_NXE2000)

#include <linux/i2c.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/nxe2000.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/regulator/fixed.h>
#include <linux/regulator/nxe2000-regulator.h>
#include <linux/power/nxe2000_battery.h>
//#include <linux/rtc/rtc-nxe2000.h>
//#include <linux/rtc.h>

#define NXE2000_I2C_BUS		(3)
#define NXE2000_I2C_ADDR	(0x64 >> 1)

/* NXE2000 IRQs */
#define NXE2000_IRQ_BASE	(IRQ_SYSTEM_END)
#define NXE2000_GPIO_BASE	(ARCH_NR_GPIOS) //PLATFORM_NXE2000_GPIO_BASE
#define NXE2000_GPIO_IRQ	(NXE2000_GPIO_BASE + 8)

//#define CONFIG_NXE2000_RTC


static struct regulator_consumer_supply nxe2000_dc1_supply_0[] = {
	REGULATOR_SUPPLY("vdd_arm_1.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc2_supply_0[] = {
	REGULATOR_SUPPLY("vdd_core_1.2V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc3_supply_0[] = {
	REGULATOR_SUPPLY("vdd_sys_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc4_supply_0[] = {
	REGULATOR_SUPPLY("vdd_ddr_1.6V", NULL),
};
static struct regulator_consumer_supply nxe2000_dc5_supply_0[] = {
	REGULATOR_SUPPLY("vdd_sys_1.6V", NULL),
};

static struct regulator_consumer_supply nxe2000_ldo1_supply_0[] = {
	REGULATOR_SUPPLY("vgps_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo2_supply_0[] = {
	REGULATOR_SUPPLY("vcam1_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo3_supply_0[] = {
	REGULATOR_SUPPLY("vsys1_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo4_supply_0[] = {
	REGULATOR_SUPPLY("vsys_1.9V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo5_supply_0[] = {
	REGULATOR_SUPPLY("vcam_2.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo6_supply_0[] = {
	REGULATOR_SUPPLY("valive_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo7_supply_0[] = {
	REGULATOR_SUPPLY("vvid_2.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo8_supply_0[] = {
	REGULATOR_SUPPLY("vdumy0_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo9_supply_0[] = {
	REGULATOR_SUPPLY("vcam_3.3V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldo10_supply_0[] = {
	REGULATOR_SUPPLY("vdumy2_1.2V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldortc1_supply_0[] = {
	REGULATOR_SUPPLY("valive_1.8V", NULL),
};
static struct regulator_consumer_supply nxe2000_ldortc2_supply_0[] = {
	REGULATOR_SUPPLY("valive_1.0V", NULL),
};


#define NXE2000_PDATA_INIT(_name, _sname, _minuv, _maxuv, _always_on, _boot_on, \
	_init_uv, _init_enable, _slp_slots) \
	static struct nxe2000_regulator_platform_data pdata_##_name##_##_sname = \
	{	\
		.regulator = {	\
			.constraints = {	\
				.min_uV		= _minuv,	\
				.max_uV		= _maxuv,	\
				.valid_modes_mask	= (REGULATOR_MODE_NORMAL |	\
									REGULATOR_MODE_STANDBY),	\
				.valid_ops_mask		= (REGULATOR_CHANGE_MODE |	\
									REGULATOR_CHANGE_STATUS |	\
									REGULATOR_CHANGE_VOLTAGE),	\
				.always_on	= _always_on,	\
				.boot_on	= _boot_on,		\
				.apply_uV	= 1,			\
			},	\
			.num_consumer_supplies =		\
				ARRAY_SIZE(nxe2000_##_name##_supply_##_sname),	\
			.consumer_supplies	= nxe2000_##_name##_supply_##_sname, \
			.supply_regulator	= 0,	\
		},	\
		.init_uV		= _init_uv,		\
		.init_enable	= _init_enable,	\
		.sleep_slots	= _slp_slots,	\
	}
/* min_uV/max_uV : Please set the appropriate value for the devices that the power supplied within a*/
/*                 range from min to max voltage according to NXE2000 specification. */
NXE2000_PDATA_INIT(dc1,      0,	 950000, 2000000, 1, 1, 1200000, 1,  4);	/* 1.2V ARM */
NXE2000_PDATA_INIT(dc2,      0,	1000000, 2000000, 1, 1, 1100000, 1,  4);	/* 1.1V CORE */
NXE2000_PDATA_INIT(dc3,      0,	1000000, 3500000, 1, 1, 3300000, 1,  0);	/* 3.3V SYS */
NXE2000_PDATA_INIT(dc4,      0,	1000000, 2000000, 1, 1, 1500000, 1, -1);	/* 1.5V DDR */
NXE2000_PDATA_INIT(dc5,      0,	1000000, 2000000, 1, 1, 1500000, 1,  4);	/* 1.5V SYS */
#if defined(CONFIG_RFKILL_NXP)
NXE2000_PDATA_INIT(ldo1,     0,	1000000, 3500000, 0, 0, 3300000, 0,  0);	/* 3.3V GPS */
#else
NXE2000_PDATA_INIT(ldo1,     0,	1000000, 3500000, 0, 0, 3300000, 1,  0);	/* 3.3V GPS */
#endif
NXE2000_PDATA_INIT(ldo2,     0,	1000000, 3500000, 1, 1, 1800000, 0,  0);	/* 1.8V CAM1 */
NXE2000_PDATA_INIT(ldo3,     0,	1000000, 3500000, 1, 0, 1900000, 1,  2);	/* 1.8V SYS1 */
NXE2000_PDATA_INIT(ldo4,     0,	1000000, 3500000, 1, 0, 1900000, 1,  2);	/* 1.9V SYS */
NXE2000_PDATA_INIT(ldo5,     0,	1000000, 3500000, 1, 1, 2800000, 0,  0);	/* 2.8V VCAM */
NXE2000_PDATA_INIT(ldo6,     0,	1000000, 3500000, 1, 0, 3300000, 1, -1);	/* 3.3V ALIVE */
NXE2000_PDATA_INIT(ldo7,     0,	1000000, 3500000, 1, 0, 2800000, 1,  1);	/* 2.8V VID */
NXE2000_PDATA_INIT(ldo8,     0,	1000000, 3500000, 0, 0,      -1, 0,  0);	/* Not Use */
NXE2000_PDATA_INIT(ldo9,     0,	1000000, 3500000, 1, 1, 2800000, 0,  0);	/* 2.8V VCAM */
NXE2000_PDATA_INIT(ldo10,    0,	1000000, 3500000, 0, 0,      -1, 0,  0);	/* Not Use */
NXE2000_PDATA_INIT(ldortc1,  0,	1700000, 3500000, 1, 0, 1800000, 1, -1);	/* 1.8V ALIVE */
NXE2000_PDATA_INIT(ldortc2,  0,	1000000, 3500000, 1, 0, 1000000, 1, -1);	/* 1.0V ALIVE */


/*-------- if nxe2000 RTC exists -----------*/
#ifdef CONFIG_NXE2000_RTC
static struct nxe2000_rtc_platform_data rtc_data = {
	.irq	= NXE2000_IRQ_BASE,
	.time	= {
		.tm_year	= 1970,
		.tm_mon		= 0,
		.tm_mday	= 1,
		.tm_hour	= 0,
		.tm_min		= 0,
		.tm_sec		= 0,
	},
};

#define NXE2000_RTC_REG	\
{	\
	.id		= 0,	\
	.name	= "rtc_nxe2000",	\
	.platform_data	= &rtc_data,	\
}
#endif
/*-------- if Nexell RTC exists -----------*/

#define NXE2000_REG(_id, _name, _sname)	\
{	\
	.id		= NXE2000_ID_##_id,	\
	.name	= "nxe2000-regulator",	\
	.platform_data	= &pdata_##_name##_##_sname,	\
}

#define NXE2000_BATTERY_REG	\
{	\
    .id		= -1,	\
    .name	= "nxe2000-battery",	\
    .platform_data	= &nxe2000_battery_data,	\
}

//==========================================
//NXE2000 Power_Key device data
//==========================================
static struct nxe2000_pwrkey_platform_data nxe2000_pwrkey_data = {
	.irq 		= NXE2000_IRQ_BASE,
	.delay_ms 	= 20,
};
#define NXE2000_PWRKEY_REG		\
{	\
	.id 	= -1,	\
	.name 	= "nxe2000-pwrkey",	\
	.platform_data 	= &nxe2000_pwrkey_data,	\
}


static struct nxe2000_battery_platform_data nxe2000_battery_data = {
	.irq 				= NXE2000_IRQ_BASE,

	.input_power_type	= INPUT_POWER_TYPE_ADP_UBC,

	.gpio_otg_usbid		= CFG_GPIO_OTG_USBID_DET,
	.gpio_otg_vbus		= CFG_GPIO_OTG_VBUS_DET,
	.gpio_pmic_vbus		= CFG_GPIO_PMIC_VUSB_DET,
	.gpio_pmic_lowbat	= CFG_GPIO_PMIC_LOWBAT_DET,

	.low_vbat_vol_mv	= 3600,
	.low_vsys_vol_mv	= 3600,
	.bat_impe			= 1891,	//1500,
	.slp_ibat			= 10,	//3600,
	.multiple			= 100,	//100%
	.monitor_time		= 60,

	/* some parameter is depend of battery type */
	.type[0] = {
		.ch_vfchg		= 0x03,	/* VFCHG	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.ch_vrchg		= 0x03,	/* VRCHG	= 0 - 4 (3.85v, 3.90v, 3.95v, 4.00v, 4.10v) */
		.ch_vbatovset	= 0xFF,	/* VBATOVSET	= 0 or 1 (0 : 4.38v(up)/3.95v(down) 1: 4.53v(up)/4.10v(down)) */
		.ch_ichg 		= 0x07,	/* ICHG		= 0 - 0x1D (100mA - 3000mA) */
		.ch_ichg_slp	= 0x0E,	/* SLEEP  ICHG	= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_adp 	= 0x18,	/* ILIM_ADP	= 0 - 0x1D (100mA - 3000mA) */
		.ch_ilim_usb 	= 0x04,	/* ILIM_USB	= 0 - 0x1D (100mA - 3000mA) */
		.ch_icchg		= 0x00,	/* ICCHG	= 0 - 3 (50mA 100mA 150mA 200mA) */
		.fg_target_vsys	= 3250,	/* This value is the target one to DSOC=0% */
		.fg_target_ibat	= 1000,	/* This value is the target one to DSOC=0% */
		.fg_poff_vbat	= 3350,	/* setting value of 0 per Vbat */
		.jt_en			= 0,	/* JEITA Enable	  = 0 or 1 (1:enable, 0:disable) */
		.jt_hw_sw		= 1,	/* JEITA HW or SW = 0 or 1 (1:HardWare, 0:SoftWare) */
		.jt_temp_h		= 50,	/* degree C */
		.jt_temp_l		= 12,	/* degree C */
		.jt_vfchg_h 	= 0x03,	/* VFCHG High  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_vfchg_l 	= 0,	/* VFCHG Low  	= 0 - 4 (4.05v, 4.10v, 4.15v, 4.20v, 4.35v) */
		.jt_ichg_h		= 0x07,	/* ICHG High  	= 0 - 0x1D (100mA - 3000mA) */
		.jt_ichg_l		= 0x04,	/* ICHG Low   	= 0 - 0x1D (100mA - 3000mA) */
	},
	/*
	.type[1] = {
		.ch_vfchg		= 0x0,
		.ch_vrchg		= 0x0,
		.ch_vbatovset	= 0x0,
		.ch_ichg		= 0x0,
		.ch_ilim_adp	= 0x0,
		.ch_ilim_usb	= 0x0,
		.ch_icchg		= 0x00,
		.fg_target_vsys	= 3300,//3000,
		.fg_target_ibat	= 1000,//1000,
		.jt_en			= 0,
		.jt_hw_sw		= 1,
		.jt_temp_h		= 40,
		.jt_temp_l		= 10,
		.jt_vfchg_h		= 0x0,
		.jt_vfchg_l		= 0,
		.jt_ichg_h		= 0x01,
		.jt_ichg_l		= 0x01,
	},
	*/

/*  JEITA Parameter
*
*          VCHG
*            |
* jt_vfchg_h~+~~~~~~~~~~~~~~~~~~~+
*            |                   |
* jt_vfchg_l-| - - - - - - - - - +~~~~~~~~~~+
*            |    Charge area    +          |
*  -------0--+-------------------+----------+--- Temp
*            !                   +
*          ICHG
*            |                   +
*  jt_ichg_h-+ - -+~~~~~~~~~~~~~~+~~~~~~~~~~+
*            +    |              +          |
*  jt_ichg_l-+~~~~+   Charge area           |
*            |    +              +          |
*         0--+----+--------------+----------+--- Temp
*            0   jt_temp_l      jt_temp_h   55
*/
};



#define NXE2000_DEV_REG 		\
	NXE2000_REG(DC1, dc1, 0),	\
	NXE2000_REG(DC2, dc2, 0),	\
	NXE2000_REG(DC3, dc3, 0),	\
	NXE2000_REG(DC4, dc4, 0),	\
	NXE2000_REG(DC5, dc5, 0),	\
	NXE2000_REG(LDO1, ldo1, 0),	\
	NXE2000_REG(LDO2, ldo2, 0),	\
	NXE2000_REG(LDO3, ldo3, 0),	\
	NXE2000_REG(LDO4, ldo4, 0),	\
	NXE2000_REG(LDO5, ldo5, 0),	\
	NXE2000_REG(LDO6, ldo6, 0),	\
	NXE2000_REG(LDO7, ldo7, 0),	\
	NXE2000_REG(LDO8, ldo8, 0),	\
	NXE2000_REG(LDO9, ldo9, 0),	\
	NXE2000_REG(LDO10, ldo10, 0),	\
	NXE2000_REG(LDORTC1, ldortc1, 0),	\
	NXE2000_REG(LDORTC2, ldortc2, 0)

static struct nxe2000_subdev_info nxe2000_devs_dcdc[] = {
	NXE2000_DEV_REG,
	NXE2000_BATTERY_REG,
	NXE2000_PWRKEY_REG,
#ifdef CONFIG_NXE2000_RTC
	NXE2000_RTC_REG,
#endif
};


#define NXE2000_GPIO_INIT(_init_apply, _output_mode, _output_val, _led_mode, _led_func) \
	{									\
		.output_mode_en = _output_mode,	\
		.output_val		= _output_val,	\
		.init_apply		= _init_apply,	\
		.led_mode		= _led_mode,	\
		.led_func		= _led_func,	\
	}
struct nxe2000_gpio_init_data nxe2000_gpio_data[] = {
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
	NXE2000_GPIO_INIT(false, false, 0, 0, 0),
};

static struct nxe2000_platform_data nxe2000_platform = {
	.num_subdevs		= ARRAY_SIZE(nxe2000_devs_dcdc),
	.subdevs			= nxe2000_devs_dcdc,
	.irq_base			= NXE2000_IRQ_BASE,
	.irq_type			= IRQ_TYPE_EDGE_FALLING,
	.gpio_base			= NXE2000_GPIO_BASE,
	.gpio_init_data		= nxe2000_gpio_data,
	.num_gpioinit_data	= ARRAY_SIZE(nxe2000_gpio_data),
	.enable_shutdown_pin	= true,
};

static struct i2c_board_info __initdata nxe2000_i2c_boardinfo[] = {
	{
		I2C_BOARD_INFO("nxe2000", NXE2000_I2C_ADDR),
		.irq			= CFG_GPIO_PMIC_INTR,
		.platform_data	= &nxe2000_platform,
	},
};
#endif  /* CONFIG_REGULATOR_NXE2000 */

/*------------------------------------------------------------------------------
 * v4l2 platform device
 */
#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <mach/nxp-v4l2-platformdata.h>
#include <mach/soc.h>

static int camera_common_set_clock(ulong clk_rate)
{
    PM_DBGOUT("%s: %d\n", __func__, (int)clk_rate);
    if (clk_rate > 0)
        nxp_soc_pwm_set_frequency(1, clk_rate, 50);
    else
        nxp_soc_pwm_set_frequency(1, 0, 0);
    msleep(1);
    return 0;
}

static bool is_camera_port_configured = false;

void camera_common_vin_setup_io(int module, bool force)
{
    if (!force && is_camera_port_configured)
        return;
    else {
        u_int *pad;
        int i, len;
        u_int io, fn;


        /* VIP0:0 = VCLK, VID0 ~ 7 */
        const u_int port[][2] = {
/* modify by cym 20150911 */
/* #if 0 */
#if 1
/* end modify */
            /* VCLK, HSYNC, VSYNC */
            { PAD_GPIO_E +  4, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  5, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  6, NX_GPIO_PADFUNC_1 },
            /* DATA */
            { PAD_GPIO_D + 28, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 29, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_D + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_D + 31, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  0, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  1, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_E +  3, NX_GPIO_PADFUNC_1 },
#endif

/* remove by cym 20150911 */
#if 0
             /* VCLK, HSYNC, VSYNC */
            { PAD_GPIO_A + 28, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_E + 13, NX_GPIO_PADFUNC_2 },
            { PAD_GPIO_E +  7, NX_GPIO_PADFUNC_2 },

            { PAD_GPIO_A + 30, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  0, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_B +  2, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  4, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_B +  6, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B +  8, NX_GPIO_PADFUNC_1 },
            { PAD_GPIO_B +  9, NX_GPIO_PADFUNC_1 }, { PAD_GPIO_B + 10, NX_GPIO_PADFUNC_1 },
#endif
/* end remove */
        };

        printk("%s\n", __func__);

        pad = (u_int *)port;
        len = sizeof(port)/sizeof(port[0]);

        for (i = 0; i < len; i++) {
            io = *pad++;
            fn = *pad++;
            nxp_soc_gpio_set_io_dir(io, 0);
            nxp_soc_gpio_set_io_func(io, fn);
        }

        is_camera_port_configured = true;
    }
}

EXPORT_SYMBOL(camera_common_vin_setup_io);
static bool camera_power_enabled = false;
static void camera_power_control(int enable)
{
    struct regulator *cam_io_28V = NULL;
    struct regulator *cam_core_18V = NULL;
    struct regulator *cam_io_33V = NULL;

    if (enable && camera_power_enabled)
        return;
    if (!enable && !camera_power_enabled)
        return;

    cam_core_18V = regulator_get(NULL, "vcam1_1.8V");
    if (IS_ERR(cam_core_18V)) {
        printk(KERN_ERR "%s: failed to regulator_get() for vcam1_1.8V", __func__);
        return;
    }
#if 0   //add by dg 2015 09 11
    cam_io_28V = regulator_get(NULL, "vcam_2.8V");
    if (IS_ERR(cam_io_28V)) {
        printk(KERN_ERR "%s: failed to regulator_get() for vcam_2.8V", __func__);
        return;
    }

    cam_io_33V = regulator_get(NULL, "vcam_3.3V");
    if (IS_ERR(cam_io_33V)) {
        printk(KERN_ERR "%s: failed to regulator_get() for vcam_3.3V", __func__);
        return;
    }
#endif
    printk("%s: %d\n", __func__, enable);
    if (enable) {
        regulator_enable(cam_core_18V);
//by dg
//      regulator_enable(cam_io_28V);
//      regulator_enable(cam_io_33V);
    } else {
// by dg
//    regulator_disable(cam_io_33V);
//    regulator_disable(cam_io_28V);
       regulator_disable(cam_core_18V);
    }

 //  regulator_put(cam_io_28V);  //by dg
     regulator_put(cam_core_18V);
//   regulator_put(cam_io_33V);  //by dg

    camera_power_enabled = enable ? true : false;
}

static bool is_back_camera_enabled = false;
static bool is_back_camera_power_state_changed = false;
static bool is_front_camera_enabled = false;
static bool is_front_camera_power_state_changed = false;

static int front_camera_power_enable(bool on);

int back_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_BACK_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
 //   PM_DBGOUT("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);

    printk("%s: is_back_camera_enabled %d, on %d\n", __func__, is_back_camera_enabled, on);

    if (on)  //power on
    {
     //   front_camera_power_enable(0);
        if (!is_back_camera_enabled)
        {
            camera_power_control(1);
            /* PD signal */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            camera_common_set_clock(24000000);
            /* mdelay(10); */
            mdelay(5);
#if defined CONFIG_VIDEO_OV5640
		nxp_soc_gpio_set_out_value(io, 0);
#else
            nxp_soc_gpio_set_out_value(io, 1);
#endif
            /* RST signal */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);
            /* mdelay(100); */
            mdelay(5);
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(1);
            is_back_camera_enabled = true;
            is_back_camera_power_state_changed = true;
        } else
        {
            is_back_camera_power_state_changed = false;
        }

    } else  //power off
    {
        if (is_back_camera_enabled)
        {
            nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_out_value(reset_io, 0);
            is_back_camera_enabled = false;
            is_back_camera_power_state_changed = true;

        } else
        {
            nxp_soc_gpio_set_out_value(io, 1);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            is_back_camera_power_state_changed = false;
        }

        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}
EXPORT_SYMBOL(back_camera_power_enable);

static bool back_camera_power_state_changed(void)
{
    return is_back_camera_power_state_changed;
}

//struct soc_camera_device ov5640_plat = {
 //   .user_width = 640,
  //  .user_height = 480,
//};
static struct i2c_board_info back_camera_i2c_boardinfo[] = {
#if 0  //dg cancel2015-09-14
    {
        I2C_BOARD_INFO("SP2518", 0x60>>1),
    },
#endif

#ifdef CONFIG_VIDEO_OV5640
    {
    	I2C_BOARD_INFO("OV5640", 0x78>>1),
    }
#endif

#ifdef CONFIG_VIDEO_TVP5150
    {
        I2C_BOARD_INFO("GM7150", 0xBA>>1),
    }
#endif
};

static int front_camera_power_enable(bool on)
{
    unsigned int io = CFG_IO_CAMERA_FRONT_POWER_DOWN;
    unsigned int reset_io = CFG_IO_CAMERA_RESET;
    PM_DBGOUT("%s: is_front_camera_enabled %d, on %d\n", __func__, is_front_camera_enabled, on);
    if (on) {
        back_camera_power_enable(0);
        if (!is_front_camera_enabled) {
            camera_power_control(1);
            /* First RST signal to low */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            nxp_soc_gpio_set_io_dir(reset_io, 1);
            nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(reset_io, 0);
            mdelay(1);

            /* PWDN signal High to Low */
            nxp_soc_gpio_set_out_value(io, 0);
            nxp_soc_gpio_set_io_dir(io, 1);
            nxp_soc_gpio_set_io_func(io, nxp_soc_gpio_get_altnum(io));
            nxp_soc_gpio_set_out_value(io, 1);
            camera_common_set_clock(24000000);
            mdelay(10);
            /* mdelay(1); */
            nxp_soc_gpio_set_out_value(io, 0);
            /* mdelay(10); */
            mdelay(10);

            /* RST signal  to High */
            nxp_soc_gpio_set_out_value(reset_io, 1);
            /* mdelay(100); */
            mdelay(5);

            is_front_camera_enabled = true;
            is_front_camera_power_state_changed = true;
        } else {
            is_front_camera_power_state_changed = false;
        }
    } else {
        if (is_front_camera_enabled) {
            nxp_soc_gpio_set_out_value(io, 1);
            is_front_camera_enabled = false;
            is_front_camera_power_state_changed = true;
        } else {
            nxp_soc_gpio_set_out_value(io, 1);
            is_front_camera_power_state_changed = false;
        }
        if (!(is_back_camera_enabled || is_front_camera_enabled)) {
            camera_power_control(0);
        }
    }

    return 0;
}

static bool front_camera_power_state_changed(void)
{
    return is_front_camera_power_state_changed;
}

#ifdef CONFIG_VIDEO_SP0838
static struct i2c_board_info front_camera_i2c_boardinfo[] = {
    {
        I2C_BOARD_INFO("SP0838", 0x18),
    },
};

#endif
static struct nxp_v4l2_i2c_board_info sensor[] = {
    {
        .board_info = &back_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 0,
    },
#ifdef CONFIG_VIDEO_SP0838
    {
        .board_info = &front_camera_i2c_boardinfo[0],
        .i2c_adapter_id = 0,
    },
#endif
};


static struct nxp_capture_platformdata capture_plat_data[] = {
#if  0
    {
        /* back_camera 656 interface */
        .module = 1,
        .sensor = &sensor[0],
        .type = NXP_CAPTURE_INF_PARALLEL,
        .parallel = {
            /* for 656 */
            .is_mipi        = false,
            .external_sync  = false, /* 656 interface */
            .h_active       = 640,//800,
            .h_frontporch   = 7,
            .h_syncwidth    = 1,
            .h_backporch    = 10,
            .v_active       = 480,//600,
            .v_frontporch   = 0,
            .v_syncwidth    = 2,
            .v_backporch    = 3,
            .clock_invert   = true,
            .port           = 0,
            .data_order     = NXP_VIN_Y0CBY1CR
            .interlace      = false,
            .clk_rate       = 24000000,
            .late_power_down = true,
            .power_enable   = back_camera_power_enable,
            .power_state_changed = back_camera_power_state_changed,
            .set_clock      = camera_common_set_clock,
            .setup_io       = camera_common_vin_setup_io,
        },
        .deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
    },
#endif

//#if 1
#ifdef CONFIG_VIDEO_OV5640
{//OV5640
   /* back_camera 656 interface */
   .module =1,
   .sensor = &sensor[0],
   .type = NXP_CAPTURE_INF_PARALLEL,
   .parallel = {
       .is_mipi        = false,
       .external_sync  = false,
       .h_active       = 1280,//1920,640
       .h_frontporch   = 0,
       .h_syncwidth    = 0,
       .h_backporch    = 2,
       .v_active       = 960,//1080,480
       .v_frontporch   = 0,
       .v_syncwidth    = 0,
       .v_backporch    = 13,
       .clock_invert   = false,
       .port           = 0,
       .data_order     = NXP_VIN_Y0CBY1CR,
       .interlace      = false,
       .clk_rate       = 24000000,
       .late_power_down = true,
       .power_enable   = back_camera_power_enable,
       .power_state_changed = back_camera_power_state_changed,
       .set_clock      = camera_common_set_clock,
       .setup_io       = camera_common_vin_setup_io,
   },
   .deci = {
       .start_delay_ms = 0,
       .stop_delay_ms  = 0,
   },
},
#endif

/* add by cym 20151228 */
#ifdef CONFIG_VIDEO_TVP5150
{//OV5640
   /* back_camera 656 interface */
   .module =1,
   .sensor = &sensor[0],
   .type = NXP_CAPTURE_INF_PARALLEL,
   .parallel = {
       .is_mipi        = false,
       .external_sync  = false,
       .h_active       = 640,//1280,//1920,640
       .h_frontporch   = 0,
       .h_syncwidth    = 0,
       .h_backporch    = 2,
       .v_active       = 480,//960,//1080,480
       .v_frontporch   = 0,
       .v_syncwidth    = 0,
       .v_backporch    = 2,//13,
       .clock_invert   = false,
       .port           = 0,
       .data_order     = NXP_VIN_CBY0CRY1,//NXP_VIN_Y0CBY1CR,
       .interlace      = false,
       .clk_rate       = 27000000,//24000000,
       .late_power_down = false,//true,
       .power_enable   = back_camera_power_enable,
       .power_state_changed = back_camera_power_state_changed,
       .set_clock      = camera_common_set_clock,
       .setup_io       = camera_common_vin_setup_io,
   },
   .deci = {
       .start_delay_ms = 0,
       .stop_delay_ms  = 0,
   },
},
#endif
/* end add */

#ifdef CONFIG_VIDEO_SP0838
    {
        /* front_camera 601 interface */
        .module = 0,
        .sensor = &sensor[1],
        .type = NXP_CAPTURE_INF_PARALLEL,
        .parallel = {
            .is_mipi        = false,
            .external_sync  = true,
            .h_active       = 640,
            .h_frontporch   = 0,
            .h_syncwidth    = 0,
            .h_backporch    = 2,
            .v_active       = 480,
            .v_frontporch   = 0,
            .v_syncwidth    = 0,
            .v_backporch    = 2,
            .clock_invert   = false,
            .port           = 0,
            .data_order     = NXP_VIN_CBY0CRY1,
            .interlace      = false,
            .clk_rate       = 24000000,
            .late_power_down = true,
            .power_enable   = front_camera_power_enable,
            .power_state_changed = front_camera_power_state_changed,
            .set_clock      = camera_common_set_clock,
            .setup_io       = camera_common_vin_setup_io,
        },
        .deci = {
            .start_delay_ms = 0,
            .stop_delay_ms  = 0,
        },
    },
#endif
    { 0, NULL, 0, },
};
/* out platformdata */
static struct i2c_board_info hdmi_edid_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_edid", 0xA0>>1),
};

static struct nxp_v4l2_i2c_board_info edid = {
    .board_info = &hdmi_edid_i2c_boardinfo,
    .i2c_adapter_id = 0,
};

static struct i2c_board_info hdmi_hdcp_i2c_boardinfo = {
    I2C_BOARD_INFO("nxp_hdcp", 0x74>>1),
};

static struct nxp_v4l2_i2c_board_info hdcp = {
    .board_info = &hdmi_hdcp_i2c_boardinfo,
    .i2c_adapter_id = 0,
};


static void hdmi_set_int_external(int gpio)
{
    nxp_soc_gpio_set_int_enable(gpio, 0);
    nxp_soc_gpio_set_int_mode(gpio, 1); /* high level */
    nxp_soc_gpio_set_int_enable(gpio, 1);
    nxp_soc_gpio_clr_int_pend(gpio);
}

static void hdmi_set_int_internal(int gpio)
{
    nxp_soc_gpio_set_int_enable(gpio, 0);
    nxp_soc_gpio_set_int_mode(gpio, 0); /* low level */
    nxp_soc_gpio_set_int_enable(gpio, 1);
    nxp_soc_gpio_clr_int_pend(gpio);
}

static int hdmi_read_hpd_gpio(int gpio)
{
    return nxp_soc_gpio_get_in_value(gpio);
}

static struct nxp_out_platformdata out_plat_data = {
    .hdmi = {
        .internal_irq = 0,
        .external_irq = 0,//PAD_GPIO_A + 19,
        .set_int_external = hdmi_set_int_external,
        .set_int_internal = hdmi_set_int_internal,
        .read_hpd_gpio = hdmi_read_hpd_gpio,
        .edid = &edid,
        .hdcp = &hdcp,
    },
};

static struct nxp_v4l2_platformdata v4l2_plat_data = {
    .captures = &capture_plat_data[0],
    .out = &out_plat_data,
};

static struct platform_device nxp_v4l2_dev = {
    .name       = NXP_V4L2_DEV_NAME,
    .id         = 0,
    .dev        = {
        .platform_data = &v4l2_plat_data,
    },
};
#endif /* CONFIG_V4L2_NXP || CONFIG_V4L2_NXP_MODULE */

/*------------------------------------------------------------------------------
 * SSP/SPI
 */
/* modify by cym 20151214 */
//#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE) \
				|| defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
/* end modify */
#include <linux/spi/spi.h>
static void spi0_cs(u32 chipselect)
{
#if (CFG_SPI0_CS_GPIO_MODE)
	if(nxp_soc_gpio_get_io_func( CFG_SPI0_CS )!= nxp_soc_gpio_get_altnum( CFG_SPI0_CS))
		nxp_soc_gpio_set_io_func( CFG_SPI0_CS, nxp_soc_gpio_get_altnum( CFG_SPI0_CS));

	nxp_soc_gpio_set_io_dir( CFG_SPI0_CS,1);
	nxp_soc_gpio_set_out_value(	 CFG_SPI0_CS , chipselect);
#else
	;
#endif
}
struct pl022_config_chip spi0_info = {
    /* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
    .com_mode = CFG_SPI0_COM_MODE,
    .iface = SSP_INTERFACE_MOTOROLA_SPI,
    /* We can only act as master but SSP_SLAVE is possible in theory */
    .hierarchy = SSP_MASTER,
    /* 0 = drive TX even as slave, 1 = do not drive TX as slave */
    .slave_tx_disable = 1,
    .rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
    .tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
    .ctrl_len = SSP_BITS_8,
    .wait_state = SSP_MWIRE_WAIT_ZERO,
    .duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
    /*
     * This is where you insert a call to a function to enable CS
     * (usually GPIO) for a certain chip.
     */
#if (CFG_SPI0_CS_GPIO_MODE)
    .cs_control = spi0_cs,
#endif
	.clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,

};
/* add by cym 20151214 */
#endif
/* end add */

/* add by cym 20160618 */
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE) \
                                || defined(CONFIG_SPI_RC522) || defined(CONFIG_SPI_RC522_MODULE) \
				|| defined(CONFIG_CAN_MCP251X) || defined(CONFIG_CAN_MCP251X_MODULE)
#include <linux/spi/spi.h>
static void spi2_cs(u32 chipselect)
{
#if (CFG_SPI2_CS_GPIO_MODE)
	if(nxp_soc_gpio_get_io_func( CFG_SPI2_CS )!= nxp_soc_gpio_get_altnum( CFG_SPI2_CS))
		nxp_soc_gpio_set_io_func( CFG_SPI2_CS, nxp_soc_gpio_get_altnum( CFG_SPI2_CS));

	nxp_soc_gpio_set_io_dir( CFG_SPI2_CS,1);
	nxp_soc_gpio_set_out_value(	 CFG_SPI2_CS , chipselect);
#else
	;
#endif
}

struct pl022_config_chip spi2_info = {
    /* available POLLING_TRANSFER, INTERRUPT_TRANSFER, DMA_TRANSFER */
    .com_mode = CFG_SPI2_COM_MODE,
    .iface = SSP_INTERFACE_MOTOROLA_SPI,
    /* We can only act as master but SSP_SLAVE is possible in theory */
    .hierarchy = SSP_MASTER,
    /* 0 = drive TX even as slave, 1 = do not drive TX as slave */
    .slave_tx_disable = 1,
    .rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
    .tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
    .ctrl_len = SSP_BITS_8,
    .wait_state = SSP_MWIRE_WAIT_ZERO,
    .duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
    /*
     * This is where you insert a call to a function to enable CS
     * (usually GPIO) for a certain chip.
     */
#if (CFG_SPI2_CS_GPIO_MODE)
    .cs_control = spi2_cs,
#endif
	.clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,

};
#endif
/* end add */

/* add by cym 20151214 */
#if 0
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
/* end add */
static struct spi_board_info spi_plat_board[] __initdata = {
    [0] = {
        .modalias        = "spidev",    /* fixup */
        .max_speed_hz    = 3125000,     /* max spi clock (SCK) speed in HZ */
        .bus_num         = 0,           /* Note> set bus num, must be smaller than ARRAY_SIZE(spi_plat_device) */
        .chip_select     = 0,           /* Note> set chip select num, must be smaller than spi cs_num */
        .controller_data = &spi0_info,
        .mode            = SPI_MODE_3 | SPI_CPOL | SPI_CPHA,
    },
};

#endif
/* add by cym 20151214 */
#endif
/* end add */

/* add by cym 20151214 */
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
#include <linux/spi/flash.h>
#include <linux/mtd/partitions.h>

static struct mtd_partition w25q32_partitions[] = {
        {
                .name   = "partition1",
                .offset = 0,
                .size   = 0x00100000,
        }, {
                .name   = "partition2",
                .offset = MTDPART_OFS_APPEND,
                .size   = MTDPART_SIZ_FULL,
        },
};

struct flash_platform_data davinci_m25P32_info =
{
        .name = "w25q32",
        .parts = w25q32_partitions,
        .nr_parts = ARRAY_SIZE(w25q32_partitions),
        .type = "w25q32",
};

static struct spi_board_info w25q32_plat_board[] __initdata = {
	[0] = {
        .modalias        = "m25p80",
        .platform_data = &davinci_m25P32_info,
        .max_speed_hz = 4000000,//20000000,//4000000, /*4MHZ*/
        .bus_num         = 0,
        .controller_data = &spi0_info,
        .chip_select     = 0,
    },
};
#endif
/* end add */

/* add by cym 20160618 */
#if defined(CONFIG_SPI_RC522) || defined(CONFIG_SPI_RC522_MODULE)
static struct spi_board_info rc522_plat_board[] __initdata = {
	[0] = {
        .modalias        = "rc522",
        .max_speed_hz = 4000000,//20000000,//4000000, /*4MHZ*/
        .bus_num         = 2,
        .controller_data = &spi2_info,
        .chip_select     = 0,
    },
};
#endif
/* end add */

// add by cch 20160920
#ifdef CONFIG_CAN_MCP251X
#include <linux/can/platform/mcp251x.h>
#define CAN_GPIO  ((PAD_GPIO_E + 13))
static int mcp251x_ioSetup(struct spi_device *spi)
{
        int err;
#if 0
        printk("mcp251x: setup gpio pins CS and External Int\n");
        err = gpio_request_one(CAN_GPIO, GPIOF_IN, "mcp251x_INT");
        if (err) {
                printk(KERN_ERR "failed to request mcp251x_INT\n");
                return -1;
        }

        s3c_gpio_cfgpin(CAN_GPIO, S3C_GPIO_SFN(0xf));
        s3c_gpio_setpull(CAN_GPIO, S3C_GPIO_PULL_NONE);
        gpio_free(CAN_GPIO);
#endif
        gpio_to_irq(CAN_GPIO);

        return 0;
}

static struct mcp251x_platform_data mcp251x_info = {
        .oscillator_frequency = 8000000,
        .board_specific_setup = mcp251x_ioSetup,
};

static struct spi_board_info mcp2515_plat_board[] __initdata = {
        [0] = {
        .modalias        = "mcp2515",
        .platform_data = &mcp251x_info,
        .max_speed_hz = 4000000,//20000000,//4000000, /*4MHZ*/
        .bus_num         = 2,
        .controller_data = &spi2_info,
        .chip_select     = 0,
        .irq            = PB_PIO_IRQ(CAN_GPIO),
    },
};

#endif
// end add

/*------------------------------------------------------------------------------
 * DW MMC board config
 */
#if defined(CONFIG_MMC_DW)

#ifdef CONFIG_MMC_NXP_CH2
static struct dw_mci_board _dwmci2_data = {
    .quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION |
				  	  DW_MCI_QUIRK_HIGHSPEED |
				  	  DW_MMC_QUIRK_HW_RESET_PW |
				      DW_MCI_QUIRK_NO_DETECT_EBIT,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 |
					  MMC_CAP_NONREMOVABLE |
			 	  	  MMC_CAP_4_BIT_DATA | MMC_CAP_CMD23 |
				  	  MMC_CAP_ERASE | MMC_CAP_HW_RESET,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0x1c) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),

	.desc_sz		= 4,
	.detect_delay_ms= 200,
	.sdr_timing		= 0x01010001,
	.ddr_timing		= 0x03030002,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH2_USE_DMA)
	.mode       	= DMA_MODE,
#else
	.mode       	= PIO_MODE,
#endif

};
#endif

#ifdef CONFIG_MMC_NXP_CH1

/*
static struct dw_mci_board _dwmci1_data = {
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION,
	.bus_hz			= 50 * 1000 * 1000,
	.caps = MMC_CAP_CMD23|MMC_CAP_NONREMOVABLE,
	.detect_delay_ms= 200,
	.cd_type = DW_MCI_CD_NONE,
	.pm_caps        = MMC_PM_KEEP_POWER | MMC_PM_IGNORE_PM_NOTIFY,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(1) | DW_MMC_SAMPLE_PHASE(0),
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH1_USE_DMA)
	.mode       	= DMA_MODE,
#else
	.mode       	= PIO_MODE,
#endif

};
*/

//dg add for mt6620 2015-09-25

static int _dwmci1_ext_cd_init(void (*notify_func)(struct platform_device *, int state))
{
        return 0;
}

static int _dwmci1_ext_cd_cleanup(void (*notify_func)(struct platform_device *, int state))
{
        return 0;
}

static int _dwmci1_get_ro(u32 slot_id)
{
        return 0;
}

static int _dwmci1_init(u32 slot_id, irq_handler_t handler, void *data)
{
        struct dw_mci *host = (struct dw_mci *)data;
        int io  = CFG_IO_MT6620_CD_PIN;
        int irq = IRQ_GPIO_START + io;
        int id  = 1, ret = 0;

        printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

        ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                                DEV_NAME_SDHC "1", (void*)host->slot[slot_id]);
        if (0 > ret)
                pr_err("dw_mmc dw_mmc.%d: fail request interrupt %d ...\n", id, irq);
        return 0;
}

static int _dwmci1_get_cd(u32 slot_id)
{
        int io = CFG_IO_MT6620_CD_PIN;
        return nxp_soc_gpio_get_in_value(io);
}

static struct dw_mci_board _dwmci1_data = {
        .quirks			= DW_MCI_QUIRK_HIGHSPEED,
        .bus_hz			= 100 * 1000 * 1000,
        .caps			= MMC_CAP_CMD23,
        .detect_delay_ms= 200,
        .cd_type		= DW_MCI_CD_EXTERNAL,
        .clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
        .init			= _dwmci1_init,
        .get_ro         = _dwmci1_get_ro,
        .get_cd			= _dwmci1_get_cd,
        .ext_cd_init	= _dwmci1_ext_cd_init,
        .ext_cd_cleanup	= _dwmci1_ext_cd_cleanup,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH0_USE_DMA)
        .mode			= DMA_MODE,
#else
        .mode			= PIO_MODE,
#endif

};
#endif

#ifdef CONFIG_MMC_NXP_CH0
static int _dwmci_ext_cd_init(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

static int _dwmci_ext_cd_cleanup(void (*notify_func)(struct platform_device *, int state))
{
	return 0;
}

static int _dwmci_get_ro(u32 slot_id)
{
	return 0;
}

static int _dwmci0_init(u32 slot_id, irq_handler_t handler, void *data)
{
	struct dw_mci *host = (struct dw_mci *)data;
	int io  = CFG_SDMMC0_DETECT_IO;
	int irq = IRQ_GPIO_START + io;
	int id  = 0, ret = 0;

	printk("dw_mmc dw_mmc.%d: Using external card detect irq %3d (io %2d)\n", id, irq, io);

	ret  = request_irq(irq, handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				DEV_NAME_SDHC "0", (void*)host->slot[slot_id]);
	if (0 > ret)
		pr_err("dw_mmc dw_mmc.%d: fail request interrupt %d ...\n", id, irq);
	return 0;
}

static int _dwmci0_get_cd(u32 slot_id)
{
	int io = CFG_SDMMC0_DETECT_IO;
	return nxp_soc_gpio_get_in_value(io);
}

static struct dw_mci_board _dwmci0_data = {
	.quirks			= DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_CMD23,
	.detect_delay_ms= 200,
	.cd_type		= DW_MCI_CD_EXTERNAL,
	.clk_dly        = DW_MMC_DRIVE_DELAY(0) | DW_MMC_SAMPLE_DELAY(0) | DW_MMC_DRIVE_PHASE(2) | DW_MMC_SAMPLE_PHASE(1),
	.init			= _dwmci0_init,
	.get_ro         = _dwmci_get_ro,
	.get_cd			= _dwmci0_get_cd,
	.ext_cd_init	= _dwmci_ext_cd_init,
	.ext_cd_cleanup	= _dwmci_ext_cd_cleanup,
#if defined (CONFIG_MMC_DW_IDMAC) && defined (CONFIG_MMC_NXP_CH0_USE_DMA)
	.mode			= DMA_MODE,
#else
	.mode			= PIO_MODE,
#endif

};
#endif

#endif /* CONFIG_MMC_DW */

/*------------------------------------------------------------------------------
 * RFKILL driver
 */
#if defined(CONFIG_RFKILL_NXP)
struct rfkill_dev_data  rfkill_dev_data =
{
	.supply_name 	= "vgps_3.3V",	// vwifi_3.3V, vgps_3.3V
	.module_name 	= "wlan",
	.initval		= RFKILL_INIT_SET | RFKILL_INIT_OFF,
    .delay_time_off	= 1000,
};

struct nxp_rfkill_plat_data rfkill_plat_data = {
	.name		= "WiFi-Rfkill",
	.type		= RFKILL_TYPE_WLAN,
	.rf_dev		= &rfkill_dev_data,
    .rf_dev_num	= 1,
};

static struct platform_device rfkill_device = {
	.name			= DEV_NAME_RFKILL,
	.dev			= {
		.platform_data	= &rfkill_plat_data,
	}
};
#endif	/* CONFIG_RFKILL_NXP */

//add by dg 
#if defined(CONFIG_MTK_COMBO_MT66XX)



void setup_mt6620_wlan_power_for_onoff(int on)
{

    int chip_pwd_low_val;
    int outValue;

    printk("[mt6620] +++ %s : wlan power %s\n",__func__, on?"on":"off");

    int value_before = nxp_soc_gpio_get_in_value(CFG_IO_MT6620_CD_PIN);
    printk("[mt6620] --- %s---CFG_IO_MT6620_CD_PIN  first is %d\n",__func__,value_before);
    msleep(100);

    if (on) {
        outValue = 0;
    } else {
        outValue = 1;
    }

    nxp_soc_gpio_set_out_value(CFG_IO_MT6620_TRIGGER_PIN, outValue);

    msleep(100);



   int value = nxp_soc_gpio_get_in_value(CFG_IO_MT6620_CD_PIN);
   // int value = nxp_soc_gpio_get_in_value(CFG_SDMMC0_DETECT_IO);

    printk("[mt6620] --- %s---CFG_IO_MT6620_CD_PIN  second is %d\n",__func__,value);

    printk("[mt6620] --- %s\n",__func__);

}
EXPORT_SYMBOL(setup_mt6620_wlan_power_for_onoff);


static struct mtk_wmt_platform_data mtk_wmt_pdata = {
    .pmu = CFG_IO_MT6620_POWER_PIN,  //EXYNOS4_GPC1(0), //RK30SDK_WIFI_GPIO_POWER_N,//RK30_PIN0_PB5, //MUST set to pin num in target system
    .rst =  CFG_IO_MT6620_SYSRST_PIN, //EXYNOS4_GPC1(1),//RK30SDK_WIFI_GPIO_RESET_N,//RK30_PIN3_PD0, //MUST set to pin num in target system
    .bgf_int = CFG_IO_MT6620_BGF_INT_PIN, // EXYNOS4_GPX2(4), //IRQ_EINT(20),//RK30SDK_WIFI_GPIO_BGF_INT_B,//RK30_PIN0_PA5,//MUST set to pin num in target system if use UART interface.
    .urt_cts = -EINVAL, // set it to the correct GPIO num if use common SDIO, otherwise set it to -EINVAL.
    .rtc = -EINVAL, //Optipnal. refer to HW design.
    .gps_sync = -EINVAL, //Optional. refer to HW design.
    .gps_lna = -EINVAL, //Optional. refer to HW design.
};
static struct mtk_sdio_eint_platform_data mtk_sdio_eint_pdata = {
   // .sdio_eint = EXYNOS4_GPX2(5),//IRQ_EINT(21) ,//RK30SDK_WIFI_GPIO_WIFI_INT_B,//53, //MUST set pin num in target system.
     .sdio_eint = CFG_IO_MT6620_WIFI_INT_PIN,
};
static struct platform_device mtk_wmt_dev = {
    .name = "mtk_wmt",
    .id = 1,
    .dev = {


        .platform_data = &mtk_wmt_pdata,
    },
};
static struct platform_device mtk_sdio_eint_dev = {
    .name = "mtk_sdio_eint",
    .id = 1,
    .dev = {
        .platform_data = &mtk_sdio_eint_pdata,
    },
};
static void __init mtk_combo_init(void)
{

    unsigned int power_io = CFG_IO_MT6620_POWER_ENABLE;
    unsigned int reset_io = CFG_IO_MT6620_SYSRST;
    unsigned int wifi_interrupt_io = CFG_IO_MT6620_WIFI_INT;
    unsigned int bga_interrupt_io  =  CFG_IO_MT6620_BGF_INT;
    unsigned int carddetect_io = CFG_IO_MT6620_CD;
    unsigned int trigger_io = CFG_IO_MT6620_TRIGGER;


    /* Power Enable  signal init*/
    nxp_soc_gpio_set_out_value(power_io, 0);
    nxp_soc_gpio_set_io_dir(power_io, 1);
    nxp_soc_gpio_set_io_func(power_io, nxp_soc_gpio_get_altnum(power_io));



    /* SYSRST  signal init*/
    nxp_soc_gpio_set_out_value(reset_io, 0);
    nxp_soc_gpio_set_io_dir(reset_io, 1);
    nxp_soc_gpio_set_io_func(reset_io, nxp_soc_gpio_get_altnum(reset_io));


    mdelay(5);


    //need config eint models for Wifi & BGA Interrupt
    nxp_soc_gpio_set_io_dir(wifi_interrupt_io, 0);
    nxp_soc_gpio_set_io_func(wifi_interrupt_io, nxp_soc_gpio_get_altnum(wifi_interrupt_io));


    nxp_soc_gpio_set_io_dir(bga_interrupt_io, 0);
    nxp_soc_gpio_set_io_func(bga_interrupt_io, nxp_soc_gpio_get_altnum(bga_interrupt_io));


    //init trigger pin and cd detect pin
    nxp_soc_gpio_set_out_value(trigger_io, 1);
    nxp_soc_gpio_set_io_dir(trigger_io, 1);
    nxp_soc_gpio_set_io_func(trigger_io, nxp_soc_gpio_get_altnum(trigger_io));


    nxp_soc_gpio_set_io_dir(carddetect_io, 0);
    nxp_soc_gpio_set_io_func(carddetect_io, nxp_soc_gpio_get_altnum(carddetect_io));









    return;
}


static int  itop4418_wifi_combo_module_gpio_init(void)
{

    mtk_combo_init();
    platform_device_register(&mtk_wmt_dev);
    platform_device_register(&mtk_sdio_eint_dev);
}

#endif---//#if defined(CONFIG_MTK_COMBO_MT66XX)

/* add by cym 20160302 for EC220 4G Module */
#if 1
#define POWER_ON_4G     (PAD_GPIO_E + 22)
#define POWER_RESET_4G     (PAD_GPIO_A + 31)

static int itop4418_ec220_4g_module_gpio_init(void)
{
	//POWER ON
	gpio_request(POWER_ON_4G,"4G Power pin");
        gpio_direction_output(POWER_ON_4G, 0);
        mdelay(100);
        gpio_free(POWER_ON_4G);

	//reset
        gpio_request(POWER_RESET_4G,"4G Rst pin");
        gpio_direction_output(POWER_RESET_4G, 0);
        mdelay(150);
        gpio_set_value(POWER_RESET_4G, 1);
        gpio_free(POWER_RESET_4G);

	return 0;
}
#endif
/* end add */

//#if defined(CONFIG_NXP_PRESS_BOARD_POWER)
#if 1


static int itop4418_pressure_board_power_gpio_init(void)
{
	//POWER ON
    unsigned int power_io = CFG_IO_PRESS_BORAD_POWER;
	/* PD signal */
	nxp_soc_gpio_set_out_value(power_io, 0);
	nxp_soc_gpio_set_io_dir(power_io, 1);
	nxp_soc_gpio_set_io_func(power_io, nxp_soc_gpio_get_altnum(power_io));
	nxp_soc_gpio_set_out_value(power_io, 1);
	/* mdelay(10); */
	mdelay(5);
	return 0;
}

#endif



/*------------------------------------------------------------------------------
 * USB HSIC power control.
 */
int nxp_hsic_phy_pwr_on(struct platform_device *pdev, bool on)
{
	return 0;
}
EXPORT_SYMBOL(nxp_hsic_phy_pwr_on);

#ifdef CONFIG_BATTERY_NXE2000
/*------------------------------------------------------------------------------
 * USB OTGVBUS power control.
 */
void nxp_otgvbus_pwr_set(int enable)
{
	nxp_soc_gpio_set_out_value(CFG_GPIO_OTG_VBUS_DET, enable);
}
EXPORT_SYMBOL(nxp_otgvbus_pwr_set);
#endif
/*------------------------------------------------------------------------------
 * HDMI CEC driver
 */
#if defined(CONFIG_NXP_HDMI_CEC)
static struct platform_device hdmi_cec_device = {
	.name			= NXP_HDMI_CEC_DRV_NAME,
};
#endif /* CONFIG_NXP_HDMI_CEC */

/*------------------------------------------------------------------------------
 * register board platform devices
 */
void __init nxp_board_devices_register(void)
{
	printk("[Register board platform devices]\n");

#if defined(CONFIG_ARM_NXP_CPUFREQ)
	printk("plat: add dynamic frequency (pll.%d)\n", dfs_plat_data.pll_dev);
	platform_device_register(&dfs_plat_device);
	#if defined(CPU_LIMIT_CONTROL)
	platform_device_register(&freq_limit_device);
	#endif
#endif

/* add by cym 20150525 */
#if defined(CONFIG_SENSORS_NXP_ADC_TEMP)
        printk("plat: add device adc temp\n");
        platform_device_register(&adc_temp_plat_device);
#endif
/* end add */

#if defined (CONFIG_FB_NXP)
	printk("plat: add framebuffer\n");
	platform_add_devices(fb_devices, ARRAY_SIZE(fb_devices));
#endif

#if defined(CONFIG_MMC_DW)
	#ifdef CONFIG_MMC_NXP_CH0
	nxp_mmc_add_device(0, &_dwmci0_data);
	#endif
    #ifdef CONFIG_MMC_NXP_CH1
	nxp_mmc_add_device(1, &_dwmci1_data);
	#endif
    #ifdef CONFIG_MMC_NXP_CH2
	nxp_mmc_add_device(2, &_dwmci2_data);
	#endif
#endif
#if defined(CONFIG_MTK_COMBO_MT66XX)
        itop4418_wifi_combo_module_gpio_init();
#endif

#if defined(CONFIG_DM9000) || defined(CONFIG_DM9000_MODULE)
	printk("plat: add device dm9000 net\n");
	platform_device_register(&dm9000_plat_device);
#endif

#if defined(CONFIG_BACKLIGHT_PWM)
	printk("plat: add backlight pwm device\n");
	platform_device_register(&bl_plat_device);
#endif

#if defined(CONFIG_MTD_NAND_NXP)
	platform_device_register(&nand_plat_device);
#endif

#if defined(CONFIG_KEYBOARD_NXP_KEY) || defined(CONFIG_KEYBOARD_NXP_KEY_MODULE)
	printk("plat: add device keypad\n");
	platform_device_register(&key_plat_device);
#endif

#if defined(CONFIG_I2C_NXP)
    platform_add_devices(i2c_devices, ARRAY_SIZE(i2c_devices));
#endif

#if defined(CONFIG_REGULATOR_NXE2000)
	printk("plat: add device nxe2000 pmic\n");
	i2c_register_board_info(NXE2000_I2C_BUS, nxe2000_i2c_boardinfo, ARRAY_SIZE(nxe2000_i2c_boardinfo));
#endif

#if defined(CONFIG_SND_CODEC_WM8976) || defined(CONFIG_SND_CODEC_WM8976_MODULE)
	printk("plat: add device asoc-wm8976\n");
	i2c_register_board_info(WM8976_I2C_BUS, &wm8976_i2c_bdi, 1);
	platform_device_register(&wm8976_dai);
#endif

#if defined(CONFIG_SND_CODEC_ALC5623) || defined(CONFIG_SND_CODEC_ALC5623_MODULE)
	printk("plat: add device asoc-alc5623\n");
	i2c_register_board_info(0, &alc5623_i2c_bdi, 1);
	platform_device_register(&alc5623_dai);
#endif

#if defined(CONFIG_SND_SPDIF_TRANSCIEVER) || defined(CONFIG_SND_SPDIF_TRANSCIEVER_MODULE)
	printk("plat: add device spdif playback\n");
	platform_device_register(&spdif_transciever);
	platform_device_register(&spdif_trans_dai);
#endif

#if defined(CONFIG_V4L2_NXP) || defined(CONFIG_V4L2_NXP_MODULE)
    printk("plat: add device nxp-v4l2\n");
    platform_device_register(&nxp_v4l2_dev);
#endif

/* remove by cym 20151214 */
#if 0
#if defined(CONFIG_SPI_SPIDEV) || defined(CONFIG_SPI_SPIDEV_MODULE)
    spi_register_board_info(spi_plat_board, ARRAY_SIZE(spi_plat_board));
    printk("plat: register spidev\n");
#endif
#endif
/* end remove */

/* add by cym 20151214 */
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	spi_register_board_info(w25q32_plat_board, ARRAY_SIZE(w25q32_plat_board));
    	printk("plat: register w25q32\n");
#endif
/* end add */

/* add by cym 20160618 */
#if defined(CONFIG_SPI_RC522) || defined(CONFIG_SPI_RC522_MODULE)
        spi_register_board_info(rc522_plat_board, ARRAY_SIZE(rc522_plat_board));
        printk("plat: register rc522\n");
#endif
/* end add */

// add by cch 20160920
#ifdef CONFIG_CAN_MCP251X
        spi_register_board_info(mcp2515_plat_board, ARRAY_SIZE(mcp2515_plat_board));
        printk("plat: register mcp2515\n");
#endif
// end add

#if defined(CONFIG_TOUCHSCREEN_GSLX680)
	printk("plat: add touch(gslX680) device\n");
	i2c_register_board_info(GSLX680_I2C_BUS, &gslX680_i2c_bdi, 1);
#endif

/* add by cym 20150812 */
#if defined(CONFIG_TOUCHSCREEN_FT5X0X)
	printk("plat: add touch(ft5x0x) device\n");
        i2c_register_board_info(FT5X0X_I2C_BUS, &ft5x0x_i2c_bdi, 1);
#endif
/* end add */

/* add by cym 20160222 */
#ifdef CONFIG_TOUCHSCREEN_TSC2007
	printk("plat: add touch(tsc2007) device\n");
        i2c_register_board_info(FT5X0X_I2C_BUS, &tsc2007_i2c_bdi, 1);
#endif
/* end add */

#if defined(CONFIG_SENSORS_MMA865X) || defined(CONFIG_SENSORS_MMA865X_MODULE)
	printk("plat: add g-sensor mma865x\n");
	i2c_register_board_info(2, &mma865x_i2c_bdi, 1);
#elif defined(CONFIG_SENSORS_MMA7660) || defined(CONFIG_SENSORS_MMA7660_MODULE)
	printk("plat: add g-sensor mma7660\n");
	i2c_register_board_info(MMA7660_I2C_BUS, &mma7660_i2c_bdi, 1);
/* add by cym 20151214 */
#elif defined(CONFIG_MXC_MMA8451) || defined(CONFIG_MXC_MMA8451_MODULE)
	printk("plat: add g-sensor mma8451\n");
        i2c_register_board_info(2, &mma8451_i2c_bdi, 1);
/* end add */
#endif

#if defined(CONFIG_RFKILL_NXP)
    printk("plat: add device rfkill\n");
    platform_device_register(&rfkill_device);
#endif

/* add by cym 20150911 */
#if defined(CONFIG_PPM_NXP)
printk("plat: add device ppm\n");
    platform_device_register(&ppm_device);
#endif
/* end add */

/* add by cym 20150921 */
#if defined(CONFIG_BUZZER_CTL)
printk("plat: add device buzzer\n");
    platform_device_register(&buzzer_plat_device);
#endif

#if defined(CONFIG_LEDS_CTL)
printk("plat: add device leds\n");
    platform_device_register(&leds_plat_device);
#endif

#if defined(CONFIG_MAX485_CTL)
printk("plat: add device max485\n");
    platform_device_register(&max485_plat_device);
#endif
/* end add */

/* add by cym 20150909 */
#if defined(CONFIG_NXPMAC_ETH)
    printk("plat: add device nxp-gmac\n");
    platform_device_register(&nxp_gmac_dev);
#endif
/* end add */
#if defined(CONFIG_NXP_HDMI_CEC)
    printk("plat: add device hdmi-cec\n");
    platform_device_register(&hdmi_cec_device);
#endif


#if 1// defined(CONFIG_NXP_PRESS_BOARD_POWER)
		printk("plat: add device pressure board power\n");
		itop4418_pressure_board_power_gpio_init();
#endif


	/* END */
	printk("\n");
}

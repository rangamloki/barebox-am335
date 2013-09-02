#include <common.h>
#include <sizes.h>
#include <io.h>
#include <init.h>
#include <asm/barebox-arm-head.h>
#include <asm/barebox-arm.h>
#include <mach/am33xx-silicon.h>
#include <mach/am33xx-clock.h>
#include <mach/generic.h>
#include <mach/sdrc.h>
#include <mach/sys_info.h>
#include <mach/syslib.h>
#include <mach/am33xx-mux.h>
#include <mach/am33xx-generic.h>
#include <mach/wdt.h>

static const struct am33xx_cmd_control pcm051_cmd = {
	.slave_ratio0 = 0x40,
	.dll_lock_diff0 = 0x0,
	.invert_clkout0 = 0x1,
	.slave_ratio1 = 0x40,
	.dll_lock_diff1 = 0x0,
	.invert_clkout1 = 0x1,
	.slave_ratio2 = 0x40,
	.dll_lock_diff2 = 0x0,
	.invert_clkout2 = 0x1,
};

/* 1x512MB */
static const struct am33xx_emif_regs MT41J256M16HA15EIT_1x512M16_regs = {
	.emif_read_latency	= 0x6,
	.emif_tim1		= 0x0888A39B,
	.emif_tim2		= 0x26517FDA,
	.emif_tim3		= 0x501F84EF,
	.sdram_config		= 0x61C04B32,
	.zq_config		= 0x50074BE4,
	.sdram_ref_ctrl		= 0x0000093B,
};

static const struct am33xx_ddr_data MT41J256M16HA15EIT_1x512M16_data = {
	.rd_slave_ratio0	= 0x3B,
	.wr_dqs_slave_ratio0	= 0x3B,
	.fifo_we_slave_ratio0	= 0x96,
	.wr_slave_ratio0	= 0x76,
};

/* 2x256MB */
static const struct am33xx_emif_regs MT41J256M8HX15E_2x256M8_regs = {
	.emif_read_latency	= 0x6,
	.emif_tim1		= 0x0668A39B,
	.emif_tim2		= 0x26337FDA,
	.emif_tim3		= 0x501F830F,
	.sdram_config		= 0x61C04832,
	.zq_config		= 0x50074BE4,
	.sdram_ref_ctrl		= 0x0000093B,
};

static const struct am33xx_ddr_data MT41J256M8HX15E_2x256M8_data = {
	.rd_slave_ratio0	= 0x3B,
	.wr_dqs_slave_ratio0	= 0x85,
	.fifo_we_slave_ratio0	= 0x100,
	.wr_slave_ratio0	= 0xC1,
};

/* 1x128M16 */
static const struct am33xx_emif_regs MT41J64M1615IT_1x128M16_regs = {
	.emif_read_latency	= 0x6,
	.emif_tim1		= 0x0888A39B,
	.emif_tim2		= 0x26247FDA,
	.emif_tim3		= 0x501F821F,
	.sdram_config		= 0x61C04A32,
	.zq_config		= 0x50074BE4,
	.sdram_ref_ctrl		= 0x0000093B,
};

static const struct am33xx_ddr_data MT41J64M1615IT_1x128M16_data = {
	.rd_slave_ratio0	= 0x3A,
	.wr_dqs_slave_ratio0	= 0x36,
	.fifo_we_slave_ratio0	= 0xA2,
	.wr_slave_ratio0	= 0x74,
};

/* 1x256M16 */
static const struct am33xx_emif_regs MT41J128M16125IT_1x256M16_regs = {
	.emif_read_latency	= 0x6,
	.emif_tim1		= 0x0888A39B,
	.emif_tim2		= 0x26337FDA,
	.emif_tim3		= 0x501F830F,
	.sdram_config		= 0x61C04AB2,
	.zq_config		= 0x50074BE4,
	.sdram_ref_ctrl		= 0x0000093B,
};

static const struct am33xx_ddr_data MT41J128M16125IT_1x256M16_data = {
	.rd_slave_ratio0	= 0x3B,
	.wr_dqs_slave_ratio0	= 0x3B,
	.fifo_we_slave_ratio0	= 0x97,
	.wr_slave_ratio0	= 0x76,
};

/* 2x512M8 */
static const struct am33xx_emif_regs MT41J512M8125IT_2x512M8_regs = {
	.emif_read_latency	= 0x6,
	.emif_tim1		= 0x0888A39B,
	.emif_tim2		= 0x26517FDA,
	.emif_tim3		= 0x501F84EF,
	.sdram_config		= 0x61C04B32,
	.zq_config		= 0x50074BE4,
	.sdram_ref_ctrl		= 0x0000093B,
};

static const struct am33xx_ddr_data MT41J512M8125IT_2x512M8_data = {
	.rd_slave_ratio0	= 0x39,
	.wr_dqs_slave_ratio0	= 0x38,
	.fifo_we_slave_ratio0	= 0x98,
	.wr_slave_ratio0	= 0x76,
};

/**
 * @brief The basic entry point for board initialization.
 *
 * This is called as part of machine init (after arch init).
 * This is again called with stack in SRAM, so not too many
 * constructs possible here.
 *
 * @return void
 */
static int pcm051_board_init(void)
{
	/* WDT1 is already running when the bootloader gets control
	 * Disable it to avoid "random" resets
	 */
	writel(WDT_DISABLE_CODE1, AM33XX_WDT_REG(WSPR));
	while (readl(AM33XX_WDT_REG(WWPS)) != 0x0);

	writel(WDT_DISABLE_CODE2, AM33XX_WDT_REG(WSPR));
	while (readl(AM33XX_WDT_REG(WWPS)) != 0x0);

	if (running_in_sdram())
		return 0;

	pll_init(MPUPLL_M_600, 25, DDRPLL_M_303);

	if (IS_ENABLED(CONFIG_512MB_MT41J128M16_1x512M16))
		am335x_sdram_init(0x18B, &pcm051_cmd,
			&MT41J256M16HA15EIT_1x512M16_regs,
			&MT41J256M16HA15EIT_1x512M16_data);
	else if (IS_ENABLED(CONFIG_512MB_MT41J256M8HX15E_2x256M8))
		am335x_sdram_init(0x18B, &pcm051_cmd,
			&MT41J256M8HX15E_2x256M8_regs,
			&MT41J256M8HX15E_2x256M8_data);
	else if (IS_ENABLED(CONFIG_128MB_MT41J64M1615IT_1x128M16))
		am335x_sdram_init(0x18B, &pcm051_cmd,
			&MT41J64M1615IT_1x128M16_regs,
			&MT41J64M1615IT_1x128M16_data);
	else if (IS_ENABLED(CONFIG_256MB_MT41J128M16125IT_1x256M16))
		am335x_sdram_init(0x18B, &pcm051_cmd,
			&MT41J128M16125IT_1x256M16_regs,
			&MT41J128M16125IT_1x256M16_data);
	else if (IS_ENABLED(CONFIG_1024MB_MT41J512M8125IT_2x512M8))
		am335x_sdram_init(0x18B, &pcm051_cmd,
			&MT41J512M8125IT_2x512M8_regs,
			&MT41J512M8125IT_2x512M8_data);

	am33xx_uart0_soft_reset();
	am33xx_enable_uart0_pin_mux();

	return 0;
}

void __naked __bare_init barebox_arm_reset_vector(uint32_t *data)
{
	am33xx_save_bootinfo(data);

	arm_cpu_lowlevel_init();

	pcm051_board_init();

	barebox_arm_entry(0x80000000, SZ_128M, 0);
}

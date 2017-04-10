#ifndef GPIO_HH
#define GPIO_HH

#define usleep(x) do {} while(0)
// Includes. {{{
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
// }}}

struct bbb_Gpio { // {{{
	uint32_t revision;		// 0
	uint32_t reserved0[3];
	uint32_t sysconfig;		// 10
	uint32_t reserved1[3];
	uint32_t eoi;			// 20
	uint32_t irqstatus_raw_0;	// 24
	uint32_t irqstatus_raw_1;	// 28
	uint32_t irqstatus_0;		// 2c
	uint32_t irqstatus_1;		// 30
	uint32_t irqstatus_set_0;	// 34
	uint32_t irqstatus_set_1;	// 38
	uint32_t irqstatus_clr_0;	// 3c
	uint32_t irqstatus_clr_1;	// 40
	uint32_t irqwaken_0;		// 44
	uint32_t irqwaken_1;		// 48
	uint32_t reserved2[50];
	uint32_t sysstatus;		// 114
	uint32_t reserved3[6];
	uint32_t ctrl;			// 130
	uint32_t oe;			// 134
	uint32_t datain;		// 138
	uint32_t dataout;		// 13c
	uint32_t leveldetect0;		// 140
	uint32_t leveldetect1;		// 144
	uint32_t risingdetect;		// 148
	uint32_t fallingdetect;		// 14c
	uint32_t debounceenable;	// 150
	uint32_t debouncingtime;	// 154
	uint32_t reserved4[14];
	uint32_t cleardataout;		// 190
	uint32_t setdataout;		// 194
};
// }}}

// Variables. {{{
static volatile bbb_Gpio *bbb_gpio[4];
static int bbb_devmem;
enum BBB_State { MUX_DISABLED, MUX_INPUT, MUX_OUTPUT };
#define USABLE(x) (x)
#define HDMI(x) (x)
#define FLASH(x) ""
#define INTERNAL ""
#define UNUSABLE ""
static char const *bbb_muxname[] = { // {{{
	UNUSABLE, UNUSABLE, USABLE("P9_22"), USABLE("P9_21"), USABLE("P9_18"), USABLE("P9_17"), INTERNAL, USABLE("P9_42"),
	HDMI("P8_35"), HDMI("P8_33"), HDMI("P8_31"), HDMI("P8_32"), UNUSABLE, UNUSABLE, USABLE("P9_26"), USABLE("P9_24"),
	UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, USABLE("P9_41"), UNUSABLE, USABLE("P8_19"), USABLE("P8_13"),
	UNUSABLE, UNUSABLE, USABLE("P8_14"), USABLE("P8_17"), UNUSABLE, UNUSABLE, USABLE("P9_11"), USABLE("P9_13"),

	FLASH("P8_25"), FLASH("P8_24"), FLASH("P8_5"), FLASH("P8_6"), FLASH("P8_23"), FLASH("P8_22"), FLASH("P8_3"), FLASH("P8_4"),
	UNUSABLE, UNUSABLE, INTERNAL, INTERNAL, USABLE("P8_12"), USABLE("P8_11"), USABLE("P8_16"), USABLE("P8_15"),
	USABLE("P9_15"), USABLE("P9_23"), USABLE("P9_14"), USABLE("P9_16"), UNUSABLE, INTERNAL, INTERNAL, INTERNAL,
	INTERNAL, UNUSABLE, UNUSABLE, INTERNAL, USABLE("P9_12"), USABLE("P8_26"), FLASH("P8_21"), FLASH("P8_20"),

	UNUSABLE, USABLE("P8_18"), USABLE("P8_07"), USABLE("P8_08"), USABLE("P8_10"), USABLE("P8_09"), HDMI("P8_45"), HDMI("P8_46"),
	HDMI("P8_43"), HDMI("P8_44"), HDMI("P8_41"), HDMI("P8_42"), HDMI("P8_39"), HDMI("P8_40"), HDMI("P8_37"), HDMI("P8_38"),
	HDMI("P8_36"), HDMI("P8_34"), UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, HDMI("P8_27"), HDMI("P8_29"),
	HDMI("P8_28"), HDMI("P8_30"), INTERNAL, INTERNAL, INTERNAL, INTERNAL, INTERNAL, INTERNAL,

	UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE,
	UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, HDMI("P9_31"), HDMI("P9_29"),
	USABLE("P9_30"), HDMI("P9_28"), USABLE("P9_42"), USABLE("P9_27"), USABLE("P9_41"), HDMI("P9_25"), UNUSABLE, UNUSABLE,
	UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE, UNUSABLE
}; // }}}
static volatile uint32_t *bbb_gpio_pad[32 * 4];
int bbb_hack_pipe[2];
// }}}

// Pin setting. {{{
static void bbb_setmux(int pin, BBB_State mode) { // {{{
	if (bbb_muxname[pin][0] == '\0') {
		if (mode != MUX_DISABLED) {
			std::cerr << "trying to set up unusable gpio " << pin << std::endl;
			abort();
		}
		return;
	}
	int modes[3] = { 0x1f, 0x37, 0x1f };
	if (write(bbb_hack_pipe[1], &modes[mode], 4) != 4)
		std::cerr << "warning: short write to modechange pipe hack" << std::endl;
	if (read(bbb_hack_pipe[0], (char *)bbb_gpio_pad[pin], 4) != 4)
		std::cerr << "warning: short read from modechange pipe hack" << std::endl;
}; // }}}

void SET_OUTPUT(int pin) { // {{{
	bbb_gpio[pin >> 5]->oe &= ~(1 << (pin & 0x1f));
	bbb_setmux(pin, MUX_OUTPUT);
} // }}}

void SET_INPUT(int pin) { // {{{
		bbb_gpio[pin >> 5]->oe |= 1 << (pin & 0x1f);
		bbb_setmux(pin, MUX_INPUT);
} // }}}

void SET_INPUT_NOPULLUP(int pin) { // {{{
	bbb_gpio[pin >> 5]->oe |= 1 << (pin & 0x1f);
	bbb_setmux(pin, MUX_DISABLED);
} // }}}

#define SET(_p) bbb_gpio[(_p) >> 5]->setdataout = 1 << ((_p) & 0x1f)
#define RESET(_p) bbb_gpio[(_p) >> 5]->cleardataout = 1 << ((_p) & 0x1f)
#define GET(_p) (bool(bbb_gpio[(_p) >> 5]->datain & (1 << ((_p) & 0x1f))))
// }}}

void init_gpio() {
	// Prepare for pinmux hack.
	if (pipe(bbb_hack_pipe)) {
		std::cerr << "unable to create pipe for pinmux hack: " << strerror(errno) << std::endl;
		abort();
	}
	// Prepare gpios.
	unsigned gpio_base[4] = { 0x44e07000, 0x4804c000, 0x481ac000, 0x481ae000 };
	unsigned pad_control_base = 0x44e10000;
	int pad_offset[32 * 4] = {
		0x148, 0x14c, 0x150, 0x154, 0x158, 0x15c, 0x160, 0x164, 0xd0, 0xd4, 0xd8, 0xdc, 0x178, 0x17c, 0x180, 0x184,
		0x11c, 0x120, 0x21c, 0x1b0, 0x1b4, 0x124, 0x20, 0x24, -1, -1, 0x28, 0x2c, 0x128, 0x144, 0x70, 0x74,

		0x0, 0x4, 0x8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x168, 0x16c, 0x170, 0x174, 0x30, 0x34, 0x38, 0x3c,
		0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x78, 0x7c, 0x80, 0x84,

		0x88, 0x8c, 0x90, 0x94, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8, 0xbc, 0xc0, 0xc4,
		0xc8, 0xcc, 0x134, 0x138, 0x13c, 0x140, 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4, 0xf8, 0xfc, 0x100, 0x104,

		0x108, 0x10c, 0x110, 0x114, 0x118, 0x188, 0x18c, 0x1e4, 0x1e8, 0x12c, 0x130, -1, -1, 0x234, 0x190, 0x194,
		0x198, 0x19c, 0x1a0, 0x1a4, 0x1a8, 0x1ac, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};
	bbb_devmem = open("/dev/mem", O_RDWR);
	if (bbb_devmem < 0) {
		std::cerr << "unable to open /dev/mem; are you root?" << std::endl;
		abort();
	}
	for (int i = 0; i < 4; ++i) {
		bbb_gpio[i] = (volatile bbb_Gpio *)mmap(0, 0x2000, PROT_READ | PROT_WRITE, MAP_SHARED, bbb_devmem, gpio_base[i]);
		if (bbb_gpio[i] == NULL) {
			std::cerr << "unable to mmap /dev/mem for gpio channel " << i << "; are you root?" << std::endl;
			abort();
		}
	}
	volatile uint32_t *bbb_padmap = (volatile uint32_t *)mmap(0, 0x2000, PROT_READ | PROT_WRITE, MAP_SHARED, bbb_devmem, pad_control_base);
	if (bbb_padmap == NULL) {
		std::cerr << "unable to mmap /dev/mem for pad control; are you root?" << std::endl;
		abort();
	}
	for (int i = 0; i < 32 * 4; ++i)
		bbb_gpio_pad[i] = (pad_offset[i] >= 0 ? &bbb_padmap[(0x800 + pad_offset[i]) / 4] : NULL);
}

void set(int pin, int time = 0) {
	SET_OUTPUT(pin);
	SET(pin);
	usleep(time);
}

void reset(int pin, int time = 0) {
	SET_OUTPUT(pin);
	RESET(pin);
	usleep(time);
}

void input(int pin, int time = 0) {
	SET_INPUT(pin);
	usleep(time);
}

int read(int pin, int time = 0) {
	usleep(time);
	return GET(pin);
}

#undef usleep
#endif

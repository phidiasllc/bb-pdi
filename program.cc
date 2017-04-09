#include "gpio.hh"
#include <stdint.h>

// Pin definitions.
#define CLK 1
#define DATA 2

// Protocol definitions.
#define LDS	0x00
#define STS	0x40
#define LD	0x20
#define ST	0x60
#define LDCS	0x80
#define STCS	0xc0
#define REPEAT	0xa0
#define KEY	0xe0

/*
   gpio.hh must define the following:

   set(pin, time = 0);
   reset(pin, time = 0);
   input(pin, time = 0);
   read(pin, time = 0);
   
   The time argument is a delay after setting the new state before returning, or before reading.  This time is measured in microseconds.
*/

int const CLOCK_TIME = 1e6 / 50e3;	// A full cycle is four times this period, so the minimum of 10kHz is 40e3.
int const DATA_DELAY = CLOCK_TIME;

uint64_t key = 0x1289AB45CDD888FF;

void pulse(int count = 1) {
	// This is the only function that alters CLK.  It is low when this function is entered.
	for (int i = 0; i < count - 1; ++i) {
		set(CLK, CLOCK_TIME);
		reset(CLK, CLOCK_TIME);
	}
	set(CLK, CLOCK_TIME);
	reset(CLK);
}

void send_frame(int data) {
	reset(DATA, DATA_DELAY);
	pulse();
	bool parity = false;
	for (int i = 0; i < 8; ++i) {
		if ((data >> i) & 1) {
			set(DATA, DATA_DELAY);
			parity = !parity;
		}
		else
			reset(DATA, DATA_DELAY);
		pulse();
	}
	if (parity)
		set(DATA, DATA_DELAY);
	else
		reset(DATA, DATA_DELAY);
	pulse();
	set(DATA, DATA_DELAY);
	pulse();
	set(DATA, DATA_DELAY);
	pulse();
}

int recv_frame() {
	while (true) {
		bool bit = read(DATA, DATA_DELAY);
		pulse();
		if (!bit)
			break;
	}
	bool parity = false;
	int ret = 0;
	for (int i = 0; i < 11; ++i) {
		if (read(DATA, DATA_DELAY)) {
			ret |= 1 << i;
			parity = !parity;
		}
		pulse();
	}
	if (parity)
		std::cerr << "parity error on input." << std::endl;
	if ((ret & (3 << 8)) != (3 << 8))
		std::cerr << "stop bits are not both high." << std::endl;
	return ret & 0xff;
}

void init() {
	init_gpio();
	reset(CLK);
	reset(DATA, 100);
	set(DATA, 50);
	pulse(16);
}

int main(int argc, char **argv) {
	(void)&argc;
	(void)&argv;

	init();
	send_frame(LDCS | 0);
	std::cout << "Status register: " << recv_frame() << std::endl;
	return 0;
}

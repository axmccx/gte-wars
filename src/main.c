#include <stdint.h>
#include <stdio.h>
#include "gpu.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"
#include "controller.h"

#define SCREEN_WIDTH     320
#define SCREEN_HEIGHT    240

int main(int argc, const char **argv) {
	initSerialIO(115200);
	initControllerBus();

	if ((GPU_GP1 & GP1_STAT_FB_MODE_BITMASK) == GP1_STAT_FB_MODE_PAL) {
		puts("Using PAL mode");
		setupGPU(GP1_MODE_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		puts("Using NTSC mode");
		setupGPU(GP1_MODE_NTSC, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	DMA_DPCR |= DMA_DPCR_CH_ENABLE(DMA_GPU);

	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
	GPU_GP1 = gp1_dispBlank(false);

	int x = 0;
	int y = 0;

	DMAChain dmaChains[2];
	bool usingSecondFrame = false;

	for (;;) {
		const int bufferX = usingSecondFrame ? SCREEN_WIDTH : 0;
		const int bufferY = 0;

		DMAChain *chain = &dmaChains[usingSecondFrame];
		usingSecondFrame = !usingSecondFrame;

		GPU_GP1 = gp1_fbOffset(bufferX, bufferY);

		chain->nextPacket = chain->data;

		uint32_t* ptr = allocatePacket(chain, 4);
		ptr[0] = gp0_texpage(0, true, false);
		ptr[1] = gp0_fbOffset1(bufferX, bufferY);
		ptr[2] = gp0_fbOffset2(
			bufferX + SCREEN_WIDTH  - 1,
			bufferY + SCREEN_HEIGHT - 2
		);
		ptr[3] = gp0_fbOrigin(bufferX, bufferY);

		ptr = allocatePacket(chain, 3);
		ptr[0] = gp0_rgb(0, 0, 0) | gp0_vramFill();
		ptr[1] = gp0_xy(bufferX, bufferY);
		ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

		// TODO make the square smaller, and different color
		// TODO draw something more interesting than a square

		ptr = allocatePacket(chain, 3);
		ptr[0] = gp0_rgb(255, 0, 192) | gp0_rectangle(false, false, false);
		ptr[1] = gp0_xy(x, y);
		ptr[2] = gp0_xy(8, 8);

		const uint16_t buttons = readControllerButtons(0);

		char buffer[256];
		sprintf(buffer, "buttons: %b", buttons);
		puts(buffer);

		if (buttons & BUTTON_UP) {
			y--;
		}
		if (buttons & BUTTON_DOWN) {
			y++;
		}
		if (buttons & BUTTON_RIGHT) {
			x++;
		}
		if (buttons & BUTTON_LEFT) {
			x--;
		}

		waitForGP0Ready();
		waitForVSync();
		sendLinkedList(chain->data);
	}

	return 0;
}

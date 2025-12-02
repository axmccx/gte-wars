#include <stdint.h>
#include <stdio.h>
#include "ps1/gpu.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"
#include "controller.h"
#include "model.h"
#include "world.h"
#include "render.h"

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

	setupGTE(SCREEN_WIDTH, SCREEN_HEIGHT);

	DMA_DPCR |= 0
		| DMA_DPCR_CH_ENABLE(DMA_GPU)
		| DMA_DPCR_CH_ENABLE(DMA_OTC);

	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
	GPU_GP1 = gp1_dispBlank(false);

	DMAChain dmaChains[2];
	bool usingSecondFrame = false;
	World world;
	worldInit(&world);

	for (;;) {
		// Get controller buttons
		const ControllerResponse controller_response = readController(0);

		// Update world
		updatePlayer(&world, controller_response);

		// Prepare for next frame
		const int bufferX = usingSecondFrame ? SCREEN_WIDTH : 0;
		const int bufferY = 0;

		DMAChain *chain = &dmaChains[usingSecondFrame];
		usingSecondFrame = !usingSecondFrame;

		GPU_GP1 = gp1_fbOffset(bufferX, bufferY);

		clearOrderingTable(chain->orderingTable, ORDERING_TABLE_SIZE);
		chain->nextPacket = chain->data;
		initOrderingChain(chain, bufferX, bufferY);

		// Build packet chain
		gte_setControlReg(GTE_TRX, world.player.x);
		gte_setControlReg(GTE_TRY, world.player.y);
		gte_setControlReg(GTE_TRZ, 2400);
		gte_setRotationMatrix(
			ONE,   0,   0,
			  0, ONE,   0,
			  0,   0, ONE
		);
		rotateCurrentMatrix(world.player.dir, world.player.rot, 0);
		buildRenderPackets(chain, world.player.model);

		// Render frame
		waitForGP0Ready();
		waitForVSync();
		sendLinkedList(&(chain->orderingTable)[ORDERING_TABLE_SIZE - 1]);
	}

	return 0;
}

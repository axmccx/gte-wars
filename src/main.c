#include <stdint.h>
#include <stdio.h>
#include "ps1/gpu.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"
#include "controller.h"
#include "model.h"
#include "world.h"

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

	setupGTE(SCREEN_WIDTH, SCREEN_HEIGHT);

	DMA_DPCR |= 0
		| DMA_DPCR_CH_ENABLE(DMA_GPU)
		| DMA_DPCR_CH_ENABLE(DMA_OTC);

	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
	GPU_GP1 = gp1_dispBlank(false);

	uint32_t colors[6] = {0x0000ff, 0x0000ff, 0x00ffff, 0xff0000, 0xff00ff, 0xffff00};

	DMAChain dmaChains[2];
	bool usingSecondFrame = false;
	World world;
	worldInit(&world);

	ObjModel *modelToRender = world.player.model;

	for (;;) {
		// Get controller buttons
		const ControllerResponse controller_response = readController(0);

		char buffer[256];
		sprintf(
			buffer,
			"buttons: %016b - left_joystick: %016b - right_joystick: %016b",
			controller_response.buttons,
			controller_response.left_joystick,
			controller_response.right_joystick
		);
		puts(buffer);

		// Update world
		updatePlayer(&world, controller_response);

		// Prepare for next frame
		const int bufferX = usingSecondFrame ? SCREEN_WIDTH : 0;
		const int bufferY = 0;

		DMAChain *chain = &dmaChains[usingSecondFrame];
		usingSecondFrame = !usingSecondFrame;

		uint32_t *ptr;

		GPU_GP1 = gp1_fbOffset(bufferX, bufferY);

		clearOrderingTable(chain->orderingTable, ORDERING_TABLE_SIZE);
		chain->nextPacket = chain->data;

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

		for (int i = 0; i < modelToRender->facesCount; i++) {
			const Face face = modelToRender->faces[i];

			gte_loadV0(&modelToRender->vertices[face.i1]);
			gte_loadV1(&modelToRender->vertices[face.i2]);
			gte_loadV2(&modelToRender->vertices[face.i3]);
			gte_command(GTE_CMD_RTPT | GTE_SF);

			uint32_t xy0 = 0;
			if (face.type == QUAD) {
				gte_command(GTE_CMD_NCLIP);

				if (gte_getDataReg(GTE_MAC0) <= 0)
					continue;

				xy0 = gte_getDataReg(GTE_SXY0);
				gte_loadV0(&modelToRender->vertices[face.i4]);
				gte_command(GTE_CMD_RTPS | GTE_SF);
				gte_command(GTE_CMD_AVSZ4 | GTE_SF);
			} else {
				gte_command(GTE_CMD_AVSZ3 | GTE_SF);
			}

			int zIndex = gte_getDataReg(GTE_OTZ);

			if ((zIndex < 0) || (zIndex >= ORDERING_TABLE_SIZE))
				continue;

			const uint32_t color = colors[5];

			if (face.type == QUAD) {
				ptr = allocatePacket(chain, zIndex, 5);
				ptr[0] = color | gp0_shadedQuad(false, false, false);
				ptr[1] = xy0;
				gte_storeDataReg(GTE_SXY0, 2 * 4, ptr);
				gte_storeDataReg(GTE_SXY1, 3 * 4, ptr);
				gte_storeDataReg(GTE_SXY2, 4 * 4, ptr);
			} else {
				ptr = allocatePacket(chain, zIndex, 4);
				ptr[0] = color | gp0_shadedTriangle(false, false, false);
				gte_storeDataReg(GTE_SXY0, 1 * 4, ptr);
				gte_storeDataReg(GTE_SXY1, 2 * 4, ptr);
				gte_storeDataReg(GTE_SXY2, 3 * 4, ptr);
			}
		}

		ptr = allocatePacket(chain, ORDERING_TABLE_SIZE - 1, 3);
		ptr[0] = gp0_rgb(0, 0, 32) | gp0_vramFill();
		ptr[1] = gp0_xy(bufferX, bufferY);
		ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

		ptr = allocatePacket(chain, ORDERING_TABLE_SIZE - 1, 4);
		ptr[0] = gp0_texpage(0, true, false);
		ptr[1] = gp0_fbOffset1(bufferX, bufferY);
		ptr[2] = gp0_fbOffset2(
			bufferX + SCREEN_WIDTH  - 1,
			bufferY + SCREEN_HEIGHT - 2
		);
		ptr[3] = gp0_fbOrigin(bufferX, bufferY);

		// Render frame
		waitForGP0Ready();
		waitForVSync();
		sendLinkedList(&(chain->orderingTable)[ORDERING_TABLE_SIZE - 1]);
	}

	return 0;
}

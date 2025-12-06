#include <stdint.h>
#include <stdio.h>
#include "ps1/gpu.h"
#include "ps1/gpucmd.h"
#include "ps1/registers.h"
#include "controller.h"
#include "model.h"
#include "world.h"
#include "render.h"
#include "rng.h"
#include "font.h"

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

	TextureInfo font;

	uploadIndexedTexture(
		&font,
		fontTexture,
		fontPalette,
		SCREEN_WIDTH * 2,
		0,
		SCREEN_WIDTH * 2,
		FONT_HEIGHT,
		FONT_WIDTH,
		FONT_HEIGHT,
		FONT_COLOR_DEPTH
	);

	DMAChain dmaChains[2];
	bool usingSecondFrame = false;
	World world;
	worldInit(&world);

	for (;;) {
		// Get controller buttons
		const ControllerResponse controller_response = readController(0);

		// Update world
		world.frameCount++;
		rng_mix(&controller_response, world.frameCount);

		updatePlayer(&world, controller_response);
		world.camera.x = (world.player.x * CAMERA_PAN_FACTOR) >> 12;
		world.camera.y = (world.player.y * CAMERA_PAN_FACTOR) >> 12;

		spawnBullets(&world, controller_response);
		updateBullets(&world);
		spawnEnemies(&world);
		detectBulletEnemyCollisions(&world);
		updateEnemies(&world);

		// Prepare for next frame
		const int bufferX = usingSecondFrame ? SCREEN_WIDTH : 0;
		const int bufferY = 0;

		DMAChain *chain = &dmaChains[usingSecondFrame];
		usingSecondFrame = !usingSecondFrame;

		GPU_GP1 = gp1_fbOffset(bufferX, bufferY);

		clearOrderingTable(chain->orderingTable, ORDERING_TABLE_SIZE);
		chain->nextPacket = chain->data;
		initOrderingChain(chain, bufferX, bufferY);
		buildPlayfieldBorder(chain, world.camera);

		// Build packet chain
		gte_setControlReg(GTE_TRX, world.player.x - world.camera.x);
		gte_setControlReg(GTE_TRY, world.player.y - world.camera.y);
		gte_setControlReg(GTE_TRZ, CAMERA_DISTANCE);
		gte_setRotationMatrix(
			ONE,   0,   0,
			  0, ONE,   0,
			  0,   0, ONE
		);
		rotateCurrentMatrix(world.player.dir, world.player.rot, 0);
		buildRenderPackets(chain, world.models.player, COLOR_CYAN);

		for (int i = 0; i < MAX_BULLETS; i++) {
			const Bullet *bullet = &world.bullets[i];

			if (bullet->alive) {
				gte_setControlReg(GTE_TRX, bullet->x - world.camera.x);
				gte_setControlReg(GTE_TRY, bullet->y - world.camera.y);
				gte_setControlReg(GTE_TRZ, CAMERA_DISTANCE);
				gte_setRotationMatrix(
					ONE,   0,   0,
					  0, ONE,   0,
					  0,   0, ONE
				);
				rotateCurrentMatrix(bullet->dir, 0, 0);
				buildRenderPackets(chain, world.models.bullet, COLOR_YELLOW);
			}
		}

		for (int i = 0; i < MAX_ENEMIES; i++) {
			const Enemy *enemy = &world.enemies[i];

			if (enemy->alive) {
				gte_setControlReg(GTE_TRX, enemy->x - world.camera.x);
				gte_setControlReg(GTE_TRY, enemy->y - world.camera.y);
				gte_setControlReg(GTE_TRZ, CAMERA_DISTANCE);
				gte_setRotationMatrix(
					ONE,   0,   0,
					  0, ONE,   0,
					  0,   0, ONE
				);
				rotateCurrentMatrix(0, enemy->rot, 0);
				buildRenderPackets(chain, enemy->model, COLOR_MAGENTA);
			}
		}

		char buffer[32];

		snprintf(buffer, sizeof(buffer), "Score: %d", world.score);
		printString(chain, &font, 4, 4, buffer);

		// Render frame
		waitForGP0Ready();
		waitForVSync();
		sendLinkedList(&(chain->orderingTable)[ORDERING_TABLE_SIZE - 1]);
	}

	return 0;
}

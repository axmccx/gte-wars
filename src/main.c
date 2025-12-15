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
#include "spawn.h"
#include "enemy.h"

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
	worldInit(&world, GAMESTATE_INTRO);
	initEnemyDefinitions(&world);

	for (;;) {
		// Get controller buttons
		const ControllerResponse controller_response = readController(0);

		// Update world
		world.frameCount++;
		rng_mix(&controller_response, world.frameCount);

		if (world.state == GAMESTATE_INTRO || world.state == GAMESTATE_GAMEOVER) {
			if (world.frameCount % 150 == 0) {
				const int off_x = rand_range(-PLAYFIELD_HALF_WIDTH, PLAYFIELD_HALF_WIDTH);
				const int off_y = rand_range(-PLAYFIELD_HALF_HEIGHT, PLAYFIELD_HALF_HEIGHT);
				emitParticles(&world, MEDIUM_PARTICLE, 100, 150, 16, world.camera.x-off_x, world.camera.y-off_y);
				emitParticles(&world, LARGE_PARTICLE, 200, 150, 32, world.camera.x-off_x, world.camera.y-off_y);
			}
		}

		if (world.state == GAMESTATE_PLAYING) {
			updatePlayer(&world, controller_response);
			world.camera.x = (world.player.x * CAMERA_PAN_FACTOR) >> 12;
			world.camera.y = (world.player.y * CAMERA_PAN_FACTOR) >> 12;

			if (world.player.alive) {
				detectPlayerEnemyCollisions(&world);
				fireBullets(&world, controller_response);
			}
			updateBullets(&world);
			spawnStateTick(&world);
			detectBulletEnemyCollisions(&world);
			updateEnemies(&world);
			if (world.lives == 0 && !world.player.alive) {
				world.state = GAMESTATE_GAMEOVER;
			}
		}

		if (world.state != GAMESTATE_PAUSED) {
			updateParticles(&world);
		}

		togglePause(&world, controller_response);

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
		world.polycount = 0;

		// Build packet chain
		if (world.state == GAMESTATE_INTRO) {
			printString(chain, &font, (SCREEN_WIDTH/2)-22, SCREEN_HEIGHT/2, "GTE WARS");
			printString(chain, &font, (SCREEN_WIDTH/2)-62, (SCREEN_HEIGHT/2) + 16, "Press any button to play");

			if (controller_response.buttons) {
				world.state = GAMESTATE_PLAYING;
			}
		}

		if (world.state == GAMESTATE_PLAYING || world.state == GAMESTATE_PAUSED) {
			if (world.player.alive) {
				gte_setControlReg(GTE_TRX, world.player.x - world.camera.x);
				gte_setControlReg(GTE_TRY, world.player.y - world.camera.y);
				gte_setControlReg(GTE_TRZ, CAMERA_DISTANCE);
				gte_setRotationMatrix(
					ONE,   0,   0,
					  0, ONE,   0,
					  0,   0, ONE
				);
				rotateCurrentMatrix(world.player.dir, world.player.rot, 0);
				buildRenderPackets(chain, world.models.player);
				world.polycount += world.models.player->facesCount;
			}

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
					buildRenderPackets(chain, world.models.bullet);
					world.polycount += world.models.bullet->facesCount;
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
					buildRenderPackets(chain, enemy->def->model);
					world.polycount += enemy->def->model->facesCount;
				}
			}

			if (world.nextWaveTimer < 200) {
				printString(chain, &font, (SCREEN_WIDTH/2)-48, 4 , "Next wave incoming!!");
			}
		}

		for (int i = 0; i < MAX_PARTICLES; i++) {
			const Particle *particle = &world.particles[i];

			if (particle->lifetime > 0) {
				gte_setControlReg(GTE_TRX, particle->x - world.camera.x);
				gte_setControlReg(GTE_TRY, particle->y - world.camera.y);
				gte_setControlReg(GTE_TRZ, depth_jitter(i, CAMERA_DISTANCE));
				gte_setRotationMatrix(
					ONE,   0,   0,
					  0, ONE,   0,
					  0,   0, ONE
				);
				rotateCurrentMatrix(particle->rx, particle->ry, particle->rz);
				buildRenderPackets(chain, particle->model);
				world.polycount += particle->model->facesCount;
			}
		}

		if (world.state == GAMESTATE_GAMEOVER) {
			printString(chain, &font, (SCREEN_WIDTH/2)-26, SCREEN_HEIGHT/2, "GAME OVER");
			printString(chain, &font, (SCREEN_WIDTH/2)-50, (SCREEN_HEIGHT/2) + 16, "Press X to restart");

			const bool x_button = (controller_response.buttons & BUTTON_CROSS) != 0;
			if (x_button) {
				worldInit(&world, GAMESTATE_PLAYING);
			}
		}

		char buffer[32];
		snprintf(buffer, sizeof(buffer), "Score: %d", world.score);
		printString(chain, &font, 4, 4, buffer);

		snprintf(buffer, sizeof(buffer), "Poly: %d", world.polycount);
		printString(chain, &font, 4, 16, buffer);

		snprintf(buffer, sizeof(buffer), "Lives: %d", world.lives);
		printString(chain, &font, SCREEN_WIDTH-46, 4, buffer);

		// Render frame
		waitForGP0Ready();
		waitForVSync();
		sendLinkedList(&(chain->orderingTable)[ORDERING_TABLE_SIZE - 1]);
	}

	return 0;
}

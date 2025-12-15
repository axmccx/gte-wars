#pragma once
#include "controller.h"
#include "model.h"
#include "enemy.h"

#define MAX_ENEMIES 60
#define MAX_BULLETS 30
#define MAX_PARTICLES 300
#define MAX_PLAYER_SPEED 30
#define PLAYER_HIT_RADIUS 12
#define BULLET_SPEED 50
#define BULLET_TIP_OFFSET 56
#define ENEMY_SPEED 10
#define ENEMY_HIT_RADIUS 80
#define CAMERA_PAN_FACTOR 2560
#define PLAYFIELD_HALF_WIDTH 2000
#define PLAYFIELD_HALF_HEIGHT 1500
#define JOYSTICK_DEAD_ZONE 64

typedef struct {
    int x, y;
} Camera;

typedef enum {
    GAMESTATE_INTRO = 0,
    GAMESTATE_PLAYING = 1,
    GAMESTATE_PAUSED = 2,
    GAMESTATE_GAMEOVER = 3
} GameState;

typedef struct World {
    GameState state;
    int frameCount;
    int nextFreeBullet;
    int nextFreeEnemy;
    int nextFreeParticle;
    int score;
    int lives;
    int respawnTimer;
    int polycount;
    int baseSpawnRate;
    int nextWaveTimer;
    uint16_t lastButtons;
    Camera camera;
    Models models;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Particle particles[MAX_PARTICLES];
} World;

static inline void normalize_direction(int *vx, int *vy) {
    int x = *vx;
    const int y = *vy;

    if (x == 0 && y == 0)
        x = 1;

    const int ax = (x < 0) ? -x : x;
    const int ay = (y < 0) ? -y : y;

    int mag = ax + ay;
    if (mag == 0) mag = 1;

    *vx = (x << 12) / mag;
    *vy = (y << 12) / mag;
}

static inline int limit_axis(int *pos, const int limit) {
    if (*pos > limit) {
        *pos = limit;
        return 1;
    }
    if (*pos < -limit) {
        *pos = -limit;
        return 1;
    }
    return 0;
}

static inline void clamp_axis(int *pos, const int limit) {
    (void)limit_axis(pos, limit);
}

static inline void bounce_axis(int *pos, int *vel, const int limit) {
    if (limit_axis(pos, limit)) *vel = -*vel;
}

void worldInit(World *world, GameState state);

void updatePlayer(World *world, ControllerResponse controller_response);

void detectPlayerEnemyCollisions(World *world);

void fireBullets(World *world, ControllerResponse controller_response);

void updateBullets(World *world);

void emitParticles(World *world, ParticleType type, int count, int lifetime, int speedSeed, int spawnX, int spawnY);

void detectBulletEnemyCollisions(World *world);

void updateParticles(World *world);

void togglePause(World *world, ControllerResponse controller_response);

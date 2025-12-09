#pragma once

#include "ps1/gte.h"

#define TRI  3
#define QUAD 4

extern const uint8_t playerShipObj[];
extern const uint8_t bulletObj[];
extern const uint8_t octahedronObj[];

typedef struct {
    uint8_t type;
    uint32_t color;
    uint16_t i1, i2, i3, i4;
} Face;

typedef struct {
    uint16_t vertexCount;
    uint16_t facesCount;
    GTEVector16 *vertices;
    Face *faces;
} ObjModel;

typedef struct {
    ObjModel *player;
    ObjModel *bullet;
    ObjModel *enemy;
    ObjModel *smallParticle;
    ObjModel *mediumParticle;
    ObjModel *largeParticle;
} Models;

typedef struct {
    int x, y, rot, dir, alive;
} Player;

typedef struct {
    int x, y, vx, vy, dir, alive;
} Bullet;

typedef struct {
    int x, y, rot, vx, vy, alive;
    ObjModel *model;
} Enemy;

typedef struct {
    int x, y, rx, ry, rz, rdx, vx, vy, lifetime;
    ObjModel *model;
} Particle;

typedef enum {
    SMALL_PARTICLE = 50,
    MEDIUM_PARTICLE = 75,
    LARGE_PARTICLE = 100,
} ParticleType;

void loadObjModel(ObjModel *obj_model, const uint8_t *data);

void generateParticle(ObjModel *obj_model, ParticleType type, uint32_t color);

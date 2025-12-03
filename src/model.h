#pragma once

#include "ps1/gte.h"

#define TRI  3
#define QUAD 4

extern const uint8_t playerShipObj[];
extern const uint8_t bulletObj[];

typedef struct {
    uint8_t type;
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
} Models;

void loadObjModel(ObjModel *obj_model, const uint8_t *data);

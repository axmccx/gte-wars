#pragma once

#include "ps1/gte.h"

#define TRI  3
#define QUAD 4

extern const uint8_t icoSphere[];
extern const uint8_t monkeyObj[];

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

void loadObjModel(ObjModel *obj_model, const uint8_t *data);

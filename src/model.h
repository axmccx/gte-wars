#pragma once

#include "ps1/gte.h"

typedef uint8_t FaceType;

#define TRI  3
#define QUAD 4

typedef struct {
    FaceType type;
    uint16_t i1, i2, i3, i4;
} Face;

typedef struct {
    uint16_t vertexCount;
    uint16_t facesCount;
    GTEVector16 *vertices;
    Face *faces;
} ObjModel;

ObjModel loadObjModel(const uint8_t *data);

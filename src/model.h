#pragma once

#include "ps1/gte.h"

typedef enum {
    TRI = 3,
    QUAD = 4
} FaceType;

typedef struct {
    uint16_t i1, i2, i3, i4;
    FaceType type;
} Face;

typedef struct {
    uint16_t vertexCount;
    uint16_t facesCount;
    GTEVector16 *vertices;
    Face *faces;
} ObjModel;

ObjModel loadObjModel(const uint8_t *data);

#pragma once

#include "ps1/gte.h"


typedef struct {
    uint16_t i1, i2, i3, i4;  // i4 unused for triangles
} Face;

typedef struct {
    uint16_t vertexCount;
    uint16_t facesCount;
    GTEVector16 *vertices;  // 0 = tri, 1 = quad
    Face *faces;
} ObjModel;

ObjModel loadObjModel(const uint8_t *data);

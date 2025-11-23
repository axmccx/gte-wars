#include <assert.h>
#include "string.h"
#include "stdlib.h"
#include "model.h"


static int16_t read_s16(const uint8_t *p) {
    return (int16_t)(p[0] | (p[1] << 8));
}

static uint16_t read_u16(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}


ObjModel loadObjModel(const uint8_t *data) {
    const uint8_t *ptr = data;
    ObjModel obj_model;

    char header[6] = {0};
    memcpy(header, ptr, 5);
    assert(strncmp(header, "MODEL", 5) == 0);
    ptr += 5;

    const uint16_t vertexCount = read_u16(ptr);
    obj_model.vertexCount = vertexCount;
    ptr += 2;

    const uint16_t facesCount = read_u16(ptr);
    obj_model.facesCount = facesCount;
    ptr += 2;

    obj_model.vertices = malloc(vertexCount * sizeof(GTEVector16));

    for (int i = 0; i < vertexCount; i++) {
        obj_model.vertices[i].x = read_s16(ptr);
        ptr += 2;

        obj_model.vertices[i].y = read_s16(ptr);
        ptr += 2;

        obj_model.vertices[i].z = read_s16(ptr);
        ptr += 2;

        obj_model.vertices[i]._padding = 0;
    }

    obj_model.faces = malloc(facesCount * sizeof(Face));

    for (int f = 0; f < facesCount; f++) {
        Face *face = &obj_model.faces[f];

        face->type = *ptr;
        ptr++;
        assert(face->type == TRI || face->type == QUAD);

        face->i1 = read_u16(ptr);
        ptr += 2;

        face->i2 = read_u16(ptr);
        ptr += 2;

        face->i3 = read_u16(ptr);
        ptr += 2;

        if (face->type == QUAD) {
            face->i4 = read_u16(ptr);
            ptr += 2;
        } else {
            face->i4 = 0;
        }
    }

    return obj_model;
}

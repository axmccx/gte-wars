#include <assert.h>
#include "string.h"
#include "stdlib.h"
#include "model.h"


ObjModel loadObjModel(const uint8_t *data) {
    const uint8_t *ptr = data;
    ObjModel obj_model;

    char header[6] = {0};
    memcpy(header, ptr, 5);
    assert(strncmp(header, "MODEL", 5) == 0);
    ptr += 5;

    const uint8_t dat_type = *ptr;
    ptr++;

    const uint16_t vertexCount = ptr[0] | (ptr[1] << 8);
    obj_model.vertexCount = vertexCount;
    ptr += 2;

    const uint16_t facesCount = ptr[0] | (ptr[1] << 8);
    obj_model.facesCount = facesCount;
    ptr += 2;

    obj_model.vertices = malloc(vertexCount * sizeof(GTEVector16));

    for (int i = 0; i < vertexCount; i++) {
        obj_model.vertices[i].x = ptr[0] | (ptr[1] << 8);
        ptr += 2;

        obj_model.vertices[i].y = ptr[0] | (ptr[1] << 8);
        ptr += 2;

        obj_model.vertices[i].z = ptr[0] | (ptr[1] << 8);
        ptr += 2;

        obj_model.vertices[i]._padding = 0;
    }

    obj_model.faces = malloc(facesCount * sizeof(Face));

    for (int f = 0; f < facesCount; f++) {
        obj_model.faces[f].i1 = ptr[0] | (ptr[1] << 8);
        ptr += 2;

        obj_model.faces[f].i2 = ptr[0] | (ptr[1] << 8);
        ptr += 2;

        obj_model.faces[f].i3 = ptr[0] | (ptr[1] << 8);
        ptr += 2;

        if (dat_type == 1) { // QUAD
            obj_model.faces[f].i4 = ptr[0] | (ptr[1] << 8);
            ptr += 2;
        } else {
            obj_model.faces[f].i4 = 0; // unused for triangle
        }
    }

    return obj_model;
}

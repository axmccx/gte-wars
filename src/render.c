#include "model.h"
#include "render.h"

void initOrderingChain(DMAChain *chain, int bufferX, int bufferY) {
    uint32_t *ptr = allocatePacket(chain, ORDERING_TABLE_SIZE - 1, 3);
    ptr[0] = gp0_rgb(0, 0, 32) | gp0_vramFill();
    ptr[1] = gp0_xy(bufferX, bufferY);
    ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

    ptr = allocatePacket(chain, ORDERING_TABLE_SIZE - 1, 4);
    ptr[0] = gp0_texpage(0, true, false);
    ptr[1] = gp0_fbOffset1(bufferX, bufferY);
    ptr[2] = gp0_fbOffset2(
        bufferX + SCREEN_WIDTH  - 1,
        bufferY + SCREEN_HEIGHT - 2
    );
    ptr[3] = gp0_fbOrigin(bufferX, bufferY);
}

void buildRenderPackets(DMAChain *chain, ObjModel *modelToRender, uint32_t color) {
    uint32_t *ptr;

    for (int i = 0; i < modelToRender->facesCount; i++) {
        const Face face = modelToRender->faces[i];

        gte_loadV0(&modelToRender->vertices[face.i1]);
        gte_loadV1(&modelToRender->vertices[face.i2]);
        gte_loadV2(&modelToRender->vertices[face.i3]);
        gte_command(GTE_CMD_RTPT | GTE_SF);

        uint32_t xy0 = 0;
        if (face.type == QUAD) {
            gte_command(GTE_CMD_NCLIP);

            if (gte_getDataReg(GTE_MAC0) <= 0)
                continue;

            xy0 = gte_getDataReg(GTE_SXY0);
            gte_loadV0(&modelToRender->vertices[face.i4]);
            gte_command(GTE_CMD_RTPS | GTE_SF);
            gte_command(GTE_CMD_AVSZ4 | GTE_SF);
        } else {
            gte_command(GTE_CMD_AVSZ3 | GTE_SF);
        }

        int zIndex = gte_getDataReg(GTE_OTZ);

        if ((zIndex < 0) || (zIndex >= ORDERING_TABLE_SIZE))
            continue;

        if (face.type == QUAD) {
            ptr = allocatePacket(chain, zIndex, 5);
            ptr[0] = color | gp0_shadedQuad(false, false, false);
            ptr[1] = xy0;
            gte_storeDataReg(GTE_SXY0, 2 * 4, ptr);
            gte_storeDataReg(GTE_SXY1, 3 * 4, ptr);
            gte_storeDataReg(GTE_SXY2, 4 * 4, ptr);
        } else {
            ptr = allocatePacket(chain, zIndex, 4);
            ptr[0] = color | gp0_shadedTriangle(false, false, false);
            gte_storeDataReg(GTE_SXY0, 1 * 4, ptr);
            gte_storeDataReg(GTE_SXY1, 2 * 4, ptr);
            gte_storeDataReg(GTE_SXY2, 3 * 4, ptr);
        }
    }
}
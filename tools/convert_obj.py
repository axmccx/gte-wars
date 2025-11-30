import os
import sys
import struct

MAX_INT = 32_767
POINT_SCALE = 16


class Vertex:
    x: int
    y: int
    z: int

    def __init__(self, obj_file_line):
        raw_point = obj_file_line.strip().split()[1:4]
        scaled_point = []
        for p in raw_point:
            fixed_p = round(float(p) * POINT_SCALE)
            clamped_p = min(MAX_INT, max(-MAX_INT, fixed_p))
            scaled_point.append(clamped_p)

        self.x, self.y, self.z = scaled_point


class Face:
    vertex_idx: list

    def __init__(self, obj_file_line):
        raw_face = obj_file_line.split()[1:]
        v_idx = []
        for part in raw_face:
            indices = part.split("/")
            idx = int(indices[0]) - 1
            v_idx.append(idx)

        if len(v_idx) == 4:
            v_idx = [v_idx[0], v_idx[3], v_idx[1], v_idx[2]]

        self.vertex_idx = v_idx

    @property
    def is_quad(self):
        return len(self.vertex_idx) == 4


def parse_obj_file(file_path):
    vertices = []
    faces = []

    with open(file_path) as f:
        for i, line in enumerate(f):
            if line.startswith("v "):
                vertices.append(Vertex(line))
            if line.startswith("f "):
                faces.append(Face(line))

    return vertices, faces


def write_model_bin(output, vertices, faces):
    with open(output, "wb") as f:
        f.write(b"MODEL")
        f.write(struct.pack("<H", len(vertices)))
        f.write(struct.pack("<H", len(faces)))

        for v in vertices:
            f.write(struct.pack("<hhh", v.x, v.y, v.z))

        for face in faces:
            is_quad = len(face.vertex_idx) == 4
            face_fmt = "<BHHHH" if is_quad else "<BHHH"
            f.write(struct.pack(face_fmt, len(face.vertex_idx), *face.vertex_idx))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {os.path.basename(sys.argv[0])} input.obj output.dat")
        sys.exit(1)

    obj_file_path = sys.argv[1]
    output_file_path = sys.argv[2]

    vertices, faces = parse_obj_file(obj_file_path)
    write_model_bin(output_file_path, vertices, faces)

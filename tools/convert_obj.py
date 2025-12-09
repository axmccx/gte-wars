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

        # expects Blender's default export settings: Forward-Axis: -Z, Up-Axis: Y
        self.x = scaled_point[0]
        self.y = -scaled_point[2] # flip Y to orient object "UP"
        self.z = scaled_point[1] # Might need to negate this, but for now models are symmetric across Z


class Face:
    vertex_idx: list
    material: str | None

    def __init__(self, obj_file_line, material=None):
        raw_face = obj_file_line.split()[1:]
        v_idx = []
        for part in raw_face:
            indices = part.split("/")
            idx = int(indices[0]) - 1
            v_idx.append(idx)

        if len(v_idx) == 4:
            v_idx = [v_idx[0], v_idx[3], v_idx[1], v_idx[2]]

        self.vertex_idx = v_idx
        self.material = material

    @property
    def is_quad(self):
        return len(self.vertex_idx) == 4


def mtl_to_ps1_color(r, g, b):
    r8 = max(0, min(255, round(float(r) * 255.0)))
    g8 = max(0, min(255, round(float(g) * 255.0)))
    b8 = max(0, min(255, round(float(b) * 255.0)))
    return (b8 << 16) | (g8 << 8) | r8


def parse_obj_file(file_path):
    vertices = []
    faces = []
    current_material = None
    mtl_filename = None

    with open(file_path) as f:
        for i, line in enumerate(f):
            if line.startswith("mtllib "):
                mtl_filename = line.split()[1].strip()
            if line.startswith("usemtl "):
                current_material = line.split()[1]
            if line.startswith("v "):
                vertices.append(Vertex(line))
            if line.startswith("f "):
                faces.append(Face(line, material=current_material))

    return vertices, faces, mtl_filename


def parse_mtl_file(file_path):
    colors = {}
    current_material = None

    with open(file_path) as f:
        for line in f:
            if line.startswith("newmtl "):
                current_material = line.split()[1]
            if line.startswith("Kd "):
                parts = line.split()
                r, g, b = float(parts[1]), float(parts[2]), float(parts[3])
                colors[current_material] = mtl_to_ps1_color(r, g, b)

    return colors


def write_model_bin(output, vertices, faces, colors):
    with open(output, "wb") as f:
        f.write(b"MODEL")
        f.write(struct.pack("<H", len(vertices)))
        f.write(struct.pack("<H", len(faces)))

        for v in vertices:
            f.write(struct.pack("<hhh", v.x, v.y, v.z))

        for face in faces:
            is_quad = len(face.vertex_idx) == 4
            face_fmt = "<BIHHHH" if is_quad else "<BIHHH"
            f.write(struct.pack(face_fmt, len(face.vertex_idx), colors.get(face.material, 0), *face.vertex_idx))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {os.path.basename(sys.argv[0])} input.obj output.dat")
        sys.exit(1)

    obj_file_path = sys.argv[1]
    output_file_path = sys.argv[2]

    vertices, faces, mtl_filename = parse_obj_file(obj_file_path)

    colors = {}
    if mtl_filename:
        obj_dir = os.path.dirname(obj_file_path)
        mtl_file_path = os.path.join(obj_dir, mtl_filename)
        colors = parse_mtl_file(mtl_file_path)

    write_model_bin(output_file_path, vertices, faces, colors)

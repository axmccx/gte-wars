
# point_scale = 4096
point_scale = 64

class Vertex:
    x: int
    y: int
    z: int

    def __init__(self, obj_file_line):
        raw_point = obj_file_line.split()[1:]
        scaled_point = []
        for p in raw_point:
            fixed_p = round(float(p) * point_scale)
            clamped_p = min(32767, max(-32767, fixed_p))
            scaled_point.append(clamped_p)

        self.x = scaled_point[0]
        self.y = scaled_point[1]
        self.z = scaled_point[2]

    def __str__(self):
        return f"{{ .x = {self.x}, .y = {self.y}, .z = {self.z} }},"


class Face:
    vertex_x: int
    vertex_y: int
    vertex_z: int
    color: str

    def __init__(self, obj_file_line, color):
        raw_face = obj_file_line.split()[1:]
        extracted_vertex_indices = []
        for part in raw_face:
            indices = part.split('/')
            v_index = int(indices[0]) - 1
            extracted_vertex_indices.append(v_index)

        self.vertex_x = extracted_vertex_indices[0]
        self.vertex_y = extracted_vertex_indices[1]
        self.vertex_z = extracted_vertex_indices[2]
        self.color = color

    def __str__(self):
        return f"{{ .vertices = {{ {self.vertex_x}, {self.vertex_y}, {self.vertex_z} }}, .color = {self.color} }},"


colors = ["0x0000ff", "0x0000ff", "0x00ffff", "0xff0000", "0xff00ff", "0xffff00"]

vertices = []
faces = []

with open("objects/monkey.obj") as f:
    for i, line in enumerate(f):
        if line.startswith('v '):
            vertices.append(Vertex(line))
        if line.startswith('f '):
            color_index = (i % len(colors))
            faces.append(Face(line, colors[color_index]))


vertex_object_c_string = f"""static const GTEVector16 objVertices[{len(vertices)}] = {{
{"    \n".join(str(v) for v in vertices)}
}};
"""

face_object_c_string = f"""static const TriFace cubeFaces[{len(faces)}] = {{
{"    \n".join(str(v) for v in faces)}
}};
"""


print(vertex_object_c_string)
print(face_object_c_string)

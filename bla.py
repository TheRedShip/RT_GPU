def parse_vector(vector_str):
    """Parse a vector string like '+13.34,+0.320,-4.670' into a list of floats."""
    return [float(coord) for coord in vector_str.split(',')]

def parse_material(fields):
    """Parse material-related fields and return the material line."""
    color = ' '.join(fields[3].split(','))
    light_luminosity = ' '.join(fields[4].split(',')) if len(fields) > 4 else '0.00,0.00,0.00'
    roughness = fields[5] if len(fields) > 5 else '0.00'
    return f"MAT {color} {light_luminosity} {roughness}"

def parse_sphere(fields, mat_index):
    """Parse a sphere line and return the sphere output line."""
    position = ' '.join(fields[1].split(','))
    radius = fields[2]
    return f"sp {position} {radius} {mat_index}"

def parse_plane(fields, mat_index):
    """Parse a plane line and return the plane output line."""
    position = ' '.join(fields[1].split(','))
    normal = ' '.join(fields[2].split(','))
    return f"pl {position} {normal} {mat_index}"

def transform_file(input_file, output_file):
    """Transform input.rt into output.rt."""
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        mat_index = 0
        for line in infile:
            line = line.strip()
            if not line or line.startswith('#'):  # Skip empty lines and comments
                continue

            fields = line.split()
            obj_type = fields[0]

            if obj_type in ('sp', 'ls'):  # Sphere or light sphere
                material = parse_material(fields)
                sphere = parse_sphere(fields, mat_index)
                outfile.write(material + '\n')
                outfile.write(sphere + '\n')
                mat_index += 1

            elif obj_type == 'pl':  # Plane
                material = parse_material(fields)
                plane = parse_plane(fields, mat_index)
                outfile.write(material + '\n')
                outfile.write(plane + '\n')
                mat_index += 1

if __name__ == "__main__":
    input_file = "input.rt"
    output_file = "output.rt"
    transform_file(input_file, output_file)
    print(f"Transformed {input_file} into {output_file}.")
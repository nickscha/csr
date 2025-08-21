#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VERTICES 50000
#define MAX_faceS 50000
#define MAX_LINE 512

typedef struct v3
{
    float x, y, z;
} v3;

typedef struct face
{
    int a, b, c;
} face;

/* Utility to generate a valid include guard name from filename */
void make_guard_name(const char *filename, char *guard)
{
    int i, j = 0;
    for (i = 0; filename[i] && j < 63; ++i)
    {
        char c = filename[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
        {
            guard[j++] = (c >= 'a' && c <= 'z') ? c - 32 : c; /* uppercase */
        }
        else
        {
            guard[j++] = '_';
        }
    }
    guard[j] = '\0';
}

v3 cross_product(v3 v1, v3 v2)
{
    v3 result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

float dot_product(v3 v1, v3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

void clean_string(const char *input, char *output, size_t max_len)
{
    size_t i, j = 0;
    for (i = 0; i < strlen(input) && j < max_len - 1; ++i)
    {
        if (isalnum(input[i]))
        {
            output[j++] = input[i];
        }
        else if (input[i] == '-' || input[i] == '.')
        {
            output[j++] = '_';
        }
    }
    output[j] = '\0';
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s model.obj output_prefix\n", argv[0]);
        return 1;
    }

    const char *obj_file = argv[1];
    const char *prefix_name = argv[2];

    char header_file[256];
    snprintf(header_file, sizeof(header_file), "%s.h", prefix_name);

    char clean_prefix[256];
    clean_string(prefix_name, clean_prefix, sizeof(clean_prefix));

    FILE *f = fopen(obj_file, "r");
    if (!f)
    {
        perror("Failed to open OBJ file");
        return 1;
    }

    v3 vertices[MAX_VERTICES];
    face faces[MAX_faceS];
    int vert_count = 0;
    int face_count = 0;
    char line[MAX_LINE];
    int total_vertices = 0;

    /* First pass to count total vertices */
    while (fgets(line, MAX_LINE, f))
    {
        if (strncmp(line, "v ", 2) == 0)
        {
            total_vertices++;
        }
    }
    rewind(f);

    /* Second pass to parse data */
    while (fgets(line, MAX_LINE, f))
    {
        if (strncmp(line, "v ", 2) == 0)
        {
            if (vert_count >= MAX_VERTICES)
            {
                fprintf(stderr, "Too many vertices\n");
                return 1;
            }
            v3 v;
            sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z);
            vertices[vert_count++] = v;
        }
        else if (strncmp(line, "f ", 2) == 0)
        {
            int idx[8];
            int n;

            if (strchr(line, '/') != NULL)
            {
                n = sscanf(line + 2, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d",
                           &idx[0], &idx[1], &idx[2], &idx[3],
                           &idx[4], &idx[5], &idx[6], &idx[7]);
                if (n < 3)
                {
                    n = sscanf(line + 2, "%d/%*d %d/%*d %d/%*d %d/%*d %d/%*d %d/%*d %d/%*d %d/%*d",
                               &idx[0], &idx[1], &idx[2], &idx[3],
                               &idx[4], &idx[5], &idx[6], &idx[7]);
                }
            }
            else
            {
                n = sscanf(line + 2, "%d %d %d %d %d %d %d %d",
                           &idx[0], &idx[1], &idx[2], &idx[3],
                           &idx[4], &idx[5], &idx[6], &idx[7]);
            }

            if (n < 3)
                continue;

            for (int i = 1; i < n - 1; ++i)
            {
                if (face_count >= MAX_faceS)
                {
                    fprintf(stderr, "Too many faces\n");
                    return 1;
                }

                int a = (idx[0] < 0) ? total_vertices + idx[0] : idx[0] - 1;
                int b = (idx[i] < 0) ? total_vertices + idx[i] : idx[i] - 1;
                int c = (idx[i + 1] < 0) ? total_vertices + idx[i + 1] : idx[i + 1] - 1;

                faces[face_count].a = a;
                faces[face_count].b = b;
                faces[face_count].c = c;

                face_count++;
            }
        }
    }
    fclose(f);

    FILE *hf = fopen(header_file, "w");
    if (!hf)
    {
        perror("Failed to open output header file");
        return 1;
    }

    char guard[64];
    make_guard_name(header_file, guard);

    fprintf(hf, "#ifndef %s\n#define %s\n\n", guard, guard);
    fprintf(hf, "static float %s_vertices[] = {\n", clean_prefix);
    for (int i = 0; i < vert_count; ++i)
    {
        fprintf(hf, "    %ff, %ff, %ff%s\n",
                vertices[i].x, vertices[i].y, vertices[i].z,
                (i < vert_count - 1) ? "," : "");
    }
    fprintf(hf, "};\n\n");

    fprintf(hf, "static int %s_indices[] = {\n", clean_prefix);
    for (int i = 0; i < face_count; ++i)
    {
        fprintf(hf, "    %d, %d, %d%s\n",
                faces[i].a, faces[i].b, faces[i].c,
                (i < face_count - 1) ? "," : "");
    }
    fprintf(hf, "};\n\n");

    fprintf(hf, "static unsigned long %s_vertices_size = %dUL;\n", clean_prefix, vert_count * 3);
    fprintf(hf, "static unsigned long %s_indices_size = %dUL;\n\n", clean_prefix, face_count * 3);
    fprintf(hf, "#endif /* %s */\n", guard);

    fclose(hf);
    printf("Header file '%s' generated with %d vertices and %d triangles.\n",
           header_file, vert_count, face_count);

    return 0;
}
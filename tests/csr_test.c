/* csr.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) software renderer (CSR).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include <stdio.h>  /* Testing only: write ppm file                                        */
#include <stdlib.h> /* Testing only: malloc/free                                           */
#include "../csr.h" /* C Software Renderer                                                 */
#include "vm.h"     /* Linear Algebra Math Library (you can use any library that you want) */
#include "perf.h"   /* Simple Performance Profiler                                         */

/* Vertex data array with interleaved position and color (RGB) */
static float vertices[] = {
    /* Position x,y,z  | Color r,g,b */
    -0.5f, -0.5f, 0.5f, 255.0f, 0.0f, 0.0f,    /* 0: Red     */
    0.5f, -0.5f, 0.5f, 0.0f, 255.0f, 0.0f,     /* 1: Green   */
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 255.0f,      /* 2: Blue    */
    -0.5f, 0.5f, 0.5f, 255.0f, 255.0f, 0.0f,   /* 3: Yellow  */
    -0.5f, -0.5f, -0.5f, 255.0f, 0.0f, 255.0f, /* 4: Magenta */
    0.5f, -0.5f, -0.5f, 0.0f, 255.0f, 255.0f,  /* 5: Cyan    */
    0.5f, 0.5f, -0.5f, 255.0f, 255.0f, 255.0f, /* 6: White   */
    -0.5f, 0.5f, -0.5f, 128.0f, 128.0f, 128.0f /* 7: Gray    */
};

/* Index data counterclockwise to form the triangles of a cube.  */
static int indices[] = {
    0, 3, 2, 0, 2, 1, /* Front face (+z normal, facing camera)   */
    4, 5, 6, 4, 6, 7, /* Back face (-z normal, away from camera) */
    3, 7, 6, 3, 6, 2, /* Top face (+y normal)                    */
    0, 1, 5, 0, 5, 4, /* Bottom face (-y normal)                 */
    1, 2, 6, 1, 6, 5, /* Right face (+x normal)                  */
    0, 4, 7, 0, 7, 3  /* Left face (-x normal)                   */
};

static unsigned long vertices_size = sizeof(vertices) / sizeof(vertices[0]);
static unsigned long indices_size = sizeof(indices) / sizeof(indices[0]);

/*
 * Saves a framebuffer to a PPM image file.
 *
 * @param filename The name of the output file.
 * @param framebuffer The framebuffer to save.
 */
static void csr_save_ppm(const char *filename, csr_model *model)
{
  FILE *fp = fopen(filename, "wb");
  if (!fp)
  {
    fprintf(stderr, "Error: Could not open file %s for writing.\n", filename);
    return;
  }

  /* PPM header */
  fprintf(fp, "P6\n%d %d\n255\n", model->width, model->height);

  /* Pixel data */
  fwrite(model->framebuffer, sizeof(csr_color), (size_t)(model->width * model->height), fp);

  fclose(fp);
}

static void csr_test_stack_alloc(void)
{
/* Define the render area */
#define WIDTH 400
#define HEIGHT 300
#define MEMORY_SIZE (WIDTH * HEIGHT * sizeof(csr_color)) + (WIDTH * HEIGHT * sizeof(float))

  unsigned char memory_total[MEMORY_SIZE] = {0};
  void *memory = (void *)memory_total;

  csr_color clear_color = {40, 40, 40, 255};
  csr_model instance = {0};

  if (!csr_init_model(&instance, memory, MEMORY_SIZE, WIDTH, HEIGHT, clear_color))
  {
    return;
  }

  {
    /* Camera setup using your linear algebra library */
    v3 look_at_pos = vm_v3_zero;
    v3 up = vm_v3(0.0f, 1.0f, 0.0f);
    v3 cam_position = vm_v3(0.0f, 0.0f, 2.0f);
    float cam_fov = 90.0f;

    m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (float)instance.width / (float)instance.height, 0.1f, 1000.0f);
    m4x4 view = vm_m4x4_lookAt(cam_position, look_at_pos, up);
    m4x4 projection_view = vm_m4x4_mul(projection, view);

    v3 rotation_axis = vm_v3(0.5f, 1.0f, 0.0);
    m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, vm_v3_zero);

    char filename[64];

    int frame;

    for (frame = 0; frame < 10; ++frame)
    {
      m4x4 model_view_projection = vm_m4x4_mul(projection_view, vm_m4x4_rotate(model_base, vm_radf(5.0f * (float)(frame + 1)), rotation_axis));

      PERF_PROFILE_WITH_NAME({ csr_clear_screen(&instance); }, "csr_clear_screen");
      PERF_PROFILE_WITH_NAME({ csr_render(&instance, vertices, vertices_size, indices, indices_size, model_view_projection.e); }, "csr_render_frame");

      /* Format the filename with the frame number */
      sprintf(filename, "output_%05d.ppm", frame);

      /* Save the result to a PPM file */
      csr_save_ppm(filename, &instance);
    }
  }
}

static void csr_test_cube_scene_with_memory_alloc(void)
{
  int width = 800;
  int height = 600;

  unsigned long memory_size = csr_memory_size(width, height);
  void *memory = malloc(memory_size);

  csr_color clear_color = {40, 40, 40, 255};
  csr_model instance = {0};

  printf("[csr] memory (MB): %10.4f\n", (double)memory_size / 1024.0 / 1024.0);

  if (!csr_init_model(&instance, memory, memory_size, width, height, clear_color))
  {
    return;
  }

  {
    /* Camera setup using your linear algebra library */
    v3 look_at_pos = vm_v3_zero;
    v3 up = vm_v3(0.0f, 1.0f, 0.0f);
    v3 cam_position = vm_v3(0.0f, 0.0f, 2.0f);
    float cam_fov = 90.0f;

    m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (float)instance.width / (float)instance.height, 0.1f, 1000.0f);
    m4x4 view = vm_m4x4_lookAt(cam_position, look_at_pos, up);
    m4x4 projection_view = vm_m4x4_mul(projection, view);

    v3 rotation_axis = vm_v3(0.5f, 1.0f, 0.0);
    m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, vm_v3_zero);

    char filename[64];

    int frame;

    for (frame = 0; frame < 10; ++frame)
    {
      m4x4 model = vm_m4x4_rotate(model_base, vm_radf(5.0f * (float)(frame + 1)), rotation_axis);
      m4x4 model_view_projection = vm_m4x4_mul(projection_view, model);

      csr_clear_screen(&instance);

      /* Render first cube */
      csr_render(&instance, vertices, vertices_size, indices, indices_size, model_view_projection.e);

      /* Render second cube */
      model = vm_m4x4_translate(vm_m4x4_identity, vm_v3(-2.0, 0.0f, -2.0f));
      model_view_projection = vm_m4x4_rotate(vm_m4x4_mul(projection_view, model), vm_radf(-2.5f * (float)(frame + 1)), vm_v3(1.0f, 1.0f, 1.0f));
      csr_render(&instance, vertices, vertices_size, indices, indices_size, model_view_projection.e);

      /* Render third cube */
      model = vm_m4x4_translate(vm_m4x4_identity, vm_v3(4.0, 0.0f, -5.0f));
      model_view_projection = vm_m4x4_mul(projection_view, model);
      csr_render(&instance, vertices, vertices_size, indices, indices_size, model_view_projection.e);

      /* Format the filename with the frame number */
      sprintf(filename, "output_%05d.ppm", frame);

      /* Save the result to a PPM file */
      csr_save_ppm(filename, &instance);
    }
  }

  free(memory);
}

int main(void)
{
  csr_test_stack_alloc();
  csr_test_cube_scene_with_memory_alloc();

  return 0;
}

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/

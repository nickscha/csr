/* csr.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) software renderer (CSR).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#include <stdio.h>  /* For file I/O operations (per user request) */
#include <stdlib.h> /* For malloc() */

#include "../csr.h"

#include "vm.h"

/*
 * Saves a framebuffer to a PPM image file.
 *
 * @param filename The name of the output file.
 * @param framebuffer The framebuffer to save.
 */
static void csr_save_ppm(const char *filename, csr_color *framebuffer)
{
  FILE *fp = fopen(filename, "wb");
  if (!fp)
  {
    fprintf(stderr, "Error: Could not open file %s for writing.\n", filename);
    return;
  }

  /* PPM header */
  fprintf(fp, "P6\n%d %d\n255\n", CSR_WIDTH, CSR_HEIGHT);

  /* Pixel data */
  fwrite(framebuffer, sizeof(csr_color), CSR_WIDTH * CSR_HEIGHT, fp);

  fclose(fp);
  printf("Successfully saved image to %s\n", filename);
}

int main(void)
{
  /* Set background clear color */
  csr_color clear_color = {40, 40, 40};

  /* Allocate memory for the framebuffer and z-buffer */
  csr_color *framebuffer = (csr_color *)malloc(CSR_WIDTH * CSR_HEIGHT * sizeof(csr_color));
  float *zbuffer = (float *)malloc(CSR_WIDTH * CSR_HEIGHT * sizeof(float));

  /* Camera setup using your linear algebra library */
  v3 look_at_pos = vm_v3_zero;
  v3 up = vm_v3(0.0f, 1.0f, 0.0f);
  v3 cam_position = vm_v3(0.0f, 0.0f, 2.0f);
  float cam_fov = 90.0f;

  m4x4 projection = vm_m4x4_perspective(vm_radf(cam_fov), (float)CSR_WIDTH / (float)CSR_HEIGHT, 0.1f, 1000.0f);
  m4x4 view = vm_m4x4_lookAt(cam_position, look_at_pos, up);
  m4x4 projection_view = vm_m4x4_mul(projection, view);

  v3 rotation_axis = vm_v3(0.5f, 1.0f, 0.0);
  m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, vm_v3_zero);

  /* Vertex data array with interleaved position and color (RGB) */
  float vertices[] = {
      /* Position (x, y, z) */                   /* Color (R, G, B) */
      -0.5f, -0.5f, 0.5f, 255.0f, 0.0f, 0.0f,    /* 0: Red */
      0.5f, -0.5f, 0.5f, 0.0f, 255.0f, 0.0f,     /* 1: Green */
      0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 255.0f,      /* 2: Blue */
      -0.5f, 0.5f, 0.5f, 255.0f, 255.0f, 0.0f,   /* 3: Yellow */
      -0.5f, -0.5f, -0.5f, 255.0f, 0.0f, 255.0f, /* 4: Magenta */
      0.5f, -0.5f, -0.5f, 0.0f, 255.0f, 255.0f,  /* 5: Cyan */
      0.5f, 0.5f, -0.5f, 255.0f, 255.0f, 255.0f, /* 6: White */
      -0.5f, 0.5f, -0.5f, 128.0f, 128.0f, 128.0f /* 7: Gray */
  };

  unsigned long num_floats_in_vertices = sizeof(vertices) / sizeof(vertices[0]);

  /* Index data to form the triangles of a cube. */
  int indices[] = {
      /* Front face (+z normal, facing camera) */
      0, 3, 2,
      0, 2, 1,

      /* Back face (-z normal, away from camera) */
      4, 5, 6,
      4, 6, 7,

      /* Top face (+y normal) */
      3, 7, 6,
      3, 6, 2,

      /* Bottom face (-y normal) */
      0, 1, 5,
      0, 5, 4,

      /* Right face (+x normal) */
      1, 2, 6,
      1, 6, 5,

      /* Left face (-x normal) */
      0, 4, 7,
      0, 7, 3};

  unsigned long num_indices = sizeof(indices) / sizeof(indices[0]);

  char filename[64];

  int frame;

  for (frame = 0; frame < 10; ++frame)
  {
    m4x4 model = vm_m4x4_rotate(model_base, vm_radf(5.0f * (float)(frame + 1)), rotation_axis);
    m4x4 model_view_projection = vm_m4x4_mul(projection_view, model);

    csr_clear_screen(framebuffer, zbuffer, clear_color);

    /* Render first cube */
    csr_render(framebuffer, zbuffer, vertices, num_floats_in_vertices, indices, num_indices, model_view_projection.e);

    /* Render second cube */
    model = vm_m4x4_translate(vm_m4x4_identity, vm_v3(-2.0, 0.0f, -2.0f));
    model_view_projection = vm_m4x4_rotate(vm_m4x4_mul(projection_view, model), vm_radf(-2.5f * (float)(frame + 1)), vm_v3(1.0f, 1.0f, 1.0f));
    csr_render(framebuffer, zbuffer, vertices, num_floats_in_vertices, indices, num_indices, model_view_projection.e);

    /* Render third cube */
    model = vm_m4x4_translate(vm_m4x4_identity, vm_v3(4.0, 0.0f, -5.0f));
    model_view_projection = vm_m4x4_mul(projection_view, model);
    csr_render(framebuffer, zbuffer, vertices, num_floats_in_vertices, indices, num_indices, model_view_projection.e);

    /* Format the filename with the frame number */
    sprintf(filename, "output_%05d.ppm", frame);

    /* Save the result to a PPM file */
    csr_save_ppm(filename, framebuffer);
  }

  /* Cleanup */
  free(framebuffer);
  free(zbuffer);

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

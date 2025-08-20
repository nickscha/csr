/* csr.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) software renderer (CSR).

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef CSR_H
#define CSR_H

/* #############################################################################
 * # COMPILER SETTINGS
 * #############################################################################
 */
/* Check if using C99 or later (inline is supported) */
#if __STDC_VERSION__ >= 199901L
#define CSR_INLINE inline
#define CSR_API static
#elif defined(__GNUC__) || defined(__clang__)
#define CSR_INLINE __inline__
#define CSR_API static
#elif defined(_MSC_VER)
#define CSR_INLINE __inline
#define CSR_API static
#else
#define CSR_INLINE
#define CSR_API static
#endif

/* #############################################################################
 * # MATH Functions
 * #############################################################################
 */
CSR_API CSR_INLINE float csr_minf(float a, float b)
{
  return (a < b) ? a : b;
}

CSR_API CSR_INLINE float csr_maxf(float a, float b)
{
  return (a > b) ? a : b;
}

CSR_API CSR_INLINE int csr_mini(int a, int b)
{
  return (a < b) ? a : b;
}

CSR_API CSR_INLINE int csr_maxi(int a, int b)
{
  return (a > b) ? a : b;
}

CSR_API CSR_INLINE void csr_pos_init(float *pos, float x, float y, float z, float w)
{
  pos[0] = x;
  pos[1] = y;
  pos[2] = z;
  pos[3] = w;
}

CSR_API CSR_INLINE void csr_v4_divf(float result[4], float v[4], float f)
{
  result[0] = v[0] / f;
  result[1] = v[1] / f;
  result[2] = v[2] / f;
  result[3] = v[3] / f;
}

CSR_API CSR_INLINE void csr_m4x4_mul_v4(float result[4], float m[16], float v[4])
{
  result[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3];
  result[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3];
  result[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3];
  result[3] = m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3];
}

/* #############################################################################
 * # RENDERING Functions
 * #############################################################################
 */
typedef struct csr_color
{
  unsigned char r;
  unsigned char g;
  unsigned char b;

} csr_color;

typedef struct csr_model
{

  int width;              /* render area width in pixels            */
  int height;             /* render area height in pixels           */
  csr_color clear_color;  /* The default clear color for the screen */
  csr_color *framebuffer; /* memory pointer for framebuffer         */
  float *zbuffer;         /* memory pointer for zbuffer             */

} csr_model;

CSR_API CSR_INLINE unsigned long csr_memory_size(int width, int height)
{
  return (unsigned long)(width * height * (int)sizeof(csr_color) + /* framebuffer size */
                         width * height * (int)sizeof(float)       /* zbuffer size     */
  );
}

CSR_API CSR_INLINE int csr_init_model(csr_model *model, void *memory, unsigned long memory_size, int width, int height, csr_color clear_color)
{
  unsigned long memory_framebuffer_size = (unsigned long)(width * height) * (unsigned long)sizeof(csr_color);

  if (memory_size < csr_memory_size(width, height))
  {
    return 0;
  }

  model->width = width;
  model->height = height;
  model->clear_color = clear_color;
  model->framebuffer = (csr_color *)memory;
  model->zbuffer = (float *)((char *)memory + memory_framebuffer_size);

  return 1;
}

CSR_API CSR_INLINE csr_color csr_color_init(unsigned char r, unsigned char g, unsigned char b)
{
  csr_color result;
  result.r = r;
  result.g = g;
  result.b = b;

  return result;
}

/* Converts a point from normalized device coordinates(NDC) to screen space. */
CSR_API CSR_INLINE void csr_ndc_to_screen(csr_model *model, float result[3], float ndc_pos[4])
{
  result[0] = (ndc_pos[0] + 1.0f) * 0.5f * (float)model->width;
  result[1] = (1.0f - ndc_pos[1]) * 0.5f * (float)model->height;
  result[2] = ndc_pos[2];
}

CSR_API CSR_INLINE void csr_clear_screen(csr_model *model)
{
  int size = model->width * model->height;
  csr_color c = model->clear_color;
  
  int i = 0;

  for (; i + 4 <= size; i += 4)
  {
    model->framebuffer[i] = c;
    model->framebuffer[i + 1] = c;
    model->framebuffer[i + 2] = c;
    model->framebuffer[i + 3] = c;
    model->zbuffer[i] = 1.0f;
    model->zbuffer[i + 1] = 1.0f;
    model->zbuffer[i + 2] = 1.0f;
    model->zbuffer[i + 3] = 1.0f;
  }

  for (; i < size; ++i)
  {
    model->framebuffer[i] = c;
    model->zbuffer[i] = 1.0f;
  }
}

/* Fills a triangle using the barycentric coordinate method with color interpolation. */
CSR_API CSR_INLINE void csr_draw_triangle(csr_model *model, float p0[3], float p1[3], float p2[3], csr_color c0, csr_color c1, csr_color c2)
{
  /* Bounding box for the triangle */
  int min_x = (int)csr_minf(p0[0], csr_minf(p1[0], p2[0]));
  int min_y = (int)csr_minf(p0[1], csr_minf(p1[1], p2[1]));
  int max_x = (int)csr_maxf(p0[0], csr_maxf(p1[0], p2[0]));
  int max_y = (int)csr_maxf(p0[1], csr_maxf(p1[1], p2[1]));

  /* Pre-calculate constants for barycentric coordinates */
  float area = (p1[1] - p2[1]) * (p0[0] - p2[0]) + (p2[0] - p1[0]) * (p0[1] - p2[1]);

  if (area == 0.0f)
  {
    return;
  }

  /* Clamp bounding box to screen dimensions */
  min_x = csr_maxi(0, min_x);
  min_y = csr_maxi(0, min_y);
  max_x = csr_mini(model->width - 1, max_x);
  max_y = csr_mini(model->height - 1, max_y);

  {
    float inv_area = 1.0f / area;

    /* Calculate barycentric coordinate derivatives with respect to x and y */
    float w0_dx = (p1[1] - p2[1]) * inv_area;
    float w1_dx = (p2[1] - p0[1]) * inv_area;
    float w2_dx = -w0_dx - w1_dx;

    float w0_dy = (p2[0] - p1[0]) * inv_area;
    float w1_dy = (p0[0] - p2[0]) * inv_area;
    float w2_dy = -w0_dy - w1_dy;

    /* Initialize barycentric coordinates at the top-left of the bounding box */
    float w0_start = (p1[1] - p2[1]) * ((float)min_x - p2[0]) + (p2[0] - p1[0]) * ((float)min_y - p2[1]);
    float w1_start = (p2[1] - p0[1]) * ((float)min_x - p0[0]) + (p0[0] - p2[0]) * ((float)min_y - p0[1]);
    float w2_start = area - w0_start - w1_start;

    int x, y;

    w0_start *= inv_area;
    w1_start *= inv_area;
    w2_start *= inv_area;

    for (y = min_y; y <= max_y; ++y)
    {
      float w0 = w0_start;
      float w1 = w1_start;
      float w2 = w2_start;

      for (x = min_x; x <= max_x; ++x)
      {
        if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f)
        {
          /* Interpolate Z-depth and color using w values */
          float z = p0[2] * w0 + p1[2] * w1 + p2[2] * w2;

          int index = y * model->width + x;

          /* Depth testing: only draw if the new pixel is closer than the existing one */
          if (z < model->zbuffer[index])
          {
            csr_color pixel_color;
            pixel_color.r = (unsigned char)(c0.r * w0 + c1.r * w1 + c2.r * w2);
            pixel_color.g = (unsigned char)(c0.g * w0 + c1.g * w1 + c2.g * w2);
            pixel_color.b = (unsigned char)(c0.b * w0 + c1.b * w1 + c2.b * w2);

            model->framebuffer[index] = pixel_color;
            model->zbuffer[index] = z;
          }
        }

        /* Increment barycentric coordinates with pre-calculated deltas */
        w0 += w0_dx;
        w1 += w1_dx;
        w2 += w2_dx;
      }

      /* Reset w values for the start of the next row */
      w0_start += w0_dy;
      w1_start += w1_dy;
      w2_start += w2_dy;
    }
  }
}

CSR_API CSR_INLINE void csr_render(csr_model *model, float *vertices, unsigned long num_vertices, int *indices, unsigned long num_indices, float projection_view_model_matrix[16])
{
  unsigned long i;

  /* Each vertex has 6 floats (3 for position, 3 for color) */
  int stride = 6;

  (void)num_vertices;

  for (i = 0; i < num_indices; i += 3)
  {
    /* Get vertex indices for the current triangle */
    int i0 = indices[i];
    int i1 = indices[i + 1];
    int i2 = indices[i + 2];

    /* Get the vertex data from the main vertex array using the indices, and convert to homogeneous coordinates */
    float pos0[4];
    float pos1[4];
    float pos2[4];

    csr_color color0 = csr_color_init((unsigned char)vertices[i0 * stride + 3], (unsigned char)vertices[i0 * stride + 4], (unsigned char)vertices[i0 * stride + 5]);
    csr_color color1 = csr_color_init((unsigned char)vertices[i1 * stride + 3], (unsigned char)vertices[i1 * stride + 4], (unsigned char)vertices[i1 * stride + 5]);
    csr_color color2 = csr_color_init((unsigned char)vertices[i2 * stride + 3], (unsigned char)vertices[i2 * stride + 4], (unsigned char)vertices[i2 * stride + 5]);

    /* 1. Vertex Processing (Model, View, Projection) */
    float v0_transformed[4];
    float v1_transformed[4];
    float v2_transformed[4];

    float v0_ndc[4];
    float v1_ndc[4];
    float v2_ndc[4];

    float v0_screen[3];
    float v1_screen[3];
    float v2_screen[3];

    csr_pos_init(pos0, vertices[i0 * stride + 0], vertices[i0 * stride + 1], vertices[i0 * stride + 2], 1.0f);
    csr_pos_init(pos1, vertices[i1 * stride + 0], vertices[i1 * stride + 1], vertices[i1 * stride + 2], 1.0f);
    csr_pos_init(pos2, vertices[i2 * stride + 0], vertices[i2 * stride + 1], vertices[i2 * stride + 2], 1.0f);

    csr_m4x4_mul_v4(v0_transformed, projection_view_model_matrix, pos0);
    csr_m4x4_mul_v4(v1_transformed, projection_view_model_matrix, pos1);
    csr_m4x4_mul_v4(v2_transformed, projection_view_model_matrix, pos2);

    /* Check if the triangle is behind the camera (clipping) */
    if (v0_transformed[3] <= 0.0f || v1_transformed[3] <= 0.0f || v2_transformed[3] <= 0.0f)
    {
      continue;
    }

    /* 2. Perspective Divide (Clip Space to NDC) */
    csr_v4_divf(v0_ndc, v0_transformed, v0_transformed[3]);
    csr_v4_divf(v1_ndc, v1_transformed, v1_transformed[3]);
    csr_v4_divf(v2_ndc, v2_transformed, v2_transformed[3]);

    /* 3. Viewport Transform (NDC to Screen Space) */
    csr_ndc_to_screen(model, v0_screen, v0_ndc);
    csr_ndc_to_screen(model, v1_screen, v1_ndc);
    csr_ndc_to_screen(model, v2_screen, v2_ndc);

    /* 4. CCW-Front Backface culling */
    {
      float ax = v1_screen[0] - v0_screen[0];
      float ay = v1_screen[1] - v0_screen[1];
      float bx = v2_screen[0] - v0_screen[0];
      float by = v2_screen[1] - v0_screen[1];
      float face = ax * by - ay * bx;

      if (face <= 0.0f)
      {
        continue;
      }
    }

    /* 5. Rasterization & Depth Testing */
    csr_draw_triangle(model, v0_screen, v1_screen, v2_screen, color0, color1, color2);
  }
}

#endif /* CSR_H */

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

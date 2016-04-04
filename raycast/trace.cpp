#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"

//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_intensity[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point p, Vector v, Vector surf_norm, Spheres *sph) {  

  Vector light_vec = get_vec(p, light1);
  float delta = vec_len(light_vec);
  normalize(&light_vec);

  float costheta = vec_dot(surf_norm, light_vec);
  Vector reflected_vec = vec_minus(vec_scale(surf_norm, 2*costheta), light_vec); 
  normalize(&reflected_vec);

  RGB_float color = {0, 0, 0};
  RGB_float diffuse = {0, 0, 0};
  RGB_float ambient = {0, 0 , 0};
  RGB_float specular = {0, 0, 0};

  // used for diffuse and specular color
  float attenuation = 1/(decay_a + decay_b * delta + decay_c * pow(delta, 2));

  ambient.r += sph->mat_ambient[0] * global_ambient[0];
  ambient.g += sph->mat_ambient[1] * global_ambient[1];
  ambient.b += sph->mat_ambient[2] * global_ambient[2];

  diffuse.r += sph->mat_diffuse[0] * light1_intensity[0] * vec_dot(surf_norm, light_vec);
  diffuse.g += sph->mat_diffuse[1] * light1_intensity[1] * vec_dot(surf_norm, light_vec);
  diffuse.b += sph->mat_diffuse[2] * light1_intensity[2] * vec_dot(surf_norm, light_vec);

  specular.r += sph->mat_specular[0] * light1_intensity[0] * pow(vec_dot(reflected_vec, v), sph->mat_shineness);
  specular.g += sph->mat_specular[1] * light1_intensity[1] * pow(vec_dot(reflected_vec, v), sph->mat_shineness);
  specular.b += sph->mat_specular[2] * light1_intensity[2] * pow(vec_dot(reflected_vec, v), sph->mat_shineness);

  color.r = global_ambient[0] * ambient.r + (light1_intensity[0] / attenuation) * (diffuse.r + specular.r);
  color.g = global_ambient[1] * ambient.g + (light1_intensity[1] / attenuation) * (diffuse.g + specular.g);        
  color.b = global_ambient[2] * ambient.b + (light1_intensity[2] / attenuation) * (diffuse.b + specular.b);                   

	return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point p, Vector v, int i) {

	RGB_float color = {0, 0, 0};

  Spheres *sph;
  Point hit;
  sph = intersect_scene(p, v, scene, &hit, 0);

  // if the closest sphere has been found
  if (sph != NULL)
  {
    Vector eye_vec = get_vec(hit, eye_pos);
    normalize(&eye_vec);

    Vector light_vec = get_vec(hit, p);
    normalize(&light_vec);

    Vector surf_norm = sphere_normal(hit, sph);
    normalize(&surf_norm);

    color = phong(hit, v, surf_norm, sph);
  }

	return color;
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
  int i, j;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  for (i=0; i<win_height; i++) {
    for (j=0; j<win_width; j++) {
      ray = get_vec(eye_pos, cur_pixel_pos);

      //
      // You need to change this!!!
      //
      // ret_color = recursive_ray_trace();
      // ret_color = background_clr; // just background for now

      // Parallel rays can be cast instead using below
      //
      // ray.x = ray.y = 0;
      // ray.z = -1.0;
      ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

// Checkboard for testing
RGB_float clr = {float(i/32), 0, float(j/32)};
ret_color = clr;

      frame[i][j][0] = GLfloat(ret_color.r);
      frame[i][j][1] = GLfloat(ret_color.g);
      frame[i][j][2] = GLfloat(ret_color.b);

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;
  }
}

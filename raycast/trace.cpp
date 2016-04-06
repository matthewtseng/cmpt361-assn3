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
extern int reflection_on;
extern int refraction_on;
extern int chessboard_on;
extern int stochastic_on;
extern int supersampling_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Ray-plane intersection and colour board
 * 
 * Used this website as reference for ray intersection with board:
 * https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
 *
 *********************************************************************/

float intersect_board(Point p, Vector v, Point *board_hit, Vector board_norm, Point *hit)
{

  // calculating distance from eye to plane
  Vector dist;
  dist.x = p.x - board_hit->x;
  dist.y = p.y - board_hit->y;
  dist.z = p.z - board_hit->z;
  float d = vec_len(dist);

  // calculating t
  float num = -(vec_dot(dist, board_norm), d);
  float denom = vec_dot(v, board_norm);
  float t = num / denom;

  if (denom == 0 && num != 0) return -1.0; // no intersection point

  if (t > 0) // intersection point
  {
    hit->x = p.x + t * v.x;
    hit->y = p.y + t * v.y;
    hit->z = p.z + t * v.z;
    return t;
  }

  return -1.0;
}

RGB_float color_board()
{
  RGB_float color;
  return color;
}

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point p, Vector v, Vector surf_norm, Spheres *sph) {  

  // calculating light vector
  Vector light_vec = get_vec(p, light1);
  float delta = vec_len(light_vec);
  normalize(&light_vec);

  // calculating reflected ray vector
  float costheta = vec_dot(surf_norm, light_vec);
  Vector reflected_vec = vec_minus(vec_scale(surf_norm, 2*costheta), light_vec); 
  normalize(&reflected_vec);

  float vr = vec_dot(v, reflected_vec);

  // place holders for each component of Phong illumination
  RGB_float color = {0, 0, 0};
  RGB_float diffuse = {0, 0, 0};
  RGB_float ambient = {0, 0, 0};
  RGB_float specular = {0, 0, 0};

  // used for diffuse and specular color
  float attenuation = 1 / (decay_a + decay_b * delta + decay_c * pow(delta, 2));

  // shadow multiplier
  float shadow = 1.0;

  // calculating ambient
  ambient.r += sph->mat_ambient[0] * global_ambient[0];
  ambient.g += sph->mat_ambient[1] * global_ambient[1];
  ambient.b += sph->mat_ambient[2] * global_ambient[2];

  if (costheta < 0)
  {
    costheta = 0;
  }

  // calculating diffuse
  diffuse.r += sph->mat_diffuse[0] * costheta;
  diffuse.g += sph->mat_diffuse[1] * costheta;
  diffuse.b += sph->mat_diffuse[2] * costheta;

  if (vr < 0)
  {
    vr = 0;
  }

  // calculating specular
  specular.r += sph->mat_specular[0] * pow(vr, sph->mat_shineness);
  specular.g += sph->mat_specular[1] * pow(vr, sph->mat_shineness);
  specular.b += sph->mat_specular[2] * pow(vr, sph->mat_shineness);

  if (shadow_on == 1) 
  {
    if (intersect_scene(p, light_vec, scene, NULL, 1) != NULL) shadow = 0.0; // if object is in shadow
  }

  // final color
  color.r += ambient.r + (light1_intensity[0] * shadow * attenuation) * (diffuse.r + specular.r);
  color.g += ambient.g + (light1_intensity[1] * shadow * attenuation) * (diffuse.g + specular.g);        
  color.b += ambient.b + (light1_intensity[2] * shadow * attenuation) * (diffuse.b + specular.b);   

	return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point p, Vector v, int step) {

	RGB_float color = background_clr;

  // place holders for refracted and reflected color to be added on 
  // to color at the end if options are turned on
  RGB_float reflected_color = {0, 0, 0};
  // RGB_float refracted_color = {0, 0, 0};

  Spheres *sph;
  Point hit;
  sph = intersect_scene(p, v, scene, &hit, 1);

  // if the closest sphere has been found
  if (sph != NULL)
  {
    // calculating light vector
    Vector light_vec = get_vec(p, light1);
    normalize(&light_vec);

    // calculating eye vector
    Vector eye_vec = get_vec(hit, eye_pos);
    normalize(&eye_vec);

    // calculating surface normal
    Vector surf_norm = sphere_normal(hit, sph);
    normalize(&surf_norm);

    color = phong(hit, eye_vec, surf_norm, sph);

    if (reflection_on == 1 && step < step_max)
    {
      // calculating reflected vector
      float costheta = vec_dot(surf_norm, light_vec);
      Vector reflected_vec = vec_minus(vec_scale(surf_norm, 2*costheta), light_vec);  
      normalize(&reflected_vec);

      reflected_color = recursive_ray_trace(hit, reflected_vec, step + 1);
      reflected_color = clr_scale(reflected_color, sph->reflectance);
      color = clr_add(color, reflected_color);
    }

    return color;
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
// RGB_float clr = {float(i/32), 0, float(j/32)};
// ret_color = clr;
      frame[i][j][0] = GLfloat(ret_color.r);
      frame[i][j][1] = GLfloat(ret_color.g);
      frame[i][j][2] = GLfloat(ret_color.b);

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;
  }
}

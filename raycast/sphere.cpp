#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 * 
 * Used these websites as reference: 
 * sci.tuomastonteri.fi/programming/sse/example3
 * siggraph.org/education/materials/HyperGraph/raytrace/rtinter1.htm
 **********************************************************************/
float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit) {

  float a, b, c, disc, t0, t1;

  a = u.x * u.x + u.y * u.y + u.z * u.z;
  b = 2.0f * (u.x * (o.x - sph->center.x) + u.y * (o.y - sph->center.x) + u.z * (o.z - sph->center.z));
  c = pow(sph->center.x, 2) + pow(sph->center.y, 2) + pow(sph->center.z, 2) +
      pow(o.x, 2) + pow(o.y, 2) + pow(o.z, 2) -
      2.0f * (o.x * sph->center.x + o.y * sph->center.y + o.z * sph->center.z) + pow(sph->radius, 2);
  disc = pow(b, 2) - 4.0f * a * c;

  if (disc >= 0) 
  {
    t0 = (-b + sqrt(disc)) / (2 * a); // for the negative value
    // t1 = (-b + sqrt(disc)) / (2 * a); // for the positive value, unused at the moment

    if (t0 > 0)
    {
      hit->x = o.x + t0 * u.x;
      hit->y = o.y + t0 * u.y;
      hit->z = o.z + t0 * u.z;

      Vector hit_vec = {hit->x - o.x, hit->y - o.y, hit->z - o.z};
      return vec_len(hit_vec);
    } 
    else
    {
      return -1.0;
    }
  }
  else
  { // no intersection point
    return -1.0;
  }

}

/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For example, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector u, Spheres *sph, Point *hit, int i) {
  Spheres *closest = NULL;

  float shortest_distance = FLT_MAX;
  float current_distance;

  while (sph != NULL) 
  {
    current_distance = intersect_sphere(o, u, sph, hit);
    if (current_distance == -1.0) return NULL;

    // checks for shortest distance and checks if current_distance returns a valid return value (i.e. not -1.0)
    if (shortest_distance > current_distance)
    {
      shortest_distance = current_distance;
      closest = sph;
    }

    sph = sph->next;
  }

	return closest;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, int sindex) {
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->next = NULL;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}

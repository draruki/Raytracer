#include "raytracing.h"
#include "ray.h"
#include "hitable.h"
#include "sphere.h"
#include "triangle.h"
#include "box.h"
#include "material.h"

#include "utils2.h"  // Used for OBJ-mesh loading
#include <stdlib.h>  // Needed for drand48()

namespace rt {

// Store scene (world) in a global variable for convenience
struct Scene {
    Sphere ground;
    std::vector<Sphere> spheres;
    std::vector<Box> boxes;
    std::vector<Triangle> mesh;
    Box mesh_bbox;
} g_scene;

bool hit_world(const Ray &r, float t_min, float t_max, HitRecord &rec)
{
    HitRecord temp_rec;
    bool hit_anything = false;
    float closest_so_far = t_max;

    if (g_scene.ground.hit(r, t_min, closest_so_far, temp_rec)) {
        hit_anything = true;
        closest_so_far = temp_rec.t;
        rec = temp_rec;
    }
    for (int i = 0; i < g_scene.spheres.size(); ++i) {
        if (g_scene.spheres[i].hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    for (int i = 0; i < g_scene.boxes.size(); ++i) {
        if (g_scene.boxes[i].hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
	if (g_scene.mesh_bbox.hit(r, t_min, closest_so_far, temp_rec))
	{
		for (int i = 0; i < g_scene.mesh.size(); ++i) 
		{
			if (g_scene.mesh[i].hit(r, t_min, closest_so_far, temp_rec)) 
			{
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}
	}
    return hit_anything;
}

glm::vec3 random_in_unit_sphere()
{
	random_double(1, 4);

	glm::vec3 p;
	do
	{
		p = 2.0f * glm::vec3(random_double(), random_double(), random_double()) - glm::vec3(1, 1, 1);
	} while (glm::length2(p) >= 1.0f);         
	return p;
}

glm::vec3 random_in_hemisphere(glm::vec3 normal)
{
	glm::vec3 in_unit_sphere = random_in_unit_sphere();
	if (glm::dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
		return in_unit_sphere;
	else
		return -in_unit_sphere;
}

// This function should be called recursively (inside the function) for
// bouncing rays when you compute the lighting for materials, like this
//
// if (hit_world(...)) {
//     ...
//     return color(rtx, r_bounce, max_bounces - 1);
// }	
//
// See Chapter 7 in the "Ray Tracing in a Weekend" book
glm::vec3 color(RTContext &rtx, const Ray &r, int max_bounces)
{
	if (max_bounces < 0)
	{
		return glm::vec3(0.0f);
	}

    HitRecord rec;
    if (hit_world(r, 0.001f, 9999.0f, rec)) {
        rec.normal = glm::normalize(rec.normal);  // Always normalise before use!
        if (rtx.show_normals) {
            return rec.normal * 0.5f + 0.5f;
        }

		Ray scattered;
		glm::vec3 attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * color(rtx, scattered, max_bounces - 1);
		return glm::vec3(0, 0, 0);
    }

    // If no hit, return sky color
    glm::vec3 unit_direction = glm::normalize(r.direction());
    float t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * rtx.ground_color + t * rtx.sky_color;
}

// MODIFY THIS FUNCTION!
void setupScene(RTContext &rtx, const char *filename)
{
    g_scene.ground = Sphere(glm::vec3(0.0f, -1000.5f, 0.0f), 1000.0f, std::make_shared<lambertian>(glm::vec3(0.2f, 0.9f, 0.2f)));
    
	g_scene.spheres = 
	{
		Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, std::make_shared<lambertian>(glm::vec3(0.1f, 0.8f, 0.4f))),
		Sphere(glm::vec3(1.0f, 0.0f, 0.0f), 0.5f, std::make_shared<metal>(glm::vec3(rtx.albedo.r, rtx.albedo.g, rtx.albedo.b))),
		Sphere(glm::vec3(-1.0f, 0.0f, 0.0f), 0.5f, std::make_shared<dielectric>(1.5f * rtx.refractive_index)),
			
		Sphere(glm::vec3(-1.0f,-0.3f, 0.9f), 0.1f, std::make_shared<dielectric>(1.7f * rtx.refractive_index)),
		Sphere(glm::vec3(-0.5f,-0.3f, 0.9f), 0.1f, std::make_shared<dielectric>(0.8f * rtx.refractive_index)),
		Sphere(glm::vec3(0.2f, -0.3f, 0.9f), 0.1f, std::make_shared<dielectric>(1.1f * rtx.refractive_index)),
		Sphere(glm::vec3(0.6f, -0.3f, 0.9f), 0.1f, std::make_shared<metal>(glm::vec3(rtx.albedo.r, rtx.albedo.g, rtx.albedo.b))),

		Sphere(glm::vec3(-0.8f,-0.3f, 1.3f), 0.1f, std::make_shared<dielectric>(1.7f * rtx.refractive_index)),
		Sphere(glm::vec3(-0.3f,-0.3f, 1.3f), 0.1f, std::make_shared<dielectric>(0.8f * rtx.refractive_index)),
		Sphere(glm::vec3(0.0f, -0.3f, 1.3f), 0.1f, std::make_shared<dielectric>(1.1f * rtx.refractive_index)),
		Sphere(glm::vec3(0.4f, -0.3f, 1.3f), 0.1f, std::make_shared<metal>(glm::vec3(rtx.albedo.r, rtx.albedo.g, rtx.albedo.b))),
    };

    //g_scene.boxes = {
    //    Box(glm::vec3(0.0f, -0.25f, -1.0f), glm::vec3(0.25f), std::make_shared<dielectric>(1.1f)),
    //    Box(glm::vec3(1.0f, -0.25f, -1.0f), glm::vec3(0.25f), std::make_shared<dielectric>(-1.1f)),
    //    Box(glm::vec3(-1.0f, -0.25f, -1.0f), glm::vec3(0.25f), std::make_shared<dielectric>(0.5f)),
    //};

	/*
    OBJMesh mesh;
    objMeshLoad(mesh, filename);
    g_scene.mesh.clear();
	float max_radius = 0.f;
    for (int i = 0; i < mesh.indices.size(); i += 3) {
        int i0 = mesh.indices[i + 0];
        int i1 = mesh.indices[i + 1];
        int i2 = mesh.indices[i + 2];
        glm::vec3 v0 = mesh.vertices[i0] + glm::vec3(0.0f, 0.135f, 0.0f);
        glm::vec3 v1 = mesh.vertices[i1] + glm::vec3(0.0f, 0.135f, 0.0f);
        glm::vec3 v2 = mesh.vertices[i2] + glm::vec3(0.0f, 0.135f, 0.0f);
		max_radius = glm::max(max_radius, v0.x);
		max_radius = glm::max(max_radius, v0.y);
		max_radius = glm::max(max_radius, v0.z);
		max_radius = glm::max(max_radius, v1.x);
		max_radius = glm::max(max_radius, v1.y);
		max_radius = glm::max(max_radius, v1.z);
		max_radius = glm::max(max_radius, v2.x);
		max_radius = glm::max(max_radius, v2.y);
		max_radius = glm::max(max_radius, v2.z);

        g_scene.mesh.push_back(Triangle(v0, v1, v2, std::make_shared<dielectric>(rtx.refractive_index)));
    }

	g_scene.mesh_bbox.center = glm::vec3(0.0f);
	g_scene.mesh_bbox.radius = glm::vec3(max_radius, max_radius, max_radius);
	*/
}

// MODIFY THIS FUNCTION!
void updateLine(RTContext &rtx, int y)
{
    int nx = rtx.width;
    int ny = rtx.height;
    float aspect = float(nx) / float(ny);
    glm::vec3 lower_left_corner(-1.0f * aspect, -1.0f, -1.0f);
    glm::vec3 horizontal(2.0f * aspect, 0.0f, 0.0f);
    glm::vec3 vertical(0.0f, 2.0f, 0.0f);
    glm::vec3 origin(0.0f, 0.0f, 0.0f);
    glm::mat4 world_from_view = glm::inverse(rtx.view);

    // You can try to parallelise this loop by uncommenting this line:
    #pragma omp parallel for schedule(dynamic)
    for (int x = 0; x < nx; ++x) {
		glm::vec3 c(0, 0, 0);
		for (int s = 0; s < rtx.per_sample_jitter; ++s)
		{
			float u = (float(x) + 0.5f + (float(random_double()) - 0.5f)) / float(nx);
			float v = (float(y) + 0.5f + (float(random_double()) - 0.5f)) / float(ny);
			Ray r(origin, lower_left_corner + u * horizontal + v * vertical);
			r.A = glm::vec3(world_from_view * glm::vec4(r.A, 1.0f));
			r.B = glm::vec3(world_from_view * glm::vec4(r.B, 0.0f));
			c += color(rtx, r, rtx.max_bounces);
		}

        if (rtx.current_frame <= 0) {
            // Here we make the first frame blend with the old image,
            // to smoothen the transition when resetting the accumulation
            glm::vec4 old = rtx.image[y * nx + x];
            rtx.image[y * nx + x] = glm::clamp(old / glm::max(1.0f, old.a), 0.0f, 1.0f);
        }
		c /= rtx.per_sample_jitter;
        rtx.image[y * nx + x] += glm::vec4(c, 1.0f);
    }
}

void updateImage(RTContext &rtx)
{
    if (rtx.freeze) return;  // Skip update
    rtx.image.resize(rtx.width * rtx.height);  // Just in case...

    updateLine(rtx, rtx.current_line % rtx.height);

    if (rtx.current_frame < rtx.max_frames) {
        rtx.current_line += 1;
        if (rtx.current_line >= rtx.height) {
            rtx.current_frame += 1;
            rtx.current_line = rtx.current_line % rtx.height;
        }
    }
}

void resetImage(RTContext &rtx)
{
    rtx.image.clear();
    rtx.image.resize(rtx.width * rtx.height);
    rtx.current_frame = 0;
    rtx.current_line = 0;
    rtx.freeze = false;
}

void resetAccumulation(RTContext &rtx)
{
    rtx.current_frame = -1;
}

} // namespace rt



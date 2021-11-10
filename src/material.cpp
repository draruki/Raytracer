
/*
#pragma once

#include "material.h"

namespace rt {
	bool material::scatter(const Ray& r, HitRecord& rec, glm::vec3 attenuation, Ray& scattered) {}

	lambertian::lambertian(const glm::vec3& a) : albedo(a) {}
	bool lambertian::scatter(const Ray& r_in, HitRecord& rec, glm::vec3& attenuation, Ray& scattered) 
	{
		glm::vec3 scatter_direction = rec.normal + random_unit_vector();
		scattered = Ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	}

	metal::metal(const glm::vec3& a) : albedo(a) {}
	bool metal::scatter(const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) 
	{
		glm::vec3 reflected = glm::reflect(glm::normalize(r_in.direction()), rec.normal);
		scattered = Ray(rec.p, reflected);
		attenuation = albedo;
		return (glm::dot(scattered.direction(), rec.normal) > 0);
	}
}

*/
#pragma once

#include "raytracing.h"
#include "ray.h"
#include "hitable.h"
#include "sphere.h"
#include "triangle.h"
#include "box.h"
#include <random>

class Ray;
class HitRecord;

inline double random_double() {
	// Returns a random real in [0,1).
	return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
	// Returns a random real in [min,max).
	return min + (max - min) * random_double();
}

glm::vec3 random_unit_vector()
{
	auto a = random_double(0, 2.0 * 3.14159265359);
	auto z = random_double(-1, 1);
	auto r = sqrt(1 - z*z);
	return glm::vec3(r*cos(a), r*sin(a), z);
}

double schlick(double cosine, double ref_idx) {
	auto r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

namespace rt {
	class material {
	public:
		virtual bool scatter(
			const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered
		) const = 0;
	};

	class lambertian : public material {
	public:
		lambertian(const glm::vec3& a) : albedo(a) {}

		virtual bool scatter(
			const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered
		) const {
			glm::vec3 scatter_direction = rec.normal + random_unit_vector();
			scattered = Ray(rec.p, scatter_direction);
			attenuation = albedo;
			return true;
		}

	public:
		glm::vec3 albedo;
	};

	class metal : public material {
	public:
		metal(const glm::vec3& a) : albedo(a) {}

		virtual bool scatter(
			const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered
		) const {
			glm::vec3 reflected = glm::reflect(glm::normalize(r_in.direction()), rec.normal);
			scattered = Ray(rec.p, reflected);
			attenuation = albedo;
			return (glm::dot(scattered.direction(), rec.normal) > 0);
		}

	public:
		glm::vec3 albedo;
	};

	bool refract(const glm::vec3& v, const glm::vec3& n, float ni_over_nt, glm::vec3& refracted) {
		glm::vec3 uv = glm::normalize(v);
		float dt = glm::dot(uv, n);
		float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1 - dt * dt);
		if (discriminant > 0) {
			refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
			return true;
		}
		else
			return false;
	}

	float schlick(float cosine, float ref_idx)
	{
		float r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}

	class dielectric : public material {
	public:
		dielectric(float ri) : ref_idx(ri) {}
		virtual bool scatter(const Ray& r_in, const HitRecord& rec, glm::vec3& attenuation,
			Ray& scattered) const {
			glm::vec3 outward_normal;
			glm::vec3 reflected = glm::reflect(r_in.direction(), rec.normal);
			float ni_over_nt;
			attenuation = glm::vec3(1.0, 1.0, 1.0);
			glm::vec3 refracted;
			float reflect_prob;
			float cosine;
			if (glm::dot(r_in.direction(), rec.normal) > 0) {
				outward_normal = -rec.normal;
				ni_over_nt = ref_idx;
				cosine = ref_idx * glm::dot(r_in.direction(), rec.normal) / r_in.direction().length();
			}
			else
			{
				outward_normal = rec.normal;
				ni_over_nt = 1.0 / ref_idx;
				cosine = -glm::dot(r_in.direction(), rec.normal) / r_in.direction().length();
			}
			if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted)) {
				reflect_prob = schlick(cosine, ref_idx);
			}
			else
			{
				scattered = Ray(rec.p, reflected);
				reflect_prob = 1.0;
			}
			if (random_double() < reflect_prob) {
				scattered = Ray(rec.p, reflected);
			}
			else {
				scattered = Ray(rec.p, refracted);
			}
			return true;
		}

		float ref_idx;
	};

}

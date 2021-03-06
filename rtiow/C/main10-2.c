#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <float.h>

#define RANDOM_IMPL
#include "random.h"
#include "vec3.h"
#include "ray.h"

struct hit_record_s;
struct material_s;

typedef bool (*scatter_t)(struct material_s *p, const ray *r_in, const struct hit_record_s *rec, vec3 attenuation, ray *scattered);

typedef struct hit_record_s {
	float t;
	vec3 p;
	vec3 normal;
	struct material_s *mat_ptr;
} hit_record;

typedef struct {
	vec3 albedo;
} lambertian_t;

typedef struct {
	vec3 albedo;
	float fuzz;
} metal_t;

typedef struct {
	float ref_idx;
} dielectric_t;

typedef struct material_s {
	scatter_t scatter;
	union {
		void *null;
		lambertian_t lambertian;
		metal_t metal;
		dielectric_t dielectric;
	} u;
} material_t;

typedef struct {
	vec3 center;
	float radius;
	material_t mat;
} sphere_t;

#define MLAMBERTIAN(ax, ay, az) ((material_t){lambertian_scatter, .u.lambertian=(lambertian_t){{ax, ay, az}}})
#define MMETAL(ax, ay, az, f) ((material_t){metal_scatter, .u.metal=(metal_t){{ax, ay, az}, f}})
#define MDIELECTRIC(r) ((material_t){dielectric_scatter, .u.dielectric=(dielectric_t){r}})

#define HSPHERE(cx, cy, cz, r, m) ((hittable_t){sphere_hit, .u.sphere=(sphere_t){{cx, cy, cz}, r, m}})
#define HSTART ((hittable_t){list_hit, .u.null=0})
#define HEND ((hittable_t){0, .u.null=0})

struct hittable_s;
typedef bool (*hit_t)(struct hittable_s *p, const ray *r, float t_min, float t_max, hit_record *rec);

typedef struct hittable_s {
	hit_t hit;
	union {
		void *null;
		sphere_t sphere;
	} u;
} hittable_t;

bool list_hit(hittable_t *p, const ray *r, float t_min, float t_max, hit_record *rec) {
	hit_record temp_rec;
	bool hit_anything = false;
	float closest_so_far = t_max;
	while (1) {
		p++;
		if (!p->hit) break;
		if (p->hit(p, r, t_min, closest_so_far, &temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			*rec = temp_rec;
		}
	}
	return hit_anything;
}

bool sphere_hit(hittable_t *p, const ray *r, float t_min, float t_max, hit_record *rec) {
	sphere_t *s = &p->u.sphere;
	vec3 oc;
	vsub(oc, r->origin, s->center);
	float a = vdot(r->direction, r->direction);
	float b = vdot(oc, r->direction);
	float c = vdot(oc, oc) - s->radius*s->radius;
	float discriminant = b*b - a*c;
	if (discriminant > 0) {
		float temp = (-b - sqrtf(discriminant))/a;
		if (temp < t_max && temp > t_min) {
			rec->t = temp;
			point_at_parameter(rec->p, r, rec->t);
			vsub(rec->normal, rec->p, s->center);
			vdiv(rec->normal, rec->normal, s->radius);
			rec->mat_ptr = &s->mat;
			return true;
		}
		temp = (-b + sqrtf(discriminant))/a;
		if (temp < t_max && temp > t_min) {
			rec->t = temp;
			point_at_parameter(rec->p, r, rec->t);
			vsub(rec->normal, rec->p, s->center);
			vdiv(rec->normal, rec->normal, s->radius);
			rec->mat_ptr = &s->mat;
			return true;
		}
	}
	return false;
}

bool lambertian_scatter(struct material_s *p, const ray *r_in, const hit_record *rec, vec3 attenuation, ray *scattered) {
	lambertian_t *l = &p->u.lambertian;
	vec3 target;
	random_in_unit_sphere(target);
	vadd(target, rec->normal, target);
	rmake(scattered, rec->p, target);
	vcopy(attenuation, l->albedo);
	return true;
}

void reflect(vec3 l, const vec3 v, const vec3 n) {
	vmul(l, 2.f * vdot(v, n), n);
	vsub(l, v, l);
}

bool metal_scatter(struct material_s *p, const ray *r_in, const hit_record *rec,
		vec3 attenuation, ray *scattered) {
	metal_t *m = &p->u.metal;
	vec3 uvector, reflected, fuzz;
	unit_vector(uvector, r_in->direction);
	reflect(reflected, uvector, rec->normal);
	random_in_unit_sphere(fuzz);
	vmul(fuzz, m->fuzz, fuzz);
	vadd(reflected, reflected, fuzz);
	rmake(scattered, rec->p, reflected);
	vcopy(attenuation, m->albedo);
	return vdot(scattered->direction, rec->normal) > 0;
}

bool refract(const vec3 v, const vec3 n, float ni_over_nt, vec3 refracted) {
	vec3 uv;
	unit_vector(uv, v);
	float dt = vdot(uv, n);
	float discriminant = 1.0f - ni_over_nt*ni_over_nt*(1.-dt*dt);
	if (discriminant > 0) {
		vec3 v1, v2;
		vmul(v1, dt, n);
		vsub(v1, uv, v1);
		vmul(v1, ni_over_nt, v1);
		vmul(v2, sqrtf(discriminant), n);
		vsub(refracted, v1, v2);
		return true;
	}
	else
		return false;
}

bool dielectric_scatter(struct material_s *p, const ray *r_in,
		const hit_record *rec, vec3 attenuation, ray *scattered) {
	dielectric_t *d = &p->u.dielectric;
	vec3 outward_normal;
	vec3 reflected;
	reflect(reflected, r_in->direction, rec->normal);
	float ni_over_nt;
	vcopy(attenuation, VEC3(1.0, 1.0, 1.0));
	vec3 refracted;

	if (vdot(r_in->direction, rec->normal) > 0) {
		vmul(outward_normal, -1, rec->normal);
		ni_over_nt = d->ref_idx;
	} else {
		vcopy(outward_normal, rec->normal);
		ni_over_nt = 1.0f / d->ref_idx;
	}

	if (refract(r_in->direction, outward_normal, ni_over_nt, refracted)) {
		rmake(scattered, rec->p, refracted);
	} else {
		rmake(scattered, rec->p, reflected);
		return false;
	}

	return true;
}

void color(vec3 col, const ray *r, hittable_t *world, int depth) {
	hit_record rec;
	// remove acne by starting at 0.001
	if (world->hit(world, r, 0.001, FLT_MAX, &rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(rec.mat_ptr, r, &rec, attenuation, &scattered)) {
			vec3 scat_col;
			color(scat_col, &scattered, world, depth + 1);
			vmulv(col, attenuation, scat_col);
		} else {
			vcopy(col, VEC3(0, 0, 0));
		}
	} else {
		vec3 unit_direction;
		unit_vector(unit_direction, r->direction);
		float t = 0.5*(unit_direction[1] + 1.0);
		vec3 col0 = {1.0, 1.0, 1.0};
		vec3 col1 = {0.5, 0.7, 1.0};
		vmul(col0, 1.0-t, col0);
		vmul(col1, t, col1);
		vadd(col, col0, col1);
	}
}

typedef struct {
	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
} camera;

void get_ray(camera *cam, ray *r, float u, float v) {
	vec3 direction, direction0, direction1;
	vmul(direction0, v, cam->vertical);
	vmul(direction1, u, cam->horizontal);
	vadd(direction, direction0, direction1);
	vadd(direction, direction, cam->lower_left_corner);
	vsub(direction, direction, cam->origin);
	rmake(r, cam->origin, direction);
}

int main() {
	pcg_srand(0);
	int nx = 200;
	int ny = 100;
	int ns = 100;
	printf("P3\n"); printf("%d %d\n", nx, ny); printf("255\n");
	hittable_t world[] = {
		HSTART,
		HSPHERE(0, 0, -1, 0.5, MLAMBERTIAN(0.1, 0.2, 0.5)),
		HSPHERE(0, -100.5, -1, 100, MLAMBERTIAN(0.8, 0.8, 0)),
		HSPHERE(1, 0, -1, 0.5, MMETAL(0.8, 0.6, 0.2, 0.0)),
		HSPHERE(-1, 0, -1, 0.5, MDIELECTRIC(1.5)),
		HEND
	};
	camera cam = {
		.lower_left_corner = {-2.0, -1.0, -1.0},
		.horizontal = {4.0, 0.0, 0.0},
		.vertical = {0.0, 2.0, 0.0},
		.origin = {0.0, 0.0, 0.0}
	};
	for (int j = ny-1; j >= 0; j--) {
		for (int i = 0; i < nx; i++) {
			vec3 col = {0, 0, 0};
			for (int s = 0; s < ns; s++) {
				float u = ((float)i + random_f()) / (float)nx;
				float v = ((float)j + random_f()) / (float)ny;
				ray r;
				get_ray(&cam, &r, u, v);
				vec3 col0;
				color(col0, &r, world, 0);
				vadd(col, col, col0);
			}
			vdiv(col, col, (float)ns);
			// Gamma 2 correction (square root)
			col[0] = sqrtf(col[0]);
			col[1] = sqrtf(col[1]);
			col[2] = sqrtf(col[2]);

			int ir = (int)(255.99f*col[0]);
			int ig = (int)(255.99f*col[1]);
			int ib = (int)(255.99f*col[2]);
			printf("%d %d %d\n", ir, ig, ib);
		}
	}
	return 0;
}

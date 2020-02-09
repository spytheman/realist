#!/bin/env -S v run

module main

import vec
import ray
import math

enum HType {
	sphere
}
struct HSphere {
	center vec.Vec3
	radius f32
}
union HData {
	sphere HSphere
}

struct Hittable {
	htype HType
	data HData
}

struct HitRec {
mut:
	t f32			// hit time
	p vec.Vec3		// hit point coords
	normal vec.Vec3		// normal at hit point
}

fn (s HSphere) hit(r ray.Ray, t_min f32, t_max f32, rec mut HitRec) bool {
	oc := r.origin() - s.center
	a := r.direction().dot(r.direction())
	b := oc.dot(r.direction())
	c := oc.dot(oc) - s.radius * s.radius
	discriminant := b * b - a * c
	if discriminant > 0 {
		mut temp := (-b - math.sqrtf(discriminant)) / a
		if temp < t_max && temp > t_min {
			rec.t = temp
			rec.p = r.point_at_parameter(rec.t)
			rec.normal = vec.div(rec.p - s.center, s.radius)
			return true
		}
		temp = (-b + math.sqrtf(discriminant)) / a
		if temp < t_max && temp > t_min {
			rec.t = temp
			rec.p = r.point_at_parameter(rec.t)
			rec.normal = vec.div(rec.p - s.center, s.radius)
			return true
		}
	}
	return false
}

fn (h Hittable) hit(r ray.Ray, t_min f32, t_max f32, rec mut HitRec) bool {
	if h.htype == .sphere {
		return h.data.sphere.hit(r, t_min, t_max, mut rec)
	}
	return false
}

fn (hh []Hittable) hit(r ray.Ray, t_min f32, t_max f32, rec mut HitRec) bool {
	mut temp_rec := HitRec{}
	mut hit_anything := false
	mut closest_so_far := t_max
	for h in hh {
		if h.hit(r, t_min, closest_so_far, mut temp_rec) {
			hit_anything = true
			closest_so_far = temp_rec.t
			*rec = temp_rec
		}
	}
	return hit_anything
}

fn color(r ray.Ray) vec.Vec3 {
	mut rec := HitRec{}
	hittables := [
		Hittable{
htype:.sphere
data:HData{sphere: HSphere{center: vec.Vec3{0, 0, -1}, radius: .5}}
		},
		Hittable{
htype:.sphere
data: HData{sphere: HSphere{center: vec.Vec3{0, -100.5, -1}, radius: 100}}
		}
	]
	if hittables.hit(r, 0, 99999, mut rec) {
		return vec.mult(0.5, rec.normal + vec.Vec3{1., 1., 1.})
	} else {
		unit_direction := r.direction().unit_vector()
		t := .5 * (unit_direction.y + 1.)
		return vec.mult(1. - t, vec.Vec3{1., 1., 1.}) + vec.mult(t, vec.Vec3{.5, .7, 1})
	}
}

fn main() {
	nx := 200
	ny := 100
	println('P3')
	println('$nx $ny')
	println(255)
	lower_left_corner := vec.Vec3 {-2., -1., -1.}
	horizontal := vec.Vec3 {4., 0., 0.}
	vertical := vec.Vec3 {0., 2., 0.}
	origin := vec.Vec3 {0., 0., 0.}
	for j := ny-1; j >= 0; j-- {
		for i := 0; i < nx; i++ {
			u := f32(i) / f32(nx)
			v := f32(j) / f32(ny)
			r := ray.Ray {
				origin,
				lower_left_corner
					+ vec.mult(u, horizontal)
					+ vec.mult(v, vertical)
			}
			col := color(r)
			ir := int(255.99 * col.x)
			ig := int(255.99 * col.y)
			ib := int(255.99 * col.z)
			println('$ir $ig $ib')
		}
	}
}

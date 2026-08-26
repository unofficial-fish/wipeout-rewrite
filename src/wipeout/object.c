#include "../types.h"
#include "../mem.h"
#include "../render.h"
#include "../utils.h"
#include "../platform.h"

#include "object.h"

Object *objects_load(char *name, texture_list_t tl) {
	uint32_t length = 0;
	uint8_t *bytes = platform_load_asset(name, &length);
	if (!bytes) {
		die("Failed to load file %s\n", name);
	}
#ifdef VERBOSE_PRINTING
	printf("load: %s\n", name);
#endif
	Object *objectList = mem_mark();
	Object *prevObject = NULL;
	uint32_t p = 0;

	while (p < length) {
		Object *object = mem_bump(sizeof(Object));
		if (prevObject) {
			prevObject->next = object;
		}
		prevObject = object;

		for (int i = 0; i < 16; i++) {
			object->name[i] = get_i8(bytes, &p);
		}

		object->mat = mat4_identity();
		object->vertices_len = get_i16(bytes, &p); p += 2;
		object->vertices = NULL; get_i32(bytes, &p);
		object->normals_len = get_i16(bytes, &p); p += 2;
		object->normals = NULL; get_i32(bytes, &p);
		object->primitives_len = get_i16(bytes, &p); p += 2;
		object->primitives = NULL; get_i32(bytes, &p);
		get_i32(bytes, &p);
		get_i32(bytes, &p);
		get_i32(bytes, &p); // Skeleton ref
		object->extent = get_i32(bytes, &p);
		object->flags = get_i16(bytes, &p); p += 2;
		object->next = NULL; get_i32(bytes, &p);

		p += 3 * 3 * 2; // relative rot matrix
		p += 2; // padding

		object->origin.x = get_i32(bytes, &p);
		object->origin.y = get_i32(bytes, &p);
		object->origin.z = get_i32(bytes, &p);

		p += 3 * 3 * 2; // absolute rot matrix
		p += 2; // padding
		p += 3 * 4; // absolute translation matrix
		p += 2; // skeleton update flag
		p += 2; // padding
		p += 4; // skeleton super
		p += 4; // skeleton sub
		p += 4; // skeleton next

		object->radius = 0;
		object->vertices = mem_bump(object->vertices_len * sizeof(vec3_t));
		for (int i = 0; i < object->vertices_len; i++) {
			object->vertices[i].x = get_i16(bytes, &p);
			object->vertices[i].y = get_i16(bytes, &p);
			object->vertices[i].z = get_i16(bytes, &p);
			p += 2; // padding

			object->radius = max(vec3_len_sq(object->vertices[i]), object->radius);
		}
		object->radius = sqrt(object->radius);


		object->normals = mem_bump(object->normals_len * sizeof(vec3_t));
		for (int i = 0; i < object->normals_len; i++) {
			object->normals[i].x = get_i16(bytes, &p);
			object->normals[i].y = get_i16(bytes, &p);
			object->normals[i].z = get_i16(bytes, &p);
			p += 2; // padding
		}

		object->primitives = mem_bump(sizeof(primitive_t) * object->primitives_len);
		for (int i = 0; i < object->primitives_len; i++) {
			primitive_t *prm = &object->primitives[i];

			prm->type = get_i16(bytes, &p);
			prm->flag = get_i16(bytes, &p);

			switch (prm->type) {
			case PRM_TYPE_F3:
				prm->psx.f3.coords[0] = get_i16(bytes, &p);
				prm->psx.f3.coords[1] = get_i16(bytes, &p);
				prm->psx.f3.coords[2] = get_i16(bytes, &p);
				p += 2; // padding
				prm->psx.f3.color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_F4:
				prm->psx.f4.coords[0] = get_i16(bytes, &p);
				prm->psx.f4.coords[1] = get_i16(bytes, &p);
				prm->psx.f4.coords[2] = get_i16(bytes, &p);
				prm->psx.f4.coords[3] = get_i16(bytes, &p);
				prm->psx.f4.color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_FT3:
				prm->psx.ft3.coords[0] = get_i16(bytes, &p);
				prm->psx.ft3.coords[1] = get_i16(bytes, &p);
				prm->psx.ft3.coords[2] = get_i16(bytes, &p);

				prm->psx.ft3.texture = texture_from_list(tl, get_i16(bytes, &p));
				p += 2; // padding
				p += 2; // padding
				prm->psx.ft3.u0 = get_i8(bytes, &p);
				prm->psx.ft3.v0 = get_i8(bytes, &p);
				prm->psx.ft3.u1 = get_i8(bytes, &p);
				prm->psx.ft3.v1 = get_i8(bytes, &p);
				prm->psx.ft3.u2 = get_i8(bytes, &p);
				prm->psx.ft3.v2 = get_i8(bytes, &p);

				p += 2; // padding
				prm->psx.ft3.color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_FT4:
				prm->psx.ft4.coords[0] = get_i16(bytes, &p);
				prm->psx.ft4.coords[1] = get_i16(bytes, &p);
				prm->psx.ft4.coords[2] = get_i16(bytes, &p);
				prm->psx.ft4.coords[3] = get_i16(bytes, &p);

				prm->psx.ft4.texture = texture_from_list(tl, get_i16(bytes, &p));
				p += 2; // padding
				p += 2; // padding
				prm->psx.ft4.u0 = get_i8(bytes, &p);
				prm->psx.ft4.v0 = get_i8(bytes, &p);
				prm->psx.ft4.u1 = get_i8(bytes, &p);
				prm->psx.ft4.v1 = get_i8(bytes, &p);
				prm->psx.ft4.u2 = get_i8(bytes, &p);
				prm->psx.ft4.v2 = get_i8(bytes, &p);
				prm->psx.ft4.u3 = get_i8(bytes, &p);
				prm->psx.ft4.v3 = get_i8(bytes, &p);
				p += 2; // padding
				prm->psx.ft4.color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_G3:
				prm->psx.g3.coords[0] = get_i16(bytes, &p);
				prm->psx.g3.coords[1] = get_i16(bytes, &p);
				prm->psx.g3.coords[2] = get_i16(bytes, &p);
				p += 2; // padding
				prm->psx.g3.color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.g3.color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.g3.color[2] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_G4:
				prm->psx.g4.coords[0] = get_i16(bytes, &p);
				prm->psx.g4.coords[1] = get_i16(bytes, &p);
				prm->psx.g4.coords[2] = get_i16(bytes, &p);
				prm->psx.g4.coords[3] = get_i16(bytes, &p);
				prm->psx.g4.color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.g4.color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.g4.color[2] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.g4.color[3] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_GT3:
				prm->psx.gt3.coords[0] = get_i16(bytes, &p);
				prm->psx.gt3.coords[1] = get_i16(bytes, &p);
				prm->psx.gt3.coords[2] = get_i16(bytes, &p);

				prm->psx.gt3.texture = texture_from_list(tl, get_i16(bytes, &p));
				p += 2; // padding
				p += 2; // padding
				prm->psx.gt3.u0 = get_i8(bytes, &p);
				prm->psx.gt3.v0 = get_i8(bytes, &p);
				prm->psx.gt3.u1 = get_i8(bytes, &p);
				prm->psx.gt3.v1 = get_i8(bytes, &p);
				prm->psx.gt3.u2 = get_i8(bytes, &p);
				prm->psx.gt3.v2 = get_i8(bytes, &p);
				p += 2; // padding
				prm->psx.gt3.color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.gt3.color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.gt3.color[2] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_GT4:
				prm->psx.gt4.coords[0] = get_i16(bytes, &p);
				prm->psx.gt4.coords[1] = get_i16(bytes, &p);
				prm->psx.gt4.coords[2] = get_i16(bytes, &p);
				prm->psx.gt4.coords[3] = get_i16(bytes, &p);

				prm->psx.gt4.texture = texture_from_list(tl, get_i16(bytes, &p));
				p += 2; // padding
				p += 2; // padding
				prm->psx.gt4.u0 = get_i8(bytes, &p);
				prm->psx.gt4.v0 = get_i8(bytes, &p);
				prm->psx.gt4.u1 = get_i8(bytes, &p);
				prm->psx.gt4.v1 = get_i8(bytes, &p);
				prm->psx.gt4.u2 = get_i8(bytes, &p);
				prm->psx.gt4.v2 = get_i8(bytes, &p);
				prm->psx.gt4.u3 = get_i8(bytes, &p);
				prm->psx.gt4.v3 = get_i8(bytes, &p);
				p += 2; // padding
				prm->psx.gt4.color[0] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.gt4.color[1] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.gt4.color[2] = rgba_from_u32(get_u32(bytes, &p));
				prm->psx.gt4.color[3] = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_TSPR:
			case PRM_TYPE_BSPR:
				prm->psx.spr.coord = get_i16(bytes, &p);
				prm->psx.spr.width = get_i16(bytes, &p);
				prm->psx.spr.height = get_i16(bytes, &p);
				prm->psx.spr.texture = texture_from_list(tl, get_i16(bytes, &p));
				prm->psx.spr.color = rgba_from_u32(get_u32(bytes, &p));
				break;

			case PRM_TYPE_SPLINE:
				prm->psx.spline.control1.x = get_i32(bytes, &p);
				prm->psx.spline.control1.y = get_i32(bytes, &p);
				prm->psx.spline.control1.z = get_i32(bytes, &p);
				p += 4; // padding
				prm->psx.spline.position.x = get_i32(bytes, &p);
				prm->psx.spline.position.y = get_i32(bytes, &p);
				prm->psx.spline.position.z = get_i32(bytes, &p);
				p += 4; // padding
				prm->psx.spline.control2.x = get_i32(bytes, &p);
				prm->psx.spline.control2.y = get_i32(bytes, &p);
				prm->psx.spline.control2.z = get_i32(bytes, &p);
				p += 4; // padding
				prm->psx.spline.color = rgba_from_u32(get_u32(bytes, &p));
				break;

			default:
				die("Unsupported primitive type %x\n", prm->type);
			} // switch
		} // each prim
	} // each object

	mem_temp_free(bytes);
	return objectList;
}


void object_draw(Object *object, mat4_t *mat) {
	vec3_t *vertex = object->vertices;

	render_set_model_mat(mat);

	// TODO: check for PRM_SINGLE_SIDED

	for (int i = 0; i < object->primitives_len; i++) {
		primitive_t *prm = &object->primitives[i];

		int coord0;
		int coord1;
		int coord2;
		int coord3;
		switch (prm->type) {
		case PRM_TYPE_GT3:
			coord0 = prm->psx.gt3.coords[0];
			coord1 = prm->psx.gt3.coords[1];
			coord2 = prm->psx.gt3.coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.uv = {prm->psx.gt3.u2, prm->psx.gt3.v2},
						.color = prm->psx.gt3.color[2]
					},
					{
						.pos = vertex[coord1],
						.uv = {prm->psx.gt3.u1, prm->psx.gt3.v1},
						.color = prm->psx.gt3.color[1]
					},
					{
						.pos = vertex[coord0],
						.uv = {prm->psx.gt3.u0, prm->psx.gt3.v0},
						.color = prm->psx.gt3.color[0]
					},
				}
			}, prm->psx.gt3.texture);

			break;

		case PRM_TYPE_GT4:
			coord0 = prm->psx.gt4.coords[0];
			coord1 = prm->psx.gt4.coords[1];
			coord2 = prm->psx.gt4.coords[2];
			coord3 = prm->psx.gt4.coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.uv = {prm->psx.gt4.u2, prm->psx.gt4.v2},
						.color = prm->psx.gt4.color[2]
					},
					{
						.pos = vertex[coord1],
						.uv = {prm->psx.gt4.u1, prm->psx.gt4.v1},
						.color = prm->psx.gt4.color[1]
					},
					{
						.pos = vertex[coord0],
						.uv = {prm->psx.gt4.u0, prm->psx.gt4.v0},
						.color = prm->psx.gt4.color[0]
					},
				}
			}, prm->psx.gt4.texture);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.uv = {prm->psx.gt4.u2, prm->psx.gt4.v2},
						.color = prm->psx.gt4.color[2]
					},
					{
						.pos = vertex[coord3],
						.uv = {prm->psx.gt4.u3, prm->psx.gt4.v3},
						.color = prm->psx.gt4.color[3]
					},
					{
						.pos = vertex[coord1],
						.uv = {prm->psx.gt4.u1, prm->psx.gt4.v1},
						.color = prm->psx.gt4.color[1]
					},
				}
			}, prm->psx.gt4.texture);

			break;

		case PRM_TYPE_FT3:
			coord0 = prm->psx.ft3.coords[0];
			coord1 = prm->psx.ft3.coords[1];
			coord2 = prm->psx.ft3.coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.uv = {prm->psx.ft3.u2, prm->psx.ft3.v2},
						.color = prm->psx.ft3.color
					},
					{
						.pos = vertex[coord1],
						.uv = {prm->psx.ft3.u1, prm->psx.ft3.v1},
						.color = prm->psx.ft3.color
					},
					{
						.pos = vertex[coord0],
						.uv = {prm->psx.ft3.u0, prm->psx.ft3.v0},
						.color = prm->psx.ft3.color
					},
				}
			}, prm->psx.ft3.texture);

			break;

		case PRM_TYPE_FT4:
			coord0 = prm->psx.ft4.coords[0];
			coord1 = prm->psx.ft4.coords[1];
			coord2 = prm->psx.ft4.coords[2];
			coord3 = prm->psx.ft4.coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.uv = {prm->psx.ft4.u2, prm->psx.ft4.v2},
						.color = prm->psx.ft4.color
					},
					{
						.pos = vertex[coord1],
						.uv = {prm->psx.ft4.u1, prm->psx.ft4.v1},
						.color = prm->psx.ft4.color
					},
					{
						.pos = vertex[coord0],
						.uv = {prm->psx.ft4.u0, prm->psx.ft4.v0},
						.color = prm->psx.ft4.color
					},
				}
			}, prm->psx.ft4.texture);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.uv = {prm->psx.ft4.u2, prm->psx.ft4.v2},
						.color = prm->psx.ft4.color
					},
					{
						.pos = vertex[coord3],
						.uv = {prm->psx.ft4.u3, prm->psx.ft4.v3},
						.color = prm->psx.ft4.color
					},
					{
						.pos = vertex[coord1],
						.uv = {prm->psx.ft4.u1, prm->psx.ft4.v1},
						.color = prm->psx.ft4.color
					},
				}
			}, prm->psx.ft4.texture);

			break;

		case PRM_TYPE_G3:
			coord0 = prm->psx.g3.coords[0];
			coord1 = prm->psx.g3.coords[1];
			coord2 = prm->psx.g3.coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = prm->psx.g3.color[2]
					},
					{
						.pos = vertex[coord1],
						.color = prm->psx.g3.color[1]
					},
					{
						.pos = vertex[coord0],
						.color = prm->psx.g3.color[0]
					},
				}
			}, RENDER_NO_TEXTURE);

			break;

		case PRM_TYPE_G4:
			coord0 = prm->psx.g4.coords[0];
			coord1 = prm->psx.g4.coords[1];
			coord2 = prm->psx.g4.coords[2];
			coord3 = prm->psx.g4.coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = prm->psx.g4.color[2]
					},
					{
						.pos = vertex[coord1],
						.color = prm->psx.g4.color[1]
					},
					{
						.pos = vertex[coord0],
						.color = prm->psx.g4.color[0]
					},
				}
			}, RENDER_NO_TEXTURE);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = prm->psx.g4.color[2]
					},
					{
						.pos = vertex[coord3],
						.color = prm->psx.g4.color[3]
					},
					{
						.pos = vertex[coord1],
						.color = prm->psx.g4.color[1]
					},
				}
			}, RENDER_NO_TEXTURE);

			break;

		case PRM_TYPE_F3:
			coord0 = prm->psx.f3.coords[0];
			coord1 = prm->psx.f3.coords[1];
			coord2 = prm->psx.f3.coords[2];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = prm->psx.f3.color
					},
					{
						.pos = vertex[coord1],
						.color = prm->psx.f3.color
					},
					{
						.pos = vertex[coord0],
						.color = prm->psx.f3.color
					},
				}
			}, RENDER_NO_TEXTURE);

			break;

		case PRM_TYPE_F4:
			coord0 = prm->psx.f4.coords[0];
			coord1 = prm->psx.f4.coords[1];
			coord2 = prm->psx.f4.coords[2];
			coord3 = prm->psx.f4.coords[3];

			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = prm->psx.f4.color
					},
					{
						.pos = vertex[coord1],
						.color = prm->psx.f4.color
					},
					{
						.pos = vertex[coord0],
						.color = prm->psx.f4.color
					},
				}
			}, RENDER_NO_TEXTURE);
			render_push_tris((tris_t) {
				.vertices = {
					{
						.pos = vertex[coord2],
						.color = prm->psx.f4.color
					},
					{
						.pos = vertex[coord3],
						.color = prm->psx.f4.color
					},
					{
						.pos = vertex[coord1],
						.color = prm->psx.f4.color
					},
				}
			}, RENDER_NO_TEXTURE);

			break;

		case PRM_TYPE_TSPR:
		case PRM_TYPE_BSPR:
			coord0 = prm->psx.spr.coord;

			render_push_sprite(
				vec3(
					vertex[coord0].x,
					vertex[coord0].y + ((prm->type == PRM_TYPE_TSPR ? prm->psx.spr.height : -prm->psx.spr.height) >> 1),
					vertex[coord0].z
				),
				vec2i(prm->psx.spr.width, prm->psx.spr.height),
				prm->psx.spr.color,
				prm->psx.spr.texture
			);

			break;

		default:
			break;

		}
	}
}

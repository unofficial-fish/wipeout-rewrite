#ifndef OBJECT_H
#define OBJECT_H

#include "../types.h"
#include "../render.h"
#include "../utils.h"
#include "image.h"

// Altering this enum (other than by adding to the end)
// will break compatibility with the assets.
enum {
	PSX_PRM_TYPE_F3 = 1,
	PSX_PRM_TYPE_FT3,
	PSX_PRM_TYPE_F4,
	PSX_PRM_TYPE_FT4,
	PSX_PRM_TYPE_G3,
	PSX_PRM_TYPE_GT3,
	PSX_PRM_TYPE_G4,
	PSX_PRM_TYPE_GT4,

	PSX_PRM_TYPE_TSPR = 10,
	PSX_PRM_TYPE_BSPR,

	PSX_PRM_TYPE_SPLINE = 20,
};

// The types above can be interpreted as flags like so.
// (You must decrement the type by one for this to work.)
enum {
	PSX_PRM_FLAG_TEXTURED = 1 << 0,
	PSX_PRM_FLAG_QUAD     = 1 << 1,
	PSX_PRM_FLAG_GOURAUD  = 1 << 2,
}; 

enum {
	// "Universal" primitives (models are converted to use these at load time.)
	PRM_TYPE_TRI,
	PRM_TYPE_QUAD,
	PRM_TYPE_SPR,
};

typedef struct {
	int16_t coord;
	uint8_t u, v;
	rgba_t color;
} primitive_vertex_t;

typedef struct Primitive {
	int8_t type;
	int8_t flag;
	union {
		struct {
			int16_t texture;
			primitive_vertex_t v[3];
		} tri;

		struct {
			int16_t texture;
			primitive_vertex_t v[4];
		} quad;

		struct {
			int16_t texture;
			int16_t coord;
			int16_t width;
			int16_t height;
			rgba_t color;
		} spr;

		/* TODO: Implement spline.
		 * The struct is commented out now to avoid paying for unimplemented features
		 * (it makes the union substantially larger.)
		 */
		/*
		struct {
			vec3_t control1;
			vec3_t position;
			vec3_t control2;
			rgba_t color;
		} spline;
		*/
	} u;
} primitive_t;

// PRIMITIVE FLAGS

#define PRM_SINGLE_SIDED 0x0001
#define PRM_SHIP_ENGINE  0x0002
#define PRM_TRANSLUCENT  0x0004

typedef struct Object {
	char name[16];

	mat4_t mat;
	int16_t vertices_len; // Number of Vertices
	vec3_t *vertices; // Pointer to 3D Points

	int16_t normals_len; // Number of Normals
	vec3_t *normals; // Pointer to 3D Normals

	int16_t primitives_len; // Number of Primitives
	primitive_t *primitives; // Pointer to Z Sort Primitives

	vec3_t origin;
	int32_t extent; // Flags for object characteristics
	int16_t flags; // Next object in list
	float radius;
	struct Object *next; // Next object in list
} Object;

Object *objects_load(char *name, texture_list_t tl);
void object_draw(Object *object, mat4_t *mat);

#endif

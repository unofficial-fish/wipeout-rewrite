#ifndef OBJECT_H
#define OBJECT_H

#include "../types.h"
#include "../render.h"
#include "../utils.h"
#include "image.h"

// Altering this enum (other than adding to the end)
// will break compatibility with the assets.
enum {
	PRM_TYPE_F3 = 1,
	PRM_TYPE_FT3,
	PRM_TYPE_F4,
	PRM_TYPE_FT4,
	PRM_TYPE_G3,
	PRM_TYPE_GT3,
	PRM_TYPE_G4,
	PRM_TYPE_GT4,

	PRM_TYPE_LF2,
	PRM_TYPE_TSPR,
	PRM_TYPE_BSPR,

	PRM_TYPE_LSF3,
	PRM_TYPE_LSFT3,
	PRM_TYPE_LSF4,
	PRM_TYPE_LSFT4,
	PRM_TYPE_LSG3,
	PRM_TYPE_LSGT3,
	PRM_TYPE_LSG4,
	PRM_TYPE_LSGT4,

	PRM_TYPE_SPLINE,

	PRM_TYPE_INFINITE_LIGHT,
	PRM_TYPE_POINT_LIGHT,
	PRM_TYPE_SPOT_LIGHT,
};

typedef struct Primitive {
	int16_t type;
	int16_t flag;
	union {
		struct {
			int16_t coords[3]; // Indices of the coords
			rgba_t color;
		} f3;

		struct {
			int16_t coords[3]; // Indices of the coords
			int16_t texture;
			uint8_t u0;
			uint8_t v0;
			uint8_t u1;
			uint8_t v1;
			uint8_t u2;
			uint8_t v2;
			rgba_t color;
		} ft3;

		struct {
			int16_t coords[4]; // Indices of the coords
			rgba_t color;
		} f4;

		struct {
			int16_t coords[4]; // Indices of the coords
			int16_t texture;
			uint8_t u0;
			uint8_t v0;
			uint8_t u1;
			uint8_t v1;
			uint8_t u2;
			uint8_t v2;
			uint8_t u3;
			uint8_t v3;
			rgba_t color;
		} ft4;

		struct {
			int16_t coords[3]; // Indices of the coords
			int16_t pad1;
			rgba_t color[3];
		} g3;

		struct {
			int16_t coords[3]; // Indices of the coords
			int16_t texture;
			uint8_t u0;
			uint8_t v0;
			uint8_t u1;
			uint8_t v1;
			uint8_t u2;
			uint8_t v2;
			rgba_t color[3];
		} gt3;

		struct {
			int16_t coords[4]; // Indices of the coords
			rgba_t color[4];
		} g4;

		struct {
			int16_t coords[4]; // Indices of the coords
			int16_t texture;
			uint8_t u0;
			uint8_t v0;
			uint8_t u1;
			uint8_t v1;
			uint8_t u2;
			uint8_t v2;
			uint8_t u3;
			uint8_t v3;
			rgba_t color[4];
		} gt4;

		/* OTHER PRIMITIVE TYPES
		*/
		struct {
			int16_t coord;
			int16_t width;
			int16_t height;
			int16_t texture;
			rgba_t color;
		} spr;

		struct {
			vec3_t control1;
			vec3_t position;
			vec3_t control2;
			rgba_t color;
		} spline;
	} psx;
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

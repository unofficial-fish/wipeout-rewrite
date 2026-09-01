#include "localization.h"
#include "mem.h"
#include "platform.h"
#include "utils.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Crude localization system.
 * We can't use GNU gettext here, many strings are hardcoded into the binary in ways it can't deal with (see game.c)
 * Limitations:
 *  Hardcoded maximum number of localizations
 *  Hardcoded maximum localization length
 *  Uses english strings as keys (brittle, cannot localize the same string differently depending on context)
 *  
 * The wipEout font contains only these glyphs:
 *     ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:.
 * that is, the uppercase members of the ISO basic latin alphabet, the Arabic numerals, colon, and full stop.
 * If your language uses diacritics, additional punctuation, another writing system, etc. you will likely have to 
 * mangle it somewhat to fit.
 * 
 * To get a colon you must type 'e' and a full stop is 'f'.
 * 
 * The translation file format is as follows:
 * EXAMPLE>EJEMPLO,
 * 
 * Mind the length of your translations. The HUD layouts were not designed with localization in mind.
 */
#define LOCALIZATIONS_MAX 256

typedef struct {
	int64_t key;
	void *value;
} hashtable_t;

hashtable_t *table;

// Fowler-Noll-Vo hash function
#define FNV_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_PRIME        0x100000001b3

static int64_t hash_str(const char *str) {
	int64_t hash = FNV_OFFSET_BASIS;
	for (int i = 0; i < strlen(str); i++) {
		hash = hash^(str[i]);
		hash = hash*FNV_PRIME;
	}
	return hash;
}

int compare(const void *a, const void *b) {
	int64_t aa = ((hashtable_t *)a)->key;
	int64_t bb = ((hashtable_t *)b)->key;

	return (aa > bb) - (aa < bb);
};

// TODO: ufish: I'm not proud of this implementation. It's inelegant.
// Also, it's string parsing in C, so it's probably broken.
void localization_init(const char *name) {
	uint32_t length = 0;
	table = mem_bump(sizeof(hashtable_t) * LOCALIZATIONS_MAX);

	char *orig = (char *)platform_load_asset(name, &length);
	if (!orig) {
		die("Failed to load file %s\n", name);
	}
	// platform_load_asset does not null terminate, nor does it leave space for a null terminator,
	// hence, acrobatics.
	char *chars = mem_temp_alloc(length+1);
	strncpy(chars, orig, length);
	chars[length] = '\0';
	mem_temp_free(orig);

	char *char_iter = chars;
	char *temp1 = mem_temp_alloc(1024);
	char *temp2 = mem_temp_alloc(1024);
	
	for (int i = 0; i < 60; i++){
		int len1, len2 = 0;
		sscanf(char_iter, "%[^>]>%n%[^/,]%n", temp1, &len1, temp2, &len2);
		size_t value_length = len2 - len1;
		int64_t key = hash_str(temp1);
		char *value = mem_bump(value_length + 1);
		strncpy(value, temp2, value_length);
		table[i] = (hashtable_t){key, value};
		while (*char_iter++ != '\n');
	}
	qsort(table, LOCALIZATIONS_MAX, sizeof(hashtable_t), compare);

	mem_temp_free(chars);
	mem_temp_free(temp1);
	mem_temp_free(temp2);
}

char *localization_replace(const char *str) {
	hashtable_t key = (hashtable_t){hash_str(str), NULL};
	hashtable_t *found = (hashtable_t *)bsearch(&key, table, LOCALIZATIONS_MAX, sizeof(hashtable_t), compare);
	if (!found) {
		return NULL;
	} else {
		return found->value;
	}
}

#ifndef STR_H
#define STR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ID_LEN 4
#define ID_SIZE (ID_LEN + 1)

#define KLID_LEN 8
#define KLID_SIZE (KLID_LEN + 1)

#define UUID_LEN 38
#define UUID_SIZE (UUID_LEN + 1)

bool getLayoutLibraryPath(char *dst, const size_t size, const char *file);
void getRegLayoutPath(char *dst, const size_t size, const char *klid);

bool isValidUUID(const char *str);

bool parseLayoutID(uint16_t *id, const char *str);
bool parseKLID(uint16_t *device_id, uint16_t *lang_id, const char *klid);

#endif

#include "Str.h"

#include "Dialog.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <RegStr.h>

bool getLayoutLibraryPath(char *dst, const size_t size, const char *file)
{
	if (!dst || size == 0 || !file) {
		return false;
	}

	if (!GetSystemDirectory(dst, (UINT)size)) {
		showSysError("getLayoutLibraryPath()", "GetSystemDirectory() failed!", GetLastError());
		return false;
	}

	snprintf(dst, size, "%s\\%s", dst, file);

	return true;
}

void getRegLayoutPath(char *dst, const size_t size, const char *klid)
{
	if (!dst || size == 0) {
		return;
	}

	snprintf(dst, size, REGSTR_PATH_CURRENT_CONTROL_SET "\\Keyboard Layouts\\%s", klid ? klid : "");
}

bool isValidUUID(const char *str)
{
	if (!str) {
		return false;
	}

	uint8_t tmp_8;
	uint16_t tmp_16;
	uint32_t tmp_32, n_bytes;
	const int n_items = sscanf_s(str, "{%8X-%4hX-%4hX-%4hX-%4hX%8X}%n%c", &tmp_32, &tmp_16, &tmp_16, &tmp_16, &tmp_16,
								 &tmp_32, &n_bytes, &tmp_8, (unsigned int)sizeof(tmp_8));

	return n_bytes == UUID_LEN && n_items == 6;
}

bool parseLayoutID(uint16_t *id, const char *str)
{
	if (!id || !str) {
		return false;
	}

	uint8_t tmp_8;
	uint32_t n_bytes;
	const int n_items = sscanf_s(str, "%4hx%n%c", id, &n_bytes, &tmp_8, (unsigned int)sizeof(tmp_8));

	return n_bytes == ID_LEN && n_items == 1;
}

bool parseKLID(uint16_t *device_id, uint16_t *lang_id, const char *klid)
{
	if (!device_id || !lang_id || !klid) {
		return false;
	}

	uint8_t tmp_8;
	uint32_t n_bytes;
	const int n_items =
		sscanf_s(klid, "%4hx%4hx%n%c", device_id, lang_id, &n_bytes, &tmp_8, (unsigned int)sizeof(tmp_8));

	return n_bytes == KLID_LEN && n_items == 2;
}

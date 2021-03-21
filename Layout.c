#include "Layout.h"

#include "Config.h"
#include "Dialog.h"
#include "Input.h"
#include "Str.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

bool installLayout(const Config *config, const Layouts *layouts)
{
	if (!config || !layouts) {
		return false;
	}

	char path[MAX_PATH];
	getLayoutLibraryPath(path, sizeof(path), config->file);
	if (!CopyFile(config->file, path, true)) {
		const DWORD error = GetLastError();
		if (error == ERROR_FILE_EXISTS) {
			showError("installLayout()", "\"%s\" already exists!", path);
		} else {
			showError("installLayout()", "CopyFile() failed to copy to \"%s\" with error %lu!", path, error);
		}

		return false;
	}

	bool ok = false;

	char layout_id[ID_SIZE];
	for (uint16_t id = 0x00c0;; ++id) {
		bool ok = true;

		for (DWORD i = 0; i < layouts->num; ++i) {
			if (layouts->list[i].id == id) {
				ok = false;
				break;
			}
		}

		if (ok) {
			snprintf(layout_id, sizeof(layout_id), "%04x", id);
			break;
		}

		if (id == USHRT_MAX) {
			showError("installLayout()", "No layout ID available in the 0x00c0-0xffff range!");
			goto FINAL;
		}
	}

	HKEY key;
	uint16_t device_id;
	char klid[KLID_SIZE];
	char reg_path[MAX_PATH];

	for (device_id = 0xa000;; ++device_id) {
		snprintf(klid, sizeof(klid), "%04x%04x", device_id, config->lang_id);
		getRegLayoutPath(reg_path, sizeof(reg_path), klid);

		DWORD disposition;
		const LSTATUS ret =
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, reg_path, 0, NULL, 0, KEY_WRITE, NULL, &key, &disposition);
		if (ret != ERROR_SUCCESS) {
			showError("installLayout()", "RegCreateKeyEx() failed with error %ld!", ret);
			goto FINAL;
		}

		if (disposition != REG_OPENED_EXISTING_KEY) {
			break;
		}

		RegCloseKey(key);

		if (device_id == USHRT_MAX) {
			showError("installLayout()", "No device ID available in the 0xa000-0xffff range!");
			goto FINAL;
		}
	}

	LSTATUS ret = RegSetKeyValue(key, NULL, "Layout Id", REG_SZ, layout_id, sizeof(layout_id));
	if (ret != ERROR_SUCCESS) {
		showError("installLayout()", "RegSetKeyValue() failed to set \"Layout Id\" with error %ld!", ret);
		goto FINAL_REG;
	}

	ret = RegSetKeyValue(key, NULL, "Layout Product Code", REG_SZ, config->product_code,
						 strlen(config->product_code) + 1);
	if (ret != ERROR_SUCCESS) {
		showError("installLayout()", "RegSetKeyValue() failed to set \"Layout Product Code\" with error %ld!", ret);
		goto FINAL_REG;
	}

	ret = RegSetKeyValue(key, NULL, "Layout File", REG_SZ, config->file, strlen(config->file) + 1);
	if (ret != ERROR_SUCCESS) {
		showError("installLayout()", "RegSetKeyValue() failed to set \"Layout File\" with error %ld!", ret);
		goto FINAL_REG;
	}

	ret = RegSetKeyValue(key, NULL, "Layout Text", REG_SZ, config->text, strlen(config->text) + 1);
	if (ret != ERROR_SUCCESS) {
		showError("installLayout()", "RegSetKeyValue() failed to set \"Layout Text\" with error %ld!", ret);
		goto FINAL_REG;
	}

	ret = RegSetKeyValue(key, NULL, "Layout Display Name", REG_SZ, config->display_name,
						 strlen(config->display_name) + 1);
	if (ret != ERROR_SUCCESS) {
		showError("installLayout()", "RegSetKeyValue() failed to set \"Layout Display Name\" with error %ld!", ret);
		goto FINAL_REG;
	}

	if (!enableLayout(device_id, config->lang_id)) {
		goto FINAL_REG;
	}

	ok = true;
FINAL_REG:
	if (!ok) {
		ret = RegDeleteKey(HKEY_LOCAL_MACHINE, reg_path);
		if (ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND) {
			showWarning("installLayout()", "RegDeleteKey() failed with error %ld!\n\nPlease delete \"%s\" manually.",
						ret, reg_path);
		}
	}

	RegCloseKey(key);
FINAL:
	if (!ok) {
		if (!DeleteFile(path)) {
			const DWORD error = GetLastError();
			if (error != ERROR_FILE_NOT_FOUND) {
				showWarning("installLayout()", "DeleteFile() failed with error %lu!\n\nPlease delete \"%s\" manually.",
							error, path);
			}
		}
	}

	return ok;
}

bool uninstallLayout(const Layout *layout)
{
	if (!layout) {
		return false;
	}

	if (!disableLayout(layout->device_id, layout->lang_id)) {
		return false;
	}

	char klid[KLID_SIZE];
	snprintf(klid, sizeof(klid), "%04x%04x", layout->device_id, layout->lang_id);

	char path[MAX_PATH];
	getRegLayoutPath(path, sizeof(path), klid);

	const LSTATUS ret = RegDeleteKey(HKEY_LOCAL_MACHINE, path);
	if (ret != ERROR_SUCCESS) {
		showError("uninstallLayout()", "RegDeleteKey() failed with error %ld!", ret);
		return false;
	}

	if (!getLayoutLibraryPath(path, sizeof(path), layout->file)) {
		return false;
	}

	if (!DeleteFile(path)) {
		const DWORD error = GetLastError();
		if (error != ERROR_FILE_NOT_FOUND) {
			showWarning("uninstallLayout()", "DeleteFile() failed with error %lu!\n\nPlease delete \"%s\" manually.",
						error, path);
			return false;
		}
	}

	return true;
}

Layouts *getLayouts()
{
	char path[MAX_PATH];
	getRegLayoutPath(path, sizeof(path), NULL);

	HKEY key;
	LSTATUS ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &key);
	if (ret != ERROR_SUCCESS) {
		showError("getLayouts()", "RegOpenKeyEx() failed with error %ld!", ret);
		return NULL;
	}

	bool ok = false;
	char *name = NULL;

	Layouts *layouts = malloc(sizeof(Layouts));
	memset(layouts, 0, sizeof(Layouts));

	DWORD max_key_size;
	ret = RegQueryInfoKey(key, NULL, NULL, NULL, &layouts->num, &max_key_size, NULL, NULL, NULL, NULL, NULL, NULL);
	if (ret != ERROR_SUCCESS) {
		showError("getLayouts()", "RegQueryInfoKey() failed with error %ld!", ret);
		goto FINAL;
	}

	name = malloc(max_key_size);

	layouts->list = malloc(sizeof(Layout) * layouts->num);
	memset(layouts->list, 0, sizeof(Layout) * layouts->num);

	for (DWORD i = 0; i < layouts->num; ++i) {
		DWORD size = max_key_size;
		ret = RegEnumKeyEx(key, i, name, &size, 0, NULL, 0, NULL);
		if (ret != ERROR_SUCCESS) {
			showError("getLayouts()", "RegEnumKeyEx() failed at index %lu with error %ld!", i, ret);
			goto FINAL;
		}

		Layout *layout = &layouts->list[i];
		if (!parseKLID(&layout->device_id, &layout->lang_id, name)) {
			showError("getLayouts()", "parseKLID() failed to parse \"%s\"!", name);
			goto FINAL;
		}

		char id[ID_SIZE];
		size = sizeof(id);
		ret = RegGetValue(key, name, "Layout Id", RRF_RT_REG_SZ, NULL, id, &size);
		if (ret == ERROR_SUCCESS) {
			if (!parseLayoutID(&layout->id, id)) {
				showError("getLayouts()", "parseID() failed to parse \"%s\" from \"%s\"!", id, name);
				goto FINAL;
			}
		} else if (ret != ERROR_FILE_NOT_FOUND) {
			showError("getLayouts()", "RegGetValue() failed to get \"Layout Id\" from \"%s\" with error %ld!", name,
					  ret);
			goto FINAL;
		}

		size = UUID_SIZE;
		if (RegGetValue(key, name, "Layout Product Code", RRF_RT_REG_SZ, NULL, NULL, &size) == ERROR_SUCCESS) {
			layout->product_code = malloc(UUID_SIZE);
			ret = RegGetValue(key, name, "Layout Product Code", RRF_RT_REG_SZ, NULL, layout->product_code, &size);
			if (ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND) {
				showError("getLayouts()",
						  "RegGetValue() failed to get \"Layout Product Code\" from \"%s\" with error %ld!", name, ret);
				goto FINAL;
			}
		}

		size = 0;
		ret = RegGetValue(key, name, "Layout File", RRF_RT_REG_SZ, NULL, NULL, &size);
		if (ret != ERROR_SUCCESS) {
			showError("getLayouts()",
					  "RegGetValue() failed to get the required size for \"Layout File\" from \"%s\" with error %ld!",
					  name, ret);
			goto FINAL;
		}

		layout->file = malloc(size);
		ret = RegGetValue(key, name, "Layout File", RRF_RT_REG_SZ, NULL, layout->file, &size);
		if (ret != ERROR_SUCCESS) {
			showError("getLayouts()", "RegGetValue() failed to get \"Layout File\" from \"%s\" with error %ld!", name,
					  ret);
			goto FINAL;
		}
	}

	ok = true;
FINAL:
	if (name) {
		free(name);
	}

	RegCloseKey(key);

	if (!ok) {
		freeLayouts(layouts);
		return NULL;
	}

	return layouts;
}

void freeLayouts(Layouts *layouts)
{
	if (!layouts) {
		return;
	}

	Layout *list = layouts->list;
	if (list) {
		for (DWORD i = 0; i < layouts->num; ++i) {
			Layout *layout = &list[i];
			if (layout->product_code) {
				free(layout->product_code);
			}

			if (layout->file) {
				free(layout->file);
			}
		}

		free(list);
	}

	free(layouts);
}
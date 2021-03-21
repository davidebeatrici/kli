#include "Input.h"

#include "Dialog.h"
#include "Str.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#define ILOT_UNINSTALL 0x00000001

typedef struct LayoutOrTipProfile
{
	enum { LOTP_INPUTPROCESSOR = 1, LOTP_KEYBOARDLAYOUT } type;
	LANGID lang_id;
	CLSID clsid;
	GUID description;
	GUID category;
	DWORD substitute_layout;
	DWORD flags;
	WCHAR id[MAX_PATH];
} LayoutOrTipProfile;

typedef BOOL(WINAPI *INSTALLLAYOUTORTIP)(LPCWSTR psz, DWORD dwFlasg);

typedef UINT(WINAPI *ENUMENABLEDLAYOUTORTIP)(LPCWSTR pszUserReg, LPCWSTR pszSystemReg, LPCWSTR pszSoftwareReg,
											 LayoutOrTipProfile *pLayoutOrTipProfile, UINT uBufLength);

bool enableLayout(const uint16_t device_id, const uint16_t lang_id)
{
	HMODULE module = LoadLibrary("input.dll");
	if (!module) {
		showError("enableLayout()", "LoadLibrary() failed to load \"input.dll\" with error %lu!", GetLastError());
		return false;
	}

	bool ok = false;

	INSTALLLAYOUTORTIP InstallLayoutOrTip = (INSTALLLAYOUTORTIP)GetProcAddress(module, "InstallLayoutOrTip");
	if (!InstallLayoutOrTip) {
		showError("enableLayout()",
				  "GetProcAddress() failed to get the address of \"InstallLayoutOrTip()\" with error %lu!",
				  GetLastError());
		goto FINAL;
	}

	wchar_t profile[MAX_PATH];
	swprintf(profile, sizeof(profile), L"%04x:%04x%04x", lang_id, device_id, lang_id);
	ok = InstallLayoutOrTip(profile, 0);
	if (!ok) {
		showError("enableLayout()", "InstallLayoutOrTip() failed!");
		goto FINAL;
	}

	char klid[KLID_SIZE];
	snprintf(klid, sizeof(klid), "%04x%04x", device_id, lang_id);
	if (!LoadKeyboardLayout(klid, KLF_REPLACELANG)) {
		showWarning("enableLayout()", "LoadKeyboardLayout() failed with error %lu!", GetLastError());
		goto FINAL;
	}

	ok = true;
FINAL:
	FreeLibrary(module);
	return ok;
}

bool disableLayout(const uint16_t device_id, const uint16_t lang_id)
{
	HMODULE module = LoadLibrary("input.dll");
	if (!module) {
		showError("disableLayout()", "LoadLibrary() failed to load \"input.dll\" with error %lu!", GetLastError());
		return false;
	}

	bool ok = false;

	INSTALLLAYOUTORTIP InstallLayoutOrTip = (INSTALLLAYOUTORTIP)GetProcAddress(module, "InstallLayoutOrTip");
	if (!InstallLayoutOrTip) {
		showError("disableLayout()",
				  "GetProcAddress() failed to get the address of \"InstallLayoutOrTip()\" with error %lu!",
				  GetLastError());
		goto FINAL;
	}

	ENUMENABLEDLAYOUTORTIP EnumEnabledLayoutOrTip =
		(ENUMENABLEDLAYOUTORTIP)GetProcAddress(module, "EnumEnabledLayoutOrTip");
	if (!EnumEnabledLayoutOrTip) {
		showError("disableLayout()",
				  "GetProcAddress() failed to get the address of \"EnumEnabledLayoutOrTip()\" with error %lu!",
				  GetLastError());
		goto FINAL;
	}

	UINT num = EnumEnabledLayoutOrTip(NULL, NULL, NULL, NULL, 0);
	const size_t size = sizeof(LayoutOrTipProfile) * num;
	LayoutOrTipProfile *profiles = malloc(size);
	num = EnumEnabledLayoutOrTip(NULL, NULL, NULL, profiles, (UINT)size);

	for (UINT i = 0; i < num; ++i) {
		const LayoutOrTipProfile *profile = &profiles[i];
		if (profile->type != LOTP_KEYBOARDLAYOUT) {
			continue;
		}

		uint16_t tmp, profile_device_id, profile_lang_id;
		swscanf_s(profile->id, L"%4hx:%4hx%4hx", &tmp, &profile_device_id, &profile_lang_id);

		if (profile_device_id == device_id && profile_lang_id == lang_id) {
			InstallLayoutOrTip(profile->id, ILOT_UNINSTALL);
		}
	}

	free(profiles);

	num = GetKeyboardLayoutList(0, NULL);
	if (!num) {
		showError("disableLayout()", "GetKeyboardLayoutList() failed to get the number of layouts with error %lu!",
				  GetLastError());
		goto FINAL;
	}

	HKL *hkls = malloc(sizeof(HKL) * num);
	if (!GetKeyboardLayoutList(num, hkls)) {
		showError("disableLayout()", "GetKeyboardLayoutList() failed to get the layouts with error %lu!",
				  GetLastError());
		goto FINAL_HKLS;
	}

	char klid[KLID_SIZE];
	snprintf(klid, sizeof(klid), "%04x%04x", device_id, lang_id);

	for (UINT i = 0; i < num; ++i) {
		if (!ActivateKeyboardLayout(hkls[i], 0)) {
			continue;
		}

		char name[KL_NAMELENGTH];
		if (!GetKeyboardLayoutName(name)) {
			continue;
		}

		// GetKeyboardLayoutName() uses uppercase letters.
		if (_stricmp(name, klid) == 0) {
			UnloadKeyboardLayout(hkls[i]);
			break;
		}
	}

	ok = true;
FINAL_HKLS:
	free(hkls);
FINAL:
	FreeLibrary(module);
	return ok;
}

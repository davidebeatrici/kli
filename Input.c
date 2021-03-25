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
		showSysError("enableLayout()", "LoadLibrary() failed to load \"input.dll\"!", GetLastError());
		return false;
	}

	bool ok = false;

	INSTALLLAYOUTORTIP InstallLayoutOrTip = (INSTALLLAYOUTORTIP)GetProcAddress(module, "InstallLayoutOrTip");
	if (!InstallLayoutOrTip) {
		showSysError("enableLayout()", "GetProcAddress() failed to get the address of \"InstallLayoutOrTip()\"!",
					 GetLastError());
		goto FINAL;
	}

	wchar_t profile[MAX_PATH];
	swprintf(profile, sizeof(profile) / sizeof(profile[0]), L"%04x:%04x%04x", lang_id, device_id, lang_id);
	ok = InstallLayoutOrTip(profile, 0);
	if (!ok) {
		showError("enableLayout()", "InstallLayoutOrTip() failed!");
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
		showSysError("disableLayout()", "LoadLibrary() failed to load \"input.dll\"!", GetLastError());
		return false;
	}

	bool ok = false;

	INSTALLLAYOUTORTIP InstallLayoutOrTip = (INSTALLLAYOUTORTIP)GetProcAddress(module, "InstallLayoutOrTip");
	if (!InstallLayoutOrTip) {
		showSysError("disableLayout()", "GetProcAddress() failed to get the address of \"InstallLayoutOrTip()\"!",
					 GetLastError());
		goto FINAL;
	}

	ENUMENABLEDLAYOUTORTIP EnumEnabledLayoutOrTip =
		(ENUMENABLEDLAYOUTORTIP)GetProcAddress(module, "EnumEnabledLayoutOrTip");
	if (!EnumEnabledLayoutOrTip) {
		showSysError("disableLayout()", "GetProcAddress() failed to get the address of \"EnumEnabledLayoutOrTip()\"!",
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
	ok = true;
FINAL:
	FreeLibrary(module);
	return ok;
}

#include "Dialog.h"

#include <stdio.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#define MAX_MESSAGE_SIZE 1024

static int showMessage(const char *title, const char *message, const uint32_t error, const uint32_t type,
					   const va_list args)
{
	if (!message) {
		return 0;
	}

	char buf[MAX_MESSAGE_SIZE];
	vsnprintf(buf, sizeof(buf), message, args);

	if (error) {
		char *error_str;
		const DWORD ret = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0,
										(LPSTR)&error_str, 0, NULL);
		if (ret) {
			snprintf(buf, sizeof(buf), "%s\n\nError %lu: %s", buf, error, error_str);
			LocalFree(error_str);
		} else {
			snprintf(buf, sizeof(buf), "%s\n\nError %lu", buf, error);
		}
	}

	return MessageBox(NULL, buf, title, type);
}

void showError(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	showMessage(title, message, 0, MB_OK | MB_ICONERROR, args);
	va_end(args);
}

void showInfo(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	showMessage(title, message, 0, MB_OK | MB_ICONINFORMATION, args);
	va_end(args);
}

void showSysError(const char *title, const char *message, const uint32_t error, ...)
{
	va_list args;
	va_start(args, error);
	showMessage(title, message, error, MB_OK | MB_ICONERROR, args);
	va_end(args);
}

void showSysWarning(const char *title, const char *message, const uint32_t error, ...)
{
	va_list args;
	va_start(args, error);
	showMessage(title, message, error, MB_OK | MB_ICONWARNING, args);
	va_end(args);
}

bool showPrompt(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	const int ret = showMessage(title, message, 0, MB_YESNO | MB_ICONWARNING, args);
	va_end(args);

	return ret == IDYES;
}

#include "Dialog.h"

#include <stdio.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#define MAX_MESSAGE_SIZE 1024

static int showMessage(const char *title, const char *message, const unsigned int type, const va_list args)
{
	char buf[MAX_MESSAGE_SIZE];
	vsnprintf(buf, sizeof(buf), message, args);
	return MessageBox(NULL, buf, title, type);
}

void showError(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	showMessage(title, message, MB_OK | MB_ICONERROR, args);
	va_end(args);
}

void showInfo(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	showMessage(title, message, MB_OK | MB_ICONINFORMATION, args);
	va_end(args);
}

void showWarning(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	showMessage(title, message, MB_OK | MB_ICONWARNING, args);
	va_end(args);
}

bool showPrompt(const char *title, const char *message, ...)
{
	va_list args;
	va_start(args, message);
	const int ret = showMessage(title, message, MB_YESNO | MB_ICONWARNING, args);
	va_end(args);

	return ret == IDYES;
}

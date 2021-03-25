#ifndef DIALOG_H
#define DIALOG_H

#include <stdbool.h>
#include <stdint.h>

void showError(const char *title, const char *message, ...);
void showInfo(const char *title, const char *message, ...);

void showSysError(const char *title, const char *message, const uint32_t error, ...);
void showSysWarning(const char *title, const char *message, const uint32_t error, ...);

bool showPrompt(const char *title, const char *message, ...);

#endif

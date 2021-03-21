#ifndef DIALOG_H
#define DIALOG_H

#include <stdbool.h>

void showError(const char *title, const char *message, ...);
void showInfo(const char *title, const char *message, ...);
void showWarning(const char *title, const char *message, ...);

bool showPrompt(const char *title, const char *message, ...);

#endif

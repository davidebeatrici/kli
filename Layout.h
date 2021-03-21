#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdbool.h>
#include <stdint.h>

typedef unsigned long DWORD;

typedef struct Config Config;

typedef struct Layout
{
	uint16_t device_id;
	uint16_t lang_id;
	uint16_t id;
	char *file;
	char *product_code;
} Layout;

typedef struct Layouts
{
	DWORD num;
	Layout *list;
} Layouts;

bool installLayout(const Config *config, const Layouts *layouts);
bool uninstallLayout(const Layout *layout);

Layouts *getLayouts();
void freeLayouts(Layouts *layouts);

#endif

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Config
{
	// [install]
	uint16_t lang_id;
	char *product_code;

	// [layout]
	char *file;
	char *text;
	char *display_name;
} Config;

bool loadConfig(Config *config, const char *ini);
void freeConfig(Config *config);

#endif

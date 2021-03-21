#include "Config.h"

#include "Dialog.h"
#include "Str.h"

#include <stdlib.h>
#include <string.h>

#include <ini.h>

static int iniHandler(void *user, const char *section, const char *name, const char *value)
{
	Config *config = user;
	if (!config) {
		return 0;
	}

	if (strcmp(section, "install") == 0) {
		if (strcmp(name, "language_id") == 0) {
			unsigned long id = strtoul(value, NULL, 16);
			if (id > USHRT_MAX) {
				return 0;
			}
			config->lang_id = (uint16_t)id;
		} else if (strcmp(name, "product_code") == 0) {
			if (!isValidUUID(value)) {
				return 0;
			}

			config->product_code = _strdup(value);
		}
	} else if (strcmp(section, "layout") == 0) {
		if (strcmp(name, "file") == 0) {
			config->file = _strdup(value);
		} else if (strcmp(name, "text") == 0) {
			config->text = _strdup(value);
		} else if (strcmp(name, "display_name") == 0) {
			config->display_name = _strdup(value);
		}
	}

	return 1;
}

bool loadConfig(Config *config, const char *ini)
{
	if (!config) {
		return false;
	}

	if (ini_parse(ini, iniHandler, config) < 0) {
		showError("loadConfig()", "ini_parse() failed to open \"%s\"!", ini);
		return false;
	}

	if (!config->lang_id) {
		showError("loadConfig()", "Missing or invalid \"language_id\" entry in \"[install]\"!");
		return false;
	}

	if (!config->product_code) {
		showError("loadConfig()", "Missing or invalid \"product_code\" entry in \"[install]\"!");
		return false;
	}

	if (!config->file) {
		showError("loadConfig()", "Missing or invalid \"file\" entry in \"[layout]\"!");
		return false;
	}

	if (!config->text) {
		showError("loadConfig()", "Missing or invalid \"text\" entry in \"[layout]\"!");
		return false;
	}

	if (!config->display_name) {
		showError("loadConfig()", "Missing or invalid \"display_name\" entry in \"[layout]\"!");
		return false;
	}

	return true;
}

void freeConfig(Config *config)
{
	if (!config) {
		return;
	}

	if (config->product_code) {
		free(config->product_code);
	}

	if (config->file) {
		free(config->file);
	}

	if (config->text) {
		free(config->text);
	}

	if (config->display_name) {
		free(config->display_name);
	}
}

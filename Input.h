#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

bool enableLayout(const uint16_t device_id, const uint16_t lang_id);
bool disableLayout(const uint16_t device_id, const uint16_t lang_id);

#endif

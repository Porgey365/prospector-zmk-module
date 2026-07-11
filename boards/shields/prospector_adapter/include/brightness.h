#pragma once

#include <stdbool.h>

/* Called by the idle-timeout logic to force the backlight off (idle=true) or
 * let normal brightness handling (ambient-light fade or fixed level) resume
 * (idle=false). No-op if idle timeout support isn't compiled in. */
void prospector_brightness_set_idle(bool idle);

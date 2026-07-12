/*
 * Turns the display off after CONFIG_PROSPECTOR_IDLE_TIMEOUT_S seconds of no
 * keyboard activity, and back on as soon as a key is pressed anywhere on the
 * split (dongle or either half -- position_state_changed is re-raised on the
 * central for peripheral key events too). Only built when the timeout is
 * nonzero; see CMakeLists.txt.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
/* Own module at a hardcoded DBG level (matches brightness.c's "als" module)
 * rather than declaring into the shared "zmk" module -- that ties compile-time
 * visibility to CONFIG_ZMK_LOG_LEVEL, which defaults low enough to compile
 * these LOG_INF calls out entirely. */
LOG_MODULE_REGISTER(prospector_idle, 4);

#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#include "brightness.h"

static bool is_idle;

static void idle_timeout_handler(struct k_work *work) {
    LOG_INF("prospector idle timeout: %d s elapsed, turning display off", CONFIG_PROSPECTOR_IDLE_TIMEOUT_S);
    is_idle = true;
    prospector_brightness_set_idle(true);
}

static K_WORK_DELAYABLE_DEFINE(idle_timeout_work, idle_timeout_handler);

static void reset_idle_timer(void) {
    if (is_idle) {
        LOG_INF("prospector idle timeout: activity detected, waking display");
        is_idle = false;
        prospector_brightness_set_idle(false);
    }

    k_work_reschedule(&idle_timeout_work, K_SECONDS(CONFIG_PROSPECTOR_IDLE_TIMEOUT_S));
}

static int idle_timeout_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    LOG_INF("prospector idle timeout: position event pos=%d state=%d", ev->position, ev->state);

    if (ev->state) {
        reset_idle_timer();
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(prospector_idle_timeout, idle_timeout_listener);
ZMK_SUBSCRIPTION(prospector_idle_timeout, zmk_position_state_changed);

static int idle_timeout_init(void) {
    LOG_INF("prospector idle timeout: armed, timeout=%d s", CONFIG_PROSPECTOR_IDLE_TIMEOUT_S);
    k_work_reschedule(&idle_timeout_work, K_SECONDS(CONFIG_PROSPECTOR_IDLE_TIMEOUT_S));
    return 0;
}

SYS_INIT(idle_timeout_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

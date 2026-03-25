/***********************************************************
 *  DIY monome compatible grid w/ Adafruit NeoTrellis
 *  for RP2040 Pi Pico
 *
 *  Supports two modes stored in flash:
 *    mode 0 (iii)    - Lua scripting via iii
 *    mode 1 (monome) - standard monome serial protocol
 *
 *
 */

#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "config.h"
#include "bsp/board.h"
#include "tusb.h"

extern "C" {
#include "flash.h"
#include "iii.h"
#include "device.h"
}

// USB descriptor selector — read by tud_descriptor_device_cb() in usb_descriptors.cpp
//   0 = DINKII descriptor (CDC+MIDI, used for iii/REPL mode)
//   1 = MONOME descriptor  (CDC+MIDI, monome VID/PID)
uint8_t g_monome_mode = 0;

// Current device mode — also read by device.cpp for dispatch decisions
//   0 = iii (Lua scripting)
//   1 = monome serial protocol
uint8_t mode = 0;

// Defined in device.cpp — blocking monome protocol loop
extern "C" void device_monome_loop();

// Defined in Adafruit_I2CDevice.cpp — lets us pump tud_task() during I2C delays
void neotrellis_set_poll_callback(void (*cb)(void));

// Core 1: lockout victim for safe flash operations from core 0.
// lfs_erase/lfs_prog in fs.c call multicore_lockout_start_blocking(),
// which requires the other core to have registered a lockout handler.
static void core1_entry() {
    multicore_lockout_victim_init();
    multicore_fifo_push_blocking(1); // signal core 0 that handler is ready
    while (true) tight_loop_contents();
}

// ***************************************************************************
// **                                 SETUP                                 **
// ***************************************************************************

void SetupBoard() {
    board_init();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    gpio_init(LED_PIN2);
    gpio_set_dir(LED_PIN2, GPIO_OUT);
    // gpio_put(LED_PIN2, 1);

}

// ***************************************************************************
// **                                  MAIN                                 **
// ***************************************************************************

int main() {
    // flash_init() must be the very first call — it locates the FS partition
    // and reads/writes the mode byte.
    flash_init();

    bool run_script = true;

    // Read saved mode BEFORE USB init so the correct descriptor is served
    // from the very first enumeration attempt.
    mode = flash_read_mode();
    g_monome_mode = mode;

    SetupBoard();

    // Start USB early — the host can begin enumeration while NeoTrellis
    // I2C init runs.  Uses tud_init (not tusb_init) to match iii-grid-one.
    tud_init(BOARD_TUD_RHPORT);

    // Pump tud_task() during the ~2s of NeoTrellis I2C init delays so the
    // host can finish CDC enumeration before iii_loop() starts.
    neotrellis_set_poll_callback(tud_task);

    device_init(); // NeoTrellis hardware init (includes mode_check)

    // Start core 1 as a multicore lockout victim before any LFS flash
    // operations (lfs_format, lfs_prog, lfs_erase). Core 1 does nothing
    // else; we wait for it to signal that its lockout handler is registered.
    multicore_launch_core1(core1_entry);
    multicore_fifo_pop_blocking(); // wait until core 1 handler is ready

    if (mode == 0) {
        gpio_put(LED_PIN2, 1);  // blue
        iii_loop(run_script); // never returns; calls device_task() internally
    } else {
        gpio_put(LED_PIN, 1);   // red
        device_monome_loop(); // never returns
    }

    return 0;
}
#include "iii.h"
#include "device.h"
#include "flash.h"
#include "fs.h"
#include "metro.h"
#include "midi.h"
#include "pico/stdlib.h"
#include "repl.h"
#include "serial.h"
#include "tusb.h"
#include "vm.h"

static void usb_pump_ms(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    tud_task();
    serial_task();
    midi_task();
    sleep_ms(1);
  }
}

void iii_loop(bool run_script) {
  // Keep USB responsive during startup so host enumeration completes before
  // potentially long Lua initialization.
  usb_pump_ms(50);

  // must come after flash_init()
  fs_init();
  usb_pump_ms(20);

  // !!! LFS fails to mount without this arbitrary delay;
  // !!! would be nice to know why / make it programmatic
  usb_pump_ms(50);

  fs_mount();
  usb_pump_ms(20);

  metro_init();
  repl_init();
  midi_set_rx_callback(vm_handle_midi);
  serial_set_rx_callback(repl_handle_bytes);

  // Give the host a chance to fully configure interfaces before running any
  // user script that could take a long time.
  usb_pump_ms(1200);

  vm_init(run_script);

  while (1) {
    tud_task();
    midi_task();
    metro_task();
    serial_task();
    device_task();
  }
}

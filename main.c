#include "main.h"

int main() {
  mcu_initialize();
  tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE,
                                 .speed = TUSB_SPEED_AUTO};
  tusb_init(BOARD_TUD_RHPORT, &dev_init);
  while (1) {
    tud_task();
    usb_task();
  }

  return 0;
}

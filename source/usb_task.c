#include "main.h"

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
struct input_state current_state = {0};
bool ktriggered = false;
bool mtriggered = false;
uint8_t delay_counter = 0;
void usb_task(void) {
  if (is_parse_need) {
    memcpy((void*)&current_state, (void*)Rxbuf, RXBS);
    is_parse_need = false;
    ktriggered = false;
    mtriggered = false;
    delay_counter = current_state.delay;
  }
  // Poll every 10ms
  const uint32_t interval_10ms = 1;
  static uint32_t start_counter = 0;

  if (counter - start_counter < interval_10ms) return;  // not enough time
  start_counter += interval_10ms;
  delay_counter = (delay_counter == 0) ? 0 : delay_counter - interval_10ms;

  if (tud_suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  } else {
    // keyboard interface
    if (tud_hid_n_ready(ITF_NUM_KEYBOARD)) {
      uint8_t const report_id = 0;
      if (((current_state.device == 0 || current_state.device == 2) &&
           !ktriggered && delay_counter == 0) ||
          (current_state.kmode == 2 && delay_counter == 0)) {
        tud_hid_n_keyboard_report(ITF_NUM_KEYBOARD, report_id,
                                  current_state.kreport.modifier,
                                  current_state.kreport.keycode);
        if (current_state.kmode != 1) ktriggered = true;
      } else {
        tud_hid_n_keyboard_report(ITF_NUM_KEYBOARD, report_id, 0, NULL);
      }
    }

    // mouse interface
    if (tud_hid_n_ready(ITF_NUM_MOUSE)) {
      uint8_t const report_id = 0;
      if (((current_state.device == 1 || current_state.device == 2) &&
           !mtriggered && delay_counter == 0) ||
          ((current_state.mmode == 2 && delay_counter == 0))) {
        tud_hid_n_mouse_report(
            ITF_NUM_MOUSE, report_id, current_state.mreport.buttons,
            current_state.mreport.x, current_state.mreport.y,
            current_state.mreport.vertical, current_state.mreport.horizon);
        if (current_state.mmode != 1) mtriggered = true;
      } else {
        tud_hid_n_mouse_report(ITF_NUM_MOUSE, report_id, 0, 0, 0, 0, 0);
      }
    }
  }
  if (current_state.delay != 0 && delay_counter == 0)
    delay_counter = current_state.delay;
}
void tud_mount_cb(void) { blink_interval_ms = BLINK_MOUNTED; }

// Invoked when device is unmounted
void tud_umount_cb(void) { blink_interval_ms = BLINK_NOT_MOUNTED; }

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

// Invoked when received SET_PROTOCOL request
// protocol is either HID_PROTOCOL_BOOT (0) or HID_PROTOCOL_REPORT (1)
void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {
  (void)instance;
  (void)protocol;

  // nothing to do since we use the same compatible boot report for both Boot
  // and Report mode.
  // TODO set a indicator for user
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report,
                                uint16_t len) {
  (void)instance;
  (void)report;
  (void)len;

  // nothing to do
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize) {
  (void)report_id;
}
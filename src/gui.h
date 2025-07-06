#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define GUI_MAX_DEVICE_SCAN (16) // Maximum number of subdevices to scan
#define GUI_STATUS_Y        (23) // Y position for status messages

void gui_init(void);
void gui_showProgramCaption(void);
void gui_keyboardClickSound(bool isEnabled);
char* gui_screensafeString(char* str, size_t strSize);
int gui_getBootScanParameters(int* scanFrom);
int gui_getBootSubdeviceID(int scanFirstDevice, int scanLastDevice); // Get the subdevice ID, scanning up to scanLastDevice devices

#endif // GUI_H

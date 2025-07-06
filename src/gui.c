#include <atari.h>
#include <conio.h>
#include "gui.h"
#include "sioboot.h"
#include <string.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b)) // Macro to get the minimum of two values
#define MAX(a, b) ((a) > (b) ? (a) : (b)) // Macro to get the maximum of two values
#define RESTRICT(value, minValue, maxValue)  ((value) < (minValue) ? (minValue) : ((value) > (maxValue) ? (maxValue) : (value))) // Macro to restrict a value within a range
#define WRAPAROUND(value, minValue, maxValue) ((value) > (maxValue) ? (minValue) : (value)) // Macro to wrap a value around a maximum value

char sioString[128]; // Buffer for strings over SIO
const char invalidScreenChars[16] = {27,28,29,30,31,125,126,127,155,156,157,158,159,253,254,255}; // Characters that are not valid for screen display

void gui_init(void) {
    // Initialize the GUI, set up colors, etc.
    gui_keyboardClickSound(false); // Disable keyboard click sound
    cursor(false); // Hide the cursor
    revers(false); // Disable reverse video mode
    bordercolor(COLOR_BLUE);
    bgcolor(COLOR_BLUE);
    textcolor(COLOR_WHITE);
    clrscr();
}

void gui_resetScreensaver(void) {
    // Reset the screensaver by clearing Attract counter
    OS.atract = 0; // Reset the attract mode counter
}

void gui_showProgramCaption(void) {
    gotoxy(0, 0); // Move cursor to the top left corner
    revers(false); // Disable reverse video mode
    cputs("         \x8A");
    revers(true); // Disable reverse video mode
    cputs(" SIO Bootloader v1.0 ");
    revers(false); // Disable reverse video mode
    cputs("\x88        \n");
}

void gui_keyboardClickSound(bool isEnabled) {
    // Enable or disable keyboard click sound
    if (isEnabled) {
        OS.noclik = 0x00; // Enable keyboard click
    } else {
        OS.noclik = 0xFF; // Disable keyboard click
    }
}

int gui_getBootScanParameters(int* scanFrom) {
    int leftMargin = wherex(); // Get the current X position for left margin
    int topMargin = wherey(); // Get the current Y position for top margin
    int scanTo = 0; // Initialize automatic scan
    int scanFromLocal = 0; // Temp variable for scanFrom
    bool fromIsSelected = false; // Flag to check if scanFrom is selected
    bool needRefresh = true; // Flag to indicate if the screen needs to be refreshed
    uint8_t tmStart;
    uint8_t consoleKeys = 0, consoleKeysPrev = 3; // Variables to store console keys pressed

    gotoxy(leftMargin, GUI_STATUS_Y); // Move cursor to the status line
    printf("\x9c\xD3\xC5\xCC\xC5\xC3\xD4 param,change \xCF\xD0\xD4\xC9\xCF\xCE value \x1B\x1F\xD3\xD4\xC1\xD2\xD4"); // Print scanning message for the subdevice
    gotoxy(leftMargin, topMargin); // Restore cursor position
    while ((consoleKeys = GTIA_READ.consol)) {
        if (consoleKeys < 7) { // Check if any console key is pressed
            if (CONSOL_SELECT(consoleKeys) && !CONSOL_SELECT(consoleKeysPrev)) {
                needRefresh = true; // Set refresh flag if any console key is pressed
                fromIsSelected = !fromIsSelected; // Toggle selection of menu
            }
            if (CONSOL_OPTION(consoleKeys) && !CONSOL_OPTION(consoleKeysPrev)) {
                needRefresh = true; // Set refresh flag if any console key is pressed
                if (fromIsSelected) {
                    ++scanFromLocal; // Increment scanFromLocal
                    scanFromLocal = WRAPAROUND(scanFromLocal, 0, GUI_MAX_DEVICE_SCAN - 1); // Increment scanTo, wrapping around if it exceeds the maximum
                } else {
                    ++scanTo; // Increment scanTo
                    scanTo = WRAPAROUND(scanTo, 0, GUI_MAX_DEVICE_SCAN - 1); // Increment scanTo, wrapping around if it exceeds the maximum
                }
            }
            needRefresh |= CONSOL_START(consoleKeys);
        }
        if (needRefresh) {
            needRefresh = false; // Reset refresh flag
            gui_resetScreensaver(); // Reset screensaver delay
            gotox(leftMargin); // Move cursor to the left margin
            revers(false); // Disable reverse video mode
            cputs("Scan bootID from :");
            revers(fromIsSelected || CONSOL_START(consoleKeys));
            cprintf(" %2d ", scanFromLocal); // Prompt for scan start
            revers(false); // Disable reverse video mode
            cputs(" to :");
            revers(!fromIsSelected || CONSOL_START(consoleKeys));
            if (scanTo) {
                cprintf(" %6d ", scanTo); // Prompt for scan end
            } else {
                cprintf(" <AUTO> "); // Prompt for scan end
            }
        }
        consoleKeysPrev = consoleKeys; // Store previous console keys state
        if (CONSOL_START(consoleKeys)) {
            break; // Exit the loop if START key is pressed
        }
    }
    
    gotoxy(leftMargin, topMargin+2); // Return to original position
    if (scanFrom) {
        *scanFrom = scanFromLocal;
    }
    return scanTo;
}

// Function to get the subdevice ID, scanning minimum scanNumDevices devices
// Returns the subdevice ID if selected by user, or -1 if no valid subdevice ID is selected
int gui_getBootSubdeviceID(int scanFirstDevice, int scanLastDevice) {
    int i;
    int leftMargin = wherex(); // Get the current X position for left margin
    int topMargin = wherey(); // Get the current Y position for top margin
    int selectedID = -1; // Initialize selected ID to -1
    uint8_t errorCode = 0; 
    char keyboardKey;
    int subdeviceList[GUI_MAX_DEVICE_SCAN]; // Array to store found subdevice IDs

    scanLastDevice = RESTRICT(scanLastDevice, scanLastDevice ? scanFirstDevice : 0, GUI_MAX_DEVICE_SCAN); // Restrict the number of devices to scan within the maximum limit

    gotoxy(leftMargin, GUI_STATUS_Y); // Move cursor to the status line
    printf("\x9cScanning", i); // Print scanning message for the subdevice
    gotoy(topMargin); // Move cursor to the top margin
    for (i=scanFirstDevice; i<16; ++i) {
        int nowY = wherey(); // Get the current Y position
        gotoxy(leftMargin, GUI_STATUS_Y); // Move cursor to the status line
        printf("Scanning chip ID %d...", i); // Print scanning message for the subdevice
        gotoxy(leftMargin, nowY); // Return to the original position
        sioboot_setSubdeviceID(i); // Set the subdevice ID for the subsequent bootloader calls
        if (!(errorCode = sioboot_getStringDescriptor(sioString, sizeof(sioString), 0))) { // Get the FW descriptive string 
            printf("%c %s\n", i+'A'+128, gui_screensafeString(sioString, strlen(sioString))); // Print the subdevice ID and the firmware descriptor
            subdeviceList[i] = i; // Store the found subdevice ID
        } else {
            subdeviceList[i] = -1; // If no device found, store -1
            if (scanLastDevice && i) {
                if (errorCode != SIOBOOT_ERR_NOERROR) {
                    printf("%c<%s> (ERR:%d)\n", i+'A', sioboot_getCommErrorString(errorCode), errorCode+1); // Print timeout message
                }
            }
        }
        if (kbhit()) { // Check if a key is pressedq
            break;  // Exit the scanning loop if a key is pressed
        }
        if (scanLastDevice) {
            if (i >= scanLastDevice) { // If the number of scanned devices exceeds the limit
                break; // Exit the scanning loop
            }
        } else {
            if (errorCode) {
                break; // Exit the scanning loop
            }
        }
    }
    gotoxy(leftMargin, GUI_STATUS_Y); // Move cursor to the status line
    printf("\x9cSelect \xE2\xEF\xEF\xF4\xC9\xC4 or press \xC5\xD3\xC3...", i); // Print message
    keyboardKey = cgetc(); // Get the pressed key
    selectedID = -1; // Set selected ID to -1 to indicate no selection
    if ((keyboardKey >= KEY_A) && (keyboardKey < (KEY_A + sizeof(subdeviceList)/sizeof(subdeviceList[0])))) { // If a detected subdevice key is pressed
            selectedID = keyboardKey - KEY_A; // Calculate the selected subdevice ID
            selectedID = subdeviceList[selectedID]; // Return the selected subdevice ID
    }
    gotoxy(leftMargin, topMargin); // Return to original position
    return selectedID; // Return -1 if no valid subdevice ID is found
}

// Function to replace screen control characters in a string with display safe characters
// This function modifies the string in place and returns it
char* gui_screensafeString(char* str, size_t strSize) {
    int i, j;
    for (i = 0; i < strSize; ++i) {
        for (j = 0; j < sizeof(invalidScreenChars); ++j) {
            if (str[i] == invalidScreenChars[j]) {
                str[i] = '?'; // Replace invalid characters with valid character
            }
        }
    }
    return str; // Return the modified string
}


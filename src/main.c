#include <atari.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gui.h"
#include "sioboot.h"

int main (void)
{
    int subdevice_ID = 0;
    int fromDeviceID; // Default scan start device ID
    gui_init(); // Initialize the GUI
    while (1)
    {
        clrscr(); // Clear the screen
        gui_showProgramCaption(); // Show the program caption
        fromDeviceID = 0;
        subdevice_ID = gui_getBootScanParameters(&fromDeviceID); // Get the boot scan parameters
        if (kbhit()) { // Check if a key is pressed
            cgetc(); // Clear the keyboard buffer
        }
        gotox(0);
        subdevice_ID = gui_getBootSubdeviceID(fromDeviceID, subdevice_ID); // Get the subdevice ID
    }
    
    return EXIT_SUCCESS;
}
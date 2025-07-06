#include <atari.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "sioboot.h"

static uint8_t sioboot_subdeviceID = 0; // Default subdevice ID

// Call the SIO vector to perform a device command
// Returns nonzero if error, zero if success
// Input is in the OS.dcb structure
/*
01 (001) -- OPERATION COMPLETE (NO ERRORS)
8A (138) -- DEVICE TIMEOUT (DOESN'T RESPOND)
8B (139) -- DEVICE NAK
8C (140) -- SERIAL BUS INPUT FRAMING ERROR
8E (142) -- SERIAL BUS DATA FRAME OVERRUN ERROR
8F (143) -- SERIAL BUS DATA FRAME CHECKSUM ERROR
90 (144) -- DEVICE DONE ERROR
*/
static uint8_t SIOV(void)
{
    __asm__ ("JSR $E459");
    return (uint8_t)(OS.dcb.dstats - 1);
}

// Call SIOV with parameters for bootloader commands
// data: Pointer to the data buffer for read/write operations
// numBytes: Number of bytes to read/write
// isRead: true for read operation, false for write operation
// commandID: Command ID for the bootloader operation
// commandParam: Command parameter for the bootloader operation
// bootDeviceID: Subdevice ID for the bootloader
// Returns: false if the SIO operation completed successfully, SIO errorcode-1 otherwise 
static bool sioboot_callSIOV(void* data, uint16_t numBytes, bool isRead, uint8_t commandID, uint8_t commandParam) {
    OS.dcb.ddevic = SIOBOOT_DEVICE_ID; // Device ID for bootloader
    OS.dcb.dunit  = 0x01;
    OS.dcb.dcomnd = commandID;
    OS.dcb.dstats = isRead ? 0x40 : 0x00; // Set status for read or write
    OS.dcb.dbuf   = data; // Pointer to the data buffer
    OS.dcb.dtimlo = 5;
    OS.dcb.dunuse = 0;
    OS.dcb.dbyt   = numBytes; // Number of bytes to transfer
    OS.dcb.daux1  = sioboot_subdeviceID; // Auxiliary byte 1
    OS.dcb.daux2  = commandParam; // Auxiliary byte 1
   return SIOV(); // Call SIO vector and return false if the SIO operation completed successfully, SIO errorcode-1 otherwise
}

void sioboot_setSubdeviceID(uint8_t bootDeviceID) {
    sioboot_subdeviceID = bootDeviceID; // Set the subdevice ID in the auxiliary byte
}

// Get a string descriptor from the bootloader
// buffer: Pointer to store the string descriptor (ZSTR)
// bufferSize: Size of the buffer in bytes
// stringID: ID of the string descriptor to retrieve    
// Returns: false if the SIO operation completed successfully, SIO errorcode-1 otherwise
bool sioboot_getStringDescriptor(char* strBuf, uint16_t strBufSize, uint8_t blStringID) {
    bool retVal;
    strBufSize = strBuf ? strBufSize : 0;
    retVal = sioboot_callSIOV((void*)strBuf, strBufSize, true, SIOBOOT_GETSTR, blStringID);
    if (strBuf && strBufSize) {
        // Ensure the string is null-terminated
        strBuf[retVal ? 0 : strBufSize - 1] = '\0';
    }
    return retVal; // Return true if the command completed with error
}

// Get a compatibility descriptor from the bootloader
// buffer: Pointer to store the compatibility descriptor 
// descriptorID: ID of the compatibility descriptor to retrieve    
// Returns: false if the SIO operation completed successfully, SIO errorcode-1 otherwise
bool sioboot_getCompatibilityDescriptor(compat_descriptor_t* buffer,uint8_t descriptorID) {
    return sioboot_callSIOV(buffer, sizeof(compat_descriptor_t), true, SIOBOOT_GETCOMPAT, descriptorID); // Return true if the command completed with error
}

const char* sioboot_getCommErrorString(uint8_t errorCode) {
    switch (errorCode) {
        case SIOBOOT_ERR_TIMEOUT: return "Timeout";
        case SIOBOOT_ERR_NAK: return "Boot refused";
        default: return "Comm Error";
    }
}
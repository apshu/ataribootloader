#ifndef SIOBOOT_H
#define SIOBOOT_H

#include <stdint.h>
#include <stdbool.h>

#define SIOBOOT_GETSTR (0x01) // Command ID for getting string descriptor
#define SIOBOOT_GETCOMPAT (0x02) // Command ID for getting compatibility descriptors
#define SIOBOOT_DEVICE_ID (0xFE) // Device ID for the bootloader

// Error codes for sioboot operations
#define SIOBOOT_ERR_NOERROR (001 - 1)       // -- OPERATION COMPLETE (NO ERRORS)
#define SIOBOOT_ERR_TIMEOUT (138 - 1)       // -- DEVICE TIMEOUT (DOESN'T RESPOND)
#define SIOBOOT_ERR_NAK     (139 - 1)       // -- DEVICE NAK
#define SIOBOOT_ERR_FERR    (140 - 1)       // -- SERIAL BUS INPUT FRAMING ERROR
#define SIOBOOT_ERR_OVERRUN (142 - 1)       // -- SERIAL BUS DATA FRAME OVERRUN ERROR
#define SIOBOOT_ERR_CSUM    (143 - 1)       // -- SERIAL BUS DATA FRAME CHECKSUM ERROR
#define SIOBOOT_ERR_DONEERR (144 - 1)       // -- DEVICE DONE ERROR

typedef union {
    uint8_t asBytes[128]; // Structure as generic data buffer
    struct {
        uint8_t bootloaderVersion; // Bootloader version
        uint8_t DEVID[16];
        uint8_t DEVIDmask[16];
        uint8_t DIA[32];
        uint8_t DIAmask[32];
        uint32_t configHEXfileAddress; // Address of the configuration in HEX file
    } asDEVID;
    struct {
        uint8_t config[64]; // Configuration bits 
        uint8_t configMask[64]; // Configuration bits mask
    } asCONFIGBITS;
} compat_descriptor_t;

void sioboot_setSubdeviceID(uint8_t bootDeviceID); // Set the subdevice ID for the bootloader
bool sioboot_getStringDescriptor(char* buffer, uint16_t bufferSize, uint8_t stringID);
bool sioboot_getCompatibilityDescriptor(compat_descriptor_t* buffer,uint8_t descriptorID);
const char* sioboot_getCommErrorString(uint8_t errorCode); // Get a string representation of the communication error code

#endif // SIOBOOT_H

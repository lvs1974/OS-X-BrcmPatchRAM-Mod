/*
 *  Released under "The GNU General Public License (GPL-2.0)"
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#ifndef __BrcmPatchRAM__
#define __BrcmPatchRAM__

#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>

#include "BrcmFirmwareStore.h"
#include "USBDeviceShim.h"

#define kDisplayName "DisplayName"
#define kBundleIdentifier "CFBundleIdentifier"
#define kIOUSBDeviceClassName "IOUSBDevice"
#ifndef kIOUSBHostDeviceClassName
#define kIOUSBHostDeviceClassName "IOUSBHostDevice"
#endif
#define kAppleBundlePrefix "com.apple."
#define kFirmwareKey "FirmwareKey"
#define kFirmwareLoaded "RM,FirmwareLoaded"

enum DeviceState
{
    kUnknown,
    kInitialize,
    kFirmwareVersion,
    kMiniDriverComplete,
    kInstructionWrite,
    kInstructionWritten,
    kFirmwareWritten,
    kResetComplete,
    kUpdateComplete,
    kUpdateNotNeeded,
    kUpdateAborted,
};

#define BrcmPatchRAM BrcmPatchRAM2

extern "C"
{
kern_return_t BrcmPatchRAM_Start(kmod_info_t*, void*);
kern_return_t BrcmPatchRAM_Stop(kmod_info_t*, void*);
}

class BrcmPatchRAM : public IOService
{
private:
    typedef IOService super;

    OSDeclareDefaultStructors(BrcmPatchRAM2);
    
    UInt16 mVendorId;
    UInt16 mProductId;
    
    USBDeviceShim mDevice;
    USBInterfaceShim mInterface;
    USBPipeShim mInterruptPipe;
    USBPipeShim mBulkPipe;
    BrcmFirmwareStore* mFirmwareStore = NULL;

    
    USBCOMPLETION mInterruptCompletion;
    IOBufferMemoryDescriptor* mReadBuffer;
    
    volatile DeviceState mDeviceState = kInitialize;
    volatile uint16_t mFirmwareVersion = 0xFFFF;
    IOLock* mCompletionLock = NULL;
    
    enum { iMaxCompletionRetries = 10 };
    int mCompletionCount = iMaxCompletionRetries;
    static UInt16 mLoopCounter;
    static uint64_t mLastTime;
    
#ifdef DEBUG
    static const char* getState(DeviceState deviceState);
#endif
    static OSString* brcmBundleIdentifier;
    static OSString* brcmIOClass;
    static OSString* brcmProviderClass;
    static void initBrcmStrings();

    void printPersonalities();
    void publishPersonality();

    bool publishResourcePersonality(const char* classname);
    BrcmFirmwareStore* getFirmwareStore();
    void uploadFirmware();
    
    void printDeviceInfo();
    int getDeviceStatus();
    
    bool resetDevice();
    bool setConfiguration(int configurationIndex);
    
    bool findInterface(USBInterfaceShim* interface);
    bool findPipe(USBPipeShim* pipe, uint8_t type, uint8_t direction);
    
    bool continuousRead();

    static void readCompletion(void* target, void* parameter, IOReturn status, uint32_t bytesTransferred);
    
    IOReturn hciCommand(void * command, uint16_t length);
    IOReturn hciParseResponse(void* response, uint16_t length, void* output, uint8_t* outputLength);
    
    IOReturn bulkWrite(const void* data, uint16_t length);
    
    uint16_t getFirmwareVersion();
    
    bool performUpgrade();
public:
    virtual IOService* probe(IOService *provider, SInt32 *probeScore) override;
    virtual bool start(IOService *provider) override;
    virtual void free() override;
    virtual const char* stringFromReturn(IOReturn rtn) override;
};

#define kBrcmPatchRAMResidency "BrcmPatchRAMResidency"
class BrcmPatchRAMResidency : public IOService
{
private:
    typedef IOService super;
    OSDeclareDefaultStructors(BrcmPatchRAMResidency);

public:
    virtual bool start(IOService *provider);
};


#endif //__BrcmPatchRAM__

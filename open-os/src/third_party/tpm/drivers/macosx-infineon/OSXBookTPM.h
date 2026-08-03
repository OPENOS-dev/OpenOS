// Copyright (C) 2006, Amit Singh <http://osxbook.com>
// Released under GPLv2.

#ifndef _OSXBOOK_TPM_H_
#define _OSXBOOK_TPM_H_

#include <libkern/libkern.h>
#include <libkern/locks.h>

#include <IOKit/assert.h>
#include <IOKit/IOLib.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/pwr_mgt/RootDomain.h>

class com_osxbook_driver_InfineonTPM : public IOService
{
    OSDeclareDefaultStructors(com_osxbook_driver_InfineonTPM)

private:
    IOPCIDevice          *nub;
    IOPMrootDomain       *pmRootDomain;
    IOACPIPlatformDevice *ad;

    IOMemoryMap *mmio_addr;
    IOMemoryMap *mmio_base;
    UInt32 TPM_INF_ADDR;
    UInt32 TPM_INF_ADDR_LEN;
    UInt32 TPM_INF_DATA;
    UInt32 TPM_INF_BASE;
    UInt32 TPM_INF_PORT_LEN;

    UInt8 inb(UInt32 port);
    void  outb(UInt8 value, UInt32 port);
    int   emptyFifo(int clearWRFifo);
    int   doWait(int wait_for_bit);
    void  doWaitAndSend(UInt8 sendByte);
    void  doWtx(void);
    void  doWtxAbort(void);
    int   doRecv(UInt8 *buf, size_t count, ssize_t *out);
    int   doSend(UInt8 *buf, size_t count, ssize_t *out);

    void printRegisters(void);

    UInt8 tpmStatus(void);
    void  tpmCancel(void);
    void  tpmStartup(void);

    lck_grp_t *tpmLockGroup;
    lck_mtx_t *bufferMutex;
    lck_mtx_t *tpmMutex;

public:
    int tpmTransmit(UInt8 *buf, size_t bufsiz, ssize_t *out);

    volatile UInt32 dataPending;

    inline void lockBuffer(void)   { lck_mtx_lock(bufferMutex); }
    inline void unlockBuffer(void) { lck_mtx_unlock(bufferMutex); }
    inline void lockTPM(void)      { lck_mtx_lock(tpmMutex); }
    inline void unlockTPM(void)    { lck_mtx_unlock(tpmMutex); }

    virtual bool       init(OSDictionary *dictionary = 0);
    virtual void       free(void);
    virtual IOService *probe(IOService *provider, SInt32 *score);
    virtual bool       start(IOService *provider);
    virtual void       stop(IOService *provider);
    virtual bool       attach(IOService *provider);
    virtual void       detach(IOService *provider);
    virtual IOReturn   powerStateDidChangeTo(IOPMPowerFlags, unsigned long,
                                             IOService *);
};

#endif /* _OSXBOOK_TPM_H_ */

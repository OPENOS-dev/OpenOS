// Copyright (C) 2006, Amit Singh <http://osxbook.com>
// Released under GPLv2.
// Borrows code from the Linux kernel Infineon TPM driver. 

#include "OSXBookTPM.h"
// #define DEBUG 1

extern "C" {
#include "InfineonTPM.h"

#include <libkern/OSTypes.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/conf.h>
#include <miscfs/devfs/devfs.h>
}

int tpm_open(dev_t dev, int flags, int devtype, struct proc *p);
int tpm_close(dev_t dev, int flags, int devtype, struct proc *p);
int tpm_read(dev_t dev, struct uio *uio, int ioflag);
int tpm_write(dev_t dev, struct uio *uio, int ioflag);
int tpm_ioctl(dev_t dev, u_long cmd, caddr_t data, int fflag, struct proc *p);
    
#define nullselect (d_select_t *)&nulldev
#define nullstop   (d_stop_t *)&nulldev
#define nullreset  (d_reset_t *)&nulldev

#define TPM_NAME  "tpm"
#define TPM_MINOR 0
#define TPM_ALLOW_MULTIPLE_OPENS 1 // for testing

static void *tpm_dev       = NULL;
static int   tpm_dev_index = -1;
static int   is_open       = 0;

static UInt8 tpmData[TPM_BUFSIZE] = { 0 };
static int   number_of_wtx = 0;

static com_osxbook_driver_InfineonTPM *instance = NULL;

static struct cdevsw tpm_dev_cdevsw = {
    tpm_open,   // d_open
    tpm_close,  // d_close
    tpm_read,   // d_read
    tpm_write,  // d_write
    eno_ioctl,  // d_ioctl
    nullstop,   // d_stop
    nullreset,  // d_stop
    0,          // d_ttys
    nullselect, // d_select
    eno_mmap,   // d_mmap
    eno_strat,  // d_strategy
    eno_getc,   // d_getc
    eno_putc,   // d_putc
    D_TTY,      // d_type
};

int
tpm_open(dev_t dev, int flags, int devtype, struct proc *p)
{
    if (!instance) {
        IOLog("no instance in open?\n");
        return EIO;
    }

    if (TPM_ALLOW_MULTIPLE_OPENS) {
        return KERN_SUCCESS;
    }

    instance->lockTPM();

    if (is_open) {
        instance->unlockTPM();
        return EBUSY;
    }

    is_open = 1;

    instance->unlockTPM();

    return KERN_SUCCESS;
}  

int
tpm_close(dev_t dev, int flags, int devtype, struct proc *p)
{
    if (!instance) {
        IOLog("no instance in close?\n");
        return EIO;
    }

    instance->lockTPM();
    is_open = 0;
    instance->unlockTPM();

    return KERN_SUCCESS;
}

int     
tpm_ioctl(dev_t dev, u_long cmd, caddr_t data, int fflag, struct proc *p)
{
    return ENODEV;
}     

int 
tpm_read(dev_t dev, struct uio *uio, int ioflag)
{       
    int ret, bytesToRead;

    if (!instance) {
        return EIO;
    }

    bytesToRead = instance->dataPending; 
    instance->dataPending = 0;

    if (bytesToRead <= 0) {
        return 0;
    }

    if (bytesToRead > uio_resid(uio)) {
        bytesToRead = uio_resid(uio);
    }

    instance->lockBuffer();
    ret = uiomove((char *)tpmData, bytesToRead, uio);
    instance->unlockBuffer();

    return ret;
}
    
int                           
tpm_write(dev_t dev, struct uio *uio, int ioflag)
{                             
    ssize_t len;
    int ret = 0, bytesToWrite = 0, maxWaitCounter = 0;

    if (!instance) {
        return EIO;
    }

    // This should be replaced by bsd_timeout() or something similar.
    while (instance->dataPending != 0) {
        IOSleep(TPM_TIMEOUT);
        maxWaitCounter++;
        if (maxWaitCounter > 120) {
            instance->lockBuffer();
            instance->dataPending = 0;
            memset(tpmData, 0, TPM_BUFSIZE);
            instance->unlockBuffer();
        }
    }

    instance->lockBuffer();

    bytesToWrite = min(uio_resid(uio), TPM_BUFSIZE);

    ret = uiomove((char *)tpmData, bytesToWrite, uio);
    if (ret != 0) {
        instance->unlockBuffer();
        return ret;
    }

    ret = instance->tpmTransmit(tpmData, TPM_BUFSIZE, &len);
    if (ret == 0) {
        instance->dataPending = len;
    }

    instance->unlockBuffer();

    return ret;
}
    
// Our superclass
#define super IOService

OSDefineMetaClassAndStructors(com_osxbook_driver_InfineonTPM, IOService)

// Our makeshift inb/outb implementation.

UInt8
com_osxbook_driver_InfineonTPM::inb(UInt32 port)
{
    if (port >= TPM_INF_BASE) {
        return ad->ioRead8(port - TPM_INF_BASE, mmio_base);
    } else {
        return ad->ioRead8(port - TPM_INF_ADDR, mmio_addr);
    }
}

void
com_osxbook_driver_InfineonTPM::outb(UInt8 value, UInt32 port)
{
    if (port >= TPM_INF_BASE) {
        ad->ioWrite8(port - TPM_INF_BASE, value, mmio_base);
    } else {
        ad->ioWrite8(port - TPM_INF_ADDR, value, mmio_addr);
    }
}

UInt8
com_osxbook_driver_InfineonTPM::tpmStatus(void)
{
    return inb(TPM_INF_BASE + STAT);
}

void
com_osxbook_driver_InfineonTPM::tpmCancel(void)
{
    // nothing
}

bool
com_osxbook_driver_InfineonTPM::init(OSDictionary *dict)
{
    super::init(dict);
    
    tpm_dev_index = cdevsw_add(-1, &tpm_dev_cdevsw);
    if (tpm_dev_index == -1) {
        IOLog("cdevsw_add failed\n");
        return KERN_FAILURE;
    }

    tpm_dev = devfs_make_node(makedev(tpm_dev_index, TPM_MINOR),
                              DEVFS_CHAR,
                              UID_ROOT,
                              GID_WHEEL,
                              0600,
                              TPM_NAME);
    if (tpm_dev == NULL) {
        IOLog("devfs_make_node failed\n");
        (void)cdevsw_remove(tpm_dev_index, &tpm_dev_cdevsw);
        return KERN_FAILURE;
    }

    return true;
}

void
com_osxbook_driver_InfineonTPM::free(void)
{
    super::free();
    int ret = -1;

    if (tpm_dev != NULL) {
        devfs_remove(tpm_dev);
        tpm_dev = NULL;
    }

    if (tpm_dev_index != -1) {
        ret = cdevsw_remove(tpm_dev_index, &tpm_dev_cdevsw);
    }

    if (ret != tpm_dev_index) {
        return;
    }

    tpm_dev_index = -1;

    return;
}

IOService *
com_osxbook_driver_InfineonTPM::probe(IOService *provider, SInt32 *score)
{
    IOService *result = super::probe(provider, score);
    return result;
}

void
com_osxbook_driver_InfineonTPM::printRegisters(void)
{
    int status;
    status = inb(STAT);
    IOLog("Status register\n");
    if (status & 1<<STAT_RDA)
        IOLog("RDA  *\n");
    if (status & 1<<STAT_IRQA)
        IOLog("IRQA *\n");
    if (status & 1<<STAT_TOK)
        IOLog("TOK  *\n");
    if (status & 1<<STAT_FOK)
        IOLog("FOK  *\n");
    if (status & 1<<STAT_LPA)
        IOLog("LPA  *\n");
    if (status & 1<<STAT_XFE)
        IOLog("XFE  *\n");

    status = inb(CMD);
    IOLog("Command register\n");
    if (status & 1<<CMD_IRQC)
        IOLog("IRQC *\n");
    if (status & 1<<CMD_RES)
        IOLog("RES  *\n");
    if (status & 1<<CMD_LP)
        IOLog("LP   *\n");
    if (status & 1<<CMD_DIS)
        IOLog("DIS  *\n");
}

// One could argue that this method commits sort of a layering violation,
// but it's nice to have everything ready-to-go for testing when the driver
// is loaded.
void
com_osxbook_driver_InfineonTPM::tpmStartup(void)
{
    int junk_ret;
    ssize_t junk_out;

    UInt8 tpmStartup_data[] = {
        0, 193,       // TPM_TAG_RQU_COMMAND
        0, 0, 0, 12,
        0, 0, 0, 153, // TPM_ORD_Startup
        0, 1
    };

    UInt8 tpm_continueselftest_data[] = {
        0, 193,       // TPM_TAG_RQU_COMMAND
        0, 0, 0, 10,
        0, 0, 0, 83   // TPM_ORD_ContinueSelfTest
    };

    junk_ret = tpmTransmit(tpmStartup_data, sizeof(tpmStartup_data), &junk_out);
    instance->dataPending = 0;
    junk_ret = tpmTransmit(tpm_continueselftest_data,
                            sizeof(tpm_continueselftest_data), &junk_out);
    instance->dataPending = 0;
}

bool
com_osxbook_driver_InfineonTPM::start(IOService *provider)
{
    bool result = super::start(provider);

    tpmLockGroup = NULL;
    bufferMutex = NULL;
    tpmMutex = NULL;
    dataPending = 0;

    instance = this;

    ad = (IOACPIPlatformDevice *)provider;
    // ad->getDeviceStatus() if you are curious

    if (ad->getDeviceMemoryCount() < 2) {
        IOLog("fatal: unexpected memory count\n");
        return KERN_FAILURE;
    }

    mmio_addr = ad->mapDeviceMemoryWithIndex(0, 0);
    mmio_base = ad->mapDeviceMemoryWithIndex(1, 0);

    TPM_INF_ADDR = mmio_addr->getPhysicalAddress();
    TPM_INF_ADDR_LEN = mmio_addr->getLength();
    TPM_INF_DATA = (TPM_INF_ADDR + 1);
    TPM_INF_BASE = mmio_base->getPhysicalAddress();
    TPM_INF_PORT_LEN = mmio_base->getLength();

    setProperty("addr", TPM_INF_ADDR, 32);
    setProperty("addr-len", TPM_INF_ADDR_LEN, 32);
    setProperty("data", TPM_INF_DATA, 32);
    setProperty("base", TPM_INF_BASE, 32);
    setProperty("port-len", TPM_INF_PORT_LEN, TPM_INF_PORT_LEN);

    UInt8 *bytep;
    UInt16 version;
    UInt16 vendorid;
    UInt16 productid;

    bytep = (UInt8 *)&vendorid;
    outb(ENABLE_REGISTER_PAIR, TPM_INF_ADDR);
    outb(IDVENL, TPM_INF_ADDR);
    bytep++;
    *bytep = inb(TPM_INF_DATA);
    bytep--;
    outb(IDVENH, TPM_INF_ADDR);
    *bytep = inb(TPM_INF_DATA);

    bytep = (UInt8 *)&productid;
    outb(IDPDL, TPM_INF_ADDR);
    bytep++;
    *bytep = inb(TPM_INF_DATA);
    bytep--;
    outb(IDPDH, TPM_INF_ADDR);
    *bytep = inb(TPM_INF_DATA);

    bytep = (UInt8 *)&version;
    outb(CHIP_ID1, TPM_INF_ADDR);
    bytep++;
    *bytep = inb(TPM_INF_DATA);
    bytep--;
    outb(CHIP_ID2, TPM_INF_ADDR);
    *bytep = inb(TPM_INF_DATA);

    UInt32 what;

    /* ((productid[0] << 8) | productid[1]) */
    bytep = (UInt8 *)&productid;
    what = *bytep << 8;
    bytep++;
    what |= *bytep;

    switch (what) {
    case 6:
        setProperty("model", "SLD 9630 TT 1.1");
        break;
    case 11:
        setProperty("model", "SLB 9635 TT 1.2");
        break;
    default:
        setProperty("model", "Unknown");
        break;
    }

    /* ((vendorid[0] << 8 | vendorid[1]) */
    bytep = (UInt8 *)&vendorid;
    what = *bytep << 8;
    bytep++;
    what |= *bytep;

    if (what == (TPM_INFINEON_DEV_VEN_VALUE)) {

        UInt8 ioh, iol;

        /* configure TPM with IO-ports */
        outb(IOLIMH, TPM_INF_ADDR);
        outb(((TPM_INF_BASE >> 8) & 0xff), TPM_INF_DATA);
        outb(IOLIML, TPM_INF_ADDR);
        outb((TPM_INF_BASE & 0xff), TPM_INF_DATA);

        /* control if IO-ports are set correctly */
        outb(IOLIMH, TPM_INF_ADDR);
        ioh = inb(TPM_INF_DATA);
        outb(IOLIML, TPM_INF_ADDR);
        iol = inb(TPM_INF_DATA);

        if ((ioh << 8 | iol) != TPM_INF_BASE) {
            IOLog("I/O ports inconsistent\n");
            return KERN_FAILURE;
        }

        outb(TPM_DAR, TPM_INF_ADDR);
        outb(0x01, TPM_INF_DATA);
        outb(DISABLE_REGISTER_PAIR, TPM_INF_ADDR);

        /* disable RESET, LP and IRQC */
        outb(RESET_LP_IRQC_DISABLE, TPM_INF_BASE + CMD);

        char tmpbuf[8];
        sprintf(tmpbuf, "0x%04x", OSSwapBigToHostInt16(version));
        setProperty("version", tmpbuf);
        sprintf(tmpbuf, "0x%04x", OSSwapBigToHostInt16(vendorid));
        setProperty("vendor-id", tmpbuf);
        sprintf(tmpbuf, "0x%04x", OSSwapBigToHostInt16(productid));
        setProperty("product-id", tmpbuf);
    }

    // emptyFifo(1);

    tpmLockGroup = lck_grp_alloc_init("com.osxbook.driver.OSXBookTPM",
                                        LCK_GRP_ATTR_NULL);
    bufferMutex = lck_mtx_alloc_init(tpmLockGroup, LCK_ATTR_NULL);
    tpmMutex = lck_mtx_alloc_init(tpmLockGroup, LCK_ATTR_NULL);

    tpmStartup();

    IOService *service = waitForService(serviceMatching("IOPMrootDomain"));
    pmRootDomain = OSDynamicCast(IOPMrootDomain, service);
    if (pmRootDomain != 0) {
        pmRootDomain->registerInterestedDriver(this);
    }

    setProperty("info-url", "http://osxbook.com");

    return result;
}

int
com_osxbook_driver_InfineonTPM::emptyFifo(int clear_wrfifo)
{
    int i, status, check = 0;

    if (clear_wrfifo) {
        for (i = 0; i < 4096; i++) {
            status = inb(TPM_INF_BASE + WRFIFO);
            if (status == 0xff) {
                if (check == 5) {
                    break;
                } else {
                    check++;
                }
            }
        }
    }

    i = 0;
    do {
        status = inb(TPM_INF_BASE + RDFIFO);
        status = inb(TPM_INF_BASE + STAT);
        i++;
        if (i == TPM_MAX_TRIES) {
            return EIO;
        }
    } while ((status & (1 << STAT_RDA)) != 0);

    return 0;
}

int
com_osxbook_driver_InfineonTPM::doWait(int wait_for_bit)
{
    int i, status;

    for (i = 0; i < TPM_MAX_TRIES; i++) {
        status = inb(TPM_INF_BASE + STAT);
        /* check the status-register if wait_for_bit is set */
        if (status & 1 << wait_for_bit) {
            break;
        }
        IOSleep(TPM_MSLEEP_TIME);
    }

    if (i == TPM_MAX_TRIES) {       /* timeout occurs */
        if (wait_for_bit == STAT_XFE) {
            IOLog("Timeout in wait(STAT_XFE)\n");
        }
        if (wait_for_bit == STAT_RDA) {
            IOLog("Timeout in wait(STAT_RDA)\n");
        }
        return EIO;
    }

    return 0;
}

void
com_osxbook_driver_InfineonTPM::doWaitAndSend(UInt8 sendByte)
{
    doWait(STAT_XFE);
    outb(sendByte, TPM_INF_BASE + WRFIFO);
}

void
com_osxbook_driver_InfineonTPM::doWtx(void)
{
    number_of_wtx++;
    IOLog("Granting WTX (%02d / %02d)\n", number_of_wtx, TPM_MAX_WTX_PACKAGES);
    doWaitAndSend(TPM_VL_VER);
    doWaitAndSend(TPM_CTRL_WTX);
    doWaitAndSend(0x00);
    doWaitAndSend(0x00);
    IOSleep(TPM_WTX_MSLEEP_TIME);
}

void
com_osxbook_driver_InfineonTPM::doWtxAbort(void)
{
    IOLog("Aborting WTX\n");
    doWaitAndSend(TPM_VL_VER);
    doWaitAndSend(TPM_CTRL_WTX_ABORT);
    doWaitAndSend(0x00);
    doWaitAndSend(0x00);
    number_of_wtx = 0;
    IOSleep(TPM_WTX_MSLEEP_TIME);
}

int
com_osxbook_driver_InfineonTPM::doRecv(UInt8 *buf, size_t count, ssize_t *out)
{
    int i;
    int ret;
    UInt32 size = 0;
    number_of_wtx = 0;

    *out = 0;

recv_begin:
    /* start receiving header */
    for (i = 0; i < 4; i++) {
        ret = doWait(STAT_RDA);
        if (ret) {
            return EIO;
        }
        buf[i] = inb(TPM_INF_BASE + RDFIFO);
    }

    if (buf[0] != TPM_VL_VER) {
        IOLog("Wrong transport protocol implementation!\n");
        return EIO;
    }

    if (buf[1] == TPM_CTRL_DATA) {
        /* size of the data received */
        size = ((buf[2] << 8) | buf[3]);

        for (i = 0; i < size; i++) {
            doWait(STAT_RDA);
            buf[i] = inb(TPM_INF_BASE + RDFIFO);
        }

        if ((size == 0x6D00) && (buf[1] == 0x80)) {
            IOLog("Error handling on vendor layer!\n");
            return EIO;
        }

        for (i = 0; i < size; i++) {
            buf[i] = buf[i + 6];
        }

        size = size - 6;
        *out = size;

        return 0;
    }

    if (buf[1] == TPM_CTRL_WTX) {
        IOLog("WTX-package received\n");
        if (number_of_wtx < TPM_MAX_WTX_PACKAGES) {
            doWtx();
            goto recv_begin;
        } else {
            doWtxAbort();
            goto recv_begin;
        }
    }

    if (buf[1] == TPM_CTRL_WTX_ABORT_ACK) {
        IOLog("WTX-abort acknowledged\n");
        *out = size;
        return 0;
    }

    if (buf[1] == TPM_CTRL_ERROR) {
        IOLog("ERROR-package received:\n");
        if (buf[4] == TPM_INF_NAK) {
            IOLog("Negative acknowledgement - retransmit command!\n");
        }
        return EIO;
    }

    return EIO;
}

int
com_osxbook_driver_InfineonTPM::tpmTransmit(UInt8  *buf,
                                            size_t  bufsiz,
                                            ssize_t *out)
{
    int ret;
    ssize_t rc;
    UInt32 count;
    UInt32 ordinal;

    *out = 0;

    count = OSSwapBigToHostConstInt32(*((UInt32 *)(buf + 2)));
    ordinal = OSSwapBigToHostConstInt32(*((UInt32 *)(buf + 6)));

#ifdef DEBUG
    IOLog("transmit: count=%d ordinal=%x\n", count, ordinal);
#endif

    if (count == 0) {
        return ENODATA;
    }

    if (count > bufsiz) {
        IOLog("invalid count value %x %x \n", count, bufsiz);
        return E2BIG;
    }

    lockTPM();

    ret = doSend((UInt8 *) buf, (size_t)count, &rc);
    if (ret != 0) {
        goto out;
    }

    // For the Infineon chip, we don't need the irq/jiffies/while loop
    // that Linux's tpm_transmit does.

    ret = doRecv((UInt8 *) buf, bufsiz, &rc);
out:
    unlockTPM();

    *out = rc;

#if DEBUG
    IOLog("transmit: returning %d (rc = %d)\n", ret, rc);
#endif

    return ret;
}

int
com_osxbook_driver_InfineonTPM::doSend(UInt8 *buf, size_t count, ssize_t *out)
{
    int i, ret;
    UInt8 count_high, count_low, count_4, count_3, count_2, count_1;

    *out = 0;

    /* Disabling Reset, LP and IRQC */
    outb(RESET_LP_IRQC_DISABLE, TPM_INF_BASE + CMD);

    ret = emptyFifo(1);
    if (ret) {
        IOLog("Timeout while clearing FIFO\n");
        return EIO;
    }

    ret = doWait(STAT_XFE);
    if (ret) {
        IOLog("doWait returned error\n");
        return EIO;
    }

    count_4 = (count & 0xff000000) >> 24;
    count_3 = (count & 0x00ff0000) >> 16;
    count_2 = (count & 0x0000ff00) >> 8;
    count_1 = (count & 0x000000ff);
    count_high = ((count + 6) & 0xffffff00) >> 8;
    count_low = ((count + 6) & 0x000000ff);

    /* Sending Header */
    doWaitAndSend(TPM_VL_VER);
    doWaitAndSend(TPM_CTRL_DATA);
    doWaitAndSend(count_high);
    doWaitAndSend(count_low);

    /* Sending Data Header */
    doWaitAndSend(TPM_VL_VER);
    doWaitAndSend(TPM_VL_CHANNEL_TPM);
    doWaitAndSend(count_4);
    doWaitAndSend(count_3);
    doWaitAndSend(count_2);
    doWaitAndSend(count_1);

    /* Sending Data */
    for (i = 0; i < count; i++) {
        doWaitAndSend(buf[i]);
    }

    *out = count;

    return 0;
}

void
com_osxbook_driver_InfineonTPM::stop(IOService *provider)
{
    if (mmio_addr) {
        mmio_addr->release();
    }

    if (mmio_base) {
        mmio_base->release();
    }

    if (bufferMutex) {
        lck_mtx_free(bufferMutex, tpmLockGroup);
        bufferMutex = NULL;
    }

    if (tpmLockGroup) {
        lck_grp_free(tpmLockGroup);
        tpmLockGroup = NULL;
    }

    if (pmRootDomain != 0) {
        pmRootDomain->deRegisterInterestedDriver(this);
    }

    super::stop(provider);
}

bool
com_osxbook_driver_InfineonTPM::attach(IOService *provider)
{
    bool result = super::attach(provider);
    return result;
}

void
com_osxbook_driver_InfineonTPM::detach(IOService *provider)
{
    super::detach(provider);
}

IOReturn
com_osxbook_driver_InfineonTPM::powerStateDidChangeTo(IOPMPowerFlags theFlags,
                                                      unsigned long,
                                                      IOService *)
{
    if (theFlags & IOPMPowerOn) {
        tpmStartup();
    } else {
        // Well, a non-ON state really. Nothing to do for now.
    }

    return IOPMAckImplied;
}

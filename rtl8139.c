#include "types.h"
#include "defs.h"
#include "x86.h"

#define RX_BUF_LEN 8192;

// static unsigned char rx_ring[RX_BUF_LEN + 16] __attribute__((__aligned__(4)));

struct RTL8139_registers {
  uchar IDR0;                   // 0x00
  uchar IDR1;                   // 0x01
  uchar IDR2;                   // 0x02
  uchar IDR3;                   // 0x03
  uchar IDR4;                   // 0x04
  uchar IDR5;                   // 0x05
  ushort reservedi0;            // 0x06 - 0x07
  uchar MAR0;                   // 0x08
  uchar MAR1;                   // 0x09
  uchar MAR2;                   // 0x0A
  uchar MAR3;                   // 0x0B
  uchar MAR4;                   // 0x0C
  uchar MAR5;                   // 0x0D
  uchar MAR6;                   // 0x0E
  uchar MAR7;                   // 0x0F
  uint TxStatus0;               // 0x10 - 0x13
  uint TxStatus1;               // 0x14 - 0x17
  uint TxStatus2;               // 0x18 - 0x1B
  uint TxStatus3;               // 0x1C - 0x1F
  uint TxAddr0;                 // 0x20 - 0x23
  uint TxAddr1;                 // 0x24 - 0x27
  uint TxAddr2;                 // 0x28 - 0x2B
  uint TxAddr3;                 // 0x2C - 0x2F
  uint RxBufStart;              // 0x30 - 0x33
  ushort EarlyRxCount;          // 0x34 - 0x35
  uchar EarlyRxStatus;          // 0x36
  uchar Cmd;                    // 0x37
  ushort CurAddrPacket;         // 0x38 - 0x39
  ushort CurBufAddr;            // 0x3A - 0x3B
  ushort IMR;                   // 0x3C - 0x3D
  ushort ISR;                   // 0x3E - 0x3F
  uint TxConfig;                // 0x40 - 0x43
  uint RxConfig;                // 0x44 - 0x47
  uint TimerCount;              // 0x48 - 0x4B
  uint MissedPacketCounter;     // 0x4C - 0x4F
  uchar Cmd9346;                // 0x50
  uchar Config0;                // 0x51
  uchar Config1;                // 0x52
  uchar reserved1;              // 0x53
  uint TimerInt;                // 0x54 - 0x57
  uchar MediaStatus;            // 0x58
  uchar config3;                // 0x59
  uchar Config4;                // 0x5A
}

struct RTL8139_registers *regs;

static inline uint readl(uint addr) {
    volatile uint *ptr = (volatile uint*)addr;
    return *ptr;
}

static inline void writel(uint addr, uint val) {
    volatile uint *ptr = (volatile uint*)addr;
    *ptr = val;
}

static inline uchar readb(uint addr) {
    volatile uchar *ptr = (volatile uchar*)addr;
    return *ptr;
}

static inline void writeb(uint addr, uchar val) {
    volatile uchar *ptr = (volatile uchar*)addr;
    *ptr = val;
}

uint read_pci_config_register(uchar bus, uchar device, uchar function, uchar offset) {
  uint address = (1 << 31) |   // Enable bit
  ((uint)bus << 16) |          // Bus number
  ((uint)device << 11) |       // Device number
  ((uint)function << 8) |      // Function numbe
  (offset & 0xfc);             // Register offset (rounded down to nearest multiple of 4)

  // Write address to address port
  outl(0xcf8, address);

  // Read data from data port
  return inl(0xcfc);
}

void write_pci_config_register(uchar bus, uchar device, uchar function, uchar offset, uint data) {
  uint address = (1 << 31) |   // Enable bit
  ((uint)bus << 16) |          // Bus number
  ((uint)device << 11) |       // Device number
  ((uint)function << 8) |      // Function numbe
  (offset & 0xfc);             // Register offset (rounded down to nearest multiple of 4)

  // Write address to address port
  outl(0xcf8, address);

  // write data to data port
  outl(0xcfc, data);
}

void nicinit() {
  int i = 0;
  for(i = 0; i < 32; i++) {
    if(read_pci_config_register(0, i, 0, 0) == 0x813910ec)
      break;
  }

  // enable PCI bus mastering
  write_pci_config_register(0, i, 0, 0x4, read_pci_config_register(0, i, 0, 0x4) | 0x7);

  // Set base address for memory mapped io
  ioaddr = (struct RTL8139_registers*) read_pci_config_register(0, i, 0, 0x14);

  // Software reset
  writeb(ioaddr + 0x37, 0x10);

  // Check that the chip has finished the reset
  int j;
  for (j = 5000000; j > 0; j--) {
    if ((readb(ioaddr + 0x37) & 0x10) == 0){
      break;
    }
  }
  cprintf("%d\n", j);
  cprintf("%x\n", readb(ioaddr + 0x37));
}

/*
void nicinit() {
  int i;
  for(i = 0; i < 32; i++) {
    if(read_pci_config_register(0, i, 0, 0) == 0x813910ec)
      break;
  }
  
  // Set base io address
  ioaddr = read_pci_config_register(0, i, 0, 0x10) & 0x3;

  // enable PCI bus mastering
  write_pci_config_register(0, i, 0, 0x4, read_pci_config_register(0, i, 0, 0x4) | 0x7);

  outb( ioaddr + 0x52, 0x0);

  // software reset
  outb(ioaddr + 0x37, 0x10);
  
  // Check that the chip has finished the reset
  for (i = 1000; i > 0; i--) {
    if ((inb(ioaddr + 0x37) & 0x10) == 0) break;
  }
  cprintf("%d\n", i);
  cprintf("%x\n", ioaddr);
 
 // outl(ioaddr + 0x30, (uint)rx_ring);
  outw(ioaddr + 0x3c, 0x0005);
 
  // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
  outl(ioaddr + 0x44, 0xf | (1 << 7)); 
 
  // Sets the RE and TE bits in command register  
  outb(ioaddr + 0x37, 0x0C);
  cprintf("%x\n", inb(ioaddr + 0x58));
}
*/

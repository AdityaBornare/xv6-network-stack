#include "types.h"
#include "defs.h"
#include "x86.h"

#define RX_BUF_LEN 8192

uint ioaddr;

static unsigned char rx_ring[RX_BUF_LEN + 16] __attribute__((__aligned__(4)));

uint read_pci_config_register(uchar bus, uchar device, uchar function, uchar offset) {
  uint address = (1 << 31) |   // Enable bit
  ((uint)bus << 16) |          // Bus number
  ((uint)device << 11) |       // Device number
  ((uint)function << 8) |      // Function numbe
  (offset & 0xFC);             // Register offset (rounded down to nearest multiple of 4)

  // Write address to address port
  outl(0xCF8, address);

  // Read data from data port
  return inl(0xCFC);
}

void nicinit() {
  int i;
  for(i = 0; i < 32; i++) {
    if(read_pci_config_register(0, i, 0, 0) == 0x813910ec)
      break;
  }
  ioaddr = (read_pci_config_register(0, i, 0, 0x10) >> 8);
  // cprintf("%x\n", read_pci_config_register(0, i, 0, 0x10));
  // cprintf("%x\n", read_pci_config_register(0, i, 0, 0x14));
  // cprintf("%x\n", read_pci_config_register(0, i, 0, 0x4));
  cprintf("%x\n", ioaddr);

  outb(ioaddr + 0x52, 0x0);
  outb(ioaddr + 0x37, 0x10);
  
  // Check that the chip has finished the reset
  for (i = 1000; i > 0; i--) {
    if ((inb(ioaddr + 0x37) & 0x10) == 0) break;
  }

  outl(ioaddr + 0x30, (uint)rx_ring);
  outw(ioaddr + 0x3C, 0x0005);
  outl(ioaddr + 0x44, 0xf | (1 << 7));  // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP
  outb(ioaddr + 0x37, 0x0C); // Sets the RE and TE bits high
}

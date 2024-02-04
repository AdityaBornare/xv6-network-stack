#include "types.h"
#include "defs.h"
#include "x86.h"

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

#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "traps.h"

#define TX_FIFO_THRESH 256	       // In bytes, rounded down to 32 byte units
#define RX_FIFO_THRESH 4           // Rx buffer level before first PCI xfer
#define RX_DMA_BURST 4             // Calculate as 16 << 4 = 256 bytes
#define TX_DMA_BURST 4             // Calculate as 16 << 4 = 256 bytes
#define NUM_TX_DESC	4              // Number of Tx descriptor registers
#define ETH_FRAME_LEN	1514	       // Max. octets in frame sans FCS
#define TX_BUF_SIZE	ETH_FRAME_LEN
#define ETH_ZLEN	60	             // Min. octets in frame sans FCS
#define RX_BUF_LEN_IDX 0           // 0, 1, 2 is allowed - 8k ,16k ,32K rx buffer
#define RTL_TIMEOUT 100000
#define ETIMEDOUT -1  		         //connection timed out

#define RX_BUF_LEN (8192 << RX_BUF_LEN_IDX)
#define RX_MAX_PKT_LENGTH 1514
#define RX_MIN_PKT_LENGTH 60
#define RX_READ_POINTER_MASK ~3

#define RTL_REG_RXCONFIG_ACCEPTBROADCAST 0x08
#define RTL_REG_RXCONFIG_ACCEPTMULTICAST 0x04
#define RTL_REG_RXCONFIG_ACCEPTMYPHYS 0x02
#define RTL_REG_RXCONFIG_ACCEPTALLPHYS 0x01

const uint rx_config = (RX_BUF_LEN_IDX << 11) | (RX_FIFO_THRESH << 13) | (RX_DMA_BURST << 8);

struct RTL8139_registers {
  uchar IDR0;                   // 0x00
  uchar IDR1;                   // 0x01
  uchar IDR2;                   // 0x02
  uchar IDR3;                   // 0x03
  uchar IDR4;                   // 0x04
  uchar IDR5;                   // 0x05
  ushort reserved0;            // 0x06 - 0x07
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
  uchar Cfg9346;                // 0x50
  uchar Config0;                // 0x51
  uchar Config1;                // 0x52
  uchar reserved1;              // 0x53
  uint TimerInt;                // 0x54 - 0x57
  uchar MediaStatus;            // 0x58
  uchar config3;                // 0x59
  uchar Config4;                // 0x5A
  uchar reserved2;              // 0x5B
  ushort MulIntrSelect;         // 0x5c - 0x5D
  uchar PciRevId;               // 0x5E
  uchar reserved3;              // 0x5F
  ushort TxStatusAllDesc;       // 0x60 - 0x61
  ushort BasicModeCtrl;         // 0x62 - 0x62
  ushort BasicModeStatus;       // 0x64 - 0x65
};

static struct nic {
  volatile struct RTL8139_registers *regs;
  uchar tx_buffer[TX_BUF_SIZE] __attribute__((__aligned__(4)));
  uchar rx_ring[RX_BUF_LEN + 16] __attribute__((__aligned__(4)));
  uint cur_tx;
  uint cur_rx;
  uchar packets_received_good;
  uchar byte_received;
  uint pci_device_no;
} nic;

enum CmdBits {
  RxBufEmpty = 0x01,
  CmdTxEnb   = 0x04,
  CmdRxEnb   = 0x08,
  CmdReset   = 0x10,
};

enum IntrStatusBits {
  RxOK       = 0x0001,
  RxErr      = 0x0002,
  TxOK       = 0x0004,
  TxErr      = 0x0008,
  RxOverflow = 0x0010,
  RxUnderrun = 0x0020,
  RxFIFOover = 0x0040,
  PCSTimeout = 0x4000,
  PCIErr     = 0x8000,

};

enum PacketHeader {
	ROK        = 0x0001,
	FAErr      = 0x0002,
	CRC        = 0x0004,
	LongPkt    = 0x0008,
	RUNT       = 0x0010,
	ISE        = 0x0020,
	BAR        = 0x2000,
	PAM        = 0x4000,
	MAR        = 0x8000,
};

struct RxStats{
  uchar rx_errors;
  uchar rx_frame_errors;
  uchar rx_length_errors;
  uchar rx_crc_errors;
  uchar rx_dropped;
};

struct RxStats RxStats;

void rtl8139_set_rx_mode() {
  uint rx_mode = RTL_REG_RXCONFIG_ACCEPTBROADCAST | RTL_REG_RXCONFIG_ACCEPTMYPHYS;
  nic.regs->RxConfig = rx_config | rx_mode;
  *((uint*)&nic.regs->MAR0) = 0xffffffff;
  *((uint*)&nic.regs->MAR4) = 0xffffffff;
}

void rtl8139_reset(){
  // Software reset
  nic.regs->Cmd = CmdReset;

  // Check that the chip has finished the reset
  for (int j = 1000; j > 0; j--) {
    if ((nic.regs->Cmd & 0x10) == 0){
      break;
    }
  }

  // Change operating mode before writing to config registers
  nic.regs->Cfg9346 =  0xC0;

  // Set the RE and TE bits in command register before setting transfer thresholds
  nic.regs->Cmd = CmdTxEnb | CmdRxEnb;

  // Set transfer thresholds
  nic.regs->RxConfig = rx_config;
  nic.regs->TxConfig = (TX_DMA_BURST << 8) | 0x03000000;

  // Set full duplex mode
  nic.regs->BasicModeCtrl |= 0x0100;

  // Change operating mode back to the normal network communication mode
  nic.regs->Cfg9346 = 0x00;

  // Initialize rx buffer
  nic.regs->RxBufStart = V2P((uint)nic.rx_ring);

  // Start the chip's Tx and Rx process
  nic.regs->MissedPacketCounter = 0;
  rtl8139_set_rx_mode();
  nic.regs->Cmd = CmdTxEnb | CmdRxEnb | RxBufEmpty;

  nic.regs->ISR = 0xff;
  // Enable interrupts by setting the interrupt mask
  nic.regs->IMR = TxOK | RxOK | TxErr;
  
  // cur_rx and cur_tx set to 0
	nic.cur_rx = 0;
  nic.cur_tx = 0;
}

void nicinit() {
  // Search in PCI configuration space
  int i = 0;
  for(i = 0; i < 32; i++) {
    if(read_pci_config_register(0, i, 0, 0) == 0x813910ec)
      break;
  }
  nic.pci_device_no = i;

  // enable PCI bus mastering
  write_pci_config_register(0, i, 0, 0x4, read_pci_config_register(0, i, 0, 0x4) | 0x7);

  // Set base address for memory mapped io
  nic.regs = (volatile struct RTL8139_registers*) read_pci_config_register(0, i, 0, 0x14);

  // MAC address
  MAC[0] = nic.regs->IDR0;
  MAC[1] = nic.regs->IDR1;
  MAC[2] = nic.regs->IDR2;
  MAC[3] = nic.regs->IDR3;
  MAC[4] = nic.regs->IDR4;
  MAC[5] = nic.regs->IDR5;

  // Configure interrupt line
  ioapicenable(IRQ_CASCADE, 0);
  ioapicenable(IRQ_NIC, 0);

  // Set the LWAKE + LWPTN to active high. This should essentially 'power on' the device.
  nic.regs->Config1 = 0x0;

  //call reset
  rtl8139_reset();
}

void rtl8139_send(void *packet, int length){
  if (length == 0)
    return;

  uint len = length;
  volatile uint txstatus;

  memmove(nic.tx_buffer, packet, length);

  //Note: RTL8139 doesn't auto-pad, send minimum payload.
  while (len < ETH_ZLEN){
    nic.tx_buffer[len++] = '\0';
  }

  // cprintf("sending %d bytes\n", len);

  *(&nic.regs->TxAddr0 + nic.cur_tx) = V2P((uint)nic.tx_buffer);
  txstatus = (TX_FIFO_THRESH << 11 | len) & 0xffffdfff;
  *(&nic.regs->TxStatus0 + nic.cur_tx) = txstatus;

  txstatus = *(&nic.regs->TxStatus0 + nic.cur_tx);
}

int rtl8139_packetOK(uint rx_status, uint rx_size, uint pkt_length) {

  uint bad_packet = (rx_status & RUNT) || (rx_status & LongPkt) || (rx_status & CRC) || (rx_status & FAErr);
	if(!bad_packet && (rx_status & ROK)) {
	  if(pkt_length > RX_MAX_PKT_LENGTH || pkt_length < RX_MIN_PKT_LENGTH)
		  return 0;

		return 1;
	}
  else
	  return 0;
}

int rtl8139_receive() {
  volatile uchar tmpCmd = nic.regs->Cmd;
	uint pkt_length;
	uchar *p_income_pkt, *rx_read_ptr;
  nic.cur_rx = nic.cur_rx % RX_BUF_LEN;
	uint rx_status = *((uint*)(nic.rx_ring + nic.cur_rx));
  uint rx_size = rx_status >> 16;     // Includes CRC
  pkt_length = rx_size - 4;

  if(!(tmpCmd & RxBufEmpty)) {
	  rx_read_ptr = nic.rx_ring + nic.cur_rx;
		p_income_pkt = rx_read_ptr + 4;

		if(rtl8139_packetOK(rx_status, rx_size, pkt_length)) {
		  if ((nic.cur_rx + pkt_length) > RX_BUF_LEN) {
        // wrap around to end of RxBuffer
        memmove(nic.rx_ring, nic.rx_ring + RX_BUF_LEN, (nic.cur_rx + pkt_length - RX_BUF_LEN));
      }

      // update Read Pointer
      nic.cur_rx = (nic.cur_rx + rx_size + 4 + 3) & RX_READ_POINTER_MASK;
      nic.regs->CurAddrPacket = nic.cur_rx - 0x10;

      nic.packets_received_good++;
      nic.byte_received += pkt_length;
      // pass data to upper layer
      ether_receive((void*) p_income_pkt, pkt_length);
    }
    else {
      nic.cur_rx = 0;
      nic.regs->Cmd = tmpCmd | CmdTxEnb | CmdRxEnb;
      nic.cur_rx = (nic.cur_rx + rx_size + 4 + 3) & RX_READ_POINTER_MASK;
      nic.regs->CurAddrPacket = nic.cur_rx - 0x10;
    }
  }
  return 1;
}

void nicintr() {
  volatile uint status = nic.regs->ISR;
  while(status) {
    if (status & TxOK) {
      cprintf("Tx OK\n");
      status &= TxOK;
      nic.regs->ISR = status;
      nic.cur_tx = (nic.cur_tx + 1) % NUM_TX_DESC;
    }

    if (status & TxErr) {
      cprintf("Transmission Error\n");
      status &= TxErr;
      nic.regs->ISR = status;
      rtl8139_reset();
    }

    if (status & RxOK) {
      cprintf("Rx OK\n");
      rtl8139_receive();
      status &= RxOK;
      nic.regs->ISR = status;
    }

    if (status & RxErr) {
      cprintf("Reception Error\n");
      status &= RxErr;
      nic.regs->ISR = status;
      rtl8139_reset();
    }
    status = nic.regs->ISR;
  }
}

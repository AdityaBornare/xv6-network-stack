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
#define RX_MAX_PKT_LENGTH 1024    // Correct this later
#define RX_MIN_PKT_LENGTH 0       // Correct this later
#define RX_READ_POINTER_MASK 0    // Correct this later

#define RTL_REG_RXCONFIG_ACCEPTBROADCAST 0x08
#define RTL_REG_RXCONFIG_ACCEPTMULTICAST 0x04
#define RTL_REG_RXCONFIG_ACCEPTMYPHYS 0x02
#define RTL_REG_RXCONFIG_ACCEPTALLPHYS 0x01

static unsigned char tx_buffer[TX_BUF_SIZE] __attribute__((__aligned__(4)));
static unsigned char rx_ring[RX_BUF_LEN + 16] __attribute__((__aligned__(4)));
unsigned int cur_tx;

const uint rx_config = (RX_BUF_LEN_IDX << 11) | (RX_FIFO_THRESH << 13) | (RX_DMA_BURST << 8);

static uchar cur_rx;
static uchar packets_received_good;
static uchar byte_received;


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
	ROK       = 0x0001,
	FAErr     = 0x0002,
	CRC        = 0x0004,
	LongPkt    = 0x0008,
	RUNT       = 0x0010,
	ISE        = 0x0020,
	BAR        = 0x0200,
	PAM        = 0x0400,
	MAR        = 0x0800,
};

struct RxStats{
  uchar rx_errors;
  uchar rx_frame_errors;
  uchar rx_length_errors;
  uchar rx_crc_errors;
  uchar rx_dropped;
};

volatile struct RTL8139_registers *regs;
struct RxStats RxStats;

void rtl8139_set_rx_mode() {
  uint rx_mode = RTL_REG_RXCONFIG_ACCEPTBROADCAST | RTL_REG_RXCONFIG_ACCEPTMULTICAST | RTL_REG_RXCONFIG_ACCEPTMYPHYS;
  regs->RxConfig = rx_config | rx_mode;
  *((uint*)&regs->MAR0) = 0xffffffff;
  *((uint*)&regs->MAR4) = 0xffffffff;
}

void rtl8139_reset(){
  // Software reset
  regs->Cmd = CmdReset;
  cur_tx = 0;

  // Check that the chip has finished the reset
  for (int j = 1000; j > 0; j--) {
    if ((regs->Cmd & 0x10) == 0){
      break;
    }
  }

  // Change operating mode before writing to config registers
  regs->Cfg9346 =  0xC0;

  // Set the RE and TE bits in command register before setting transfer thresholds
  regs->Cmd = CmdTxEnb | CmdRxEnb;

  // Set transfer thresholds
  regs->RxConfig = rx_config;
  regs->TxConfig = (TX_DMA_BURST << 8) | 0x03000000;
  
  // Set full duplex mode
  regs->BasicModeCtrl |= 0x0100;

  // Change operating mode back to the normal network communication mode
  regs->Cfg9346 = 0x00;

  // Initialize rx buffer
  regs->RxBufStart = (uint)rx_ring;

  // Start the chip's Tx and Rx process
  regs->MissedPacketCounter = 0;
  rtl8139_set_rx_mode();
  regs->Cmd = CmdTxEnb | CmdRxEnb;

  regs->ISR = 0xff;
  // Enable interrupts by setting the interrupt mask
  regs->IMR = TxOK | RxOK | TxErr;

	//cur_rx set to 0
	cur_rx = 0;
}

int rtl8139_packetOK() {
	
	int bad_packet = (regs->ISR & RUNT) || (regs->ISR & LongPkt) || (regs->ISR & CRC) || (regs->ISR & FAErr);
  
  uint ring_offset = cur_rx % RX_BUF_LEN;	
	uint rx_status =  *(uint* )(rx_ring + ring_offset);
	uint rx_size = rx_status >> 16;     // Includes CRC

	int pkt_size = rx_size - 4;

	if(!bad_packet && (regs->ISR & ROK)) {
     
	  if(pkt_size > RX_MAX_PKT_LENGTH || rx_size < RX_MIN_PKT_LENGTH) {
		  
		  return 0;
		}

	  packets_received_good++;
	  byte_received += pkt_size;
	  
		return 1;
	}

  else
	  return 0;

}


int rtl8139_receive() {
  
  uchar tmpCmd;
	uint pkt_length;
	uchar *p_income_pkt, *rx_read_ptr;
  uint rx_read_ptr_offset = cur_rx % RX_BUF_LEN;

	unsigned long rx_status =  *(unsigned long* )(rx_ring + rx_read_ptr_offset);
  uint rx_size = rx_status >> 16;     // Includes CRC

  pkt_length = rx_size - 4;

	while(1){
	  
		tmpCmd = regs->Cmd;

		if(tmpCmd & RxBufEmpty) 
			break;

	}
	do {
	  rx_read_ptr = rx_ring + rx_read_ptr_offset;
		p_income_pkt = rx_read_ptr + 4;
		
		if(rtl8139_packetOK()){

		  	if ( (rx_read_ptr_offset + pkt_length) > RX_BUF_LEN ){

           //wrap around to end of RxBuffer
           memmove( rx_ring + RX_BUF_LEN , rx_ring,
           (rx_read_ptr_offset + pkt_length - RX_BUF_LEN));
        }
        
        //copy the packet out here
        memmove(p_income_pkt, p_income_pkt, pkt_length - 4);//don't copy 4 bytes CRC

        //update Read Pointer
        rx_read_ptr_offset = (rx_read_ptr_offset + pkt_length + 4 + 3) & RX_READ_POINTER_MASK;
        //4:for header length(PktLength include 4 bytes CRC)
        //3:for dword alignment

        regs->CurAddrPacket &= rx_read_ptr_offset - 0x10;

    }
    else{
      rtl8139_set_rx_mode();
      break;
    }
    
    tmpCmd = regs->Cmd;
    
    }while (!(tmpCmd & RxBufEmpty));
        
    return 1;
}

void nicinit() {
  // Search in PCI configuration space
  int i = 0;
  for(i = 0; i < 32; i++) {
    if(read_pci_config_register(0, i, 0, 0) == 0x813910ec)
      break;
  }

  // enable PCI bus mastering
  write_pci_config_register(0, i, 0, 0x4, read_pci_config_register(0, i, 0, 0x4) | 0x7);
  
  // Set base address for memory mapped io
  regs = (volatile struct RTL8139_registers*) read_pci_config_register(0, i, 0, 0x14);

  // Configure interrupt line
  ioapicenable(9, 0);
  ioapicenable(11, 0);

  // Set the LWAKE + LWPTN to active high. This should essentially 'power on' the device.
  regs->Config1 = 0x0;
  
  //call reset
  rtl8139_reset();

  cprintf("BMSR = %x\n", regs->BasicModeStatus);
}

void delay(int microseconds) {
  uint start_ticks = ticks;

  // Convert microseconds to ticks (assuming 10ms timer interrupt)
  uint desired_ticks = (microseconds * 10) / 1000;

  while ((ticks - start_ticks) < desired_ticks) {
    // Wait until desired number of ticks has passed
  }
}

// packet transmission: returns 0 on success, -1 on error/timeout
int rtl8139_send(void *packet, int length){
  if (length == 0)
    return -1;

  uint len = length;
  volatile uint txstatus;

  memmove(tx_buffer, packet, length);

  //Note: RTL8139 doesn't auto-pad, send minimum payload.
  while (len < ETH_ZLEN){
    tx_buffer[len++] = '\0';
  }

  cprintf("sending %d bytes\n", len);

  *(&regs->TxAddr0 + cur_tx) = V2P((uint)tx_buffer);
  txstatus = (TX_FIFO_THRESH << 11 | len) & 0xffffdfff;
  *(&regs->TxStatus0 + cur_tx) = txstatus;

  txstatus = *(&regs->TxStatus0 + cur_tx);
  /*
  if (!(status & TxOK)) {
    cprintf("tx timeout/error (%d usecs), status %x txstatus %x\n",10 * i, status, txstatus);

    rtl8139_reset();
    return -1;
  }
  */

  // if (status & TxOK)
		// cur_tx = (cur_tx + 1) % NUM_TX_DESC;
  // cur_tx = (cur_tx + 1) % NUM_TX_DESC;
  // cprintf("tx done, status %x txstatus %x\n",status, txstatus);
  return 0;
}

void nicintr() {
  if (regs->ISR & TxOK) {
    cprintf("ISR before %x\n", regs->ISR);
    volatile uint status = regs->ISR;
    status &= TxOK;
    regs->ISR = status;
    cprintf("ISR after %x\n", regs->ISR);
    cur_tx = (cur_tx + 1) % NUM_TX_DESC;
  }
  
  else if (regs->ISR & TxErr) {
    cprintf("Transmission Error\n");
    rtl8139_reset();
    volatile uint status = regs->ISR;
    status &= TxErr;
    regs->ISR = status;
  }
}

#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#include <kern/pci.h>

// Kernel functions
int attach_E1000(struct pci_func *pcif);
int E1000_transmit(void * data_addr, uint16_t length);

#define E1000_TXDARR_LEN     32 /* Length of the transmit descriptor ring */

/* Transmit Descriptor: sourced from e1000_hw.h */
struct e1000_tx_desc {
    uint64_t buffer_addr;       /* Address of the descriptor's data buffer */
    uint16_t length;    /* Data buffer length */
    uint8_t cso;        /* Checksum offset */
    uint8_t cmd;        /* Descriptor control */
    uint8_t status;     /* Descriptor status */
    uint8_t css;        /* Checksum start */
    uint16_t special;
};

/* Transmit Descriptor bit definitions */
#define E1000_TXD_DTYP_D     0x00100000         /* Data Descriptor */
#define E1000_TXD_DTYP_C     0x00000000         /* Context Descriptor */
#define E1000_TXD_POPTS_IXSM (uint8_t)0x01      /* Insert IP checksum */
#define E1000_TXD_POPTS_TXSM (uint8_t)0x02      /* Insert TCP/UDP checksum */
#define E1000_TXD_CMD_EOP    (uint8_t)0x01      /* End of Packet */
#define E1000_TXD_CMD_IFCS   (uint8_t 0x02      /* Insert FCS (Ethernet CRC) */
#define E1000_TXD_CMD_IC     (uint8_t)0x04      /* Insert Checksum */
#define E1000_TXD_CMD_RS     (uint8_t)0x08      /* Report Status */
#define E1000_TXD_CMD_RPS    (uint8_t)0x10      /* Report Packet Sent */
#define E1000_TXD_CMD_DEXT   (uint8_t)0x20      /* Descriptor extension (0 = legacy) */
#define E1000_TXD_CMD_VLE    (uint8_t)0x40      /* Add VLAN tag */
#define E1000_TXD_CMD_IDE    (uint8_t)0x80      /* Enable Tidv register */
#define E1000_TXD_STAT_DD    (uint8_t)0x01      /* Descriptor Done */
#define E1000_TXD_STAT_EC    (uint8_t)0x02      /* Excess Collisions */
#define E1000_TXD_STAT_LC    (uint8_t)0x04      /* Late Collisions */
#define E1000_TXD_STAT_TC    (uint8_t)0x04      /* Tx Underrun */
#define E1000_TXD_STAT_TU    (uint8_t)0x08      /* Transmit underrun */
#define E1000_TXD_CMD_TCP    (uint8_t)0x01      /* TCP packet */
#define E1000_TXD_CMD_IP     (uint8_t)0x02      /* IP packet */
#define E1000_TXD_CMD_TSE    (uint8_t)0x04      /* TCP Seg enable */

/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */

#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */

/* Transmit Control */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000100    /* collision threshold */
#define E1000_TCTL_COLD   0x00040000    /* collision distance */

/* Transmit IPG Register */
#define E1000_TIPG_IPGT   10
#define E1000_TIPG_IPGR1  8<<10
#define E1000_TIPG_IPGR2  6<<20

/* Ethernet Specifications */
#define E1000_ETH_PACKET_LEN 1518
typedef uint8_t packet_t[E1000_ETH_PACKET_LEN];

#endif	// JOS_KERN_E1000_H

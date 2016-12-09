#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

// LAB 6: Your driver code here

volatile void *e1000addr;
struct e1000_tx_desc txd_arr[E1000_TXDARR_LEN] __attribute__((aligned(4096)));
struct e1000_rx_desc rxd_arr[E1000_RXDARR_LEN] __attribute__((aligned(4096)));
packet_t txd_bufs[E1000_TXDARR_LEN] __attribute__((aligned(4096)));
packet_t rxd_bufs[E1000_RXDARR_LEN] __attribute__((aligned(4096)));
uint64_t macaddr = 0;

uint16_t _read_eeprom(uint32_t addr)
{
    volatile uint32_t *eerd = (uint32_t *)(e1000addr+E1000_EERD);
    volatile uint16_t data = 0;
    *eerd = E1000_EERD_START | addr;
    while ((*eerd & E1000_EERD_START) == 1); // Continually poll until we have a response

    data = *eerd >> 16;
    return data;
}

uint64_t E1000_get_macaddr()
{
    if (macaddr > 0)
        return macaddr;

    uint64_t word0 = _read_eeprom(E1000_EERD_MAC_WD0);
    uint64_t word1 = _read_eeprom(E1000_EERD_MAC_WD1);
    uint64_t word2 = _read_eeprom(E1000_EERD_MAC_WD2);
    uint64_t word3 = (uint64_t)0x8000;
    macaddr = word3<<48 | word2<<32 | word1<<16 | word0;
    return macaddr;
}

/* Reset the TXD array entry corresponding to the given
 * index such that it may be resused for another packet.
 */
void _reset_tdr(int index)
{
    txd_arr[index].buffer_addr = (uint32_t)(PADDR(txd_bufs+index));
    txd_arr[index].cso = 0;
    txd_arr[index].cmd = E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    txd_arr[index].status = 0;
    txd_arr[index].css = 0;
    txd_arr[index].special = 0;
}

/* Reset the RXD array entry corresponding to the given
 * index such that it may be reused for another packet.
 */
void _reset_rdr(int index)
{
    rxd_arr[index].buffer_addr = (uint32_t)(PADDR(rxd_bufs+index));
    rxd_arr[index].status = 0;
    rxd_arr[index].special = 0;
}
int E1000_attach(struct pci_func *pcif)
{
    pci_func_enable(pcif);
    e1000addr = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);


    // Initialize MMIO Region
    // Transmit initialization
    *(uint32_t *)(e1000addr+E1000_TDBAL) = (uint32_t)(PADDR(txd_arr)); // Indicates start of descriptor ring buffer
    *(uint32_t *)(e1000addr+E1000_TDBAH) = 0; // Make sure high bits are set to 0
    *(uint16_t *)(e1000addr+E1000_TDLEN) = (uint16_t)(sizeof(struct e1000_tx_desc)*E1000_TXDARR_LEN); // Indicates length of descriptor ring buffer
    *(uint32_t *)(e1000addr+E1000_TDH) = 0;
    *(uint32_t *)(e1000addr+E1000_TDT) = 0;
    *(uint32_t *)(e1000addr+E1000_TCTL) = (E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_CT | E1000_TCTL_COLD);
    *(uint16_t *)(e1000addr+E1000_TIPG) = (uint16_t)(E1000_TIPG_IPGT | E1000_TIPG_IPGR1 | E1000_TIPG_IPGR2);

    // Initialize CMD bits for transmit descriptors
    for (int i = 0; i < E1000_TXDARR_LEN; i++){
        _reset_tdr(i);
        txd_arr[i].status = E1000_TXD_STAT_DD;
    }

    // Receive initialization
    uint64_t macaddr_local = E1000_get_macaddr();
    *(uint32_t *)(e1000addr+E1000_RAL) = (uint32_t)(macaddr_local & 0xffffffff); // Set mac address for filtering
    *(uint32_t *)(e1000addr+E1000_RAH) = (uint32_t)(macaddr_local>>32);
    *(uint32_t *)(e1000addr+E1000_RDBAL) = (uint32_t)(PADDR(rxd_arr)); // Indicates start of descriptor ring buffer
    *(uint32_t *)(e1000addr+E1000_RDBAH) = 0; // Make sure high bits are set to 0
    *(uint16_t *)(e1000addr+E1000_RDLEN) = (uint16_t)(sizeof(struct e1000_rx_desc)*E1000_RXDARR_LEN); // Indicates length of descriptor ring buffer
    *(uint32_t *)(e1000addr+E1000_RDH) = 0;
    *(uint32_t *)(e1000addr+E1000_RDT) = E1000_RXDARR_LEN;

    for (int i = 0; i< E1000_RXDARR_LEN; i++)
        _reset_rdr(i);
    *(uint32_t *)(e1000addr+E1000_RCTL) = (E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_BSIZE | E1000_RCTL_SECRC);
    return 0;
}

int E1000_transmit(void * data_addr, uint16_t length)
{
    static uint32_t nextindex = 0;
    struct e1000_tx_desc *nextdesc = (&txd_arr[nextindex]);

    if (!(nextdesc->status & E1000_TXD_STAT_DD))
        return -E_TXD_FULL; // no free descriptors, buffer is full

    if (length > E1000_ETH_PACKET_LEN)
        length = E1000_ETH_PACKET_LEN;

    _reset_tdr(nextindex);  // Reset this TDR
    nextdesc->length = length;
    memcpy((txd_bufs+nextindex), data_addr, length);

    nextindex = (nextindex+1)%E1000_TXDARR_LEN;
    *(uint32_t *)(e1000addr+E1000_TDT) = nextindex;
    return 0;
}

int E1000_receive(void * data_addr, uint16_t *len_store)
{
    static uint32_t nextindex = 0;
    struct e1000_rx_desc *nextdesc = (&rxd_arr[nextindex]);

    if (!(nextdesc->status & E1000_RXD_STAT_DD))
        return -E_RXD_EMPTY; // Buffer is empty

    *len_store = nextdesc->length;
    memcpy(data_addr, (rxd_bufs+nextindex), nextdesc->length);
    _reset_rdr(nextindex);

    *(uint32_t *)(e1000addr+E1000_RDT) = nextindex;
    nextindex = (nextindex+1)%E1000_RXDARR_LEN;
    return 0;
}

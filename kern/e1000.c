#include <kern/e1000.h>
#include <kern/pmap.h>

// LAB 6: Your driver code here

volatile uint32_t *e1000addr;

int attach_E1000(struct pci_func *pcif)
{
    pci_func_enable(pcif);
    e1000addr = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    uint32_t dev_status = *(e1000addr+2);
    return 0;
}

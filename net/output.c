#include "ns.h"
#include "inc/lib.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
        int r, val;
        envid_t from_env = 1;
        struct jif_pkt *pkt_page = (struct jif_pkt *)REQVA;

        r = sys_page_alloc(0, pkt_page, PTE_U|PTE_W|PTE_P);
        if (r < 0)
            panic("output: could not allocate a page of memory, %e", r);

        while (true)
        {
            val = ipc_recv(&from_env, pkt_page, NULL);
            if (from_env != ns_envid){
                cprintf("Bad recv envid in output\n");
                continue;
            }
            if (val != NSREQ_OUTPUT){
                cprintf("Non-NSREQ_OUTPUT request sent to output\n");
                continue;
            }
            sys_send_packet((void *) pkt_page->jp_data, pkt_page->jp_len);
            sys_page_unmap(0, pkt_page);
        }
}

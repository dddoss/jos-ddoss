#include "ns.h"
#include "inc/lib.h"
#include "inc/error.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

        int r = sys_page_alloc(0, (void *)REQVA, PTE_U|PTE_W|PTE_P);
        if (r < 0)
            panic("ns_input: could not alloc page of memory");
        r = sys_page_alloc(0, (void *)(REQVA+PGSIZE), PTE_U|PTE_W|PTE_P);
        union Nsipc *nsipc_page_1 = (union Nsipc *)REQVA;
        union Nsipc *nsipc_page_2 = (union Nsipc *)(REQVA+PGSIZE);
        bool page1 = true;
        while (true)
        {
            union Nsipc *nsipc_page;
            if (page1)
                nsipc_page = nsipc_page_1;
            else
                nsipc_page = nsipc_page_2;
            page1 = !page1;

            r = sys_recv_packet(&nsipc_page->pkt.jp_data, (size_t *)&nsipc_page->pkt.jp_len);
            if (r == -E_RXD_EMPTY){
                sys_yield();
                continue;
            }
            else if (r<0)
                panic("error in sys_recv_packet, %e", r);
            ipc_send(ns_envid, NSREQ_INPUT, nsipc_page, PTE_P|PTE_W|PTE_U);
        }
}

// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
        if (!(err & FEC_WR))
            panic("pgfault: non-write page fault: %x", err);
        
        int pdx = PDX(addr);
        int ptx = PTX(addr);
        pte_t pte = *(pte_t *)(UVPT | (pdx) << 12 | ptx*4);
        
        if (!(pte & PTE_COW))
            panic("pgfault: page not copy-on-write");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
        void *rounded_addr = (void *) ROUNDDOWN(utf->utf_fault_va, PGSIZE);
        if ((r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_U|PTE_W))<0)
            panic("pgfault: sys_page_alloc error=%e", r);
        memmove(PFTEMP, rounded_addr, PGSIZE);
        if ((r = sys_page_map(0, PFTEMP, 0, rounded_addr, PTE_P|PTE_U|PTE_W))<0)
            panic("pgfault: sys_page_map error=%e", r);
        if ((r = sys_page_unmap(0, PFTEMP)) < 0)
            panic("pgfault: sys_page_unmap error=%e", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
        uint32_t *addr = (uint32_t *)(pn*PGSIZE);
        if (ROUNDDOWN((int)addr, PGSIZE) == UXSTACKTOP-PGSIZE){
            return -1; // Skip mapping UXSTACK
        }
        int pdx = PDX(pn*PGSIZE);
        int ptx = PTX(pn*PGSIZE);;
        pde_t pde = *(pde_t *)(UVPT | UVPT >> 10 | ptx);
        pte_t pte = *(pte_t *)(UVPT | (pdx) << 12 | ptx*4);

        int flags = pte & (PTE_W | PTE_U | PTE_P | PTE_COW);
        if ((flags & PTE_W) || (flags & PTE_COW)){
            flags |= PTE_COW;
            flags &= ~PTE_W;
        }

        if ((r = sys_page_map(0, addr, envid, addr, flags)) < 0)
            panic("sys_page_map: %e", r);
        if (flags & PTE_COW){
            if ((r = sys_page_map(0, addr, 0, addr, flags) < 0))
                panic("sys_page_map: %e", r);
        }
        return 0;
        
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
        envid_t envid;
        int pdi, pti;
        pde_t pde;
        pte_t pte;
        int r;
    
        set_pgfault_handler(pgfault);
        envid = sys_exofork();
        if (envid < 0)
            panic("fork: call to sys exofork errored with %e", envid);
        if (envid == 0) {
            // Child process: correct thisenv
            thisenv = &envs[ENVX(sys_getenvid())];
            return 0;
        }

        // Parent process
        // Copy page mappings
        for (pdi = 0; pdi <= (int)PDX(UTOP-1); pdi++) {

            pde = *(pde_t *)(UVPT | UVPT >> 10 | pdi*4);
            if(!(pde & PTE_P) || !(pde & PTE_U))
                continue;
            for (pti = 0; pti < NPTENTRIES; pti++) {
                pte = *(pte_t *)(UVPT | (pdi) << 12 | pti*4);
                if (!(pte & PTE_P) || !(pte & PTE_U))
                    continue;
                r = duppage(envid, pdi*NPTENTRIES+pti);
            }
        }

        extern void _pgfault_upcall(void);
        sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
        sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_P | PTE_U | PTE_W);
        sys_page_map(envid, (void *)(UXSTACKTOP-PGSIZE), 0, UTEMP, PTE_P | PTE_U | PTE_W);
        memmove(UTEMP, (void *)(UXSTACKTOP-PGSIZE), PGSIZE);
        sys_page_unmap(0, UTEMP);


        if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
            panic("sys_env_set_status: %e", r);
                

        return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}

// User-level IPC library routines

#include <inc/lib.h>


// Same footprint as ipc_recv, with one caveat:
// If from_env >= 0, causes sys_ipc_recv to selectively receive 
// from a specified envid, blocking on all other attempted sends
int32_t
ipc_recv_select(envid_t from_env, envid_t *from_env_store, void *pg, int *perm_store){
	// LAB 4: Your code here.
        int r;
        if (pg == NULL)
            pg = (void *)UTOP;
        if ((r = sys_ipc_recv(pg, from_env)) < 0){
            // Error in sys_ipc_recv
            if (from_env_store != NULL)
                *from_env_store = 0;
            if (perm_store != NULL)
                *perm_store = 0;
            return r;
        }
        else{
            if (from_env_store != NULL)
                *from_env_store = thisenv->env_ipc_from;
            if (perm_store != NULL)
                *perm_store = thisenv->env_ipc_perm;
            return thisenv->env_ipc_value;
        }
        return -1; // Not reached
}

// Receive a value via IPC and return it.
// If 'pg' is nonnull, then any page sent by the sender will be mapped at
//	that address.
// If 'from_env_store' is nonnull, then store the IPC sender's envid in
//	*from_env_store.
// If 'perm_store' is nonnull, then store the IPC sender's page permission
//	in *perm_store (this is nonzero iff a page was successfully
//	transferred to 'pg').
// If the system call fails, then store 0 in *fromenv and *perm (if
//	they're nonnull) and return the error.
// Otherwise, return the value sent by the sender
//
// Hint:
//   Use 'thisenv' to discover the value and who sent it.
//   If 'pg' is null, pass sys_ipc_recv a value that it will understand
//   as meaning "no page".  (Zero is not the right value, since that's
//   a perfectly valid place to map a page.)
int32_t
ipc_recv(envid_t *from_env_store, void *pg, int *perm_store)
{
    return ipc_recv_select(-1, from_env_store, pg, perm_store);
}

// Send 'val' (and 'pg' with 'perm', if 'pg' is nonnull) to 'toenv'.
// This function keeps trying until it succeeds.
// It should panic() on any error other than -E_IPC_NOT_RECV.
//
// Hint:
//   Use sys_yield() to be CPU-friendly.
//   If 'pg' is null, pass sys_ipc_try_send a value that it will understand
//   as meaning "no page".  (Zero is not the right value.)
void
ipc_send(envid_t to_env, uint32_t val, void *pg, int perm)
{
    int r;
    if (pg == NULL)
        pg = (void *)UTOP; // Above user mappable region

    while(true){
        if ((r = sys_ipc_try_send(to_env, val, pg, perm)) < 0){
            if (r == -E_IPC_NOT_RECV){
                sys_yield();
                continue;
            }
            else
                panic("Error in ipc_send: %e", r);
        }
        else
            return;
    }
}

// Find the first environment of the given type.  We'll use this to
// find special environments.
// Returns 0 if no such environment exists.
envid_t
ipc_find_env(enum EnvType type)
{
	int i;
	for (i = 0; i < NENV; i++)
		if (envs[i].env_type == type)
			return envs[i].env_id;
	return 0;
}

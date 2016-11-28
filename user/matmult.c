// Concurrent version of matrix multiplication for a 3x3 and Nx3 matrix.
// See http://spinroot.com/courses/summer/Papers/hoare_1978.pdf section 6.2

#include <inc/lib.h>

envid_t parent_id;

void
north_node(){
    envid_t south;

    // Get neighbor environment(s) from parent
    south = ipc_recv_select(parent_id, 0, 0, 0); 
    while(1){
        ipc_send(south, 0, 0, 0);
    }
}

void
west_node(){
    envid_t east;
    int val;

    // Get neighbor environment(s) from parent
    east = ipc_recv_select(parent_id, 0, 0, 0); 
    while(1){
        val = ipc_recv_select(parent_id, 0, 0, 0);
        ipc_send(east, val, 0, 0);
    }
}

void
south_node(bool end){
    envid_t north;
    int val;

    // Get neighbor environment(s) from parent
    north = ipc_recv_select(parent_id, 0, 0, 0); 
    while(1){
        val = ipc_recv_select(north, 0, 0, 0);
        cprintf("%d ", val);
        if (end)
            cprintf("\n");
    }
}

void
east_node() {
    envid_t west;
    int val;

    // Get neighbor environment(s) from parent
    west = ipc_recv_select(parent_id, 0, 0, 0); 
    while(1){
        val = ipc_recv_select(west, 0, 0, 0);
    }
}

void
center_node(int A_val) {
    envid_t north, west, south, east;
    int from_north, from_west;
    int to_south;

    // Get neighbor environment(s) from parent
    north = ipc_recv_select(parent_id, 0, 0, 0); 
    west = ipc_recv_select(parent_id, 0, 0, 0); 
    south = ipc_recv_select(parent_id, 0, 0, 0); 
    east = ipc_recv_select(parent_id, 0, 0, 0); 
    while(1){
        from_north = ipc_recv_select(north, 0, 0, 0);
        from_west = ipc_recv_select(west, 0, 0, 0);
        to_south = from_north+from_west*A_val;
        ipc_send(south, to_south, 0, 0);
        ipc_send(east, from_west, 0, 0);
    }
}

void
create_nodes(envid_t node_ids[5][5], int A[3][3]){
    int i, j, id;

    // initialize non-existant envids
    for (i = 0; i < 5; i++)
        for (j = 0; j < 5; j++)
            node_ids[i][j] = -1;
    // fork north nodes
    for (i = 0; i < 3; i++){
        if ((id = fork()) < 0)
            panic("fork: %e", id);
        if (id == 0)
            north_node();
        else{
            node_ids[0][i+1] = id;
        }
    }
    // fork west nodes
    for (i = 0; i < 3; i++){
        if ((id = fork()) < 0)
            panic("fork: %e", id);
        if (id == 0)
            west_node();
        else
            node_ids[i+1][0] = id;
    }
    // fork south nodes
    for (i = 0; i < 3; i++){
        if ((id = fork()) < 0)
            panic("fork: %e", id);
        if (id == 0)
            south_node(i==2);
        else
            node_ids[4][i+1] = id;
    }
    // fork east nodes
    for (i = 0; i < 3; i++){
        if ((id = fork()) < 0)
            panic("fork: %e", id);
        if (id == 0)
            east_node();
        else
            node_ids[i+1][4] = id;
    }
    // fork center nodes
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            if ((id = fork()) < 0)
                panic("fork: %e", id);
            if (id == 0)
                center_node(A[i][j]);
            else
                node_ids[i+1][j+1] = id;
        }
    }
}

void
start_nodes(envid_t node_ids[5][5]){
    int i, j;
    
    // start north nodes
    for (i = 0; i < 3; i++){
        ipc_send(node_ids[0][i+1], node_ids[1][i+1], 0, 0);
    }
    // start west nodes
    for (i = 0; i < 3; i++){
        ipc_send(node_ids[i+1][0], node_ids[i+1][1], 0, 0);
    }
    // start south nodes
    for (i = 0; i < 3; i++){
        ipc_send(node_ids[4][i+1], node_ids[3][i+1], 0, 0);
    }
    // start east nodes
    for (i = 0; i < 3; i++){
        ipc_send(node_ids[i+1][4], node_ids[i+1][3], 0, 0);
    }
    // start center nodes
    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            ipc_send(node_ids[i+1][j+1], node_ids[i][j+1], 0, 0);
            ipc_send(node_ids[i+1][j+1], node_ids[i+1][j], 0, 0);
            ipc_send(node_ids[i+1][j+1], node_ids[i+2][j+1], 0, 0);
            ipc_send(node_ids[i+1][j+1], node_ids[i+1][j+2], 0, 0);
        }
    }
}

void
umain(int argc, char **argv)
{
        // Input parameters--these may be arbitrarily changed
        int A[3][3] = {
            {1, 0, 0} ,
            {0, 2, 0} ,
            {0, 0, 1}
        };
        // Make sure in_length matches the actual length of IN
        int in_length = 8;
        int IN[8][3] = {
            {0, 1, 2} ,
            {1, 2, 3} ,
            {2, 3, 4} ,
            {3, 4, 5} ,
            {4, 5, 6} ,
            {5, 6, 7} ,
            {6, 7, 8} ,
            {7, 8, 9} ,
        };
	int i, j, id;
        envid_t node_ids[5][5];

        parent_id = sys_getenvid();
        create_nodes(node_ids, A);
        start_nodes(node_ids);

        // Send through IN matrix
        for (i = 0; i < in_length; i++)
            for (j = 0; j < 3; j++){
                ipc_send(node_ids[j+1][0], IN[i][j], 0, 0);
            }
}


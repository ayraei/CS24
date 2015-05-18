#include <stdlib.h>
#include <stdio.h>

void get_ids(int *uid, int *gid);

int main() {
    /* Call get_ids and prints the User and Group IDs. */
    int uid, gid;
    get_ids(&uid, &gid);
    printf("User ID is %d. Group ID is %d.\n", uid, gid);
    return 0;
}

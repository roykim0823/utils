#include <string.h>
#include <stdlib.h>

void *rrr;

int main(void) {
    // Store in RRR the address of the newly allocated BBB block
    rrr = strdup("bbb");

    // oops, we lost the start address of the BBB block
    rrr = NULL;

    return 0;
}

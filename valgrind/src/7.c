#include <string.h>
#include <stdlib.h>

void **rrr;

int main(void) {
    // Store in RRR the address of the newly allocated AAA block
    rrr = malloc(sizeof(void **));

    // Store in AAA the address of the newly allocated BBB block
    *rrr = strdup("bbb");

    // We setup a valid interior-pointer to 1 byte inside AAA
    // (start-pointer to AAA is lost)
    rrr = (void**)((char*)(rrr)+1);

    return 0;
}


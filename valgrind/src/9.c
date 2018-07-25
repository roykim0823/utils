#include <string.h>
#include <stdlib.h>

void **rrr;

int main(void) {
    // Store in RRR the address of the newly allocated AAA block
    rrr = malloc(sizeof(void **));

    // Store in AAA the address of the newly allocated BBB block
    *rrr = strdup("bbb");

    // We setup in AAA, a valid interior pointer to 1 byte inside
    // BBB (start-pointer to BBB is lost)
    *rrr = ((char*)(*rrr)+1);

    // oops, we lose the start address of the AAA block
    rrr = NULL;

    return 0;
}


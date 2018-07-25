#include <string.h>
#include <stdlib.h>

void **rrr;

int main(void) {
    // Store in RRR the address of the newly allocated AAA block
    rrr = malloc(sizeof(void **));

    // Store in AAA the address of the newly allocated BBB block
    *rrr = strdup("bbb");

    return 0;
}

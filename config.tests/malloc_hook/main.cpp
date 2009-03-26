#include <malloc.h>
#include <stdio.h>

void* testmalloc(size_t, const void*)
{ return (void*)0xDEADBEEF; }

int main( int, char ** )
{
    typeof(__malloc_hook) old_malloc_hook = __malloc_hook;
    __malloc_hook = testmalloc;
    if (malloc(1024) == (void*)0xDEADBEEF) {
        return 0;
    }
    __malloc_hook = old_malloc_hook;
    fprintf(stderr, "__malloc_hook compiles, but doesn't seem to work.\n");
    return 1;
}


#include <asm/page.h> // PAGE_SIZE,PAGE_MASK,PAGE_ALIGN

int main( int , char ** ) {
#ifdef PAGE_SIZE
    long page_size=PAGE_SIZE
        ;
#else
#error foo
#endif
#ifdef PAGE_MASK
    long page_mask=PAGE_MASK
        ;
#else
#error foo
#endif
#ifdef KERNEL
    long kernel=1;
#endif
    return 0;
}


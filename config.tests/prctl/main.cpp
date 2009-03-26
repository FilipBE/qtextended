#include <sys/prctl.h>

int main( int, char ** )
{
    char *name = "foo";
    return prctl(PR_SET_NAME, (unsigned long)name, 0, 0, 0);
}


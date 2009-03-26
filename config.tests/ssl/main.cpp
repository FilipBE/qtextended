#define main foo
#include MAIN_FILE
#undef main
int main( int argc, char **argv )
{
    return 0;
}


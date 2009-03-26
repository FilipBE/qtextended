#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

extern "C" {
    char * item_generator(const char * text, int num);
    char * command_generator(const char * text, int num);
    char ** item_completion(const char * text, int start, int end);
}

int main( int /*argc*/, char ** /*argv*/ )
{
    item_completion("foo", 0, 2);
    return 0;
}

char ** item_completion(const char * text, int start, int end)
{
    char ** matches = (char **)NULL;
    const char * non_space = text;
    while(*non_space == ' ') ++non_space;


    if(start == non_space - text)
        matches = rl_completion_matches(text, command_generator);
    else
        matches = rl_completion_matches(text, item_generator);

    rl_attempted_completion_over = 1;
    return (matches);
}

char * command_generator(const char * /*t*/, int /*num*/)
{
    return 0;
}

char * item_generator(const char * /*t*/, int /*num*/)
{
    return 0;
}


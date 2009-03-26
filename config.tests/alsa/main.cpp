#include <alsa/asoundlib.h>
int main(int, char**)
{
    snd_seq_t *seq_handle;
    snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
    return 0;
}

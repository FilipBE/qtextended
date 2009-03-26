/* Header created to match g711.c from Sun */

#ifndef G711_H
#define G711_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned char linear2alaw(short pcm_val);
short alaw2linear(unsigned char a_val);
unsigned char linear2ulaw(short pcm_val);
short ulaw2linear(unsigned char u_val);

#ifdef __cplusplus
};
#endif

#endif

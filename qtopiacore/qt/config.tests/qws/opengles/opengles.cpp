#include <GLES/gl.h>
#include <GLES/egl.h>

int main(int, char **)
{
    eglInitialize(0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    return 0;
}

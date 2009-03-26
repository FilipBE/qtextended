PROJECTS=
qtopia_project(subdirs)
CONFIG+=ordered
SUBDIRS+=tools
win32:SUBDIRS+=winmain
SUBDIRS+=corelib network gui opengl xml sql svg script testlib

SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
    oommanager.h \
    lowmemorytask.h \
    memorymonitor.h

SOURCES+=\
    oommanager.cpp \
    lowmemorytask.cpp \
    memorymonitor.cpp

oomconf [
    hint=image optional
    files=$$SERVER_PWD/etc/default/Trolltech/oom.conf
    path=/etc/default/Trolltech
]


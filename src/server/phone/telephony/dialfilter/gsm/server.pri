REQUIRES=enable_cell
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/dialfilter/abstract\
    /src/server/phone/telephony/dialercontrol\

HEADERS+=\
        gsmkeyactions.h \
        gsmkeyfilter.h \
        gsmfiltertask.h

SOURCES+=\
        gsmkeyactions.cpp \
        gsmkeyfilter.cpp \
        gsmfiltertask.cpp


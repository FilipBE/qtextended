REQUIRES=enable_cell
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/callpolicymanager/abstract\
    /src/server/pim/servercontactmodel\

HEADERS+=\
        cellmodemmanager.h

SOURCES+=\
        cellmodemmanager.cpp


TEMPLATE=app
CONFIG+=embedded
TARGET=ssl
SINGLEEXEC=$$(SINGLEEXEC)
equals(SINGLEEXEC,1) {
    SOURCES+=main.cpp
    DEFINES+=MAIN_FILE=$$define_string($$(QT_DEPOT)/config.tests/unix/openssl/openssl.cpp)
    LIBS+=-lssl -lcrypto
} else {
    SOURCES+=$$(QT_DEPOT)/config.tests/unix/openssl/openssl.cpp
}


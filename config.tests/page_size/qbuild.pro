TEMPLATE=app
CONFIG+=embedded
TARGET=page_size
SOURCES=main.cpp
KERNEL=$$(KERNEL)
equals(KERNEL,1):DEFINES+=__KERNEL__

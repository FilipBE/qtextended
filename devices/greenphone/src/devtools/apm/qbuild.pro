TEMPLATE=app
CONFIG+=embedded
TARGET=apm.bin

SOURCES=apm.c

script.files=apm
script.path=/bin
script.hint=script
INSTALLS+=script

pkg.domain=trusted


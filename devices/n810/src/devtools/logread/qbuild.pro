requires(contains(arch,arm))

TEMPLATE=app
CONFIG+=embedded
TARGET=logread

SOURCES	= logread.c

pkg.desc=Log reader to replace missing binary on N810

TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/src/libraries/qtopiasecurity
SOURCEPATH+=/src/libraries/qtopiabase
requires(enable_sxe)

TARGET=tst_qpackageregistry_SXE_INSTALLER

DEFINES+=SXE_INSTALLER

SOURCES=tst_qpackageregistry_SXE_INSTALLER.cpp qpackageregistry.cpp qsxepolicy.cpp keyfiler.cpp qtopiasxe.cpp
HEADERS+=qpackageregistry.h keyfiler_p.h qsxepolicy.h qtopiasxe.h


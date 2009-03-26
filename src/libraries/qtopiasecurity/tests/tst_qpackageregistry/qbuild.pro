TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/src/libraries/qtopiasecurity
requires(enable_sxe)

TARGET=tst_qpackageregistry
SOURCES=tst_qpackageregistry.cpp

# VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity
# INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiasecurity

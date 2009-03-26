TEMPLATE=app
CONFIG+=qt
TARGET=qtuitestrunner

QT*=script
MODULES*=qtuitestrunner

SOURCEPATH+=/src/libraries/qtuitest

include(qtuitestrunner.pro)


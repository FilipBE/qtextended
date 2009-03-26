TEMPLATE=app
TARGET=pngscale

CONFIG+=qt
# pngscale also converts .svg files into .png format
QT*=svg

SOURCES=\
    main.cpp


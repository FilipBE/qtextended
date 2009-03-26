TEMPLATE=app
TARGET=photoviewer

CONFIG+=qtopia
QTOPIA+=gfx 

pkg [
    name=gfxsample
    desc="GFX photoviewer sample for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=\
    main.cpp\
    gfximageloader.cpp\
    gfxcanvas.cpp\
    imagecollection.cpp\
    gfxcanvaslist.cpp\
    simplehighlight.cpp\
    gfxmenu.cpp\
    timeview.cpp\
    softkeybar.cpp\
    camera.cpp\
    tagdialog.cpp\
    textedit.cpp\
    photoriver.cpp\
    header.cpp

HEADERS=\
    gfximageloader.h\
    gfxcanvas.h\
    imagecollection.h\
    gfxcanvaslist.h\
    simplehighlight.h\
    gfxmenu.h\
    timeview.h\
    softkeybar.h\
    camera.h\
    tagdialog.h\
    textedit.h\
    photoriver.h\
    header.h

# Install rules

target [
    hint=sxe
    domain=trusted
]


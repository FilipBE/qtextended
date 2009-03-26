TEMPLATE=app
TARGET=radioplayer

CONFIG+=qtopia
QTOPIA+=pim
MODULES+=homeui webliteclient

requires(equals(QTOPIA_UI,home))

pkg [
    name=radioplayer
    desc="Radio player application."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCES=radioplayer.cpp

pics [
    hint=image
    files=radio.png
    path=/pics/weblite/radioplayer
]

desktop [
    hint=desktop
    files=radioplayer.desktop
    path=/apps/Applications
]


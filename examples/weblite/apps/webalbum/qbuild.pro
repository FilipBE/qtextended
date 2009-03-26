TEMPLATE=app
TARGET=webalbum

CONFIG+=qtopia
QTOPIA+=pim
MODULES+=homeui webliteclient weblitefeeds

requires(equals(QTOPIA_UI,home))

pkg [
    name=webalbum
    desc="Web album application."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCES=\
    webalbum.cpp

pics [
    hint=image
    files=webalbum.png
    path=/pics/weblite/album
]

desktop [
    hint=desktop
    files=webalbum.desktop
    path=/apps/Applications
]

settings [
    hint=image
    files=webalbum.conf
    path=/etc/default/Trolltech
]


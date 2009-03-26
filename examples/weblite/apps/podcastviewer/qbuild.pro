TEMPLATE=app
TARGET=podcastviewer

CONFIG+=qtopia
QTOPIA+=pim media
MODULES+=homeui webliteclient weblitefeeds

requires(equals(QTOPIA_UI,home))

pkg [
    name=podcastviewer
    desc="Podcast viewer application for Qt Extended."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCES=\
    podcastviewer.cpp

pics [
    hint=image
    files=podcastviewer.png
    path=/pics/weblite/podcastviewer
]

desktop [
    hint=desktop
    files=podcastviewer.desktop
    path=/apps/Applications
]

settings [
    hint=image
    files=podcastviewer.conf 
    path=/etc/default/Trolltech
]

service [
    hint=image
    files=podcasts.xml
    path=/etc
]


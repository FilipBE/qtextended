TEMPLATE=app
TARGET=webweather

CONFIG+=qtopia
QTOPIA+=pim
MODULES+=homeui webliteclient weblitefeeds

requires(equals(QTOPIA_UI,home))

pkg [
    name=weather
    desc="Yahoo weather application."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCES=weather.cpp

pics [
    hint=pics
    files=pics/*
    path=/pics/weblite/weather
]

desktop [
    hint=desktop
    files=weather.desktop
    path=/apps/Applications
]

settings [
    hint=image
    files=weather.conf
    path=/etc/default/Trolltech
]


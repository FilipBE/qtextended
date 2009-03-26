STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=classic-theme
    desc="Classic theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=classic.conf
    path=/etc/themes
    trtarget=Theme-Classic
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/classic
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/classic
]

bgimage [
    hint=background
    files=background.png
    path=/pics/themes/classic
]


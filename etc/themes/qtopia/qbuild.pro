STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=qtopia-theme
    desc="Qtopia theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=qtopia.conf
    path=/etc/themes
    trtarget=Theme-Qtopia
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/qtopia
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/qtopia
]

bgimage [
    hint=background
    files=\
        background.png\
        ladybug.png
    path=/pics/themes/qtopia
]


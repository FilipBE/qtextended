STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=deskphone-theme
    desc="Deskphone theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=deskphone.conf
    path=/etc/themes
    trtarget=Theme-Deskphone
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/deskphone
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/deskphone
]

bgimage [
    hint=background
    files=sunflower.png
    path=/pics/themes/deskphone
]

colors [
    hint=image
    files=Deskphone.scheme
    path=/etc/colors
]


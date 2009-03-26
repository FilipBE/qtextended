STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=crisp-theme
    desc="Crisp theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=crisp.conf
    path=/etc/themes
    trtarget=Theme-Crisp
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/crisp
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/crisp
]

bgimage [
    hint=background
    files=background.png
    path=/pics/themes/crisp
]


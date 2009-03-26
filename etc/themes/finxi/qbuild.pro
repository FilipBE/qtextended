STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=finxi-theme
    desc="Finxi theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=finxi.conf
    path=/etc/themes
    trtarget=Theme-Finxi
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/finxi
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/finxi
]

bgimage [
    hint=background
    files=background.png
    path=/pics/themes/finxi
]


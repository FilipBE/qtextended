STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=smart-theme
    desc="Smart theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=smart.conf
    path=/etc/themes
    trtarget=Theme-Smart
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/smart
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/smart
]

bgimage [
    hint=background
    files=background.png
    path=/pics/themes/smart
]


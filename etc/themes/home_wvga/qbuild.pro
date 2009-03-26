STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=home-wvga-theme
    desc="Home WVGA theme for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

conf [
    hint=themecfg
    files=home_wvga.conf
    path=/etc/themes
    trtarget=Theme-home_wvga
]

data [
    hint=image
    files=*.xml *rc
    path=/etc/themes/home_wvga
]

pics [
    hint=pics
    files=pics/*
    path=/pics/themes/home_wvga
]

bgimage [
    hint=background
    files=sunflower.png
    path=/pics/themes/home_wvga
]

colors [
    hint=image
    files=home_wvga.scheme
    path=/etc/colors
]


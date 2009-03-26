!equals(QTOPIA_UI,home) {
    # Phone Edition additional projects

    THEMES=finxi
    PROJECTS-=\
        settings/beaming
    PROJECTS*=\
        settings/startupflags\
        ../examples/webviewer\
        ../examples/fbinteg\
        ../examples/whereabouts/mappingdemo

} else {
    # Home edition projects 

    THEMES=home_wvga

    PROJECTS*=\
        tools/pimdata\
        tools/pimdatagui\
        tools/phonesim

    PROJECTS-=\
        3rdparty/plugins/inputmethods/pkim\
        applications/camera\
        applications/helpbrowser\
        applications/mediarecorder\
        applications/mediaplayer\
        applications/textedit\
        applications/todo\
        games/fifteen\
        games/mindbreaker\
        games/minesweep\
        games/parashoot\
        games/qasteroids\
        games/snake\
        libraries/handwriting\
        libraries/qtopiaprinting\
        plugins/codecs/wavrecord\
        plugins/inputmethods/dockedkeyboard\
        plugins/inputmethods/keyboard\
        plugins/inputmethods/predictivekeyboard\
        plugins/qtopiaprinting/bluetooth\
        settings/appearance\
        settings/beaming\
        settings/hwsettings\
        settings/homescreen\
        settings/language\
        settings/light-and-power\
        settings/logging\
        settings/packagemanager\
        settings/profileedit\
        settings/security\
        settings/sipsettings\
        settings/speeddial\
        settings/systemtime\
        settings/words\
        tools/atinterface\
        tools/device_updater/plugin\
        tools/printserver\
        tools/qdsync/app\
        tools/qdsync/common\
        tools/qdsync/pim

    SERVER_PROJECTS-=\
        server/comm/usbgadget\
        server/ui/usbgadgetselector\
        server/infrastructure/camera\
        server/infrastructure/signalstrength\
        server/phone/browserscreen/gridbrowser \
        server/phone/browserscreen/wheelbrowser \
        server/phone/callscreen/themed \
        server/phone/contextlabel/base \
        server/phone/contextlabel/themed \
        server/phone/header/themed \
        server/phone/powermanager\
        server/phone/samples/e1 \
        server/phone/samples/e2 \
        server/phone/samples/e3 \
        server/phone/secondarydisplay/themed \
        server/phone/telephony/atemulator\
        server/phone/telephony/callpolicymanager/asterisk\
        server/phone/ui/components/simapp \
        server/ui/components/calibrate\
        server/ui/components/firstuse
}

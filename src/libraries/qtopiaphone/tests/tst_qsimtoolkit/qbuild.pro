TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/src/server/phone/ui/components/simapp

QTOPIA*=phone

TARGET=tst_qsimtoolkit
contains(DEFINES, QT_QWS_GREENPHONE):requires(false)

DEFINES+=DUMMY_TEST_SOUNDS

HEADERS=tst_qsimtoolkit.h \
        simapp.h \
        simwidgets.h \
        simicons.h \
        testphonecall.h
SOURCES=tst_qsimtoolkit.cpp \
        displaytext.cpp \
        getinkey.cpp \
        getinput.cpp \
        moretime.cpp \
        playtone.cpp \
        pollinterval.cpp \
        refresh.cpp \
        setupmenu.cpp \
        selectitem.cpp \
        sendsms.cpp \
        sendss.cpp \
        sendussd.cpp \
        setupcall.cpp \
        pollingoff.cpp \
        setupidlemodetext.cpp \
        poweroffcard.cpp \
        poweroncard.cpp \
        providelocalinfo.cpp \
        setupeventlist.cpp \
        performcardapdu.cpp \
        getreaderstatus.cpp \
        timermanagement.cpp \
        runatcommand.cpp \
        senddtmf.cpp \
        languagenotification.cpp \
        launchbrowser.cpp \
        openchannel.cpp \
        closechannel.cpp \
        receivedata.cpp \
        senddata.cpp \
        getchannelstatus.cpp \
        smsppdownload.cpp \
        smscbdownload.cpp \
        eventdownload.cpp \
        mosmscontrol.cpp \
        callcontrolbysim.cpp \
        simapp.cpp \
        simwidgets.cpp \
        simicons.cpp \
        testphonecall.cpp

include(/tests/shared/qfuturesignal.pri)

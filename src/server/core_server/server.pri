x11:MODULES*=Xtst
UNIFIED_NCT_LUPDATE=1

HEADERS+=\
         alarmcontrol.h \
         applicationlauncher.h\
         contentserver.h\
         defaultbattery.h\
         devicebuttontask.h \
         environmentsetuptask.h\
         idletaskstartup.h\
         pressholdgate.h\
         qabstractmessagebox.h\
         qabstractserverinterface.h \
         qcopfile.h \
         qcoprouter.h \
         qdeviceindicatorsprovider.h \
         qtopiainputevents.h \
         qtopiapowermanager.h\
         qtopiapowermanagerservice.h \
         qtopiaserverapplication.h \
         qtopiaservertasks_p.h \
         systemsuspend.h \
         timecontrol.h \
         timemonitor.h \
         timeupdateservice.h \
         uifactory.h \
         virtualkeyboardservice.h \
         windowmanagement.h 

SOURCES+=\
         alarmcontrol.cpp \
         applicationlauncher.cpp\
         contentserver.cpp\
         devicebuttontask.cpp \
         defaultbattery.cpp\
         environmentsetuptask.cpp\
         idletaskstartup.cpp\
         pressholdgate.cpp\
         qabstractmessagebox.cpp\
         qabstractserverinterface.cpp \
         qcopfile.cpp \
         qcoprouter.cpp \
         qdeviceindicatorsprovider.cpp \
         qtopiapowermanager.cpp\
         qtopiapowermanagerservice.cpp \
         qtopiaserverapplication.cpp\
         systemsuspend.cpp \
         timecontrol.cpp \
         timemonitor.cpp \
         timeupdateservice.cpp \
         uifactory.cpp \
         virtualkeyboardservice.cpp 

X11.TYPE=CONDITIONAL_SOURCES
X11.CONDITION=x11
X11.SOURCES=\
    qtopiainputevents_x11.cpp \
    windowmanagement_x11.cpp 

QWS.TYPE=CONDITIONAL_SOURCES
QWS.CONDITION=qws
QWS.SOURCES=\
    qtopiainputevents.cpp\
    windowmanagement.cpp 

launcherservice [
    hint=image
    files=$$SERVER_PWD/services/Launcher/qpe
    path=/services/Launcher
]

qtopiapowermanager [
    hint=image
    files=$$SERVER_PWD/services/QtopiaPowerManager/qpe
    path=/services/QtopiaPowerManager
]

suspendservice [
    hint=image
    files=$$SERVER_PWD/services/Suspend/qpe
    path=/services/Suspend
]

timeupdateservice [
    hint=image
    files=$$SERVER_PWD/services/TimeUpdate/qpe
    path=/services/TimeUpdate
]

virtualkeyboardservice [
    hint=image
    files=$$SERVER_PWD/services/VirtualKeyboard/qpe
    path=/services/VirtualKeyboard
]

tasks [
    hint=image
    files=$$SERVER_PWD/etc/Tasks.cfg
    path=/etc
]

settings [
    hint=image
    files=\
        $$SERVER_PWD/etc/default/Trolltech/locale.conf\
        $$SERVER_PWD/etc/default/Trolltech/Security.conf\
        $$SERVER_PWD/etc/default/Trolltech/IniValueSpace.conf\
        $$SERVER_PWD/etc/default/Trolltech/qpe.conf\
        $$SERVER_PWD/etc/default/Trolltech/UIFactory.conf\
        $$SERVER_PWD/etc/default/Trolltech/ServerWidgets.conf
    path=/etc/default/Trolltech
]

settings2 [
    hint=image optional
    files=\
        $$SERVER_PWD/etc/default/Trolltech/Hardware.conf\
        $$SERVER_PWD/etc/default/Trolltech/Launcher.conf
    path=/etc/default/Trolltech
]


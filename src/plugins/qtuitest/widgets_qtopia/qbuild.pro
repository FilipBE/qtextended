TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qtuitestwidgets

PLUGIN_TYPE=qtuitest_widgets
PLUGIN_FOR=qtopia

MODULES*=qtuitest
QTOPIA+=theming
# This uses symbols from another plugin!
CONFIG-=link_test

SOURCEPATH+=../widgets_qt

SOURCES += \
        localqtopiawidget.cpp \
        remotewidget.cpp \
        remotewidgetadaptor.cpp \
        testcallmanager.cpp \
        testcontextlabel.cpp \
        testdialer.cpp \
        testiconselector.cpp \
        testnumberdisplay.cpp \
        testoptionsmenu.cpp \
        testphonecalc.cpp \
        testphonelauncherview.cpp \
        testphonequickdialerscreen.cpp \
        testphonetouchdialerscreen.cpp \
        testpkim.cpp \
        testpredictivekeyboard.cpp \
        testqtopiafactory.cpp \
        testsmoothlist.cpp \
        testthemedhomescreen.cpp \
        testthemedview.cpp \
        testthemelistmodel.cpp

HEADERS += \
        localqtopiawidget.h \
        remotewidget.h \
        remotewidgetadaptor.h \
        testcallmanager.h \
        testcontextlabel.h \
        testdialer.h \
        testiconselector.h \
        testnumberdisplay.h \
        testoptionsmenu.h \
        testphonecalc.h \
        testphonelauncherview.h \
        testphonequickdialerscreen.h \
        testphonetouchdialerscreen.h \
        testpkim.h \
        testpredictivekeyboard.h \
        testqtopiafactory.h \
        testsmoothlist.h \
        testthemedhomescreen.h \
        testthemedview.h \
        testthemelistmodel.h


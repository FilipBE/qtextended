# NOTE: this file is used for building QtUiTest from outside of Qtopia.
# Please don't delete this file or the non-qbuild portions of it.

!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

SOURCES += \
        localwidget.cpp \
        testabstractbutton.cpp \
        testabstractitemmodel.cpp \
        testabstractitemview.cpp \
        testabstractspinbox.cpp \
        testcalendarwidget.cpp \
        testcheckbox.cpp \
        testcombobox.cpp \
        testdateedit.cpp \
        testdatetimeedit.cpp \
        testfactory.cpp \
        testgenericcheckwidget.cpp \
        testgenerictextwidget.cpp \
        testgroupbox.cpp \
        testlabel.cpp \
        testlineedit.cpp \
        testmenu.cpp \
        testpushbutton.cpp \
        testtabbar.cpp \
        testtext.cpp \
        testtextedit.cpp \
        testtimeedit.cpp \
        testwidget.cpp

HEADERS += \
        localwidget.h \
        testabstractbutton.h \
        testabstractitemmodel.h \
        testabstractitemview.h \
        testabstractspinbox.h \
        testcalendarwidget.h \
        testcheckbox.h \
        testcombobox.h \
        testdateedit.h \
        testdatetimeedit.h \
        testfactory.h \
        testgenericcheckwidget.h \
        testgenerictextwidget.h \
        testgroupbox.h \
        testlabel.h \
        testlineedit.h \
        testwidgetslog.h \
        testmenu.h \
        testpushbutton.h \
        testtabbar.h \
        testtext.h \
        testtextedit.h \
        testtimeedit.h \
        testwidget.h

!qbuild:!standalone{
    depends(libraries/qtuitest)
}

standalone {
    TEMPLATE=lib
    CONFIG+=plugin
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    VPATH+=$$PWD
    INCLUDEPATH+=$$PWD
    INCLUDEPATH+=$$(QTOPIA_DEPOT_PATH)/src/libraries/qtuitest
    TARGET=qtwidgets
}

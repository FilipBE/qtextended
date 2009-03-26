TEMPLATE=app
CONFIG+=qtopia unittest

QTOPIA*=mail
MODULES*=sqlite

TARGET=tst_qmailstore
RESOURCES += $$PWD/../../../../../src/libraries/qtopiamail/qtopiamail.qrc

DEFINES*=QTOPIA_CONTENT_INSTALLER

SOURCES*=$$PWD/../../../../../src/tools/dbmigrate/migrateengine.cpp \
         $$PWD/../../../../../src/tools/dbmigrate/qtopiapim/pimmigrate.cpp \
         $$PWD/../../../../../src/tools/dbmigrate/qtopiaphone/phonemigrate.cpp \
         tst_qmailstore.cpp

HEADERS*=$$PWD/../../../../../src/tools/dbmigrate/migrateengine.h \
         $$PWD/../../../../../src/tools/dbmigrate/qtopiapim/pimmigrate.h \
         $$PWD/../../../../../src/tools/dbmigrate/qtopiaphone/phonemigrate.h


TEMPLATE=plugin
TARGET=pkim

PLUGIN_FOR=qtopia
PLUGIN_TYPE=inputmethods

CONFIG+=qtopia singleexec
MODULES*=inputmatch handwriting

HEADERS      = pkim.h \
               pkimpl.h \
	       modepicker.h \
               charmatch.h \
	       charlist.h
SOURCES      = pkimpl.cpp \
               pkim.cpp \
	       modepicker.cpp \
               charmatch.cpp \
	       charlist.cpp

im.files=$$QTOPIA_DEPOT_PATH/etc/im/pkim/*
im.path=/etc/im/pkim
INSTALLS+=im
pics.files=$$QTOPIA_DEPOT_PATH/pics/pkim/*
pics.path=/pics/pkim
INSTALLS+=pics
dictinternet.files=$$QTOPIA_DEPOT_PATH/etc/dict/internet
dictinternet.path=/etc/dict
dictinternet.hint=dawg
INSTALLS+=dictinternet
fsim.files = $$QTOPIA_DEPOT_PATH/etc/qimpen/fstext.qpt\
	$$QTOPIA_DEPOT_PATH/etc/qimpen/fsnum.qpt\
	$$QTOPIA_DEPOT_PATH/etc/qimpen/fscombining.qpt\
	$$QTOPIA_DEPOT_PATH/etc/qimpen/fs.conf
fsim.path=/etc/qimpen/
INSTALLS+=fsim

pkg.domain=trusted

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=im-pkim.html im-handwriting.html
help.hint=help
INSTALLS+=help

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES


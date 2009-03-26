TEMPLATE=app
CONFIG*=qtopia qtopia_main no_tr link_test
QTOPIA*=phone
TARGET=testapp4
SOURCES*=main.cpp

desktop.files=testapp4.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pkg.domain=trusted


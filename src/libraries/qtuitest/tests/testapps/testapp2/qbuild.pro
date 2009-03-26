TEMPLATE=app
TARGET=testapp2
CONFIG*=qtopia qtopia_main no_tr link_test
QTOPIA*=phone
SOURCES*=main.cpp

desktop.files=testapp2.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pkg.domain=trusted


TEMPLATE=app
QTOPIA*=phone
TARGET=testapp3
CONFIG*= qtopia qtopia_main no_tr link_test
SOURCES*=main.cpp

desktop.files=testapp3.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pkg.domain=trusted


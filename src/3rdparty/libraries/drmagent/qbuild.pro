MODULE_NAME=drmagent
requires(exists(/config.tests/locate_drmagent.pri))
include(/config.tests/locate_drmagent.pri)
dep.libs.TYPE=DEPENDS PERSISTED SDK
dep.libs.EVAL="LIBS+="$$DRMAGENT

install_lib.commands=\
    "cp -aRp "$$DRMAGENT"* "$$QTOPIA_IMAGE"/lib"
release:!isEmpty(MKSPEC.STRIP):install_lib.commands+=\
    $$MKSPEC.STRIP" "$$MKSPEC.STRIPFLAGS_LIB" "$$QTOPIA_IMAGE"/lib/"$$basename($$DRMAGENT)
install_lib.path=/lib
install_lib.hint=image

TEMPLATE=app
TARGET=sxe_policy_runner

CONFIG+=embedded

pkg [
    name=sxe_policy_runner
    desc="SXE policy runner for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=main.c


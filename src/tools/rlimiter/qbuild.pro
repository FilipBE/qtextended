TEMPLATE=app
TARGET=rlimiter

CONFIG+=embedded

pkg [
    name=rlimiter
    desc="Resource limiter for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=\
    main.c


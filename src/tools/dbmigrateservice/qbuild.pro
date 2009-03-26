TEMPLATE=app
TARGET=dbmigrate

CONFIG+=qtopia singleexec
MODULES*=sqlite

pkg [
    name=dbmigrate-service
    desc="Database migration service for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    dbmigrateservice.h

SOURCES=\
    main.cpp\
    dbmigrateservice.cpp

# Install rules

dbmigrateservice [
    hint=image
    files=services/DBMigrationEngine/dbmigrate
    path=/services/DBMigrationEngine
]

dbmigrateDSservice [
    hint=image
    files=etc/qds/DBMigrationEngine
    path=/etc/qds
]


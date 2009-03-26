UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/ui/abstractinterfaces/taskmanager\
    /src/server/ui/launcherviews/base\

HEADERS+=\
        taskmanager.h \
        taskmanagerservice.h \
        favoritesservice.h \
        qfavoriteserviceslist.h 

SOURCES+=\
        taskmanager.cpp \
        taskmanagerservice.cpp \
        favoritesservice.cpp \
        qfavoriteserviceslist.cpp 

taskmanagerdesktop [
    hint=desktop
    files=$$SERVER_PWD/taskmanager.desktop
    path=/apps/Settings
]

taskmanagerservice [
    hint=image
    files=$$SERVER_PWD/services/TaskManager/qpe
    path=/services/TaskManager
]

favoritesservice [
    hint=image
    files=$$SERVER_PWD/services/Favorites/qpe
    path=/services/Favorites
]


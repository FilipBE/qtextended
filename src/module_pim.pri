#This file contains projects that make up the PIM module.

# Addressbook, datebook & todo
PROJECTS*=\
    libraries/qtopiamail \
    3rdparty/libraries/vobject \
    libraries/qtopiaaudio \
    libraries/qtopiapim \
    applications/addressbook 

SERVER_PROJECTS*=\
    server/pim/savetocontacts \                 #simple msg box to save contacts
    server/pim/servercontactmodel \             #customized contact model for server
    server/pim/buddysyncer                      #presence synchronization

enable_cell{      
    SERVER_PROJECTS*=server/pim/simcard                                  #SIM Card phonebook sync
}

!equals(QTOPIA_UI,home) {
    PROJECTS*=\
        applications/datebook \
        applications/todolist \
        tools/qdsync/pim \
        plugins/codecs/wavrecord \      #mediarecorder requires this
        applications/mediarecorder \

    # Low-level synchronization support
    PROJECTS*=\
        tools/qdsync/app \
        tools/qdsync/common/qdsync_common

    SERVER_PROJECTS*=\
        tools/qdsync/server
}

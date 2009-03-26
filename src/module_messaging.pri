#This file contains projects that make up the Messaging module.

PROJECTS*=\
        applications/qtmail \
        3rdparty/libraries/vobject\
        libraries/qtopiapim \
        libraries/qtopiamail \
        plugins/composers/email \
        plugins/composers/generic \
        plugins/viewers/generic \
        plugins/viewers/conversation \
        tools/messageserver \
        tools/sysmessages
 
enable_cell {
    PROJECTS*=\
        libraries/qtopiasmil \
        plugins/composers/mms \
        plugins/viewers/smil

}

enable_qtopiamedia {
    equals(QTOPIA_UI,home):PROJECTS*=\
        libraries/homeui\
        plugins/viewers/voicemail \
        plugins/composers/videomail 
}

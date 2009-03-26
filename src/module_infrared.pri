#This file contains projects that make up the Infrared module.

SERVER_PROJECTS*=\
        server/comm/obex \                #obex support for various services
        server/comm/session \             #base session management
        server/comm/filetransfer \        #file transfer via obex/push
        server/ui/filetransferwindow \    #monitors fiel transfers
        server/infrared/powermgr \        #infrared session/power manager
        server/infrared/beaming \         #infrared file send service
        server/infrared/obexpush          #infrared obex push support

PROJECTS*=\
    settings/beaming \
    3rdparty/libraries/vobject\
    libraries/qtopiapim \                 #vcard support


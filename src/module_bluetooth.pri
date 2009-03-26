#This file contains projects that make up the Bluetooth module.

PROJECTS*=\
    libraries/qtopiaaudio\
    3rdparty/libraries/vobject\
    libraries/qtopiapim \                  #vcard support
    libraries/qtopiaprinting \             #Bluetooth printing
    tools/printserver \                    #Bluetooth printing 

PROJECTS*=\
    settings/btsettings \                   #Bluetooth settings app
    applications/bluetooth \                #Bluetooth FTP
    plugins/network/bluetooth \             #Bluetooth DUN client
    plugins/qtopiaprinting/bluetooth        #Bluetooth printing
    
SERVER_PROJECTS*=\
    server/comm/session \             #base session management
    server/comm/obex \                #obex support for various services
    server/comm/filetransfer \        #file transfer via obex/push
    server/ui/filetransferwindow \    #monitors file transfers
    server/bluetooth/powermgr \       #Bluetooth session/power manager
    server/bluetooth/obexpush \       #Bluetooth obex push support
    server/bluetooth/filepush \       #Bluetooth Push service
    server/bluetooth/servicemgr \     #Bluetooth service backend
    server/bluetooth/pinhelper \      #Bluetooth passkey agent
    server/bluetooth/audiovolumemgr \ #Bluetooth audio volume manager
    server/bluetooth/serial \         #Bluetooth serial profile support
    server/bluetooth/dun \            #Bluetooth dial-up server profile
    server/bluetooth/ftp \            #Bluetooth ftp profile support
    server/bluetooth/scomisc \        #Bluetooth SCO socket support
    server/bluetooth/hs \             #Bluetooth headset support
    server/media/volumemanagement \   #volume control backend
    server/net/netserver \            #network server required for DUN client support
    server/ui/volumedlg               #shown when volume keys pressed


enable_telephony:SERVER_PROJECTS*=\
    server/bluetooth/hf              #Bluetooth handsfree support (AT modem emulator)

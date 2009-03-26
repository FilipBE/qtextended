#This file contains projects that make up the Telephony module.

PROJECTS*=\
    libraries/qtopiaphone \
    3rdparty/libraries/vobject\
    libraries/qtopiapim \
    libraries/qtopiaaudio \
    settings/profileedit \
    tools/atinterface \
    3rdparty/libraries/gsm \
    3rdparty/libraries/inputmatch \
    plugins/codecs/wavrecord \
    plugins/themeitems/dialerlineedit

SERVER_PROJECTS*=\
    server/phone/telephony/phoneserver/base \    #starts telephony backend
    server/phone/telephony/atemulator \          #activates atinterface/at emulator app
    server/phone/ui/components/calltypeselector\ #choose call type dialog
    server/phone/telephony/dialfilter/abstract \ #dial filter interface
    server/phone/telephony/dialproxy \           #dial service and dial string processing
    server/phone/telephony/networkregistration \ #update user about net reg changes
    server/phone/media/audiohandler/abstract \   #audiohandler interface
    server/phone/media/audiohandler/call \       #audio management for speaker/headset/handset redirection
    server/phone/media/audiohandler/dialtone \   #audio management for dial tones
    server/phone/media/dtmfaudio \               #plays dtmf tones (commonly used by desk phones)
    server/phone/dialer/abstract \               #abstract dialer UI interface
    server/phone/dialer/touch \                  #touchscreen based dialer UI 
    server/phone/dialer/keypad \                 #keypad based dialer UI
    server/phone/callhistory/abstract \          #abstract call history interface
    server/phone/callhistory/default \           #the only callhistory implementation so far
    server/phone/callscreen/abstract \           #abstract callscreen interface
    server/phone/callscreen/themed \             #themed phone callscreen
    server/phone/telephony/callpolicymanager/abstract \ #call policy
    server/phone/telephony/dialercontrol \       #dialer control
    server/phone/telephony/msgcontrol \          #message control/management
    server/phone/telephony/ringcontrol \         #controls ringtone
    server/phone/profileprovider \               #QPhoneProfile API backend
    server/pim/savetocontacts \                  #simple msg box to save contacts
    server/pim/servercontactmodel \              #customized contact model for server
    server/infrastructure/messageboard \         #messsage board for server
    server/infrastructure/signalstrength \       #QSignalSource backend
    server/phone/ui/callcontactmodelview \    #model/view/delegate for callcontact details

equals(QTOPIA_UI,home) {
    PROJECTS*=\
              libraries/homeui 
    SERVER_PROJECTS*=\
              server/phone/callscreen/deskphone           #deskphone callscreen
}

enable_qtopiamedia:SERVER_PROJECTS*=server/phone/telephony/videoringtone   #video ringtone support



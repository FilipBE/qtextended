#This file contains projects that make up the IP communications module.
    
#this module is dependent on telephony module
include(module_telephony.pri)

SERVER_PROJECTS*=\
    server/phone/telephony/callpolicymanager/voip \     #voip manager
    server/phone/telephony/callpolicymanager/jabber\    #jabber manager
    server/phone/telephony/callpolicymanager/asterisk\  #asterisk manager
    server/phone/telephony/presenceservice \            #manages DND/Away/Online presence
    server/phone/ui/components/presenceeditor           #presence status editor dialog
      
PROJECTS*=\
    3rdparty/libraries/g711 \
    settings/gtalksettings 

enable_dbus:PROJECTS*=\
    tools/telepathyagent


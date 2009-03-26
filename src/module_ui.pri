#This file contains projects that make up the UI module.

PROJECTS*=\
    3rdparty/libraries/vobject \
    libraries/qtopiapim \
    settings/appearance \
    settings/homescreen \
    settings/speeddial 

# homeui library - enabled for everything until other .pro files have been fixed
equals(QTOPIA_UI,home) {
    PROJECTS*=libraries/homeui \
              plugins/styles/home 
    SERVER_PROJECTS*=\
              server/phone/browserscreen/deskphone \      #deskphone browser UI
              server/phone/contextlabel/deskphone\        #deskphone context label
}

!equals(QTOPIA_UI,home) {
    SERVER_PROJECTS*=\
        server/ui/components/homescreenwidgets \    #Homescreen widgets for smart theme
        server/ui/components/touchscreenlockdlg \   #locks touchscreen device
        server/phone/browserscreen/wheelbrowser \   #wheel based browser UI
        server/phone/contextlabel/base \            #abstract context label that handled keys (derived from contextlabel/abstract)
        server/phone/contextlabel/themed \          #themed context label
        server/phone/header/themed \                #themed header
        server/phone/secondarydisplay/themed        #themed secondary display widget

    enable_cell {
        SERVER_PROJECTS*=\
              server/phone/samples/e1 \                           #Server widget example E1
              server/phone/samples/e2 \                           #Server widget example E2
              server/phone/samples/e3                            #Server widget example E3

        PROJECTS*=\
              settings/serverwidgets                              #only enabled when e1,e2,e3 enabled
    }
}

SERVER_PROJECTS*=\
    server/infrastructure/messageboard \        #messsage board for homescreen
    server/infrastructure/softmenubar \         #QSoftMenuBar API backend
    server/phone/serverinterface/phonelauncher \#phone implementation of QAbstractServerInterface
    server/ui/abstractinterfaces/stdmessagebox \#standard message box
    server/phone/secondarydisplay/abstract \    #abstract secondary display widget
    server/phone/browserscreen/abstract \       #abstract browser interface
    server/phone/contextlabel/abstract \        #abstract context label
    server/phone/header/abstract \              #abstract header
    server/phone/homescreen/abstract \          #abstract home/idle screen interface
    server/phone/homescreen/themed \            #themeable home/idle screen
    server/phone/homescreen/basic               #non-ui home/idle screen 
    
#    server/ui/launcherviews/hierarchdocumentview\#hierarchical document launcher view

equals(QTOPIA_UI,mobile) {
    THEMES*=\
        # Best for platform edition (no telephony)
        smart\
        # Best for phone edition
        classic\
        crisp\
        finxi\
        qtopia
}

equals(QTOPIA_UI,home) {
    THEMES*=\
        deskphone\
        home_wvga
}


#This file contains projects that support developers or provide demo implementations. 
#Usually they are not part of shipped devices

PROJECTS*=\
    tools/spygrind \
    tools/vsexplorer \
    settings/logging \
    3rdparty/applications/micro_httpd \
    settings/appservices \

SERVER_PROJECTS*=\
    server/memory/base \                        #OOM component
    server/memory/testmonitor                   #optional monitor for development purposes
    
enable_cell {
    # This isn't supported but it's included anyway
    PROJECTS*=\
        tools/phonesim\
        tools/phonesim/lib/phonesim
}



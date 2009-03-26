include($$path(.,project)/foo.pri)
include(!$$path(.,project)/foo.pri)
include($$path(.,project)/bar/foo.pri)
include(!$$path(.,project)/bar/foo.pri)

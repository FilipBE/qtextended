!cross_compile {
	DIRECTFBCFLAGS = $$system(directfb-config --cflags)
	DIRECTFBLIBS = $$system(directfb-config --libs)
}

QMAKE_CXXFLAGS += $$DIRECTFBCFLAGS
LIBS += $$DIRECTFBLIBS 

TEMPLATE=plugin
CONFIG+=qtopia
TARGET=nokiaaudiohardware

QTOPIA*=comm audio

PLUGIN_FOR=qtopia
PLUGIN_TYPE=audiohardware

HEADERS		=  nokiaaudioplugin.h
SOURCES	        =  nokiaaudioplugin.cpp

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES


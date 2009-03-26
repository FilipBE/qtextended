# Setup the PROJECTS variables

PROJECTS*=\
    libraries/qt\
    libraries/qtopiacore\
    tools/pngscale\
    tools/svgtopicture

enable_qvfb:PROJECTS*=tools/qt/qvfb

# These projects are just stubs for system libraries
PROJECTS*=\
    3rdparty/libraries/alsa\
    3rdparty/libraries/pulse\
    3rdparty/libraries/crypt\
    3rdparty/libraries/pthread\
    3rdparty/libraries/realtime\
    3rdparty/libraries/resolv\
    3rdparty/libraries/mathlib\
    3rdparty/libraries/Xtst\

build_qtopia {
    projects_include_modules()

    # Load a device-specific file (if it exists)
    !isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/projects.pri):include($$DEVICE_CONFIG_PATH/projects.pri)
    # custom projects (not for device builds)
    isEmpty(DEVICE_CONFIG_PATH):include(custom.pri)
    else:exists($$DEVICE_CONFIG_PATH/custom.pri):include($$DEVICE_CONFIG_PATH/custom.pri)
    # local projects
    exists($$QPEDIR/src/local.pri):include($$QPEDIR/src/local.pri)
}


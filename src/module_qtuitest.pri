#This file contains projects that make up the Qtopia Test module.

!x11 {
    PROJECTS*=\
        # QtUitest plugin interfaces
        libraries/qtuitest \
        # QtUitest reference implementation (plugins)
        plugins/qtuitest/server \
        plugins/qtuitest/widgets_qt \
        plugins/qtuitest/widgets_qtopia \
        plugins/qtuitest/application/qtuitest_appslave \
        # QtUitest script interpreter
        tools/qtuitestrunner/liboverrides \
        tools/qtuitestrunner \
        tools/qtuitestrunner/lib_qtopia/qtuitestrunner \
        # performance test helpers
        plugins/qtopiacore/gfxdrivers/perftestqvfb \
        plugins/qtopiacore/gfxdrivers/perftestlinuxfb
}

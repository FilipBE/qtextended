# Implicit dependencies

###
### NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
###
### This code must match the logic found in src/qtopiadesktop/build/bin/Qtopia/BlackMagic.pm
###
### NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
###

qt {
    part_of_qtopiadesktop|qtopiadesktop|contains(PROJECT_TYPE,desktop) {
        !containsre($$QTOPIA_ID,^libraries/qt):depends(libraries/qt/*)
    } else {
        !containsre($$QTOPIA_ID,^libraries/qtopiacore):depends(libraries/qtopiacore/*)
    }
}

qtopia {
    !no_qtopiabase:depends(libraries/qtopiabase)
    !contains(PROJECT_TYPE,core):depends(libraries/qtopia)
}

qtopiadesktop:!contains(PROJECT_TYPE,core) {
    depends(libraries/qtopiadesktop)
}

systemtest:depends(tools/qtuitestrunner/lib)
unittest|integrationtest {
    contains(PROJECT_type,desktop) {
        depends(libraries/qt/qtestlib)
    } else {
        depends(libraries/qtopiacore/qtestlib)
    }
}

# This is for directories that don't have a qbuild.pro file
#message(blank.pri)

MYVAR=$$path($$project(),existing)
isEmpty(MYVAR):error(blank.pri: $$project() does not exist!)

# We want to warn about unported projects in PROJECTS
CONFIG+=no_qbuild_pro
CONFIG+=defaults

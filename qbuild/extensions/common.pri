# This is loaded in all cases
#message(common.pri)

# Set the initial configuration
CONFIG=functions mkspec rules templates subdirs

# This is used to sniff out qbuild-specific parts of qbuild.pro
CONFIG*=qbuild

# Be strict about disabled projects (that are listed in PROJECTS)
# If this is not set, only projects that are disabled due to dependencies become errors.
#CONFIG+=strict_projects


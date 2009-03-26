# This is an example of a distribution-specific config.pri.
# See doc/html/buildsystem.html for more information.

# to add a component to a project list, (for example, mymediaplayer)
#
# PROJECTS*=myapps/mymediaplayer

# to remove a component from a project list
#
# PROJECTS-=applications/mediaplayer

# To make project lists be configuration dependent
#
# useMyMediaPlayer:APP_PROJECTS*=myapps/mymediaplayer
# useMyMediaPlayer:APP_PROJECTS-=applications/mediaplayer
#
# and when running configure, pass an additional argument
#
# configure <config_options> -config "CONFIG+=useMyMediaPlayer"

# to make a project list be dependent on existing items in the project list
#
# contains(PROJECTS,myapps/mymediaplayer) {
#     PROJECTS*=myplugins/myaudiocodec
# }

# For more information see "The Qt Extended Build System"
# in the Qt Extended documentation.


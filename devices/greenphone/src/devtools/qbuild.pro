flash.files=flash-files/*
flash.path=/../

startup.files=startup/qpe.sh \
              startup/qpe.env
startup.path=/
startup.hint=script
INSTALLS+=startup

script.files=scripts/*
script.path=/bin
script.hint=script
INSTALLS+=script

f_dir.files=.directory
f_dir.path=/apps/Devtools
f_dir.trtarget=Devtools
f_dir.hint=desktop nct prep_db
MODULES*=qtopia::prep_db
INSTALLS+=f_dir

desktop.files=desktop/network-services-start.desktop
desktop.files+=desktop/network-services-stop.desktop
desktop.files+=desktop/sdcard-umount.desktop
desktop.files+=desktop/sdio-storage.desktop
desktop.files+=desktop/sdio-wifi.desktop
desktop.path=/apps/Devtools
desktop.depends+=install_docapi_f_dir
desktop.hint=desktop
INSTALLS+=desktop

pics.files=*.png\
           *.svg
pics.path=/pics/devtools
pics.hint=pics
INSTALLS+=pics

help.source=help
help.files=qpe-devtools*
help.hint=help
INSTALLS+=help


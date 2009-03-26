startup.files=startup/qpe.sh \
                startup/qpe.env

startup.path=/
startup.hint=script
INSTALLS+=startup

!equals(QTOPIA_UI,home) {
    # QBuild doesn't have a $$files function (though it probably should)
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

    desktop.files+=desktop/usbnet-start.desktop
    desktop.files+=desktop/usbnet-stop.desktop
    desktop.files+=desktop/usbnet-restart.desktop
    desktop.files+=desktop/maemo.desktop
    desktop.path=/apps/Devtools
    desktop.depends+=install_docapi_f_dir
    desktop.hint=desktop
    INSTALLS+=desktop

    pics.files=*.png\
                *.svg
    pics.path=/pics/devtools
    pics.hint=pics
    INSTALLS+=pics
}

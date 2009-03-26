session_conf.path=/dbus/session/services
session_conf.commands=\
	"#(v)perl -pe 's,\@DBUS_SESSION_SOCKET_DIR\@,"$$QTOPIA_PREFIX"/dbus/session,;s,\@EXPANDED_DATADIR\@,"$$QTOPIA_PREFIX"/dbus/session,' "$$path(session.conf.in,project)" >"$$QTOPIA_IMAGE"/dbus/session.conf"
session_conf.hint=image

qtopia_dbus_script.path=/bin
qtopia_dbus_script.commands=\
	"#(v)perl -pe 's,\@DBUSEXEC\@,"$$DBUS_PREFIX"/bin/dbus-daemon,;s,\@DBUSSESSIONCONFIG\@,"$$QTOPIA_PREFIX"/dbus/session.conf,' "$$path(qtopia-dbus-daemon.in,project)" >"$$QTOPIA_IMAGE"/bin/qtopia-dbus-daemon"\
        "#(v)chmod a+x "$$QTOPIA_IMAGE"/bin/qtopia-dbus-daemon"
qtopia_dbus_script.hint=image


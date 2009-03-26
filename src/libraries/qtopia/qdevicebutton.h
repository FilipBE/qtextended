/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QDEVICEBUTTON_H
#define QDEVICEBUTTON_H

#include <qpixmap.h>
#include <qstring.h>
#include <qtopiaservices.h>

/*
    This class represents a physical button on a Qt Extended device.  A device may
    have n "user programmable" buttons, which are numbered 1..n.  The location
    and number of buttons will vary from device to device.  userText() and pixmap()
    may be used to describe this button to the user in help documentation.
*/
class QTOPIA_EXPORT QDeviceButton {
public:
    QDeviceButton();
    virtual ~QDeviceButton();

    int keycode() const;
    QString context() const;
    QString userText() const;
    QPixmap pixmap() const;
    QtopiaServiceRequest pressedAction() const;
    QtopiaServiceRequest heldAction() const;
    QtopiaServiceRequest releasedAction() const;
    bool pressedActionMappable() const { return m_PressedActionMappable; }
    bool heldActionMappable() const { return m_HeldActionMappable; }
    bool releasedActionMappable() const { return m_releasedActionMappable; }
    void setKeycode(int keycode);
    void setContext(const QString& context);
    void setUserText(const QString& text);
    void setPixmap(const QString& pmn);
    void setPressedAction(const QtopiaServiceRequest& action);
    void setHeldAction(const QtopiaServiceRequest& action);
    void setReleasedAction(const QtopiaServiceRequest& action);
    void setPressedActionMappable(bool);
    void setHeldActionMappable(bool);
    void setReleasedActionMappable(bool);
    bool operator==(const QDeviceButton &e) const;

private:
    int m_Keycode;
    QString m_UserText;
    QString m_PixmapName;
    QPixmap m_Pixmap;
    QString m_Context;
    QtopiaServiceRequest m_PressedAction;
    QtopiaServiceRequest m_HeldAction;
    QtopiaServiceRequest m_releasedAction;
    bool m_PressedActionMappable;
    bool m_HeldActionMappable;
    bool m_releasedActionMappable;
};

#endif

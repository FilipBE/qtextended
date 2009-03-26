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

#include "rotation.h"

#include <qtopiaapplication.h>
#include <QApplication>
#include <QtopiaServiceRequest>

#include <QDir>
#include <QtopiaIpcEnvelope>
//#include <QDebug>
//#include <QScreen>
#include <stdlib.h>
#include <QDesktopWidget>
#include <QWSDisplay>
#include <QValueSpaceItem>
#include <QKeyEvent>
#include <QTimer>

RotationSettings::RotationSettings( QWidget* parent,  Qt::WFlags fl )
    : QDialog( parent, fl ),
      changeRotTo(-1)
{
    setupUi(this);
    reset();

    rot0->setIcon(QIcon(":icon/up"));
    connect(rot0, SIGNAL(clicked()),this, SLOT(applyRotation()));

    rot90->setIcon(QIcon(":icon/left"));
    connect(rot90, SIGNAL(clicked()),this, SLOT(applyRotation()));

    rot180->setIcon(QIcon(":icon/down"));
    connect(rot180, SIGNAL(clicked()),this, SLOT(applyRotation()));

    rot270->setIcon(QIcon(":icon/right"));
    connect(rot270, SIGNAL(clicked()),this, SLOT(applyRotation()));

    rotation_message->setAlignment(Qt::AlignCenter);
    rotation_message->setWordWrap(true);

    grabKeyboard();

    QString display = qgetenv("QWS_DISPLAY");
    qLog(Hardware) << display;
    // transformed can be capitalized or not.
    if ( display.indexOf("ransformed:") == -1 )
        rotation_message->setText(tr("Rotion will not work because the Transformed driver is not in use."));
}

RotationSettings::~RotationSettings()
{
    releaseKeyboard();
}

static int initrot = -1;

void RotationSettings::applyRotation()
{
    QPushButton *sendButton = qobject_cast<QPushButton *>(sender());
//hacky at best
    setRotation(sendButton->objectName().remove("rot").toInt());
}

void RotationSettings::setRotation(int r)
{
    initrot = getCurrentOrientation();
    int rot =  (r + initrot) % 360;
    QtopiaServiceRequest svreq("RotationManager", "setCurrentRotation(int)");
    svreq << rot;
    svreq.send();
}


void RotationSettings::reject()
{
      QtopiaServiceRequest svreq("RotationManager", "defaultRotation()");
    svreq.send();

    QDialog::reject();
}

void RotationSettings::reset()
{
    int r = getDefaultOrientation() / 90;
    if ( initrot == -1 )
        initrot = r;

    r = (r-initrot+360) % 360;

    rot0->setChecked(r == 0);
    rot90->setChecked(r == 90);
    rot180->setChecked(r == 180);
    rot270->setChecked(r == 270);
}


void RotationSettings::done( int r )
{
    QDialog::done(r);
    close();
}

int RotationSettings::getCurrentOrientation()
{
    QValueSpaceItem *rotation = new QValueSpaceItem("/UI/Rotation/Current");
    return rotation->value().toUInt();
}


int RotationSettings::getDefaultOrientation()
{
    QValueSpaceItem *rotation = new QValueSpaceItem("/UI/Rotation/Default");
    return rotation->value().toUInt();
}

void RotationSettings::keyPressEvent(QKeyEvent *ke)
{
    qLog(Hardware) << __PRETTY_FUNCTION__ << ke->key()
                   << ke->nativeModifiers()
                   << ke->nativeScanCode()
                   << ke->nativeVirtualKey ();
    unsigned int qtKeyCode = 0;

    qtKeyCode = transformDirKey( ke->key());
    qLog(Hardware) << qtKeyCode;
    switch ( qtKeyCode /* ke->key()*/ )
    {
    case Qt::Key_Up:
        rot0->setFocus();
        rot0->setDown(true);
        changeRotTo = 0;
         QTimer::singleShot(50,this, SLOT(setRotation()));
        rot0->setDown(false);
        break;
    case Qt::Key_Left:
        rot90->setFocus();
        rot90->setDown(true);
        changeRotTo = 90;
        QTimer::singleShot(50,this, SLOT(setRotation()));
         rot90->setDown(false);
        break;
    case Qt::Key_Down:
        rot180->setFocus();
        rot180->setDown(true);
        changeRotTo = 180;
        QTimer::singleShot(50,this, SLOT(setRotation()));
         rot180->setDown(false);
        break;
    case Qt::Key_Right:
        rot270->setFocus();
        rot270->setDown(true);
        changeRotTo = 270;
        QTimer::singleShot(50,this, SLOT(setRotation()));
         rot270->setDown(false);
        break;
    default:
        QDialog::keyPressEvent(ke);
        break;
    };
}

// from QWSKeyboardHandler
int RotationSettings::transformDirKey(int key)
{
    if ( key >= Qt::Key_Left  && key <= Qt::Key_Down ) {
        int dir_keyrot = -1;
        switch (qgetenv("QWS_CURSOR_ROTATION").toInt()) {
        case 90: dir_keyrot = 1; break;
        case 180: dir_keyrot = 2; break;
        case 270: dir_keyrot = 3; break;
        default: dir_keyrot = 0; break;
        }

        int xf = getCurrentOrientation() / 90;

        return (key - Qt::Key_Left + xf) %4 + Qt::Key_Left;
    }
    return key;
}

void RotationSettings::setRotation()
{
    if (changeRotTo == -1)
        return;
    setRotation(changeRotTo);
    changeRotTo = -1;
}


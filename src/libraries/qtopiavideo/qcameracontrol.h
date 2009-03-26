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

#ifndef QTOPIA_CAMERACONTROL_H
#define QTOPIA_CAMERACONTROL_H

#include <QObject>
#include "qcamera.h"


class ControlData;


class QTOPIAVIDEO_EXPORT QCameraControl : public QObject
{
    Q_OBJECT
public:

    enum GUIElement
    {
        Slider = 0x1,
        Menu,
        CheckBox,
        PushButton
    };

    enum CanonicalName
    {
        Brightness = 0x1,
        Contrast,
        Saturation,
        Hue,
        AutoWhiteBalance,
        RedBalance,
        BlueBalance,
        Gamma,
        AutoGain,
        Exposure,
        Gain,
        FlashControl,
        Custom
    };

    explicit QCameraControl(QCameraControl::CanonicalName name, quint32 id,
                            QString& description, int min,
                            int max,  int step, int default_value_index,
                            QStringList& value_strings,
                            QCameraControl::GUIElement gui, QString customName = QString());

    ~QCameraControl();


    quint32 id() const;

    QString description() const;

    CanonicalName name() const;

    QString custom() const;

    qint32 step() const;

    qint32 min() const;

    qint32 max() const;

    QStringList valueStrings() const;

    qint32 defaultValue() const;

    GUIElement gui() const;


private:
    ControlData* m_d;
};


#endif



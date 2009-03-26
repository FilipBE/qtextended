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

#ifndef QMEDIADEVICE_H
#define QMEDIADEVICE_H

#include <QString>
#include <QVariant>
#include <QIODevice>

#include <qtopiaglobal.h>


class QTOPIAMEDIA_EXPORT QMediaDevice : public QIODevice
{
    Q_OBJECT

public:

    struct Info
    {
        enum DataType { Raw, PCM };

        DataType    type;
        union
        {
            struct /* Raw */
            {
                qint64      dataSize;
            };

            struct /* PCM */
            {
                int         frequency;
                int         bitsPerSample;
                int         channels;
                int         volume;
            };
        };
    };

    virtual Info const& dataType() const = 0;

    virtual bool connectToInput(QMediaDevice* input) = 0;
    virtual void disconnectFromInput(QMediaDevice* input) = 0;
};


#endif

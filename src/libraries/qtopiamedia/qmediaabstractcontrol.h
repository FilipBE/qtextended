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

#ifndef QMEDIAABSTRACTCONTROL_H
#define QMEDIAABSTRACTCONTROL_H

#include <QVariant>

#include <qtopiaglobal.h>

class QMediaContent;


class QMediaAbstractControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaAbstractControl : public QObject
{
    Q_OBJECT

public:
    QMediaAbstractControl(QMediaContent* mediaContent,
                          QString const& name);
    ~QMediaAbstractControl();

protected:
    typedef QList<QVariant> SlotArgs;

    QVariant value(QString const& name, QVariant const& defaultValue= QVariant()) const;
    void setValue(QString const& name, QVariant const& value);

    void proxyAll();
    void forward(QString const& slot, SlotArgs const& args = SlotArgs());

private slots:
    void controlAvailable(const QString& name);
    void controlUnavailable(const QString& name);

signals:
    void valid();
    void invalid();

private:
    QMediaAbstractControlPrivate*   d;
};

#endif

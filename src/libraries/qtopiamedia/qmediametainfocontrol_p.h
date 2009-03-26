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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QMEDIAINFOCONTROL_H
#define QMEDIAINFOCONTROL_H

#include <QObject>

#include <qtopiaglobal.h>

class QMediaContent;

class QMediaMetaInfoControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaMetaInfoControl : public QObject
{
    Q_OBJECT

public:
    explicit QMediaMetaInfoControl(QMediaContent* mediaContent);
    ~QMediaMetaInfoControl();

    QString value(QString const& name) const;

    static QString name();

signals:
    void valueChanged(QString const& name);

private:
    Q_DISABLE_COPY(QMediaMetaInfoControl);

    QMediaMetaInfoControlPrivate* d;
};


#endif  // QMEDIAINFOCONTROL_H

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


#include <QValueSpaceItem>
#include <QMediaContent>
#include <qmediahandle_p.h>

#include <QDebug>

#include <qmediametainfocontrol_p.h>


class QMediaMetaInfoControlPrivate : public QObject
{
    Q_OBJECT

public:
    QMediaMetaInfoControlPrivate(QMediaContent* mediaContent)
    {
        dataItem = new QValueSpaceItem("/Media/Sessions/" + QMediaHandle::getHandle(mediaContent).toString() + "/Data", this);
        connect(dataItem, SIGNAL(contentsChanged()), SLOT(dataChanged()));
        dataChanged();
    }

    QString value(QString const& name) const
    {
        return metaInfo[name];
    }

signals:
    void valueChanged(QString const& name);

private slots:
    void dataChanged()
    {
        QStringList data = dataItem->value().toString().split(",");

        foreach (QString const& pair, data) {
            int index = pair.indexOf(':');
            if (index != -1) {
                QString name = pair.left(index);
                QString value = pair.mid(index + 1);

                metaInfo[name] = value;

                emit valueChanged(name);
            }
        }
    }

private:
    QValueSpaceItem*        dataItem;
    QMap<QString, QString>  metaInfo;
};


QMediaMetaInfoControl::QMediaMetaInfoControl(QMediaContent* mediaContent):
    QObject(mediaContent),
    d(new QMediaMetaInfoControlPrivate(mediaContent))
{
    connect(d, SIGNAL(valueChanged(QString)), SIGNAL(valueChanged(QString)));
}

QMediaMetaInfoControl::~QMediaMetaInfoControl()
{
    delete d;
}

QString QMediaMetaInfoControl::value(QString const& name) const
{
    return d->value(name);
}

#include "qmediametainfocontrol.moc"


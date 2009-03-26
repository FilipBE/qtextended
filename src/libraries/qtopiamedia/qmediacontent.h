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

#ifndef QMEDIACONTENT_H
#define QMEDIACONTENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QContent>


class QMediaHandle;


class QMediaContentPrivate;

class QTOPIAMEDIA_EXPORT QMediaContent : public QObject
{
    Q_OBJECT

    friend class QMediaContentPrivate;
    friend class QMediaHandle;

public:
    explicit QMediaContent(QUrl const& url,
                           QString const& domain = QLatin1String("Media"),
                           QObject* parent = 0);
    explicit QMediaContent(QContent const& content,
                           QString const& domain = QLatin1String("Media"),
                           QObject* parent = 0);
    ~QMediaContent();

    QStringList controls() const;

    static QStringList supportedMimeTypes();
    static QStringList supportedUriSchemes(QString const& mimeType);

    static void playContent(QUrl const& url, QString const& domain = "Media");
    static void playContent(QContent const& content, QString const& domain = "Media");

signals:
    void controlAvailable(const QString& name);
    void controlUnavailable(const QString& name);
    void mediaError(const QString& mediaError);

private:
    Q_DISABLE_COPY(QMediaContent);

    QMediaHandle handle() const;

    QMediaContentPrivate*   d;
};


#endif

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

#ifndef QTRANSLATABLESETTINGS_H
#define QTRANSLATABLESETTINGS_H

#include <qtopiaglobal.h>
#include <qsettings.h>

class QTranslatableSettingsPrivate;

class QTOPIA_EXPORT QTranslatableSettings : public QSettings
{
    Q_OBJECT
    QTranslatableSettingsPrivate *d;

public:
    explicit QTranslatableSettings(const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QTranslatableSettings(Scope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QTranslatableSettings(Format format, Scope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QTranslatableSettings(const QString &fileName, Format format, QObject *parent = 0);
    explicit QTranslatableSettings(QObject *parent = 0);

    ~QTranslatableSettings();

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    QStringList allKeys() const;
    QStringList childKeys() const;

    bool contains(const QString &key) const;

private:
    void initTranslation();
    static void stripTranslations(QStringList&);
};

#endif

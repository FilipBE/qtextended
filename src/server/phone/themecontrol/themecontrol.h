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

#ifndef THEMECONTROL_H
#define THEMECONTROL_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QMap>

#include "qtopiaserverapplication.h"

class ThemedView;
class QThemedView;
class QAbstractThemeWidgetFactory;

class ThemeControl : public QObject
{
    Q_OBJECT
public:
    ThemeControl(QObject *parent = 0);

    void registerThemedView(ThemedView *, const QString &);
    void registerThemedView(QThemedView *, const QString &);
    bool exportBackground() const;

    void refresh();

    void setThemeWidgetFactory(QAbstractThemeWidgetFactory *);

signals:
    void themeChanging();
    void themeChanged();

private Q_SLOTS:
    void sysMessage(const QString& message, const QByteArray &data);

private:
    QString findFile(const QString &) const;
    void doTheme(ThemedView *, const QString &);
    void doThemeWidgets(ThemedView *view);
    void doTheme(QThemedView *, const QString &);
    void polishWindows();


    QString m_themeName;
    bool m_exportBackground;

    QMap<QString, QString> m_themeFiles;
    QList<QPair<ThemedView *, QString> > m_themes;
    QList<QPair<QThemedView *, QString> > m_qthemes;

    QAbstractThemeWidgetFactory *m_widgetFactory;
};

QTOPIA_TASK_INTERFACE(ThemeControl);

#endif

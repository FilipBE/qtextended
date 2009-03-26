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

#ifndef QSOFTMENUBARPROVIDER_H
#define QSOFTMENUBARPROVIDER_H

#include <QObject>
#include <QSharedDataPointer>
#include <QPixmap>

class QWSWindow;
class QSoftMenuBarProviderPrivate;
class QSoftMenuBarProvider : public QObject
{
Q_OBJECT
public:
    QSoftMenuBarProvider(QObject *parent = 0);
    virtual ~QSoftMenuBarProvider();

    class MenuButtonPrivate;
    class MenuButton
    {
    public:
        MenuButton();
        MenuButton(const MenuButton &);
        MenuButton &operator=(const MenuButton &);
        ~MenuButton();

        int index() const;
        int key() const;
        QString text() const;
        QPixmap pixmap() const;
        QString pixmapName() const;

    private:
        friend class QSoftMenuBarProvider;
        QSharedDataPointer<MenuButtonPrivate> d;
    };

    int keyCount() const;
    MenuButton key(int ii) const;
    QList<MenuButton> keys() const;

signals:
    void keyChanged(const QSoftMenuBarProvider::MenuButton &);
    void keyChanged(int);

private slots:
    void message(const QString &msg, const QByteArray &data);
    void activeChanged(const QString &, const QRect &, WId);

private:
    QSoftMenuBarProviderPrivate *d;
};

#endif

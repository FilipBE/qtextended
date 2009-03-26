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

#ifndef QMAILMESSAGEDELEGATE_H
#define QMAILMESSAGEDELEGATE_H

#include <QtopiaItemDelegate>
#include <qtopiaglobal.h>

class QMailMessageDelegatePrivate;

class QTOPIAMAIL_EXPORT QMailMessageDelegate : public QtopiaItemDelegate
{
public:
    enum DisplayMode
    {
        QtmailMode,
        AddressbookMode
    };

    QMailMessageDelegate(DisplayMode mode, QWidget* parent);
    virtual ~QMailMessageDelegate();

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);

    bool displaySelectionState() const;
    void setDisplaySelectionState(bool set);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QMailMessageDelegatePrivate* d;
};

#ifdef QTOPIA_HOMEUI

class QtopiaHomeMailMessageDelegatePrivate;

class QTOPIAMAIL_EXPORT QtopiaHomeMailMessageDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
public:
    enum DisplayMode
    {
        QtmailMode,
        QtmailUnifiedMode,
        AddressbookMode
    };

    QtopiaHomeMailMessageDelegate(DisplayMode mode, QWidget* parent);
    virtual ~QtopiaHomeMailMessageDelegate();

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);

    bool displaySelectionState() const;
    void setDisplaySelectionState(bool set);

    QFont titleFont(const QStyleOptionViewItem &option) const;

    QRect replyButtonRect(const QRect &rect) const;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QtopiaHomeMailMessageDelegatePrivate* d;
};

#endif // QTOPIA_HOMEUI

#endif

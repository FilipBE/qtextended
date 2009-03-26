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
#ifndef CONTACTDETAILS_H
#define CONTACTDETAILS_H

#include <qcontact.h>
#include <qtopiaservices.h>

#include "qpimdelegate.h"
#include <QWidget>

class QTabWidget;
class ContactBrowser;
class ContactOverview;
#if defined(QTOPIA_TELEPHONY)
class ContactCallHistoryList;
#endif
class ContactMessageHistoryList;
class QContactModel;

class ContactDetails : public QWidget
{
    Q_OBJECT

public:
    ContactDetails( QWidget *parent );
    ~ContactDetails();

    QContact entry() const {return ent;}

public slots:
    void init( const QContact &entry );
    void modelChanged();

signals:
    void externalLinkActivated();
    void closeView();

    void callContact();
    void textContact();
    void emailContact();
    void editContact();
    void personaliseContact();

    void highlighted(const QString&);

private:
    typedef enum {NoLink = 0, Dialer, Messaging, Email} LinkType;

    QContact ent;
    QString mLink;

    QTabWidget * mTabs;
    ContactOverview * mQuickTab;
    ContactBrowser * mDetailsTab;
#if defined(QTOPIA_TELEPHONY)
    ContactCallHistoryList *mCallHistoryTab;
#endif
    ContactMessageHistoryList *mMessageHistoryTab;
    QContactModel *mModel;
};


// -------------------------------------------------------------
// ContactHistoryDelegate
// -------------------------------------------------------------
#ifndef QTOPIA_HOMEUI
class ContactHistoryDelegate : public QPimDelegate
{
    Q_OBJECT

public:
    explicit ContactHistoryDelegate( QObject * parent = 0 );
    virtual ~ContactHistoryDelegate();

    enum ContactHistoryRole {
        SubLabelRole = Qt::UserRole,
        SecondaryDecorationRole = Qt::UserRole+1,
        UserRole = Qt::UserRole+2
    };

    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const;

    QList<StringPair> subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    QSize mPrimarySize;
    QSize mSecondarySize;
};

#else
class DeskphoneContactDelegate : public QAbstractItemDelegate
{
    Q_OBJECT;
public:
    DeskphoneContactDelegate(QWidget *parent);

    QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    virtual void getInfo(const QModelIndex& index, QPixmap&, QColor&, QString&, QString&) const;
    QFont mNameFont;
    QWidget *mParent;
};

class ContactHistoryDelegate : public DeskphoneContactDelegate
{
    Q_OBJECT;
public:
    ContactHistoryDelegate(QWidget *parent) : DeskphoneContactDelegate(parent) {}

    enum ContactHistoryRole {
        SubLabelRole = Qt::UserRole,
        SecondaryDecorationRole = Qt::UserRole+1,
        UserRole = Qt::UserRole+2
    };

protected:
    void getInfo(const QModelIndex& index, QPixmap&, QColor&, QString&, QString&) const;
};
#endif // QTOPIA_HOMEUI

#endif

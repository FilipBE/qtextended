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
#ifndef DESKPHONEDETAILS_H
#define DESKPHONEDETAILS_H

#include <QWidget>
#include <QPushButton>
#include <QMap>
#include <QContact>
#include <QFieldDefinition>

#include <qcontact.h>
#include <qtopiaservices.h>

#include "deskphonewidgets.h"

class QStackedWidget;
class QScrollArea;
class QLabel;
class QContactModel;
class QtopiaServiceRequest;
class ContactPictureButton;
class QVBoxLayout;
class FramedContactWidget;
class SqueezeLabel;

class DeskphoneContactView;
#if defined(QTOPIA_TELEPHONY)
class ContactCallHistoryList;
#endif
class ContactMessageHistoryList;
class QContactModel;

class DeskphoneContactDetails : public QWidget
{
    Q_OBJECT

public:
    DeskphoneContactDetails( QWidget *parent );
    ~DeskphoneContactDetails();

    QContact entry() const {return ent;}

public slots:
    void init( const QContact &entry );
    void modelChanged();

signals:
    void callContact();
    void textContact();
    void emailContact();
    void editContact();
    void personaliseContact();

    void closeView();
    void highlighted(const QString&);

private slots:
    void showDetails();
    void showCalls();
    void showMessages();
    void updateMenu();

private:
    typedef enum {NoLink = 0, Dialer, Messaging, Email} LinkType;

    QContact ent;

    QStackedWidget *mStack;
    QAction *mShowDetails;
    QAction *mShowCalls;
    QAction *mShowMessages;

    DeskphoneContactView *mDeskTab;
#if defined(QTOPIA_TELEPHONY)
    ContactCallHistoryList *mCallHistoryTab;
    ContactHeader *mCallHistoryHeader;
    bool mCallHistoryDirty;
#endif
    ContactMessageHistoryList *mMessageHistoryTab;
    ContactHeader *mMessageHistoryHeader;
    bool mMessageHistoryDirty;

    QContactModel *mModel;
};

//===========================================================================

class DeskphoneContactView : public QWidget
{
    Q_OBJECT

public:
    DeskphoneContactView(QWidget *parent);
    ~DeskphoneContactView();

    void setModel(QContactModel *model);

public slots:
    void init(const QContact &entry);

private slots:
    void numberClicked(QContactFieldDefinition def);
    void messageClicked(QContactFieldDefinition def);
    void emailClicked();
    void buddyClicked(QContactFieldDefinition def);
    void buddyChatClicked(QContactFieldDefinition def);

private:
    void populateForm();
    void addNumberField(const QContactFieldDefinition &def, const QString &value);
    void addBuddyField(const QContactFieldDefinition &def);
    void addEmailField(const QString &label, const QString &value);
    void addAddressField(const QString &label, const QString &value);
    void addMiscField(ContactMiscFieldWidget::MiscType type, const QString &value);

private:
    QContact mContact;
    bool mInitedGui;
    ColumnSizer mGroup;
    FramedContactWidget* mPortrait;
    SqueezeLabel *mNameLabel;
    SqueezeLabel *mCompanyLabel;
    SqueezeLabel *mPresenceLabel;
    QContactModel *mModel;
    QScrollArea *mScrollArea;
    QWidget *mFormContainer;
    QMap<QAbstractButton*,QString> mFieldMap;
    QColor mLabelColor;

    QVBoxLayout *mDetailsLayout;
};

#endif

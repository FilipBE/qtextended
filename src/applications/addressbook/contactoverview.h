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
#ifndef CONTACTOVERVIEW_H
#define CONTACTOVERVIEW_H

#include <qcontact.h>

#include <QWidget>

class QAbstractButton;
class QLabel;
class QScrollArea;

class ContactOverview : public QWidget
{
    Q_OBJECT

public:
    ContactOverview( QWidget *parent );
    virtual ~ContactOverview();

    QContact entry() const {return ent;}

public slots:
    void init( const QContact &entry );

    void updateCommands();

signals:
    void externalLinkActivated();
    void closeView();
    void callContact();
    void textContact();
    void emailContact();
    void editContact();

protected:
    void keyPressEvent( QKeyEvent *e );
    void focusInEvent( QFocusEvent *e );
    void resizeEvent( QResizeEvent *e);

private:
    QContact ent;
    bool mInitedGui;
    QAbstractButton *mCall;
    QAbstractButton *mText;
    QAbstractButton *mEmail;
    QAbstractButton *mEdit;
    QList<QAbstractButton*> buttons;
    QLabel* mPortrait;
    QLabel *mNameLabel;
    QContactModel *mModel;
    QScrollArea *mScrollArea;
    bool mSetFocus;
    bool bDialer;
    bool bSMS;
    bool bEmail;
};

#endif

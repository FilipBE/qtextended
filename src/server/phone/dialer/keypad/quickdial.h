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

#ifndef QUICKDIAL_H
#define QUICKDIAL_H

#include <qtopiaservices.h>

#include "qabstractdialerscreen.h"

class NumberDisplay;
class QuickDialModel;
class DtmfAudio;
class QTimer;
class CallContactListView;

class PhoneQuickDialerScreen : public QAbstractDialerScreen
{
    Q_OBJECT
public:
    PhoneQuickDialerScreen( QWidget *parent, Qt::WFlags fl = 0 );
    ~PhoneQuickDialerScreen();

    virtual void reset();
    virtual void setDigits(const QString &digits);
    virtual void appendDigits(const QString &digits);
    virtual QString digits() const;
    virtual void doOffHook();
    virtual void doOnHook();

protected:
    bool eventFilter( QObject *o, QEvent *e );

signals:
    void numberSelected(const QString&, const QUniqueId&);

protected slots:
    void rejectEmpty(const QString&);
    void selectedNumber( const QString &num );
    void selectedNumber( const QString &num, const QUniqueId &cnt );
    void showEvent( QShowEvent *e );
    void hideEvent( QHideEvent *e );
    void resetDelayedDialTimer();
    void delayedDialTimeout();
    void numberKeyPressed( int key );

private:
    void appendDigits( const QString &digits, bool refresh,
                       bool speedDial = true );

    NumberDisplay *mNumberDS;
    CallContactListView *mDialList;
    QString mNumber;
    bool mSpeedDial;
    QuickDialModel *mDialModel;
    DtmfAudio *dtmf;
    QTimer *delayedDialTimer;
    bool delayedDial;
};

#endif

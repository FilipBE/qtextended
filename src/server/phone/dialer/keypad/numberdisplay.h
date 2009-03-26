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

#ifndef NUMBERDISPLAY_H
#define NUMBERDISPLAY_H

#include <QWidget>
#include <QList>
#include <QString>

class QTimer;
class QAction;
class NumberDisplayMultiTap;

class NumberDisplay : public QWidget
{
    Q_OBJECT
public:
    NumberDisplay( QWidget *parent );
    void appendNumber( const QString &numbers, bool speedDial = true );
    void setNumber( const QString &number );

    void clear();

    void setWildcardNumber(const QString &);

public slots:
    void backspace();
    QString number() const;
    QString wildcardNumber() const;

    QSize sizeHint() const;
protected slots:
    void addPhoneNumberToContact();
    void enableAction( const QString &n );
    void sendMessage();
    void composeTap(const QChar &);
    void completeTap(const QChar &);

protected:
    void keyPressEvent( QKeyEvent *e );

    void checkForStartTap();

    void setText( const QString &txt );
    QString text() const;

    void paintEvent( QPaintEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
    void timerEvent( QTimerEvent *e );

signals:
    void numberChanged(const QString&);
    void numberSelected(const QString &);
    void hangupActivated();
    void speedDialed(const QString&);
    void numberKeyPressed(int key);

private:
    void processNumberChange();

    int mLargestCharWidth;
    QList<int> mFontSizes;

    QString mWildcardNumber;
    QString mNumber;

    QAction *mNewAC;
    QAction *mSendMessage;
    int tid_speeddial;
    NumberDisplayMultiTap *tap;
    QChar composeKey;
    bool delayEmitNumberChanged;
};

#endif

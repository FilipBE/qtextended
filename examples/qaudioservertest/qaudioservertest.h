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

#ifndef QAUDIOSERVERTEST_H
#define QAUDIOSERVERTEST_H
#include "ui_qaudioservertestbase.h"

#include <qaudiointerface.h>

class QAudioServerTestBase : public QWidget, public Ui_QAudioServerTestBase
{
public:
    QAudioServerTestBase( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QAudioServerTestBase();
};

class QAudioServerTest : public QAudioServerTestBase
{
    Q_OBJECT
public:
    QAudioServerTest( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QAudioServerTest();

private slots:
    void selectAudioMedia1();
    void selectAudioRingTone1();
    void selectAudioPhone1();

    void selectAudioMedia2();
    void selectAudioRingTone2();
    void selectAudioPhone2();

    void startAudio1();
    void startAudio2();
    void audioStopped1();
    void audioStopped2();
    void audioStarted1();
    void audioStarted2();

    void updateProgress1();
    void updateProgress2();

private:
    QTimer*         timer1;
    QTimer*         timer2;
    QByteArray      domain1;
    QByteArray      domain2;

    QAudioInterface       *client1;
    QAudioInterface       *client2;
};

#endif

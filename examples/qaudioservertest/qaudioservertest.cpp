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

#include "qaudioservertest.h"
#include <qpushbutton.h>
#include <QTimer>

#include <QAudioInterface>


QAudioServerTestBase::QAudioServerTestBase( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    setupUi( this );
}

QAudioServerTestBase::~QAudioServerTestBase()
{
}

QAudioServerTest::QAudioServerTest( QWidget *parent, Qt::WFlags f )
    : QAudioServerTestBase( parent, f )
{
    client1 = 0;
    client2 = 0;
    connect(AudioMedia1, SIGNAL(clicked()), this, SLOT(selectAudioMedia1()));
    connect(AudioRingTone1, SIGNAL(clicked()), this, SLOT(selectAudioRingTone1()));
    connect(AudioPhone1, SIGNAL(clicked()), this, SLOT(selectAudioPhone1()));

    connect(AudioMedia2, SIGNAL(clicked()), this, SLOT(selectAudioMedia2()));
    connect(AudioRingTone2, SIGNAL(clicked()), this, SLOT(selectAudioRingTone2()));
    connect(AudioPhone2, SIGNAL(clicked()), this, SLOT(selectAudioPhone2()));

    connect(Audio1, SIGNAL(clicked()), this, SLOT(startAudio1()));
    connect(Audio2, SIGNAL(clicked()), this, SLOT(startAudio2()));

    domain1.clear();
    domain2.clear();
    domain1.append("Media");
    domain2.append("Media");

    timer1 = new QTimer(this);
    connect(timer1, SIGNAL(timeout()), this, SLOT(updateProgress1()));
    timer2 = new QTimer(this);
    connect(timer2, SIGNAL(timeout()), this, SLOT(updateProgress2()));
    progressBar1->setRange(0,100);
    progressBar2->setRange(0,100);
}

QAudioServerTest::~QAudioServerTest()
{
}

void QAudioServerTest::selectAudioMedia1()
{
    if(AudioRingTone1->isChecked()) AudioRingTone1->setChecked(false);
    if(AudioPhone1->isChecked()) AudioPhone1->setChecked(false);
    domain1.clear();
    domain1.append("Media");
}

void QAudioServerTest::selectAudioRingTone1()
{
    if(AudioMedia1->isChecked()) AudioMedia1->setChecked(false);
    if(AudioPhone1->isChecked()) AudioPhone1->setChecked(false);
    domain1.clear();
    domain1.append("RingTone");
}

void QAudioServerTest::selectAudioPhone1()
{
    if(AudioMedia1->isChecked()) AudioMedia1->setChecked(false);
    if(AudioRingTone1->isChecked()) AudioRingTone1->setChecked(false);
    domain1.clear();
    domain1.append("Phone");
}

void QAudioServerTest::selectAudioMedia2()
{
    if(AudioRingTone2->isChecked()) AudioRingTone2->setChecked(false);
    if(AudioPhone2->isChecked()) AudioPhone2->setChecked(false);
    domain2.clear();
    domain2.append("Media");
}

void QAudioServerTest::selectAudioRingTone2()
{
    if(AudioMedia2->isChecked()) AudioMedia2->setChecked(false);
    if(AudioPhone2->isChecked()) AudioPhone2->setChecked(false);
    domain2.clear();
    domain2.append("RingTone");
}

void QAudioServerTest::selectAudioPhone2()
{
    if(AudioMedia2->isChecked()) AudioMedia2->setChecked(false);
    if(AudioRingTone2->isChecked()) AudioRingTone2->setChecked(false);
    domain2.clear();
    domain2.append("Phone");
}

void QAudioServerTest::startAudio1()
{
    if(client1 != 0)
        delete client1;

    progressBar1->setValue(0);

    client1 = new QAudioInterface(domain1, this);
    connect(client1, SIGNAL(audioStarted()), this, SLOT(audioStarted1()));
    connect(client1, SIGNAL(audioStopped()), this, SLOT(audioStopped1()));
    client1->startAudio();
}

void QAudioServerTest::startAudio2()
{
    if(client2 != 0)
        delete client2;

    progressBar2->setValue(0);

    client2 = new QAudioInterface(domain2, this);
    connect(client2, SIGNAL(audioStarted()), this, SLOT(audioStarted2()));
    connect(client2, SIGNAL(audioStopped()), this, SLOT(audioStopped2()));
    client2->startAudio();
}

void QAudioServerTest::audioStopped1()
{
    timer1->stop();
}

void QAudioServerTest::audioStarted1()
{
    timer1->start(1000);
}

void QAudioServerTest::audioStopped2()
{
    timer2->stop();
}

void QAudioServerTest::audioStarted2()
{
    timer2->start(1000);
}

void QAudioServerTest::updateProgress1()
{
    int current = progressBar1->value();
    current = current + 5;
    if(current == 100) {
        if(client1 != 0)
            delete client1;
        current = 0;
        timer1->stop();
        client1 = 0;
    }
    progressBar1->setValue(current);
}

void QAudioServerTest::updateProgress2()
{
    int current = progressBar2->value();
    current = current + 5;
    if(current == 100) {
        if(client2 != 0)
            delete client2;
        current = 0;
        timer2->stop();
        client2 = 0;
    }
    progressBar2->setValue(current);
}


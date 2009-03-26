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

#include <QDebug>
#include "qaudiomixertest.h"
#include <qpushbutton.h>

#include "qaudiomixer.h"

QAudioMixer* mixer;

QAudioMixerTestBase::QAudioMixerTestBase( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    setupUi( this );
}

QAudioMixerTestBase::~QAudioMixerTestBase()
{
}

QAudioMixerTest::QAudioMixerTest( QWidget *parent, Qt::WFlags f )
    : QAudioMixerTestBase( parent, f )
{
    connect(nextButton, SIGNAL(clicked()), this, SLOT(next()));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(prev()));
    connect(muteButton, SIGNAL(clicked()), this, SLOT(mute()));

    mixer = new QAudioMixer();
    connect(mixer, SIGNAL(audioChanged()), this, SLOT(updateStatus()));
    connect(horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateAudio(int)));
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(selectionChanged(int)));

    QList<QAudioElement*> e = mixer->elements();

    current = e.at(0);
    updateStatus();
}

QAudioMixerTest::~QAudioMixerTest()
{
}

void QAudioMixerTest::next()
{
    QList<QAudioElement*> e = mixer->elements();

    for(int i = 0; i < e.size(); ++i) {
        if(e.at(i) == current) {
            if(current == e.last()) {
                current = e.first();
            } else {
                current = e.at(i+1);
            }
            break;
        }
    }
    updateStatus();
}

void QAudioMixerTest::prev()
{
    QList<QAudioElement*> e = mixer->elements();

    for(int i = 0; i < e.size(); ++i) {
        if(e.at(i) == current) {
            if(current == e.first()) {
                current = e.last();
            } else {
                current = e.at(i-1);
            }
            break;
        }
    }
    updateStatus();
}

void QAudioMixerTest::mute()
{
    if(current->isMuted() != muteButton->isChecked()) {
        current->setMute(muteButton->isChecked());
    }
}

void QAudioMixerTest::updateStatus()
{
    // Update name
    QString str = current->getName();
    str.append(" [");
    str.append(QString("%1").arg(current->getIndex()));
    str.append("]");
    label->setText(str);

    // Update mute button
    if(current->isMuted()) muteButton->setChecked(true);
    else muteButton->setChecked(false);

    comboBox->clear();

    if(current->isPlayback()) {
        horizontalSlider->setMinimum(current->getMinimum());
        horizontalSlider->setMaximum(current->getMaximum());
        horizontalSlider->setValue(current->getValue());
    } else if(current->isRecord()) {
        horizontalSlider->setMinimum(current->getMinimum());
        horizontalSlider->setMaximum(current->getMaximum());
        horizontalSlider->setValue(current->getValue());
    } else if(current->isOption()) {
        for(int i = 0; i < current->enumOptions.size(); ++i) {
            comboBox->insertItem(i,current->enumOptions.at(i));
            if(!current->enumOptions.at(i).compare(current->getOption())) {
                comboBox->setCurrentIndex(i);
            }
        }
    }
}

void QAudioMixerTest::updateAudio(int val)
{
    if((int)current->getValue() != val) {
        current->setValue(val);
    }
}

void QAudioMixerTest::selectionChanged(int index)
{
    if(index == -1) return;
    if(current->enumOptions.at(index).compare(current->getOption())) {
        comboBox->setCurrentIndex(index);
        current->setOption(current->enumOptions.at(index));
        updateStatus();
    }
}

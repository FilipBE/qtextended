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

#ifndef QAUDIOMIXERTEST_H
#define QAUDIOMIXERTEST_H
#include "ui_qaudiomixertestbase.h"

#include <qaudiomixer.h>

class QAudioMixerTestBase : public QWidget, public Ui_QAudioMixerTestBase
{
public:
    QAudioMixerTestBase( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QAudioMixerTestBase();
};

class QAudioMixerTest : public QAudioMixerTestBase
{
    Q_OBJECT
public:
    QAudioMixerTest( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QAudioMixerTest();

private slots:
    void next();
    void prev();
    void mute();
    void updateStatus();
    void updateAudio(int val);
    void selectionChanged(int index);

private:
    QAudioElement*   current;
};

#endif

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
#ifndef VOLUMEIMPL_H
#define VOLUMEIMPL_H

#include "ui_volume.h"
#include <QDialog>

class VolumeWidget;
class VolumeDialogImplPrivate;

class VolumeDialogImpl : public QDialog
{
    Q_OBJECT
public:
    VolumeDialogImpl( QWidget* parent = 0, Qt::WFlags fl = 0 );

    int  setVolume( bool up );

    static const int TIMEOUT = 1500;

signals:
    void volumeChanged( bool up);
    void setText(QString volume);

protected:
    void timerEvent( QTimerEvent *e );

private slots:
    void resetTimer();
    void valueSpaceVolumeChanged();

private:
    void screenUpdate();

    int initialized;
    int old_slot;
    int m_tid;
    int m_oldValue;
    VolumeWidget *volumeWidget;
    VolumeDialogImplPrivate  *m_d;
};

class VolumeDialogTask : public QObject
{
    Q_OBJECT
public:
    VolumeDialogTask( QObject *parent = 0 );
    ~VolumeDialogTask();
private slots:
    void volumeChanged(bool up);
};

#endif


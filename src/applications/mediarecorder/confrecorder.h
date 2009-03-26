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

#ifndef CONFRECORDER_H
#define CONFRECORDER_H

#include <qdialog.h>

struct QualitySetting;
class MediaRecorderPluginList;
class QStorageDeviceSelector;
class QComboBox;
class QFileSystem;


class ConfigureRecorder : public QDialog
{
    Q_OBJECT

public:
    ConfigureRecorder( QualitySetting *qualities, MediaRecorderPluginList *plugins, QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ConfigureRecorder();

public:
    int currentQuality() const { return quality; }
    const QFileSystem* fileSystem();
    QString documentPath();
    void processPopup();
    void saveConfig();

protected slots:
    void setQuality( int index );
    void setChannels( int index );
    void setSampleRate( int index );
    void setFormat( int index );

private:
    void loadConfig();

    QualitySetting *qualities;
    MediaRecorderPluginList *plugins;
    int quality;

    QComboBox *presetQualities;
    QComboBox *channels;
    QComboBox *sampleRate;
    QComboBox *format;
    QStorageDeviceSelector *storageLocation;
};

#endif

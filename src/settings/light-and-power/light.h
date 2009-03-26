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

#ifndef LIGHT_H
#define LIGHT_H

#include <QDialog>
#include <QPowerStatus>

#include "ui_lightsettingsbase.h"

class QSettings;

class LightSettings : public QDialog
{
    Q_OBJECT

public:
    LightSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~LightSettings();

protected:
    void accept();
    void reject();

private slots:
    void applyBrightness();
    void applyMode();
    void powerTypeChanged(int);
    void pushSettingStatus();
    void receive( const QString& msg, const QByteArray& data );
    void sysMessage(const QString&, const QByteArray&);
    void updateLightOffMinValue( int dimValue );
    void updateSuspendMinValue( int );

private:
    QString status();
    void setStatus( const QString& details );
    void pullSettingStatus();
    void saveConfig();

private:
    struct PowerMode {
        PowerMode()
        {
            intervalDim = 20;
            intervalLightOff = 30;
            intervalSuspend = 60;
            brightness = 255;
            initBrightness = 255;

            dim = true;
            lightoff = true;
            suspend = true;
            networkedsuspend = false;
        }

        bool dim;
        bool lightoff;
        bool suspend;
        bool networkedsuspend;
        int intervalDim;
        int intervalLightOff;
        int intervalSuspend;
        int initBrightness;
        int brightness;
        bool canSuspend;
    };

    class LightSettingsContainer : public QFrame, private Ui::LightSettingsBase
    {
        public:
            LightSettingsContainer( QWidget *parent = 0, Qt::WFlags f = 0 )
                : QFrame( parent, f )
                {
                    setFrameShape( QFrame::NoFrame );
                    setupUi( this );

                }
            friend class LightSettings;
    };

    void writeMode(QSettings &config, PowerMode *mode);

private:
    PowerMode batteryMode;
    PowerMode externalMode;
    PowerMode *currentMode;

    QPowerStatus powerStatus;

    LightSettingsContainer *b;
    QMenu* contextMenu;
    bool isFromActiveProfile; // when viewing the status from profile
    bool isStatusView;
};

#endif

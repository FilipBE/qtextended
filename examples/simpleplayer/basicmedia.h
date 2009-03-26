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

#ifndef BASICMEDIA_H
#define BASICMEDIA_H

#include <QWidget>
#include <QString>

class QMediaContent;
class QMediaControl;
class QMediaContentContext;
class QMediaVideoControl;
class QVBoxLayout;

class BasicMedia : public QWidget
{
Q_OBJECT

public:
    BasicMedia(QWidget* parent);
    ~BasicMedia();

    void stop();
    void start();
    void setFilename(QString filename);
    void volumeup();
    void volumedown();

protected:
    void keyReleaseEvent( QKeyEvent * );

private:
    QMediaContentContext* context;
    QMediaContent*        m_mediaContent;
    QMediaControl*        m_control;
    QMediaVideoControl*   m_video;

    int  m_state;
    int  volume;

private slots:
    void mediaVideoControlValid();
    void mediaControlValid();

private:
    QWidget*     videoWidget;
    QString      vfile;
    QVBoxLayout* layout;
};
#endif

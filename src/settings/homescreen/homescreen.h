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

#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QDialog>
#include <QContent>
#include <QPowerStatus>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QValueSpaceItem;

class HomescreenSettings : public QDialog
{
    Q_OBJECT
public:
    HomescreenSettings(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~HomescreenSettings() {}

protected:
    void accept();
    void reject();

protected slots:
    void editPhoto();
    void editSecondaryPhoto();
    void homeScreenActivated(int index = -1);
    void appMessage(const QString&,const QByteArray&);

private:
    QPushButton *image;
    QComboBox *imageMode;
    QPushButton *secondaryImage;
    QComboBox *secondaryImageMode;
    QComboBox *homeScreen;
    QCheckBox *lock;
    QCheckBox *time;
    QCheckBox *date;
    QCheckBox *op;
    QCheckBox *profile;
    QCheckBox *location;
    QLabel* powerNote;
    QPowerStatus powerStatus;
    QValueSpaceItem* screenSaver_vsi;

    QContent hsImage;
    QContent secondaryHsImage;
};

#endif

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
#ifndef LANGUAGESETTINGS_H
#define LANGUAGESETTINGS_H


#include <QStringList>
#include "ui_languagesettingsbase.h"

class QListWidgetItem;
class LanguageModel;
class QModelIndex;
class QListView;
class QAction;

class LanguageSettings : public QDialog, public Ui::LanguageSettingsBase
{
    Q_OBJECT
public:
    LanguageSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    virtual ~LanguageSettings();

    void setConfirm(bool cfm=true);
    void setNotify(bool notify = true);

    static QStringList dictLanguages();

protected:
    void accept();
    void reject();
    void done(int);
    QStringList langAvail;
private slots:
    void applyLanguage(const QModelIndex&);
    void reset();
    void inputToggled(const QModelIndex&);
    void inputToggled();
    void newLanguageSelected();
private:
    void updateActions(const QModelIndex& idx);
    void forceChosen();

    QString chosenLanguage;
    QStringList inputLanguages;
    bool confirmChange;
    bool notifySystem;

    static QStringList langs;
    LanguageModel *model;
    QListView *listView;
    QAction *a_input;
};

#endif

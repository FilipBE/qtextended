/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtGui/QDialog>
#include "ui_preferencesdialog.h"

QT_BEGIN_NAMESPACE

class FontPanel;
class QHelpEngineCore;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QHelpEngineCore *helpEngine, QWidget *parent = 0,
        bool enableHelpServer = true);
    ~PreferencesDialog();

    void showDialog();

private slots:
    void updateAttributes(QListWidgetItem *item);
    void updateFilterMap();
    void addFilter();
    void removeFilter();
    void addDocumentationRemote();
    void addDocumentationLocal();
    void removeDocumentation();
    void applyChanges();
    void appFontSettingToggled(bool on);
    void appFontSettingChanged(int index);
    void browserFontSettingToggled(bool on);
    void browserFontSettingChanged(int index);

signals:
    void updateBrowserFont();
    void updateApplicationFont();

private:
    void updateFilterPage();
    void updateFontSettingsPage();
    void loadNetworkSettings();

    Ui::PreferencesDialogClass m_ui;
    QHelpEngineCore *m_helpEngine;
    bool m_hideFiltersTab;
    bool m_hideDocsTab;
    QMap<QString, QStringList> m_filterMapBackup;
    QMap<QString, QStringList> m_filterMap;
    QStringList m_removedFilters;
    QStringList m_docsBackup;
    QStringList m_regDocs;
    QStringList m_unregDocs;
    FontPanel *m_appFontPanel;
    FontPanel *m_browserFontPanel;
    bool m_appFontChanged;
    bool m_browserFontChanged;
    bool m_enableHelpServer;
};

QT_END_NAMESPACE

#endif // SETTINGSDIALOG_H

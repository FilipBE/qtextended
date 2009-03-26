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
#ifndef EFFECTDIALOG_H
#define EFFECTDIALOG_H

#include <QDialog>
#include <QMap>

class QModelIndex;
class QStackedWidget;
class QListView;
class EffectSettingsWidget;
class EffectModel;

class EffectDialog : public QDialog
{
    Q_OBJECT
public:
    EffectDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);

public slots:
    void done(int result);

signals:
    void effectSelected(const QString &plugin, const QString &effect, const QMap<QString, QVariant> &settings);

private slots:
    void effectActivated(const QModelIndex &index);

private:
    QStackedWidget *m_stack;
    QListView *m_view;
    QMap<QString, EffectSettingsWidget *> m_settingsWidgets;
    EffectModel *m_model;
    QString m_selectedPlugin;
    QString m_selectedEffect;

};

#endif

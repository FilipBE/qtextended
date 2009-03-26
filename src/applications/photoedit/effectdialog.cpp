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
#include "effectdialog.h"
#include "effectmodel.h"
#include "effectsettingswidget.h"

#include <QStackedWidget>
#include <QBoxLayout>
#include <QListView>
#include <QtopiaItemDelegate>
#include <QtopiaApplication>

EffectDialog::EffectDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_stack(0)
    , m_view(0)
    , m_model(0)

{
    m_model = new EffectModel(this);

    m_view = new QListView;
    m_view->setModel(m_model);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setItemDelegate(new QtopiaItemDelegate(this));

    if (!Qtopia::mousePreferred())
        m_view->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::SelectCurrent);

    connect(m_view, SIGNAL(activated(QModelIndex)),
        this, SLOT(effectActivated(QModelIndex)));

    m_stack = new QStackedWidget;
    m_stack->addWidget(m_view);

    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_stack);

    setLayout(layout);
    setWindowTitle(tr("Effects"));

    QtopiaApplication::setMenuLike(this, true);
}

void EffectDialog::done(int result)
{
    if (m_stack->currentWidget() != m_view) {
        if (result == QDialog::Accepted) {
            emit effectSelected(
                m_selectedPlugin,
                m_selectedEffect,
                static_cast<EffectSettingsWidget *>(m_stack->currentWidget())->settings());

            QDialog::done(result);
        }
        QtopiaApplication::setMenuLike(this, true);

        m_stack->setCurrentWidget(m_view);
        setWindowTitle(tr("Effects"));
    } else {
        QDialog::done(result);
    }
}

void EffectDialog::effectActivated(const QModelIndex &index)
{
    m_selectedPlugin = m_model->plugin(index);
    m_selectedEffect = m_model->effect(index);

    QList<EffectParameter> parameters = m_model->parameters(index);

    if (!parameters.isEmpty()) {
        EffectSettingsWidget *settingsWidget = m_settingsWidgets.value(m_selectedEffect);

        if (!settingsWidget) {
            settingsWidget = new EffectSettingsWidget(parameters);

            m_stack->addWidget(settingsWidget);
        } else {
            settingsWidget->resetDefaults();
        }
        QtopiaApplication::setMenuLike(this, false);

        m_stack->setCurrentWidget(settingsWidget);
        setWindowTitle(index.data(Qt::DisplayRole).toString());
    } else {
        emit effectSelected(m_selectedPlugin, m_selectedEffect, QMap<QString, QVariant>());

        accept();
    }
}

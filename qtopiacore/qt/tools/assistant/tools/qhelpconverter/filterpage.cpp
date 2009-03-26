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

#include <QtGui/QMessageBox>
#include "filterpage.h"

QT_BEGIN_NAMESPACE

FilterPage::FilterPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Filter Settings"));
    setSubTitle(tr("Specify the filter attributes for the "
        "documentation. If filter attributes are used, "
        "also define a custom filter for it. Both, the "
        "filter attributes and the custom filters are "
        "optional."));

    m_ui.setupUi(this);
    m_ui.customFilterWidget->headerItem()->setText(0, tr("Filter Name"));
    m_ui.customFilterWidget->headerItem()->setText(1, tr("Filter Attributes"));
    m_ui.customFilterWidget->setRootIsDecorated(false);
    m_ui.removeButton->setDisabled(true);
    connect(m_ui.addButton, SIGNAL(clicked()),
        this, SLOT(addFilter()));
    connect(m_ui.removeButton, SIGNAL(clicked()),
        this, SLOT(removeFilter()));
}

bool FilterPage::validatePage()
{
    m_filterAttributes.clear();
    foreach (QString f, m_ui.filterLineEdit->text().split(QLatin1Char(','))) {
        if (!f.trimmed().isEmpty())
            m_filterAttributes.append(f.trimmed());
    }

    m_customFilters.clear();
    QSet<QString> names;
    QSet<QString> atts;
    QString str;
    CustomFilter customFilter;
    QTreeWidgetItem *item = 0;
    for (int i=0; i<m_ui.customFilterWidget->topLevelItemCount(); ++i) {
        item = m_ui.customFilterWidget->topLevelItem(i);
        str = item->text(0);
        if (str.isEmpty() || names.contains(str)) {
            QMessageBox::critical(this, tr("Custom Filters"),
                tr("The custom filter \'%1\' is defined multiple times.")
                .arg(str));
            return false;
        }
        names.insert(str);
        customFilter.name = str;

        str.clear();
        QStringList lst;
        foreach (QString s, item->text(1).split(QLatin1Char(','))) {
            QString st = s.trimmed();
            if (!st.isEmpty()) {
                str += QLatin1Char(',') + st;
                lst.append(st);
            }
        }
        if (atts.contains(str)) {
            QMessageBox::critical(this, tr("Custom Filters"),
                tr("The attributes for custom filter \'%1\' are defined multiple times.")
                .arg(customFilter.name));
            return false;
        }
        atts.insert(str);
        customFilter.filterAttributes = lst;
        m_customFilters.append(customFilter);
    }
    return true;
}

QStringList FilterPage::filterAttributes() const
{
    return m_filterAttributes;
}

QList<CustomFilter> FilterPage::customFilters() const
{
    return m_customFilters;
}

void FilterPage::addFilter()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui.customFilterWidget);
    item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable|Qt::ItemIsSelectable);
    item->setText(0, QLatin1String("unfiltered"));
    item->setText(1, QLatin1String(""));
    m_ui.customFilterWidget->editItem(item, 0);
    m_ui.removeButton->setDisabled(false);
}

void FilterPage::removeFilter()
{
    QModelIndex idx = m_ui.customFilterWidget->currentIndex();
    if (!idx.isValid())
        return;
    QTreeWidgetItem *item = m_ui.customFilterWidget->takeTopLevelItem(idx.row());
    delete item;
    if (!m_ui.customFilterWidget->topLevelItemCount())
        m_ui.removeButton->setDisabled(true);
}

QT_END_NAMESPACE

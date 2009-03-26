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

#include <QtGui/QFileDialog>

#include "pathpage.h"

QT_BEGIN_NAMESPACE

PathPage::PathPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Source File Paths"));
    setSubTitle(tr("Specify the paths where the sources files "
        "are located. By default, all files in those directories "
        "matched by the file filter will be included."));

    m_ui.setupUi(this);
    connect(m_ui.addButton, SIGNAL(clicked()),
        this, SLOT(addPath()));
    connect(m_ui.removeButton, SIGNAL(clicked()),
        this, SLOT(removePath()));

    m_ui.filterLineEdit->setText(QLatin1String("*.html, *.htm, *.png, *.jpg, *.css"));

    registerField(QLatin1String("sourcePathList"), m_ui.pathListWidget);
    m_firstTime = true;
}

void PathPage::setPath(const QString &path)
{
    if (!m_firstTime)
        return;

    m_ui.pathListWidget->addItem(path);
    m_firstTime = false;
    m_ui.pathListWidget->setCurrentRow(0);
}

QStringList PathPage::paths() const
{
    QStringList lst;
    for (int i = 0; i<m_ui.pathListWidget->count(); ++i)
        lst.append(m_ui.pathListWidget->item(i)->text());
    return lst;
}

QStringList PathPage::filters() const
{
    QStringList lst;
    foreach (QString s, m_ui.filterLineEdit->text().split(QLatin1Char(','))) {
        lst.append(s.trimmed());
    }
    return lst;
}

void PathPage::addPath()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        tr("Source File Path"));
    if (!dir.isEmpty())
        m_ui.pathListWidget->addItem(dir);
}

void PathPage::removePath()
{
    QListWidgetItem *i = m_ui.pathListWidget
        ->takeItem(m_ui.pathListWidget->currentRow());
    delete i;
    if (!m_ui.pathListWidget->count())
        m_ui.removeButton->setEnabled(false);
}

QT_END_NAMESPACE

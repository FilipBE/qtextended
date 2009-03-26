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

#include <QtCore/QFile>
#include <QtCore/QVariant>

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

#include "inputpage.h"
#include "adpreader.h"

QT_BEGIN_NAMESPACE

InputPage::InputPage(AdpReader *reader, QWidget *parent)
    : QWizardPage(parent)
{
    m_adpReader = reader;
    setTitle(tr("Input File"));
    setSubTitle(tr("Specify the .adp or .dcf file you want "
        "to convert to the new Qt help project format and/or "
        "collection format."));

    m_ui.setupUi(this);
    connect(m_ui.browseButton, SIGNAL(clicked()),
        this, SLOT(getFileName()));

    registerField(QLatin1String("adpFileName"), m_ui.fileLineEdit);
}

void InputPage::getFileName()
{
    QString f = QFileDialog::getOpenFileName(this, tr("Open file"), QString(),
        tr("Qt Help Files (*.adp *.dcf)"));
    if (!f.isEmpty())
        m_ui.fileLineEdit->setText(f);
}

bool InputPage::validatePage()
{
    QFile f(m_ui.fileLineEdit->text().trimmed());
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("File Open Error"),
            tr("The specified file could not be opened!"));
        return false;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_adpReader->readData(f.readAll());
    QApplication::restoreOverrideCursor();
    if (m_adpReader->hasError()) {
        QMessageBox::critical(this, tr("File Parsing Error"),
            tr("Parsing error in line %1!").arg(m_adpReader->lineNumber()));
        return false;
    }

    return true;
}

QT_END_NAMESPACE

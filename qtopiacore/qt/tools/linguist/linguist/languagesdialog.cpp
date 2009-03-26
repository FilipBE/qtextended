/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "languagesdialog.h"
#include "trwindow.h"
#include "languagesmanager.h"
#include "messagemodel.h"
#include <QFileDialog>
#include <QCheckBox>

QT_BEGIN_NAMESPACE

LanguagesDialog::LanguagesDialog(LanguagesManager *languagesManager, QWidget *w) : QDialog(w), m_languagesManager(languagesManager)
{
    setupUi(this);
    openFileButton->setIcon(QIcon(TrWindow::resourcePrefix() + QLatin1String("/fileopen.png")));
    connect(languagesList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    connect(m_languagesManager, SIGNAL(listChanged()), this, SLOT(messageModelListChanged()));

    setupList();
    update();
}

void LanguagesDialog::messageModelListChanged()
{
    setupList();
    update();
}

void LanguagesDialog::setupList()
{
    int row = selectedRow();
    languagesList->clear();

    foreach (MessageModel *messageModel, m_languagesManager->auxModels()) {
        QString locale(localeString(messageModel->language(), messageModel->country()));
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, locale);
        item->setText(1, QFileInfo(messageModel->srcFileName()).fileName());
        languagesList->addTopLevelItem(item);
    }

    selectRow(row);
}

QString LanguagesDialog::localeString(QLocale::Language lang, QLocale::Country country)
{
    return QLocale::languageToString(lang)
        + QLatin1Char('/') + QLocale::countryToString(country);
}

int LanguagesDialog::selectedRow()
{
    QList<QTreeWidgetItem *> rows = languagesList->selectedItems();
    if (rows.isEmpty())
        return -1;
    return languagesList->indexOfTopLevelItem(rows.first());
}

void LanguagesDialog::selectRow(int row)
{
    if (row >= 0)
        languagesList->setCurrentItem(languagesList->topLevelItem(row));
}

void LanguagesDialog::on_openFileButton_clicked()
{
    QString varFilt;

    QFileInfo mainFile(m_languagesManager->mainModel()->srcFileName());
    QString mainFileDir = mainFile.absolutePath();
    QString mainFileBase = mainFile.baseName();
    int pos = mainFileBase.indexOf(QLatin1Char('_'));
    if (pos > 0)
        varFilt = tr("Related files (%1);;")
            .arg(mainFileBase.left(pos) + QLatin1String("_*.") + mainFile.completeSuffix());

    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Translation File"), mainFileDir,
        varFilt +
        tr("Related files (%1);;"
           "Qt translation sources (*.ts);;"
           "XLIFF localization files (*.xlf);;"
           "All files (*)"));
    foreach (const QString &name, fileNames)
        m_languagesManager->openAuxLanguageFile(name);
}

void LanguagesDialog::on_removeButton_clicked()
{
    m_languagesManager->removeAuxLanguage(m_languagesManager->auxModels().at(selectedRow()));
}

void LanguagesDialog::on_upButton_clicked()
{
    const int row = selectedRow();
    selectRow(row - 1);
    MessageModel *model = m_languagesManager->auxModels().at(row);
    m_languagesManager->moveAuxLanguage(model, m_languagesManager->getPos(model) - 1);
}

void LanguagesDialog::on_downButton_clicked()
{
    const int row = selectedRow();
    selectRow(row + 1);
    MessageModel *model = m_languagesManager->auxModels().at(row);
    m_languagesManager->moveAuxLanguage(model, m_languagesManager->getPos(model) + 1);
}

void LanguagesDialog::selectionChanged()
{
    const int row = selectedRow();
    const bool auxLangSelected = row >= 0;

    removeButton->setEnabled(auxLangSelected);
    upButton->setEnabled(auxLangSelected && row != 0);
    downButton->setEnabled(auxLangSelected && row != languagesList->topLevelItemCount() - 1);
}

QT_END_NAMESPACE

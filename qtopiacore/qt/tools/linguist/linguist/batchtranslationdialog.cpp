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

#include "batchtranslationdialog.h"
#include <QtCore>
#include <QtGui>
#include <QtGui/QProgressDialog>
#include "phrase.h"
#include "messagemodel.h"

QT_BEGIN_NAMESPACE

CheckableListModel::CheckableListModel(QObject *parent)
: QStandardItemModel(parent)
{
}

Qt::ItemFlags CheckableListModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

BatchTranslationDialog::BatchTranslationDialog(MessageModel *model, QWidget *w)
 : QDialog(w), m_model(this), m_messagemodel(model)
{
    m_ui.setupUi(this);
    connect(m_ui.runButton, SIGNAL(clicked()), this, SLOT(startTranslation()));
    connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_ui.moveUpButton, SIGNAL(clicked()), this, SLOT(movePhraseBookUp()));
    connect(m_ui.moveDownButton, SIGNAL(clicked()), this, SLOT(movePhraseBookDown()));

    m_ui.phrasebookList->setModel(&m_model);
    m_ui.phrasebookList->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_ui.phrasebookList->setSelectionMode(QAbstractItemView::SingleSelection);
}


void BatchTranslationDialog::setPhraseBooks(const QList<PhraseBook *> &phrasebooks)
{
    m_model.clear();
    m_model.insertColumn(0);
    m_phrasebooks = phrasebooks;
    int count = phrasebooks.count();
    m_model.insertRows(0, count);
    for (int i = 0; i < count; ++i) {
        QString name = phrasebooks[i]->friendlyPhraseBookName();
        m_model.setData(m_model.index(i, 0), name);
        m_model.setData(m_model.index(i, 0), Qt::Checked, Qt::CheckStateRole);
    }
}

PhraseBook *BatchTranslationDialog::GetNamedPhraseBook(const QString &name)
{
    for (int i = 0; i < m_phrasebooks.count(); ++i) {
        if (m_phrasebooks[i]->friendlyPhraseBookName() == name) return m_phrasebooks[i];
    }
    return 0;
}

void BatchTranslationDialog::startTranslation()
{
    int translatedcount = 0;
    QCursor oldCursor = cursor();
    setCursor(Qt::BusyCursor);
    int messageCount = m_messagemodel->getMessageCount();

    QProgressDialog *dlgProgress;
    dlgProgress = new QProgressDialog(tr("Searching, please wait..."), tr("&Cancel"), 0, messageCount, this);
    dlgProgress->show();

    MessageModel::iterator it = m_messagemodel->begin();
    int msgidx = 0;
    bool doProcess = true;
    for ( ;it.current() && doProcess; ++it) {
        MessageItem *m  = it.current();
        if ( m_ui.ckOnlyUntranslated->isChecked() ) {
            if (!m->translation().isEmpty()) continue;
        }
        // Go through them in the order the user specified in the phrasebookList
        for (int b = 0; b < m_model.rowCount() && doProcess; ++b) {
            QVariant checkState = m_model.data(m_model.index(b, 0), Qt::CheckStateRole);
            if (checkState == Qt::Checked) {
                QVariant pbname = m_model.data(m_model.index(b, 0));
                PhraseBook *pb = GetNamedPhraseBook(pbname.toString());
                foreach (const Phrase *ph, pb->phrases()) {
                    if (ph->source() == m->sourceText() && !m->finished()) {
                        m->setTranslation(ph->target());
                        m->setFinished(m_ui.ckMarkFinished->isChecked());
                        ++translatedcount;
                    }
                }
                if (dlgProgress->wasCanceled()) {
                    doProcess = false;
                    break;
                }
            }
            qApp->processEvents();
        }
        ++msgidx;
        dlgProgress->setValue(msgidx);
    }
    dlgProgress->setValue(messageCount);

    setCursor(oldCursor);
    m_messagemodel->updateAll();
    emit finished();
    QMessageBox::information(this, tr("Linguist batch translator"),
        tr("Batch translated %n entries", "", translatedcount), QMessageBox::Ok);

    //### update stats
    //### update translationcount etc.
}

void BatchTranslationDialog::movePhraseBookUp()
{
    QModelIndexList indexes = m_ui.phrasebookList->selectionModel()->selectedIndexes();
    if (indexes.count() <= 0) return;

    QModelIndex sel = indexes[0];
    int row = sel.row();
    if (row > 0) {
        QModelIndex other = m_model.index(row - 1, 0);
        QMap<int, QVariant> seldata = m_model.itemData(sel);
        m_model.setItemData(sel, m_model.itemData(other));
        m_model.setItemData(other, seldata);
        m_ui.phrasebookList->selectionModel()->setCurrentIndex(other, QItemSelectionModel::ClearAndSelect);
    }
}

void BatchTranslationDialog::movePhraseBookDown()
{
    QModelIndexList indexes = m_ui.phrasebookList->selectionModel()->selectedIndexes();
    if (indexes.count() <= 0) return;

    QModelIndex sel = indexes[0];
    int row = sel.row();
    if (row < m_model.rowCount() - 1) {
        QModelIndex other = m_model.index(row + 1, 0);
        QMap<int, QVariant> seldata = m_model.itemData(sel);
        m_model.setItemData(sel, m_model.itemData(other));
        m_model.setItemData(other, seldata);
        m_ui.phrasebookList->selectionModel()->setCurrentIndex(other, QItemSelectionModel::ClearAndSelect);
    }
}

QT_END_NAMESPACE

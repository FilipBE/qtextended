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

#include "errorsview.h"
#include "messagemodel.h"
#include <QList>
#include <QListView>
#include <QString>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE


ErrorsView::ErrorsView(QWidget *parent) :
    QWidget(parent)
{
    m_list = new QStandardItemModel(this);
    QListView *listView = new QListView(this);
    listView->setModel(m_list);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(listView);
}

void ErrorsView::clear()
{
    m_list->clear();
}

void ErrorsView::addError(const ErrorType type, const QString &arg)
{
    QString error;
    switch (type) {
      case SuperfluousAccelerator:
        addError(tr("Accelerator possibly superfluous in translation."));
        break;
      case MissingAccelerator:
        addError(tr("Accelerator possibly missing in translation."));
        break;
      case PunctuationDiffer:
        addError(tr("Translation does not end with the same punctuation as the source text."));
        break;
      case IgnoredPhrasebook:
        addError(tr("A phrase book suggestion for '%1' was ignored.").arg(arg));
        break;
      case PlaceMarkersDiffer:
        addError(tr("Translation does not refer to the same place markers as in the source text."));
        break;
      default:
        addError(tr("Unknown error"));
        break;
    }
}

QList<QString> ErrorsView::errorList()
{
    QList<QString> errors;
    for (int i = 0; i < m_list->rowCount(); i++) {
        QStandardItem *item = m_list->item(i);
        errors.append(item->text());
    }
    return errors;
}

void ErrorsView::addError(const QString &error)
{
    QStandardItem *item = new QStandardItem(QIcon(*MessageModel::pxDanger), error);
    item->setEditable(false);
    m_list->appendRow(QList<QStandardItem*>() << item);
}

QT_END_NAMESPACE

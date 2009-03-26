/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

/*
TRANSLATOR qdesigner_internal::ListWidgetEditor
*/

#include "listwidgeteditor.h"
#include <abstractformbuilder.h>
#include <iconloader_p.h>
#include <formwindowbase_p.h>
#include <qdesigner_utils_p.h>

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerIconCacheInterface>

#include <QtGui/QComboBox>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

ListWidgetEditor::ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    m_form = qobject_cast<FormWindowBase *>(form);
    ui.itemIconSelector->setFormEditor(form->core());
    ui.itemIconSelector->setEnabled(false);
    ui.itemIconSelector->setPixmapCache(m_form->pixmapCache());
    ui.itemIconSelector->setIconCache(m_form->iconCache());

    QIcon upIcon = createIconSet(QString::fromUtf8("up.png"));
    QIcon downIcon = createIconSet(QString::fromUtf8("down.png"));
    QIcon minusIcon = createIconSet(QString::fromUtf8("minus.png"));
    QIcon plusIcon = createIconSet(QString::fromUtf8("plus.png"));
    ui.moveItemUpButton->setIcon(upIcon);
    ui.moveItemDownButton->setIcon(downIcon);
    ui.newItemButton->setIcon(plusIcon);
    ui.deleteItemButton->setIcon(minusIcon);

    connect(m_form->iconCache(), SIGNAL(reloaded()), this, SLOT(cacheReloaded()));
}

ListWidgetEditor::~ListWidgetEditor()
{
}

void ListWidgetEditor::fillContentsFromListWidget(QListWidget *listWidget)
{
    setWindowTitle(tr("Edit List Widget"));
    for (int i=0; i<listWidget->count(); ++i) {
        QListWidgetItem *oldItem = listWidget->item(i);
        QListWidgetItem *item = oldItem->clone();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.listWidget->addItem(item);
    }

    if (ui.listWidget->count() > 0)
        ui.listWidget->setCurrentRow(0);
    else
        updateEditor();
}

void ListWidgetEditor::fillContentsFromComboBox(QComboBox *comboBox)
{
    setWindowTitle(tr("Edit Combobox"));
    for (int i=0; i<comboBox->count(); ++i) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(comboBox->itemText(i));
        item->setIcon(qVariantValue<QIcon>(comboBox->itemData(i)));
        item->setData(QAbstractFormBuilder::resourceRole(), comboBox->itemData(i, QAbstractFormBuilder::resourceRole()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.listWidget->addItem(item);
    }

    if (ui.listWidget->count() > 0)
        ui.listWidget->setCurrentRow(0);
    else
        updateEditor();
}

void ListWidgetEditor::on_newItemButton_clicked()
{
    int row = ui.listWidget->currentRow() + 1;

    QListWidgetItem *item = new QListWidgetItem;
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(tr("New Item"));
    if (row < ui.listWidget->count())
        ui.listWidget->insertItem(row, item);
    else
        ui.listWidget->addItem(item);

    ui.listWidget->setCurrentItem(item);
    ui.listWidget->editItem(item);
}

void ListWidgetEditor::on_deleteItemButton_clicked()
{
    int row = ui.listWidget->currentRow();

    if (row != -1)
        delete ui.listWidget->takeItem(row);

    if (row == ui.listWidget->count())
        row--;
    if (row < 0)
        updateEditor();
    else
        ui.listWidget->setCurrentRow(row);
}

void ListWidgetEditor::on_moveItemUpButton_clicked()
{
    int row = ui.listWidget->currentRow();
    if (row <= 0)
        return; // nothing to do

    ui.listWidget->insertItem(row - 1, ui.listWidget->takeItem(row));
    ui.listWidget->setCurrentRow(row - 1);
}

void ListWidgetEditor::on_moveItemDownButton_clicked()
{
    int row = ui.listWidget->currentRow();
    if (row == -1 || row == ui.listWidget->count() - 1)
        return; // nothing to do

    ui.listWidget->insertItem(row + 1, ui.listWidget->takeItem(row));
    ui.listWidget->setCurrentRow(row + 1);
}

void ListWidgetEditor::on_listWidget_currentRowChanged(int)
{
    updateEditor();
}

void ListWidgetEditor::on_listWidget_itemChanged(QListWidgetItem *)
{
    updateEditor();
}

void ListWidgetEditor::on_itemIconSelector_iconChanged(const PropertySheetIconValue &icon)
{
    int currentRow = ui.listWidget->currentRow();
    if (currentRow == -1)
        return;
    QListWidgetItem *item = ui.listWidget->item(currentRow);

    item->setData(QAbstractFormBuilder::resourceRole(), qVariantFromValue(icon));
    item->setIcon(m_form->iconCache()->icon(icon));
}

int ListWidgetEditor::count() const
{
    return ui.listWidget->count();
}

PropertySheetIconValue ListWidgetEditor::icon(int row) const
{
    return qVariantValue<PropertySheetIconValue>(ui.listWidget->item(row)->data(QAbstractFormBuilder::resourceRole()));
}

QString ListWidgetEditor::text(int row) const
{
    return ui.listWidget->item(row)->text();
}

void ListWidgetEditor::cacheReloaded()
{
    reloadIconResources(m_form->iconCache(), ui.listWidget);
}

void ListWidgetEditor::updateEditor()
{
    bool currentItemEnabled = false;

    bool moveRowUpEnabled = false;
    bool moveRowDownEnabled = false;

    QListWidgetItem *item = ui.listWidget->currentItem();
    if (item) {
        currentItemEnabled = true;
        int currentRow = ui.listWidget->currentRow();
        if (currentRow > 0)
            moveRowUpEnabled = true;
        if (currentRow < ui.listWidget->count() - 1)
            moveRowDownEnabled = true;
    }

    ui.moveItemUpButton->setEnabled(moveRowUpEnabled);
    ui.moveItemDownButton->setEnabled(moveRowDownEnabled);
    ui.deleteItemButton->setEnabled(currentItemEnabled);
    ui.itemIconSelector->setEnabled(currentItemEnabled);

    QString itemText;
    PropertySheetIconValue itemIcon;

    if (item) {
        itemText = item->text();
        itemIcon = qVariantValue<PropertySheetIconValue>(item->data(QAbstractFormBuilder::resourceRole()));
    }
    ui.itemIconSelector->setIcon(itemIcon);
}

QT_END_NAMESPACE

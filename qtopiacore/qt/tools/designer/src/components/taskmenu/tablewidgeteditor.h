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

#ifndef TABLEWIDGETEDITOR_H
#define TABLEWIDGETEDITOR_H

#include "ui_tablewidgeteditor.h"

QT_BEGIN_NAMESPACE

class QTableWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TableWidgetEditor: public QDialog
{
    Q_OBJECT
public:
    TableWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~TableWidgetEditor();

    void fillContentsFromTableWidget(QTableWidget *tableWidget);

    void fillTableWidgetFromContents(QTableWidget *tableWidget);

private slots:

    void on_tableWidget_currentCellChanged(int currentRow, int currnetCol, int, int);
    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_itemIconSelector_iconChanged(const PropertySheetIconValue &icon);

    void on_columnsListWidget_currentRowChanged(int col);
    void on_columnsListWidget_itemChanged(QListWidgetItem *item);

    void on_newColumnButton_clicked();
    void on_deleteColumnButton_clicked();
    void on_moveColumnUpButton_clicked();
    void on_moveColumnDownButton_clicked();

    void on_columnIconSelector_iconChanged(const PropertySheetIconValue &icon);

    void on_rowsListWidget_currentRowChanged(int row);
    void on_rowsListWidget_itemChanged(QListWidgetItem *item);

    void on_newRowButton_clicked();
    void on_deleteRowButton_clicked();
    void on_moveRowUpButton_clicked();
    void on_moveRowDownButton_clicked();

    void on_rowIconSelector_iconChanged(const PropertySheetIconValue &icon);

    void cacheReloaded();
private:
    void copyContents(QTableWidget *sourceWidget, QTableWidget *destWidget);
    void updateEditor();
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);
    void moveRowsUp(int fromRow, int toRow);
    void moveRowsDown(int fromRow, int toRow);

    Ui::TableWidgetEditor ui;
    FormWindowBase *m_form;
    bool m_updating;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TABLEWIDGETEDITOR_H

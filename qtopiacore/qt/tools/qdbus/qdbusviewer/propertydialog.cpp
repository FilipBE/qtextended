/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#include "propertydialog.h"

#include <QHeaderView>
#include <QLayout>
#include <QDebug>

PropertyDialog::PropertyDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    buttonBox = new QDialogButtonBox;
    propertyTable = new QTableWidget;
    label = new QLabel;

    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyTable->setColumnCount(2);
    const QStringList labels = QStringList() << "Name" << "Value";
    propertyTable->setHorizontalHeaderLabels(labels);
    propertyTable->horizontalHeader()->setStretchLastSection(true);
    propertyTable->setEditTriggers(QAbstractItemView::AllEditTriggers);

    connect(buttonBox, SIGNAL(accepted()), SLOT(accept()), Qt::QueuedConnection);
    connect(buttonBox, SIGNAL(rejected()), SLOT(reject()), Qt::QueuedConnection);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    layout->addWidget(propertyTable);
    layout->addWidget(buttonBox);
}

void PropertyDialog::setInfo(const QString &caption)
{
    label->setText(caption);
}

void PropertyDialog::addProperty(const QString &aname, QVariant::Type type)
{
    int rowCount = propertyTable->rowCount();
    propertyTable->setRowCount(rowCount + 1);

    QString name = aname;
    if (name.isEmpty())
        name = "argument " + QString::number(rowCount + 1);
    QTableWidgetItem *nameItem = new QTableWidgetItem(name + " (" + QVariant::typeToName(type) + ")");
    nameItem->setFlags(nameItem->flags() &
            ~(Qt::ItemIsEditable | Qt::ItemIsSelectable));
    propertyTable->setItem(rowCount, 0, nameItem);

    QTableWidgetItem *valueItem = new QTableWidgetItem;
    valueItem->setData(Qt::DisplayRole, QVariant(type));
    propertyTable->setItem(rowCount, 1, valueItem);
}

int PropertyDialog::exec()
{
    propertyTable->resizeColumnToContents(0);
    propertyTable->setFocus();
    propertyTable->setCurrentCell(0, 1);
    return QDialog::exec();
}

QList<QVariant> PropertyDialog::values() const
{
    QList<QVariant> result;

    for (int i = 0; i < propertyTable->rowCount(); ++i)
        result << propertyTable->item(i, 1)->data(Qt::EditRole);

    return result;
}


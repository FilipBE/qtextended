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

#include "itemdialog.h"
#include "item.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>

QT_BEGIN_NAMESPACE

QList<ItemDialog *> ItemDialog::openDialogs;

ItemDialog::ItemDialog(QWidget *parent, const Item *item)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QGridLayout *layout = new QGridLayout(this);
    QLabel *lblPix = new QLabel();
    const BusinessCard *bcard = static_cast<const BusinessCard *>(item);
    if (bcard) {
        QPixmap pm(bcard->bigPicture());
        if (pm.isNull())
            pm = QPixmap(QLatin1String(":/trolltech/qthack/images/qt.png"));
        lblPix->setPixmap(pm);
    } else {
        lblPix->setPixmap(item->pixmapName());
    }

    QLabel *lblName = new QLabel(item->name());
    QLabel *lblDesc = new QLabel(item->description());
    lblDesc->setWordWrap(true);
    QFont descFont = lblDesc->font();
    descFont.setItalic(true);
    descFont.setPointSize(descFont.pointSize() - 2);
    lblDesc->setFont(descFont);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(btnBox, SIGNAL(rejected()), this, SLOT(close()));

    layout->addWidget(lblPix, 0, 0, 2, 2);
    layout->addWidget(lblName, 0, 2, 1, 1);
    layout->addWidget(lblDesc, 1, 2, 1, 1);
    layout->addWidget(btnBox, 2, 3, 1, 1);
    openDialogs.append(this);
}

void ItemDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    openDialogs.removeAll(this);
    if (openDialogs.isEmpty()) {
        parentWidget()->activateWindow();
    } else {
        ItemDialog *itemDialog = openDialogs.last();
        itemDialog->show();
        itemDialog->raise();
        itemDialog->activateWindow();
    }
}

QT_END_NAMESPACE

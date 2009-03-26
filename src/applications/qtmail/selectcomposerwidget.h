/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef SELECTCOMPOSERWIDGET_H
#define SELECTCOMPOSERWIDGET_H

#include <QPair>
#include <QMailMessage>
#include <QString>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class SelectListWidget;

class SelectComposerWidget : public QWidget
{
    Q_OBJECT

public:
    SelectComposerWidget( QWidget* parent );

    void setSelected(const QString& selected, QMailMessage::MessageType type);

    QString singularKey() const;

    QList<QMailMessage::MessageType> availableTypes() const;

    QPair<QString, QMailMessage::MessageType> currentSelection() const;

signals:
    void selected(const QPair<QString, QMailMessage::MessageType> &);
    void cancel();

protected slots:
    void accept(QListWidgetItem* item);
    void refresh();

protected:
    void keyPressEvent(QKeyEvent *e);
    void init();

private:
    SelectListWidget* m_listWidget;
};

#endif

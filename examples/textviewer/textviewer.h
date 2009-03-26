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

#ifndef TEXTVIEWER_H
#define TEXTVIEWER_H
#include <QContentSet>
#include <QMainWindow>
#include <QTextEdit>
class QDocumentSelector;

class TextViewer : public QMainWindow
{
    Q_OBJECT
public:
    TextViewer( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~TextViewer();
    void keyPressEvent(QKeyEvent *e);

private slots:
    void openDocument();
    void documentSelected(const QContent & docContent);

private:
    QTextEdit *textArea;
    QDocumentSelector *docSelector;
};

#endif

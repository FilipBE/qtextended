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

#ifndef NOTESDEMO_H
#define NOTESDEMO_H

#include <QDialog>
#include <QContent>

class QDocumentSelector;
class QTextEdit;
class QStackedLayout;
class QTextDocument;

class NotesDemo : public QDialog
{
    Q_OBJECT
public:
    NotesDemo( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

public slots:
    virtual void done( int result );

private slots:
    void newDocument();
    void openDocument( const QContent &document );

private:
    bool readContent( QTextDocument *document, QContent *content );
    bool writeContent( QTextDocument *document, QContent *content );

    QStackedLayout *layout;
    QDocumentSelector *documentSelector;
    QTextEdit *editor;
    QContent currentDocument;
};

#endif

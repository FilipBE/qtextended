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

#include "notesdemo.h"
#include <QStackedLayout>
#include <QDocumentSelector>
#include <QTextEdit>
#include <QTextDocument>
#include <QTimeString>
#include <QtDebug>

/*!
    \class NotesDemo
    \brief The NotesDemo application is a simple text editor demonstrating the use of a QDocumentSelector.

    It is comprised of a QDocumentSelector and a QTextEdit in a dialog with a stacked layout so only
    one widget is visible at a time.  The document selector is initially visible and selecting a document
    or creating a new document will display the document in the text editor.  Accepting or canceling the
    editor will return to the document selector, saving the document if it was accepted, and accepting or
    canceling the document selector will exit the application.
*/

/*!
    Constructs a NotesDemo dialog which is a child of \a parent and has the given window
    \a flags.
 */
NotesDemo::NotesDemo( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags )
{
    // Create a new document selector which lists documents with the MIME type text/plain
    // sorted so the most recently edited documents appear first.
    documentSelector = new QDocumentSelector;

    documentSelector->setFilter( QContentFilter::mimeType( "text/plain" ) );
    documentSelector->setSortMode( QDocumentSelector::ReverseChronological );

    // Enable the new document option so a 'New' document selection appears at the start of the
    // documents list and in the context menu.
    documentSelector->enableOptions( QDocumentSelector::NewDocument );

    // Connect to the newSelected() and documentSelected() signal so we're notified when the user
    // selects a document.
    connect( documentSelector, SIGNAL(newSelected()),
             this, SLOT(newDocument()) );
    connect( documentSelector, SIGNAL(documentSelected(QContent)),
             this, SLOT(openDocument(QContent)) );

    // Construct the text editor widget.
    editor = new QTextEdit;

    // Create a new stacked layout and add the document selector and text editor widgets to it.
    // As the layout is given the dialog as a parent it is automatically set as the layout for
    // the dialog, and the widgets added to it are re-parented to the dialog.  The document
    // will be the initial widget shown as it was added to the layout first.
    layout = new QStackedLayout( this );

    layout->addWidget( documentSelector );
    layout->addWidget( editor );
}

/*!
    Creates a new text document and displays the text editor.
*/
void NotesDemo::newDocument()
{
    // Set the current document to a new QContent, and set it's name and type.
    // We're just using the time and date for the name but another application
    // may want to prompt the user for a name, or use the first bit of text in
    // the document.
    currentDocument = QContent();

    currentDocument.setName( "Note " + QTimeString::localYMDHMS( QDateTime::currentDateTime() ) );
    currentDocument.setType( "text/plain" );

    // Display the editor.
    layout->setCurrentWidget( editor );
}

/*!
    Opens a text \a document selected from the QDocumentSelector and displays the text editor.
*/
void NotesDemo::openDocument( const QContent &document )
{
    // Sets the current document to the one selected.
    currentDocument = document;

    // Read in the text from the document, if the read is successful display the text editor.
    if ( readContent( editor->document(), &currentDocument ) ) {
        layout->setCurrentWidget( editor );
    }
}

/*!
    Closes the currently displayed widget.  If that is editor widget and the \a result
    is QDialog::Accepted the changes made in the editor will be saved.

    Closing the editor widget will return the document selector to the display, and closing
    the document selector will close the application.
*/
void NotesDemo::done( int result )
{
    if ( layout->currentWidget() == editor ) {
        // The current widget is the editor so finish editing the document and return to
        // the document selector. If the dialog was accepted write the changes to the
        // document, and commit the document QContent.
        if ( result == QDialog::Accepted ) {
            if ( !writeContent(editor->document(), &currentDocument ) ) {
                qWarning() << "Writing the content failed";
            } else if ( !currentDocument.commit() ) {
                qWarning() << "Committing the new content failed";
            }
        }

        editor->document()->clear();

        layout->setCurrentWidget( documentSelector );
    } else {
        // The current widget is the document selector, so close the dialog and the application.
        QDialog::done( result );
    }
}

/*!
    Reads the contents of the text document \a content into \a document.

    Returns true if the read was successful and false otherwise.
*/
bool NotesDemo::readContent( QTextDocument *document, QContent *content )
{
    // Attempt to open the content in read-only mode.  If the open succeeds QContent
    // will construct a new I/O device and return a pointer to it, the caller takes
    // ownership of the I/O device and is responsible for deleting it.
    QIODevice *ioDevice = content->open( QIODevice::ReadOnly );
    if ( !ioDevice ) {
        qWarning() << "Could not open the new content object to read from!!";
        return false;
    }
    QByteArray bytes = ioDevice->readAll();

    // Convert the string from an 8-bit ASCII byte array and set it as the plain text
    // content of the document.
    document->setPlainText( QString::fromAscii( bytes ) );

    // Close the I/O device and destroy it.
    ioDevice->close();
    delete ioDevice;

    return true;
}

/*!
    Writes the contents of \a document to the text document \a content.  It is left up to the
    calling code to \l {QContent::commit()}{commit} the changes in \a content to the document
    system.

    Returns true if the write was successful and false otherwise.
*/
bool NotesDemo::writeContent( QTextDocument *document, QContent *content )
{
    // Attempt to open the content in write-only mode.  If the open succeeds QContent
    // will construct a new I/O device and return a pointer to it, the caller takes
    // ownership of the I/O device and is responsible for deleting it.
    QIODevice *ioDevice = content->open( QIODevice::WriteOnly );
    if ( !ioDevice ) {
        qWarning() << "Could not open the new content object to write to!!";
        return false;
    }

    // Gets the plain text content of the text document and converts it to an 8-bit
    // ASCII byte array before writing it to the I/O device.
    // (This assumes that the notes are short enough to fit into memory. For longer
    // documents, use QTextDocument::begin(), QTextDocument::end() and QTextDocument::findBlock(int).)
    int bytesWritten = ioDevice->write( document->toPlainText().toAscii() );

    // Close the I/O device and delete it.
    ioDevice->close();
    delete ioDevice;

    if ( bytesWritten < 0 ) {
        qWarning() << "Error while trying to create a new notes object!!";
        return false;
    } else {
        return true;
    }
}

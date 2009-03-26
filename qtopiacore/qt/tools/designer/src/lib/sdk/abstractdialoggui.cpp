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

#include "abstractdialoggui_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QDesignerDialogGuiInterface
    \since 4.4
    \internal

    \brief The QDesignerDialogGuiInterface allows integrations of \QD to replace the
           message boxes displayed by \QD by custom dialogs.

    \inmodule QtDesigner

    QDesignerDialogGuiInterface provides virtual functions that can be overwritten
    to display message boxes and file dialogs.
    \sa QMessageBox, QFileDialog
*/

/*!
    \enum QDesignerDialogGuiInterface::Message

    This enum specifies the context from within the message box is called.

   \value FormLoadFailureMessage      Loading of a form failed
   \value UiVersionMismatchMessage    Attempt to load a file created with an old version of Designer
   \value ResourceLoadFailureMessage  Resources specified in a file could not be found
   \value TopLevelSpacerMessage       Spacer items detected on a container without layout
   \value PropertyEditorMessage       Messages of the propert yeditor
   \value SignalSlotEditorMessage     Messages of the signal / slot editor
   \value FormEditorMessage           Messages of the form editor
   \value PreviewFailureMessage       A preview could not be created
   \value PromotionErrorMessage       Messages related to promotion of a widget
   \value ResourceEditorMessage       Messages of the resource editor
   \value ScriptDialogMessage         Messages of the script dialog
   \value SignalSlotDialogMessage     Messages of the signal slot dialog
   \value OtherMessage                Unspecified context
*/

/*!
    Constructs a QDesignerDialogGuiInterface object.
*/

QDesignerDialogGuiInterface::QDesignerDialogGuiInterface()
{
}

/*!
    Destroys the QDesignerDialogGuiInterface object.
*/
QDesignerDialogGuiInterface::~QDesignerDialogGuiInterface()
{
}

/*!
    \fn QMessageBox::StandardButton QDesignerDialogGuiInterface::message(QWidget *parent, Message context, QMessageBox::Icon icon, const QString &title, const QString &text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)

     Opens a message box as child of \a parent within the context \a context, using \a icon, \a title, \a text, \a buttons and \a defaultButton
     and returns the button chosen by the user.
*/

/*!
    \fn QString QDesignerDialogGuiInterface::getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir, QFileDialog::Options options)

     Opens a file dialog as child of \a parent using the parameters \a caption, \a dir and \a options that prompts the
     user for an existing directory. Returns a directory selected by the user.
*/

/*!
    \fn QString QDesignerDialogGuiInterface::getOpenFileName(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options)

    Opens a file dialog as child of \a parent using the parameters \a caption, \a dir, \a filter, \a selectedFilter and \a options
    that prompts the user for an existing file. Returns a file selected by the user.
*/

/*!
    \fn QStringList QDesignerDialogGuiInterface::getOpenFileNames(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options)

    Opens a file dialog as child of \a parent using the parameters \a caption, \a dir, \a filter, \a selectedFilter and \a options
    that prompts the user for a set of existing files. Returns one or more existing files selected by the user.
*/

/*!
    \fn QString QDesignerDialogGuiInterface::getSaveFileName(QWidget *parent, const QString &caption, const QString &dir, const QString &filter, QString *selectedFilter, QFileDialog::Options)

    Opens a file dialog as child of \a parent using the parameters \a caption, \a dir, \a filter, \a selectedFilter and \a options
    that prompts the user for a file. Returns a file selected by the user. The file does not have to exist.
*/

QT_END_NAMESPACE

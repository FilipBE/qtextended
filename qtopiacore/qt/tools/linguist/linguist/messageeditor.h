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

#ifndef MSGEDIT_H
#define MSGEDIT_H

#include "messagemodel.h"

#include <QFrame>
#include <QLocale>
#include <QScrollArea>
#include <QShortcut>

QT_BEGIN_NAMESPACE

class QBoxLayout;
class QDockWidget;
class QMainWindow;
template <class T, typename U> class QMap;
class QMenu;

class QTextEdit;
class QTreeView;

class Phrase;
class PhraseModel;
class EditorPage;
class ExpandingTextEdit;
class LanguagesManager;
class MessageEditor;
class FormatTextEdit;
class FormWidget;

class EditorPage : public QFrame
{
    Q_OBJECT

public:
    EditorPage(QLocale::Language targetLanguage, MessageEditor *parent = 0, const char *name = 0);
    FormWidget *activeTranslation() const;
    int activeTranslationNumerus() const;
    QStringList translations() const;
    void setNumerusForms(const QStringList &numerusForms);
    int currentTranslationEditor();

public slots:
    void setTargetLanguage(QLocale::Language lang);

protected:
    void updateCommentField();

private:
    void addPluralForm(const QString &label);
    void handleChanges();
    void showNothing();
    void setAuxLanguages(const QStringList &languages);

    QBoxLayout *m_layout;
    FormWidget *m_source;
    QList<FormWidget *> auxTranslations;
    FormatTextEdit *cmtText;
    QLocale::Language m_targetLanguage;
    QStringList m_numerusForms;
    QString     m_invariantForm;
    bool        m_pluralEditMode;
    QList<FormWidget*> m_transTexts;
    friend class MessageEditor;

private slots:
    void sourceSelectionChanged();
    void translationSelectionChanged();

signals:
    void selectionChanged();
    void currentTranslationEditorChanged();
};

class MessageEditor : public QScrollArea
{
    Q_OBJECT
public:
    MessageEditor(LanguagesManager *languages, QMainWindow *parent = 0);

    void showNothing();
    void showMessage(const MessageItem *item, const ContextItem *context);
    void setNumerusForms(const QStringList &numerusForms);
    bool eventFilter(QObject *, QEvent *);
    void setTranslation(const QString &translation, int numerus);

signals:
    void translationChanged(const QStringList &translations);
    void finished(bool finished);
    void prevUnfinished();
    void nextUnfinished();
    void updateActions(bool enable);

    void undoAvailable(bool avail);
    void redoAvailable(bool avail);
    void cutAvailable(bool avail);
    void copyAvailable(bool avail);
    void pasteAvailable(bool avail);

public slots:
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void beginFromSource();
    void setEditorFocus();
    void updateUndoRedo();
    void messageModelListChanged();
    void setTranslation(const QString &translation);

private slots:
    void emitTranslationChanged();
    void updateButtons();
    void updateCanPaste();
    void clipboardChanged();

    void updateCutAndCopy();

private:
    void setEditingEnabled(bool enabled);

    LanguagesManager *m_languages;

    EditorPage *editorPage;

    const MessageItem *m_currentMessage;
    const ContextItem *m_currentContext;

    bool cutAvail;
    bool copyAvail;

    bool mayOverwriteTranslation;
    bool clipboardEmpty;
};

QT_END_NAMESPACE

#endif

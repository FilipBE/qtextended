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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef TEXTPROPERTYEDITOR_H
#define TEXTPROPERTYEDITOR_H

#include "shared_global_p.h"
#include "shared_enums_p.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

    class PropertyLineEdit;

    // Inline-Editor for text properties. Does escaping of newline characters
    // to '\n' and back and provides validation modes. The interface
    // corresponds to that of QLineEdit.
    class QDESIGNER_SHARED_EXPORT TextPropertyEditor : public QWidget
    {
        TextPropertyEditor(const TextPropertyEditor &);
        TextPropertyEditor& operator=(const TextPropertyEditor &);
        Q_OBJECT
        Q_PROPERTY(QString text READ text WRITE setText USER true)
    public:
        enum EmbeddingMode {
            // Stand-alone widget
            EmbeddingNone,
                // Disable frame
                EmbeddingTreeView,
                // For editing in forms
                EmbeddingInPlace
        };

        enum UpdateMode {
            // Emit textChanged() as the user types
            UpdateAsYouType,
            // Emit textChanged() only when the user finishes (for QUrl, etc.)
            UpdateOnFinished
        };

        TextPropertyEditor(QWidget *parent = 0, EmbeddingMode embeddingMode = EmbeddingNone, TextPropertyValidationMode validationMode = ValidationMultiLine);

        TextPropertyValidationMode textPropertyValidationMode() const { return m_validationMode; }
        void setTextPropertyValidationMode(TextPropertyValidationMode vm);

        UpdateMode updateMode() const                { return m_updateMode; }
        void setUpdateMode(UpdateMode um) { m_updateMode = um; }

        QString text() const;

        virtual QSize sizeHint () const;
        virtual QSize minimumSizeHint () const;

        void setAlignment(Qt::Alignment alignment);

        bool hasAcceptableInput() const;

        // installs an event filter object on the private QLineEdit
        void installEventFilter(QObject *filterObject);

        // Replace newline characters by literal "\n" for inline editing
        // in mode ValidationMultiLine
        static QString stringToEditorString(const QString &s, TextPropertyValidationMode validationMode = ValidationMultiLine);

        // Replace literal "\n"  by actual new lines in mode ValidationMultiLine
        static QString editorStringToString(const QString &s, TextPropertyValidationMode validationMode = ValidationMultiLine);

        // Returns whether newline characters are valid in validationMode.
        static bool multiLine(TextPropertyValidationMode validationMode);

    signals:
        void textChanged(const QString &text);
        void editingFinished();

    public slots:
        void setText(const QString &text);
        void selectAll();
        void clear();

    protected:
        void resizeEvent(QResizeEvent * event );

    private slots:
        void slotTextChanged(const QString &text);
        void slotTextEdited();
        void slotEditingFinished();

    private:
        void setRegExpValidator(const QString &pattern);
        void markIntermediateState();

        TextPropertyValidationMode m_validationMode;
        UpdateMode m_updateMode;
        PropertyLineEdit* m_lineEdit;

        // Cached text containing real newline characters.
        QString m_cachedText;
        bool m_textEdited;
    };
}

QT_END_NAMESPACE

#endif // TEXTPROPERTYEDITOR_H

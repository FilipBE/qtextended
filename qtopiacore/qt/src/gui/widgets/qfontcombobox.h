/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QFONTCOMBOBOX_H
#define QFONTCOMBOBOX_H

#include <QtGui/qcombobox.h>
#include <QtGui/qfontdatabase.h>

#ifndef QT_NO_FONTCOMBOBOX

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

class QFontComboBoxPrivate;

class Q_GUI_EXPORT QFontComboBox : public QComboBox
{
    Q_OBJECT
    Q_FLAGS(FontFilters)
    Q_PROPERTY(QFontDatabase::WritingSystem writingSystem READ writingSystem WRITE setWritingSystem)
    Q_PROPERTY(FontFilters fontFilters READ fontFilters WRITE setFontFilters)
    Q_PROPERTY(QFont currentFont READ currentFont WRITE setCurrentFont NOTIFY currentFontChanged)
    Q_ENUMS(FontSelection)

public:
    explicit QFontComboBox(QWidget *parent = 0);
    ~QFontComboBox();

    void setWritingSystem(QFontDatabase::WritingSystem);
    QFontDatabase::WritingSystem writingSystem() const;

    enum FontFilter {
        AllFonts = 0,
        ScalableFonts = 0x1,
        NonScalableFonts = 0x2,
        MonospacedFonts = 0x4,
        ProportionalFonts = 0x8
    };
    Q_DECLARE_FLAGS(FontFilters, FontFilter)

    void setFontFilters(FontFilters filters);
    FontFilters fontFilters() const;

    QFont currentFont() const;
    QSize sizeHint() const;

public Q_SLOTS:
    void setCurrentFont(const QFont &f);

Q_SIGNALS:
    void currentFontChanged(const QFont &f);

protected:
    bool event(QEvent *e);

private:
    Q_DISABLE_COPY(QFontComboBox)
    Q_DECLARE_PRIVATE(QFontComboBox)
    Q_PRIVATE_SLOT(d_func(), void _q_currentChanged(const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_updateModel())
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontComboBox::FontFilters)

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_FONTCOMBOBOX
#endif

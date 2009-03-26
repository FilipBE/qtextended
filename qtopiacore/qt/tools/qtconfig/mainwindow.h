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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "mainwindowbase.h"

QT_BEGIN_NAMESPACE

class MainWindow : public MainWindowBase
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void closeEvent(QCloseEvent *);


public slots:
    virtual void buildPalette();
    virtual void buildFont();
    virtual void tunePalette();
    virtual void paletteSelected(int);
    virtual void styleSelected(const QString &);
    virtual void familySelected(const QString &);
    virtual void substituteSelected(const QString &);
    virtual void removeSubstitute();
    virtual void addSubstitute();
    virtual void downSubstitute();
    virtual void upSubstitute();
    virtual void removeFontpath();
    virtual void addFontpath();
    virtual void downFontpath();
    virtual void upFontpath();
    virtual void browseFontpath();
    virtual void fileSave();
    virtual void fileExit();
    virtual void somethingModified();
    virtual void helpAbout();
    virtual void helpAboutQt();
    virtual void pageChanged(QWidget *);


private:
    void buildActive();
    void buildActiveEffect();
    void buildInactive();
    void buildInactiveEffect();
    void buildDisabled();
    void buildDisabledEffect();

    void updateColorButtons();
    void updateFontSample();

    void setPreviewPalette(const QPalette &);

    void setModified(bool);

    QPalette editPalette, previewPalette;
    QStyle *previewstyle;
    QStringList fontpaths;
    bool modified;
};

QT_END_NAMESPACE

#endif // MAINWINDOW_H

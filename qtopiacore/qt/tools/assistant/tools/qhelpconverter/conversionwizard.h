/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#ifndef CONVERSIONWIZARD_H
#define CONVERSIONWIZARD_H

#include <QtGui/QWizard>
#include "adpreader.h"

QT_BEGIN_NAMESPACE

class InputPage;
class GeneralPage;
class FilterPage;
class IdentifierPage;
class PathPage;
class FilesPage;
class OutputPage;
class FinishPage;
class HelpWindow;

class ConversionWizard : public QWizard
{
    Q_OBJECT

public:
    ConversionWizard();
    void setAdpFileName(const QString &fileName);

private slots:
    void pageChanged(int id);
    void showHelp(bool toggle);
    void convert();

private:
    enum Pages {Input_Page, General_Page, Filter_Page,
        Identifier_Page, Path_Page, Files_Page, Output_Page,
        Finish_Page};
    void initializePage(int id);
    QStringList getUnreferencedFiles(const QStringList &files);
    bool eventFilter(QObject *obj, QEvent *e);
    
    AdpReader m_adpReader;
    InputPage *m_inputPage;
    GeneralPage *m_generalPage;
    FilterPage *m_filterPage;
    IdentifierPage *m_identifierPage;
    PathPage *m_pathPage;
    FilesPage *m_filesPage;
    OutputPage *m_outputPage;
    FinishPage *m_finishPage;
    QStringList m_files;
    HelpWindow *m_helpWindow;
};

QT_END_NAMESPACE

#endif

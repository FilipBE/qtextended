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

#include "languagesmanager.h"
#include "messagemodel.h"
#include <QMessageBox>

QT_BEGIN_NAMESPACE

LanguagesManager::LanguagesManager(MessageModel *mainLanguage, QObject *w) : m_w(w)
{
    m_messageModels.append(mainLanguage);
}

LanguagesManager::~LanguagesManager()
{
    foreach (MessageModel *mm, m_messageModels) {
        mm->clearContextList();
        delete mm;
    }
}

bool LanguagesManager::openAuxLanguageFile( const QString &name )
{
    if (name.isEmpty())
        return false;

    foreach (MessageModel *model, m_messageModels)
        if (model->srcFileName() == QFileInfo(name).absoluteFilePath())
            return true;

    MessageModel *auxModel = new MessageModel(m_w);
    if (!auxModel->load(name)) {
        return false;
    }
    m_messageModels.append(auxModel);
    emit listChanged();

    return true;
}

void LanguagesManager::removeAuxLanguage(MessageModel *model)
{
    m_messageModels.removeOne(model);
    delete model;
    emit listChanged();
}

void LanguagesManager::moveAuxLanguage(MessageModel *model, int newPos)
{
    m_messageModels.removeOne(model);
    m_messageModels.insert(newPos, model);
    emit listChanged();
}

QList<MessageModel*> LanguagesManager::auxModels()
{
    QList<MessageModel*> auxLanguages(m_messageModels);
    auxLanguages.removeFirst();
    return auxLanguages;
}

QT_END_NAMESPACE

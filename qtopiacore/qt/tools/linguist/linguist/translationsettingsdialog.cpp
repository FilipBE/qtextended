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


#include "translationsettingsdialog.h"
#include "messagemodel.h"
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

TranslationSettingsDialog::TranslationSettingsDialog(QWidget *w /*= 0*/) : QDialog(w)
{
    m_ui.setupUi(this);

    for (int i = QLocale::C + 1; i < QLocale::LastLanguage; ++i) {
        QString lang = QLocale::languageToString(QLocale::Language(i));
        m_ui.cbLanguageList->addItem(lang, QVariant(int(i)));
    }
    m_ui.cbLanguageList->model()->sort(0, Qt::AscendingOrder);

    for (int i = QLocale::AnyCountry; i < QLocale::LastCountry; ++i) {
        QString country = QLocale::countryToString(QLocale::Country(i));
        m_ui.cbCountryList->addItem(country, QVariant(int(i)));
    }
    m_ui.cbCountryList->model()->sort(0, Qt::AscendingOrder);
    m_ui.cbCountryList->insertItem(0, tr("Any Country"), QVariant(0));

}

void TranslationSettingsDialog::setMessageModel(MessageModel *model)
{
    m_messageModel = model;
}

void TranslationSettingsDialog::on_buttonBox_accepted()
{
    int itemindex = m_ui.cbLanguageList->currentIndex();
    QVariant var = m_ui.cbLanguageList->itemData(itemindex);
    QLocale::Language lang = QLocale::Language(var.toInt());
    m_messageModel->setLanguage(lang);

    itemindex = m_ui.cbCountryList->currentIndex();
    var = m_ui.cbCountryList->itemData(itemindex);
    QLocale::Country country = QLocale::Country(var.toInt());
    m_messageModel->setCountry(country);
    accept();
}

void TranslationSettingsDialog::showEvent(QShowEvent *e)
{
    Q_UNUSED(e);
    QLocale::Language lang = m_messageModel->language();
    int itemindex = m_ui.cbLanguageList->findData(QVariant(int(lang)));
    m_ui.cbLanguageList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);


    QLocale::Country country = m_messageModel->country();
    itemindex = m_ui.cbCountryList->findData(QVariant(int(country)));
    m_ui.cbCountryList->setCurrentIndex(itemindex == -1 ? 0 : itemindex);
}

QT_END_NAMESPACE

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

#ifndef ADDRESSSELECTORWIDGET_P_H
#define ADDRESSSELECTORWIDGET_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QWidget>
#include <QModelIndex>
#include <qtopiaglobal.h>

class AddressListWidget;
class AddressFilterWidget;
class CheckableContactModel;
class QModelIndex;

class QTOPIAMAIL_EXPORT AddressSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    enum SelectionMode
    {
        PhoneSelection,
        EmailSelection,
        InstantMessageSelection
    };

    AddressSelectorWidget(SelectionMode mode = EmailSelection, QWidget* parent = 0);

    SelectionMode selectionMode() const;
    void setSelectionMode(SelectionMode m);
    QStringList selectedAddresses() const;
    void setSelectedAddresses(const QStringList& addressString);
    void reset();
    bool showingFilter() const;
    void setShowFilter(bool val);

private slots:
    void currentChanged(const QModelIndex& current, const QModelIndex&);
    void indexClicked(const QModelIndex& index);
    void filterTextChanged(const QString& filterText);

private:
    void init();
    void showEvent(QShowEvent *e);
    
    void updateLabel();

private:
    SelectionMode m_selectionMode;
    AddressListWidget* m_contactListView;
    CheckableContactModel* m_contactModel;
    QStringList m_explictAddresses;
    AddressFilterWidget* m_filterWidget;
    QModelIndex m_currentIndex;
};

#endif

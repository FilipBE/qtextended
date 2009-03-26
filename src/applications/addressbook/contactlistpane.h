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
#ifndef CONTACTLISTPANE_H
#define CONTACTLISTPANE_H

#include <QWidget>
#include "qcontact.h"
#include "qcontactmodel.h"
#include "qpimsource.h"
#include "quniqueid.h"
#include "qcategorymanager.h"

class QTextEntryProxy;
class QLabel;
class QContactModel;
class QVBoxLayout;
class QModelIndex;
class QAbstractItemDelegate;
class QSmoothContactListView;
class ContactsAlphabetRibbon;

class ContactListPane : public QWidget
{
    Q_OBJECT
    public:
        ContactListPane(QWidget *w, QContactModel* model);

        QContact currentContact() const;
        void setCurrentContact(const QContact& contact);
        void showCategory(const QCategoryFilter &f);
        void resetSearchText();

        QContactModel* contactModel() const { return mModel; }

        void setJumpField(QContactModel::Field f) {mJumpField = f; mJumpIndices.clear();}
    signals:
        void contactActivated( QContact c );
        void closeView();
        void currentChanged(const QModelIndex &, const QModelIndex &);

    public slots:
        void contactsChanged();

#ifdef QTOPIA_CELL
        void showLoadLabel(bool);
#endif

    protected:
        bool eventFilter( QObject *o, QEvent *e );
        void closeEvent( QCloseEvent *e );

    protected slots:
        void updateIcons();
        void search( const QString &k );
        void contactActivated(const QModelIndex &);
        void jump(int idx);

    protected:
        QSmoothContactListView *mListView;
        QTextEntryProxy *mTextProxy;
#ifdef QTOPIA_CELL
        QLabel *mLoadingLabel;
#endif
        QLabel *mCategoryLabel;
        QContactModel *mModel;
        QVBoxLayout *mLayout;
        QAbstractItemDelegate *mDelegate;
        QLabel *mFindIcon;
        ContactsAlphabetRibbon *mRibbon;
        QContactModel::Field mJumpField;
        QStringList mJumpTexts;
        QMap<QString, QModelIndex> mJumpIndices;
};

#endif

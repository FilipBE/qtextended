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

#include "groupview.h"
#include "addressbook.h"

#include <QContactModel>
#include <QContactListView>
#include <QCategoryManager>
#include <QTextEntryProxy>


#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtopiaApplication>
#include <QKeyEvent>

#include <QtopiaItemDelegate>

#if defined(QTOPIA_TELEPHONY)
#include "../../settings/profileedit/ringtoneeditor.h"
#endif

class GroupViewData
{
public:
    GroupViewData(GroupView *widget) : w(widget),
                   groupModel(0), categories(0),
                   view(0), mDefaultIcon(0), mAllowMultiple(false)
    {}

    ~GroupViewData() { delete mDefaultIcon;}

    GroupView *w;

    QStandardItemModel *groupModel;
    QCategoryManager *categories;

    QContactListView *view;
    QIcon *mDefaultIcon;
    QIcon *defaultIcon();

    bool mAllowMultiple;

    void initGroupModel();

    void repopulateCategories(QStringList checkedCategories);

    enum {
        CatIDRole = Qt::UserRole + 1
    };
};

void GroupViewData::initGroupModel()
{
    groupModel = new QStandardItemModel(w);
    categories = new QCategoryManager("Address Book", w);

    repopulateCategories(QStringList());
}

QIcon *GroupViewData::defaultIcon()
{
    if (mDefaultIcon == NULL)
        mDefaultIcon = new QIcon(":icon/contactgroup");
    return mDefaultIcon;
}

// checkedCategories is only used for multiple selection..
void GroupViewData::repopulateCategories(QStringList checkedCategories)
{
    // may be possible to just modify the model rather than repopulate it.
    // depends on if category ordering is stable and
    // the concept of steping through each list to determine what removals, then
    // inserts are required.
    QString lastId;
    if (view && view->currentIndex().isValid()) {
        lastId = groupModel->data(view->currentIndex(), CatIDRole).toString();
    }

    groupModel->clear();

    QList<QString> ids = categories->categoryIds();

    foreach(QString id, ids) {
        QStandardItem *item = new QStandardItem;
        QIcon icon = categories->icon(id);
        QString label = categories->label(id);
        if (icon.isNull())
            icon = *defaultIcon();
        item->setData(label, Qt::DisplayRole);
        item->setData(icon, Qt::DecorationRole); // Qt wants a QIcon, not a QPixmap
        item->setData(id, CatIDRole);

        if (mAllowMultiple) {
            item->setCheckable(true);
            if (checkedCategories.contains(id))
                item->setCheckState(Qt::Checked);
        }
        groupModel->appendRow(item);
    }

    if (!lastId.isEmpty()) {
        QModelIndexList list = groupModel->match(groupModel->index(0, 0), CatIDRole, lastId, 1, Qt::MatchExactly);
        if (list.count())
            view->setCurrentIndex(list[0]);
    }
}

GroupView::GroupView(bool allowMultiple, QWidget *parent)
    : QWidget(parent)
{
    d = new GroupViewData(this);
    d->mAllowMultiple = allowMultiple;
    d->view = new QContactListView;
    d->view->setFrameStyle(QFrame::NoFrame);
    QAbstractItemDelegate *del = new QtopiaItemDelegate(d->view);
    d->view->setItemDelegate(del);
    d->view->setSelectionMode(QListView::SingleSelection);
    d->view->setFocusPolicy(Qt::StrongFocus);
    d->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(d->view);

    d->initGroupModel();

    d->view->setModel(d->groupModel);

    setLayout(mainLayout);

    d->view->installEventFilter(this);

    connect(d->view, SIGNAL(activated(QModelIndex)),
            this, SLOT(activateIndex(QModelIndex)));
    connect(d->view->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(setCurrentIndex(QModelIndex,QModelIndex)));
    connect(d->categories, SIGNAL(categoriesChanged()), this, SLOT(updateGroups()));
}

GroupView::~GroupView()
{
    delete d;
}

void GroupView::setCurrentGroup(const QString &id)
{
    for (int row = 0; row < d->groupModel->rowCount(); row++) {
        QModelIndex index = d->groupModel->index(row, 0);
        if (d->groupModel->data(index, GroupViewData::CatIDRole).toString() == id) {
            d->view->setCurrentIndex(index);
            break;
        }
    }
}

QString GroupView::currentGroup() const
{
    QModelIndex index = d->view->currentIndex();
    if (!index.isValid())
        return QString();

    return d->groupModel->data(index, GroupViewData::CatIDRole).toString();
}

bool GroupView::isCurrentSystemGroup() const
{
    QModelIndex index = d->view->currentIndex();
    if (!index.isValid())
        return false;

    QString id = d->groupModel->data(index, GroupViewData::CatIDRole).toString();
    return d->categories->isSystem(id);
}

QStringList GroupView::selectedGroups() const
{
    QStringList ret;

    if (d->mAllowMultiple) {
        QModelIndexList mil = d->groupModel->match(d->groupModel->index(0,0), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly);
        foreach (QModelIndex mi, mil) {
            ret << d->groupModel->data(mi, GroupViewData::CatIDRole).toString();
        }
    } else {
        ret << currentGroup();
    }

    return ret;
}

void GroupView::setSelectedGroups(QStringList sl)
{
    for(int i=0; i < d->groupModel->rowCount(); i++) {
        QStandardItem * si = d->groupModel->item(i);
        if (si) {
            if (sl.contains(si->data(GroupViewData::CatIDRole).toString()))
                si->setCheckState(Qt::Checked);
            else
                si->setCheckState(Qt::Unchecked);
        }
    }

    // Make the first row current (but don't select it)
    d->view->setCurrentIndex(d->groupModel->index(0,0));
}

QModelIndex GroupView::currentIndex() const
{
    return d->view->currentIndex();
}

void GroupView::setCurrentIndex(const QModelIndex& idx)
{
    d->view->setCurrentIndex(idx);
}

QItemSelectionModel *GroupView::selectionModel() const
{
    return d->view->selectionModel();
}

void GroupView::addGroup()
{
    // show dialog for name, then
    GroupEdit edit;
    if(QtopiaApplication::execDialog(&edit) == QDialog::Accepted && !edit.name().isEmpty()) {
        QString name = edit.name();

        QString id = d->categories->idForLabel(name);
        if ( id.isEmpty() )
            id = d->categories->add(name);
        setCurrentGroup(id);
    }
}

void GroupView::removeCurrentGroup()
{
    QModelIndex index = d->view->currentIndex();
    if (!index.isValid())
        return;
    QString id = d->groupModel->data(index, GroupViewData::CatIDRole).toString();
    // TODO - check if we need a confirmation dialog
    d->categories->remove(id);
}

void GroupView::editCurrentGroup()
{
    QModelIndex index = d->view->currentIndex();
    if (!index.isValid())
        return;

    QString id = d->groupModel->data(index, GroupViewData::CatIDRole).toString();

    GroupMembers members;
    members.setGroup(id);
    members.setObjectName("view-category");

    // dialog already does the work of modifying contacts as already has the model
    QtopiaApplication::execDialog(&members);
}

#if defined(QTOPIA_TELEPHONY)
void GroupView::setGroupRingTone()
{
    // show dialog for name, then
    QString curId = d->groupModel->data(d->view->currentIndex(), GroupViewData::CatIDRole).toString();
    QContent curTone( d->categories->ringTone( curId ) );
    RingToneSelector selector;
    selector.setCurrentTone( curTone );
    if( QtopiaApplication::execDialog( &selector ) == QDialog::Accepted ) {
        d->categories->setRingTone( currentGroup(), selector.selectedRingTone() );
    }
}
#endif

void GroupView::renameCurrentGroup()
{
    QModelIndex index = d->view->currentIndex();
    if (!index.isValid())
        return;

    QString id = d->groupModel->data(index, GroupViewData::CatIDRole).toString();
    QString name = d->groupModel->data(index, Qt::DisplayRole).toString();

    // dialog
    GroupEdit edit;
    edit.setName(name);
    if(QtopiaApplication::execDialog(&edit) == QDialog::Accepted
            && name != edit.name() && !edit.name().isEmpty()) {
        d->categories->setLabel(id, edit.name());
    }

}

void GroupView::updateGroups()
{
    QStringList checked = selectedGroups();
    d->repopulateCategories(checked);
}

void GroupView::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
}

bool GroupView::eventFilter( QObject *o, QEvent *e )
{
    if(o == d->view && e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent *)e;
        if (ke->key() == Qt::Key_Back ) {
            emit closeView();
            return true;
        }
    }
    return false;
}

void GroupView::setCurrentIndex(const QModelIndex &current, const QModelIndex &)
{
    QString id;
    if (current.isValid())
        id = d->groupModel->data(current, GroupViewData::CatIDRole).toString();
    emit groupHighlighted(id);
}

void GroupView::activateIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    QString id = d->groupModel->data(index, GroupViewData::CatIDRole).toString();
    emit groupActivated(id);
}

class GroupMembersModel : public QContactModel
{
public:
    GroupMembersModel(QObject *parent = 0)
        : QContactModel(parent)
    {
    }

    ~GroupMembersModel()
    {
    }

    void setGroup(const QString &id) {
        mId = id;
        reset();
    }

    QString group() const { return mId; }

    void clearUpdate() {
        modifiedContacts.clear();
        reset();
    }

    QVariant data(const QModelIndex &index , int role) const;
    bool setData(const QModelIndex &, const QVariant &, int);

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return QContactModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    void applyUpdate();

private:
    QList<QUniqueId> modifiedContacts;
    QString mId;
};

QVariant GroupMembersModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole) {
        QUniqueId recid = id(index);
        QModelIndex catIndex = index.sibling(index.row(), QContactModel::Categories);
        QStringList categories = QContactModel::data(catIndex, Qt::EditRole).toStringList();

        if (modifiedContacts.contains(recid))
            return categories.contains(mId) ? Qt::Unchecked : Qt::Checked;
        else
            return categories.contains(mId) ? Qt::Checked : Qt::Unchecked;
    }
    return QContactModel::data(index, role);
}

bool GroupMembersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        bool check = (value.toInt() == Qt::Checked);

        QUniqueId recid = id(index);
        QModelIndex catIndex = index.sibling(index.row(), QContactModel::Categories);
        QStringList categories = QContactModel::data(catIndex, Qt::EditRole).toStringList();

        if (categories.contains(mId)) {
            if (check)
                modifiedContacts.removeAll(recid);
            else
                modifiedContacts.append(recid);
        } else {
            if (check)
                modifiedContacts.append(recid);
            else
                modifiedContacts.removeAll(recid);
        }
        return true;
    }
    return QContactModel::setData(index, value, role);
}

void GroupMembersModel::applyUpdate()
{
    // model based of what we already have
    foreach (QUniqueId recid, modifiedContacts) {
        // done this way to allow future optimization of the model so as
        // not to access unsused information.
        QModelIndex index = QContactModel::index(recid);
        index = index.sibling(index.row(), QContactModel::Categories);
        QStringList categories = QContactModel::data(index, Qt::EditRole).toStringList();

        if (categories.contains(mId))
            categories.removeAll(mId);
        else
            categories.append(mId);

        QContactModel::setData(index, categories, Qt::EditRole);
    }
    modifiedContacts.clear();
}

class GroupMembersData
{
public:
    GroupMembersData() : model(0), view(0), searchBar(0) {}

    GroupMembersModel *model;
    QContactListView *view;
    QTextEntryProxy *searchBar;
    QString mId;
};

GroupMembers::GroupMembers(QWidget *parent)
    : QDialog(parent)
{
    d = new GroupMembersData;
    d->model = new GroupMembersModel;
    d->view = new QContactListView;
    d->view->setUniformItemSizes(true);
    d->searchBar = new QTextEntryProxy(this, d->view);

    d->view->setModel(d->model);
    d->view->setItemDelegate(new QtopiaItemDelegate(d->view));

    QtopiaApplication::setInputMethodHint( d->view, QtopiaApplication::Text );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(d->view);

    int h = d->searchBar->sizeHint().height();
    QLabel *l = new QLabel;
    l->setPixmap(QIcon(":icon//view").pixmap(h-4, h-4));
    l->setMargin(2);

    QHBoxLayout *findLayout = new QHBoxLayout;
    findLayout->addWidget(l);
    findLayout->addWidget(d->searchBar);

    d->view->setFrameStyle(QFrame::NoFrame);
    d->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mainLayout->addLayout(findLayout);

    setLayout(mainLayout);

    connect( d->searchBar, SIGNAL(textChanged(QString)),
             this, SLOT(search(QString)) );
}

GroupMembers::~GroupMembers()
{}

void GroupMembers::search( const QString &text )
{
    if (text.isEmpty()) {
        d->model->clearFilter();
    } else {
        d->model->setFilter( text );
    }
}

void GroupMembers::setGroup(const QString &id)
{
    d->model->setGroup(id);
}

QString GroupMembers::group() const
{
    return d->model->group();
}

void GroupMembers::accept()
{
    d->model->applyUpdate();
    QDialog::accept();
}

void GroupMembers::reject()
{
    d->model->clearUpdate();
    QDialog::reject();
}

GroupEdit::GroupEdit(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Group Name:"));
    QVBoxLayout *layout = new QVBoxLayout;
    mName = new QLineEdit;
    layout->addWidget(mName);
    layout->addItem(new QSpacerItem( 1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(layout);
}


GroupEdit::~GroupEdit()
{
}

void GroupEdit::setName(const QString &name)
{
    mName->setText(name);
}

QString GroupEdit::name() const
{
    return mName->text();
}

#if defined(QTOPIA_TELEPHONY)
RingToneSelector::RingToneSelector(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Group Ringtone"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    mList = new RingToneSelect(this);
    mList->setAllowNone(true);
    layout->addWidget(mList);
    QtopiaApplication::setMenuLike(this, true);
    connect(mList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(accept()));
}

RingToneSelector::~RingToneSelector()
{
}

void RingToneSelector::setCurrentTone( const QContent &tone )
{
    mList->setCurrentTone( tone );
}

QString RingToneSelector::selectedRingTone() const
{
    return mList->currentTone().fileName();
}
#endif

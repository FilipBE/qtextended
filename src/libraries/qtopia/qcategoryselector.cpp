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

#include <qcategoryselector.h>
#include <qcategorymanager.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>

#include <QDir>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QLayout>
#include <QToolButton>
#include <QFile>
#include <QPushButton>
#include <QObject>
#include <QTimer>
#include <QCheckBox>
#include <QListView>
#include <QDesktopWidget>
#include <QAbstractListModel>
#include <QtopiaItemDelegate>
#include <QComboBox>
#include <QListView>
#include <QVBoxLayout>
#include <QFrame>
#include <QKeyEvent>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>

#include <stdlib.h>

class QCategoryEditorData;
class QCategoryEditor : public QDialog
{
    Q_OBJECT

public:
    explicit QCategoryEditor(QWidget *parent);
    ~QCategoryEditor();

    QString name() const;
    bool global() const;

    void setName(const QString &name);
    void setGlobal(bool global);

    void setCategoryId(const QString &id);
    QString categoryId() const;

public slots:
    void accept();

private:
    QCategoryEditorData *d;
};

class QCategoryEditorData
{
public:
    QCategoryEditorData() {}
    ~QCategoryEditorData() {}

    QLineEdit *nameField;
    QCheckBox *globalCheck;
    QString categoryId;
};

/*
  \class QCategoryEditor
    \inpublicgroup QtBaseModule
  \internal
  \ingroup categories
  \brief The QCategoryEditor class allows the user to change a user category.

  This class aids in the editing of user categories but it does not have intimate knowledge of
  the category system. It exposes things the user can edit (name, global) but does not handle things
  the user cannot edit (scope, icon).

  \code
    QCategoryEditor editor;
    editor.setName( categoryName );
    editor.setGlobal( categoryIsGlobal );
    if ( editor.exec() ) {
        categoryName = editor.name();
        categoryIsGlobal = editor.global();
        // update QCategoryManager
    }
  \endcode

  \sa Categories
*/

/*
  Constructs a QCategoryEditor with the given \a parent.
*/
QCategoryEditor::QCategoryEditor(QWidget *parent)
: QDialog(parent)
{
    d = new QCategoryEditorData();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(5);

    d->nameField = new QLineEdit();
    layout->addWidget(d->nameField);

    d->globalCheck = new QCheckBox(tr("Global"));
    layout->addWidget(d->globalCheck);

    setWindowModality(Qt::WindowModal);
}

/*
  Destroys a QCategoryEditor.
*/
QCategoryEditor::~QCategoryEditor()
{
}

/*
  \internal
  The dialog will be accepted only if the category has some text.
*/
void QCategoryEditor::accept()
{
    if(name().isEmpty())
        reject();
    else
        QDialog::accept();
}

/*
  Returns the name of the category being edited.
  \sa setName()
*/
QString QCategoryEditor::name() const
{
    return d->nameField->text().simplified();
}

/*
  Returns true if the category being edited exists in the global scope; false otherwise.
  \sa setGlobal()
*/
bool QCategoryEditor::global() const
{
    return d->globalCheck->isChecked();
}

/*
  Sets the \a name of the category being edited.
  \sa name()
*/
void QCategoryEditor::setName(const QString &name)
{
    d->nameField->setText(name.simplified());
}

/*
  Sets the category being edited to the global scope if \a global is true.
  \sa global()
*/
void QCategoryEditor::setGlobal(bool global)
{
    d->globalCheck->setChecked(global);
}

/*
    Sets the \a id of the category being edited.

    For new categories the id will be an empty string.
*/
void QCategoryEditor::setCategoryId(const QString &id)
{
    d->categoryId = id;
}

/*
    Return the id of the category being edited.

    For new categories the id will be an empty string.
*/
QString QCategoryEditor::categoryId() const
{
    return d->categoryId;
}

// For combo box, filters or sets..
class CategoryItem
{
public:
    enum CategoryItemType
    {
        Single,
        All,
        Unfiled,
        Ellipse
    };

    CategoryItem(CategoryItemType e)
        : t(e), isChecked(false) {}
    CategoryItem(const QString &s)
        : t(Single), id(s), isChecked(false) {}

    CategoryItem &operator=(const CategoryItem &o)
    {
        t = o.t;
        id = o.id;
        isChecked = o.isChecked;
        return *this;
    }

    CategoryItemType t;
    QString id;
    bool isChecked;
};

class CategoryItemList : public QAbstractListModel
{
    Q_OBJECT
public:
    CategoryItemList(QObject *p, QCategoryManager *m, bool includeUnfiled, bool includeAll,
        bool includeEllipse)
        : QAbstractListModel(p), allowChecks(true), singleSelection(false), mCats(m)
    {
        generateList(includeUnfiled, includeAll, includeEllipse);
        showEllipse = includeEllipse;
        showUnfiled = includeUnfiled;
        showAll = includeAll;

        connect(m, SIGNAL(categoriesChanged()), this, SLOT(categoriesChanged()));
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QString itemText(int pos) const;
    QString itemId(int pos) const;
    int itemPos(const QString& id) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool isAll(int pos);
    bool isUnfiled(int pos);
    bool isChecked(int pos);
    void toggleItem(int pos);
    bool isEllipse(int pos);
    bool isEditable(int pos) const;
    bool isGlobal(int pos) const;

    void setAllowChecks(bool b);
    void setSingleSelection(bool b);

    bool isValidIndex(int pos);
    int allPos();
    int unfiledPos();
    int ellipsePos();
    void generateList(bool includeUnfiled, bool includeAll, bool includeEllipse);
    void setUnfiled();
public slots:
    void categoriesChanged();

private:
    void setCheck(int pos, bool check); // Private because it doesn't check that what
                                        // it is checking is valid.

    bool showEllipse;
    bool showAll;
    bool showUnfiled;
    bool allowChecks;
    bool singleSelection;
    QList<CategoryItem> list;
    QCategoryManager *mCats;
};

void CategoryItemList::generateList(bool includeUnfiled, bool includeAll, bool includeEllipse)
{
    QStringList selections;

    for(int i = 0; i < list.count(); i++)
        if(list[i].isChecked)
            selections.append(list[i].id);

    list.clear();

    if ( includeAll ) {
        // if there is an all item, it is a reasonable default
        CategoryItem i(CategoryItem::All);
        i.isChecked = selections.isEmpty();
        list.append(i);
    }
    if ( includeUnfiled ) {
        // if there is an unfiled, but no all item, unfiled
        // is a reasonable default
        CategoryItem i(CategoryItem::Unfiled);
        i.isChecked = !includeAll && selections.isEmpty();
        list.append(i);
    }

    QList<QString> ids =  mCats->categoryIds();
    foreach(QString id, ids) {
#ifdef QTOPIA_HIDE_SYSTEM_CATEGORIES
        if ( !QCategoryManager().isSystem(id) )
#endif
            list.append(id);
    }

    if ( includeEllipse )
        list.append(CategoryItem(CategoryItem::Ellipse));

    for(int i = 0; i < selections.count(); i++)
    {
        for(int j = 0; j < list.count(); j++)
        {
            if(list[j].id == selections[i])
            {
                list[j].isChecked = true;
                break;
            }
        }
    }
}

void CategoryItemList::categoriesChanged()
{
    generateList(showUnfiled, showAll, showEllipse);
    reset();
}

int CategoryItemList::allPos()
{
    for( int i = 0; i < rowCount(); ++i )
    {
        if( isAll( i ) )
            return i;
    }

    return -1;
}

int CategoryItemList::ellipsePos()
{
    for( int i = 0; i < rowCount(); ++i )
    {
        if( isEllipse( i ) )
            return i;
    }

    return -1;
}

int CategoryItemList::unfiledPos()
{
    for( int i = 0; i < rowCount(); ++i )
    {
        if( isUnfiled( i ) )
            return i;
    }

    return -1;
}

int CategoryItemList::itemPos(const QString& id) const
{
    for( int i = 0; i < rowCount(); ++i )
    {
        if (list[i].t == CategoryItem::Single && list[i].id == id)
            return i;
    }

    return -1;
}

void CategoryItemList::setAllowChecks(bool b)
{
    allowChecks = b;
}

void CategoryItemList::setSingleSelection(bool b)
{
    singleSelection = b;
}

int CategoryItemList::rowCount(const QModelIndex &) const
{
    return list.count();
}

bool CategoryItemList::isAll(int pos)
{
    return list[pos].t == CategoryItem::All;
}

bool CategoryItemList::isUnfiled(int pos)
{
    return list[pos].t == CategoryItem::Unfiled;
}

bool CategoryItemList::isEditable(int pos) const
{
    return list[pos].t == CategoryItem::Single && !QCategoryManager().isSystem(list[pos].id);
}

bool CategoryItemList::isGlobal(int pos) const
{
    return (list[pos].t == CategoryItem::Single) &&
        mCats->isGlobal(list[pos].id);
}

QString CategoryItemList::itemText(int pos) const
{
    if ( pos >= list.count() || pos < 0 )
        pos = 0;
    switch(list[pos].t)
    {
        case CategoryItem::Single:
            return mCats->label(list[pos].id);

        case CategoryItem::All:
            return QCategoryManager::allLabel();

        case CategoryItem::Unfiled:
            return QCategoryManager::unfiledLabel();

        case CategoryItem::Ellipse:
            return QCategoryManager::multiLabel();
    }

    return QString();
}

QString CategoryItemList::itemId(int pos) const
{
    if ( pos >= list.count() || pos < 0 )
        pos = 0;
    if (list[pos].t == CategoryItem::Single)
        return list[pos].id;

    return QString();
}

QVariant CategoryItemList::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return itemText(index.row());
    else if(role == Qt::CheckStateRole && allowChecks)
    {
        int pos = index.row();

        switch(list[pos].t)
        {
            case CategoryItem::Ellipse:
                return QVariant();
            case CategoryItem::Single:
            case CategoryItem::All:
            case CategoryItem::Unfiled:
                return list[pos].isChecked ? Qt::Checked : Qt::Unchecked;
        }
    } else if (role == Qt::DecorationRole) {
        if (isGlobal(index.row()))
            return QIcon(":icon/globe");
    }
    return QVariant();
}

bool CategoryItemList::isChecked(int pos)
{
    return list[pos].isChecked;
}

void CategoryItemList::toggleItem(int pos)
{
    switch(list[pos].t) {
        case CategoryItem::Ellipse:
            break;
        case CategoryItem::Single:
            if (!singleSelection) {
                if (list[pos].isChecked) {
                    setCheck(pos, false);
                    // check if now is unfiled.
                    if (showAll || showUnfiled) {
                        bool allUnchecked = true;
                        for(int i = 0; i < list.count(); ++i) {
                            if (list[i].t == CategoryItem::Single && list[i].isChecked)
                                allUnchecked = false;
                        }
                        CategoryItem::CategoryItemType type
                            = showAll
                            ?  CategoryItem::All
                            : CategoryItem::Unfiled;
                        if (allUnchecked)
                            for(int i = 0; i < list.count(); ++i)
                                if (list[i].t == type)
                                    setCheck(i, true);
                    }
                } else {
                    setCheck(pos, true);
                    for(int i = 0; i < list.count(); ++i)
                        if (list[i].t != CategoryItem::Single)
                            setCheck(i, i == pos);
                }
                break;
            }
            // else fallthrough
        case CategoryItem::All:
        case CategoryItem::Unfiled:
            for(int i = 0; i < list.count(); ++i)
                setCheck(i, i == pos);
            break;
    }
}

void CategoryItemList::setCheck(int pos, bool check)
{
    if(isChecked(pos) != check)
    {
        list[pos].isChecked = check;
        dataChanged(createIndex(pos, 0), createIndex(pos, 0));
    }
}

void CategoryItemList::setUnfiled()
{
    int pos = unfiledPos();
    if (pos != -1) {
        if (!list[pos].isChecked)
            toggleItem(pos);
    } else {
        // no unfiled.  uncheck all.
        for (int i = 0; i < list.count(); ++i)
            setCheck(i, false);
    }
}

bool CategoryItemList::isEllipse(int pos)
{
    return (list[pos].t == CategoryItem::Ellipse);
}

bool CategoryItemList::isValidIndex(int pos)
{
    return pos >= 0 && pos < list.count();
}

class QCategoryListView : public QListView
{
    Q_OBJECT
signals:
    void rowChanged(int row);

protected:
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &)
    {
        scrollTo(current);
        emit rowChanged(current.row());
    }
};

class QCategoryListDelegate : public QtopiaItemDelegate
{
public:
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem opt(option);
        opt.decorationPosition = QStyleOptionViewItem::Right;
        QtopiaItemDelegate::paint(painter, opt, index);
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QVariant value = index.data(Qt::SizeHintRole);
        if (value.isValid())
            return qvariant_cast<QSize>(value);
        QRect decorationRect = QRect(QPoint(0, 0), option.decorationSize);
        QRect displayRect = rect(option, index, Qt::DisplayRole);
        QRect checkRect = rect(option, index, Qt::CheckStateRole);
        QRect addDecorationRect = rect(option, index, Qtopia::AdditionalDecorationRole);

        doLayout(option, &checkRect, &decorationRect, &displayRect, &addDecorationRect, true);

        return (decorationRect|displayRect|checkRect|addDecorationRect).size();
    }
};

class QCategorySelectData : public QObject
{
    Q_OBJECT
public:
    enum SelectorWidget {
        Unspecified = 0,
        ComboBox,
        ListView,
        Dialog
    };

    QCategorySelectData(const QString &s, QCategorySelector::ContentFlags f, QCategorySelector *parent);

    void setType();
    QCategoryFilter::FilterType selectedFilterType() const;


    QCategoryManager *cats;
    CategoryItemList *model;
    QList<QString> comboMultiSelection;
    int comboPos;

    SelectorWidget type;

    QCategoryListView *listView;
    QComboBox *comboBox;
    QToolButton *dialogButton;

    QCategorySelector::ContentFlags flags;

    QCategoryFilter selectedCategories;

    QString scope;

    bool manuallyUpdating;
    bool allOptionPresent;

    QAction *newCatAction;
    QAction *editCatAction;
    QAction *deleteCatAction;

    QString idToSelect;
    int rowToSelect;

    QCategoryDialog *selectorDialog;
    QCategoryEditor *editorDialog;

public slots:
    void rowChanged(int row);
    void newCategory();
    void editCategory();
    void deleteCategory();
    void categoriesChanged();
    void selectUnfiled();
    void editorAccepted();

signals:
    void categoriesSelected(const QList<QString> &);
    void filterSelected(const QCategoryFilter &);
};

QCategorySelectData::QCategorySelectData( const QString &s,
        QCategorySelector::ContentFlags f, QCategorySelector *parent )
    : QObject(parent), model(0), listView(0), comboBox(0), dialogButton(0), flags(f)
    , selectorDialog(0), editorDialog(0)
{
    manuallyUpdating = false;
    scope = s;

    comboPos = -1;
    rowToSelect = -1;

    //
    //  Determine the type of widget used
    //

    if ( ((f & QCategorySelector::ViewMask) == QCategorySelector::ListView) )
        type = ListView;
    else if ( ((f & QCategorySelector::ViewMask) == QCategorySelector::ComboView) )
        type = ComboBox;
    else if ( ((f & QCategorySelector::ViewMask) == QCategorySelector::DialogView) )
        type = Dialog;
    else
        setType();

    //
    //  Prepare a list of categories to include
    //

    cats = new QCategoryManager(s, this);
    allOptionPresent = ((f & QCategorySelector::IncludeAll) != 0);

    bool showEllipse = !Qtopia::mousePreferred() && type == ComboBox && (f & QCategorySelector::SingleSelection) != 0;

    bool showUnfiled;
    if (type == ListView)
        showUnfiled = (f & (QCategorySelector::IncludeUnfiled | QCategorySelector::IncludeAll)) == (QCategorySelector::IncludeUnfiled | QCategorySelector::IncludeAll);
    else
        showUnfiled = ((f & QCategorySelector::IncludeUnfiled) != 0);

    //
    //  Create the widget (view), and a model to use with it.
    //

    model = new CategoryItemList(this, cats, showUnfiled, allOptionPresent, showEllipse);
    model->setAllowChecks(type == ListView);
    model->setSingleSelection((f & QCategorySelector::SingleSelection) != 0);

    if( type == ListView ) {
        QVBoxLayout *vb = new QVBoxLayout(parent);
        vb->setMargin( 0 );
        vb->setSpacing( 0 );

        listView = new QCategoryListView();
        listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        listView->setItemDelegate(new QCategoryListDelegate());
        listView->setModel(model);
        listView->setSelectionMode(QListView::SingleSelection);
        listView->selectionModel()->select(
                model->index(0, 0), QItemSelectionModel::Select);
        listView->setUniformItemSizes(true);
        vb->addWidget(listView);

        connect(listView, SIGNAL(activated(QModelIndex)),
            parent, SLOT(listActivated(QModelIndex)));

        listView->installEventFilter(parent);

        QMenu *contextMenu = QSoftMenuBar::menuFor(parent);

        newCatAction = new QAction(QIcon(":icon/new"), tr("New Category"), this);
        connect(newCatAction, SIGNAL(triggered()), this, SLOT(newCategory()));
        newCatAction->setWhatsThis(tr("Create a new category."));
        contextMenu->addAction(newCatAction);

        editCatAction = new QAction(QIcon(":icon/edit"), tr("Edit Category"), this);
        connect(editCatAction, SIGNAL(triggered()), this, SLOT(editCategory()));
        editCatAction->setWhatsThis(tr("Edit the highlighted category."));
        editCatAction->setVisible(false);
        contextMenu->addAction(editCatAction);

        QAction *unfiledAction = new QAction(QIcon(), tr("Uncheck all"), this);
        connect(unfiledAction, SIGNAL(triggered()), this, SLOT(selectUnfiled()));

        unfiledAction->setWhatsThis(tr("Uncheck all selected categories."));
        contextMenu->addAction(unfiledAction);


        deleteCatAction = new QAction(QIcon(":icon/trash"), tr("Delete Category"), this);
        connect(deleteCatAction, SIGNAL(triggered()), this, SLOT(deleteCategory()));
        deleteCatAction->setWhatsThis(tr("Delete the highlighted category."));
        deleteCatAction->setVisible(false);
        contextMenu->addAction(deleteCatAction);

        connect(listView, SIGNAL(rowChanged(int)), this, SLOT(rowChanged(int)));

        QSoftMenuBar::setLabel(listView, Qt::Key_Back, QSoftMenuBar::Back);
    } else if( type == ComboBox ) {
        QHBoxLayout *hb = new QHBoxLayout(parent);
        hb->setMargin(0);
        hb->setSpacing(0);

        comboBox = new QComboBox();
        int sz = comboBox->style()->pixelMetric(QStyle::PM_SmallIconSize);
        comboBox->setIconSize(QSize(sz,sz));
        comboBox->setItemDelegate(new QCategoryListDelegate());
        comboBox->setModel(model);
        comboBox->setMinimumSize( 1, comboBox->minimumHeight() );
        hb->addWidget(comboBox);

        connect(comboBox, SIGNAL(activated(int)), parent, SLOT(comboSelection(int)));

        if( Qtopia::mousePreferred() )
        {
            QToolButton *eb = new QToolButton(parent);
            hb->addWidget(eb);
            eb->setText( "..." );
            eb->setFocusPolicy( Qt::TabFocus );
            eb->setFixedHeight( eb->sizeHint().height() );
            eb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            connect(eb, SIGNAL(clicked()), parent, SLOT(showDialog()));
        } else {
            parent->setFocusPolicy(Qt::TabFocus);
            parent->setFocusProxy(comboBox);
        }
    } else {
        QVBoxLayout *layout = new QVBoxLayout( parent );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        dialogButton = new  QToolButton(parent);
        dialogButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        layout->addWidget(dialogButton);
        parent->setFocusPolicy(Qt::TabFocus);
        parent->setFocusProxy(dialogButton);
        connect(dialogButton, SIGNAL(clicked()), parent, SLOT(showDialog()));
    }

   connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(categoriesChanged()));
}

void QCategorySelectData::rowChanged(int row)
{
    bool enableEdit = model->isEditable(row);

    editCatAction->setVisible(enableEdit);
    deleteCatAction->setVisible(enableEdit);
    idToSelect = model->itemId(row);
    rowToSelect = -1;
}

void QCategorySelectData::categoriesChanged()
{
    if ((!idToSelect.isEmpty() || rowToSelect != -1) && listView) {
        if (rowToSelect == -1)
            rowToSelect = model->itemPos(idToSelect);
        if (rowToSelect == -1)
            rowToSelect = 0;
        QModelIndex idx = model->index(rowToSelect, 0);
        if (idx.isValid())
            listView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent);
        idToSelect.clear();
        rowToSelect = -1;
    }
}

void QCategorySelectData::newCategory()
{
    if (!editorDialog) {
        editorDialog = new QCategoryEditor(static_cast<QWidget *>(parent()));

        connect(editorDialog, SIGNAL(accepted()), this, SLOT(editorAccepted()));
        connect(editorDialog, SIGNAL(finished(int)), listView, SLOT(setFocus()));
    }

    editorDialog->setWindowTitle(tr("New Category"));
    editorDialog->setCategoryId(QString());

    QtopiaApplication::showDialog(editorDialog);
}

void QCategorySelectData::editCategory()
{
    int currentRow = listView->currentIndex().row();

    if (model->isEditable(currentRow)) {
        if (!editorDialog) {
            editorDialog = new QCategoryEditor(static_cast<QWidget *>(parent()));

            connect(editorDialog, SIGNAL(accepted()), this, SLOT(editorAccepted()));
            connect(editorDialog, SIGNAL(finished(int)), listView, SLOT(setFocus()));
        }

        editorDialog->setWindowTitle(tr("Edit Category"));
        editorDialog->setCategoryId(model->itemId(currentRow));
        editorDialog->setName(model->itemText(currentRow));
        editorDialog->setGlobal(model->isGlobal(currentRow));

        QtopiaApplication::showDialog(editorDialog);
    }
}

void QCategorySelectData::editorAccepted()
{
    QString categoryId = editorDialog->categoryId();
    QString name = editorDialog->name();
    bool isGlobal = editorDialog->global();

    if (categoryId.isNull()) {
        if(!cats->containsLabel(name, isGlobal)) {
            // Add it and store the id for selecting later
            idToSelect = cats->add(name, QString(), isGlobal);
            rowToSelect = -1;
        }
    } else {
        if (cats->label(categoryId) != name || cats->isGlobal(categoryId) != isGlobal) {
            cats->setLabel(categoryId, name);
            cats->setGlobal(categoryId, isGlobal);
            idToSelect = categoryId;
            rowToSelect = -1;
        }
    }
}

void QCategorySelectData::deleteCategory()
{
    int currentRow = listView->currentIndex().row();

    if(model->isEditable(currentRow))
    {
        if(QMessageBox::question(listView, tr("Delete Category"),
            tr("<qt>Deleting '%1' change all things in this category to 'unfiled'. Are you sure?</qt>")
            .arg(model->itemText(currentRow)), QMessageBox::Yes, QMessageBox::No)
            == QMessageBox::Yes)
        {
            // Select the next category, if any
            // otherwise the first
            rowToSelect = -1;
            idToSelect = model->itemId(currentRow + 1);
            if (idToSelect.isEmpty())
                rowToSelect = 0; // select the first row

            cats->remove(model->itemId(currentRow));
        }
        listView->setFocus();
    }
}

void QCategorySelectData::selectUnfiled()
{
    int pos = model->unfiledPos();

    if( pos > -1 )
    {
        if( comboBox ) {
            comboBox->setCurrentIndex( pos );
            comboPos = pos;
        } else if( listView ) {
            listView->setCurrentIndex( model->index( pos, 0 ) );
            model->setUnfiled();
        } else if( dialogButton ) {
            selectedCategories = QCategoryFilter( QCategoryFilter::Unfiled );
            dialogButton->setText( QCategoryManager::unfiledLabel() );
        }
    } else {
        if (listView)
            model->setUnfiled();
    }
}

void QCategorySelectData::setType()
{
    type = ComboBox;
}

QCategoryFilter::FilterType QCategorySelectData::selectedFilterType() const
{
    if(comboBox)
    {
        CategoryItemList* list = (CategoryItemList*)comboBox->model();

        if(comboPos > -1)
        {
            if(list->isAll(comboPos))
                return QCategoryFilter::All;
            else if(list->isUnfiled(comboPos))
                return QCategoryFilter::Unfiled;
            else
                return QCategoryFilter::List;
        }
    }
    else if(listView)
    {
        CategoryItemList* list = (CategoryItemList*)listView->model();

        bool somethingChecked = false;

        for(int i = 0; i < list->rowCount(); i++)
        {
            if(list->isChecked(i))
            {
                somethingChecked = true;
                if(list->isAll(i))
                    return QCategoryFilter::All;
                else if(list->isUnfiled(i))
                    return QCategoryFilter::Unfiled;
            }
        }

        if(somethingChecked)
            return QCategoryFilter::List;
    }
    else if(dialogButton)
    {
        if( selectedCategories.acceptAll() )
            return QCategoryFilter::All;
        else if( selectedCategories.acceptUnfiledOnly() )
            return QCategoryFilter::Unfiled;
        else
            return QCategoryFilter::List;
    }

    if(allOptionPresent)
        return QCategoryFilter::All;
    else
        return QCategoryFilter::Unfiled;
}

/*!
  \class QCategorySelector
    \inpublicgroup QtBaseModule

  \ingroup categories
  \brief The QCategorySelector widget allows users to select categories for
  filtering or for applying to an item.

  QCategorySelector is a widget that shows categories the user can choose from.
  Its appearance and behavior are determined by flags given to the constructor.
  Control over the categories that are visible is achieved via the select...() functions.

  QCategorySelector automatically updates when the system's categories are changed. It provides an
  interface for the user to add/edit/delete user categories.

  QCategorySelector is typically used as a \l QCategorySelector::Filter or \l QCategorySelector::Editor;
  
  Here's an example of using it as a filter.

  \code
    MyWidget::MyWidget()
    {
        QCategorySelector *sel = new QCategorySelector( "myappscope", QCategorySelector::Filter );
        connect( sel, SIGNAL(filterSelected(QCategoryFilter)),
                 this, SLOT(filterCategories(QCategoryFilter)) );
        QSettings settings("mycompany", "myapp");
        QCategoryFilter f;
        f.readConfig( settings, "filter" );
        sel->selectFilter( f );
    }

    void MyWidget::filterCategories( const QCategoryFilter &filter )
    {
        QSettings settings("mycompany", "myapp");
        filter.writeConfig( settings, "filter" );
        foreach ( item, itemList )
            item->setVisible( filter.accepted( item->categories() ) );
    }
  \endcode

  Here's an example of using QCategorySelector an editor.

  \code
    MyWidget::MyWidget( MyRecord *item )
    {
        QCategorySelector *sel = new QCategorySelector( "myappscope", QCategorySelector::Editor );
        sel->selectCategories( item->categories() );
        connect( sel, SIGNAL(categoriesSelected(QList<QString>)),
                 item, SLOT(setCategories(QList<QString>)) );
    }
  \endcode

  \sa Categories
*/

/*!
  \fn void QCategorySelector::categoriesSelected(const QList<QString> &list)
  This signal is emitted when the selected categories changes. The selected categories are
  indicated by \a list.

  Note that this function does not indicate if \c All or \c Unfiled is selected. Use
  filterSelected() to determine if these items are selected.
*/

/*!
  \fn void QCategorySelector::filterSelected(const QCategoryFilter &filter)

  This signal is emitted when the selected categories changes. The selected categories are
  indicated by \a filter.

  \sa QCategoryFilter::requiredCategories()
*/

/*!
  \fn void QCategorySelector::listActivated(int pos)
  \internal

  This signal is emitted when the item at position \a pos
  in the category list is clicked.
*/

/*!
  \enum QCategorySelector::ContentFlag

  \value IncludeAll Include the All categories option.
  \value IncludeUnfiled Include the Unfiled categories option.
  \value SingleSelection Allow selection of a single category. The default is to allow combinations of categories.
  \value ListView Display as a list view.
  \value ComboView Display as a combo box. Note that this will encourage exclusive selection of categories.
  \value DialogView Display as a button that opens a dialog containing a list view.
  \value Filter Alias for IncludeAll|IncludeUnfiled.
  \value Editor Alias for IncludeUnfiled.
  \value ViewMask Masks the view selection flags.

  The list view looks like this.
  \image qcategoryselector-list.png

  The combo box looks like this.
  \image qcategoryselector-combo-down.png
  \image qcategoryselector-combo-up.png

  The button looks like this.
  \image qcategoryselector-button.png
*/

/*!
  Constructs a QCategorySelector as an editor widget with the given \a parent.
*/
QCategorySelector::QCategorySelector( QWidget *parent )
    : QWidget( parent )
{
    d = new QCategorySelectData(QString(), Editor, this);
}

/*!
  Constructs a QCategorySelector with the given \a parent.
  Unless it is null, \a scope limits the categories that can be seen.
  The flags in \a f determine the apperance and behavior of the QCategorySelector.
*/
QCategorySelector::QCategorySelector( const QString &scope, ContentFlags f, QWidget *parent )
    : QWidget( parent )
{
    d = new QCategorySelectData(scope, f, this);
}

/*!
  \internal
  The combo box selection has changed.
*/
void QCategorySelector::comboSelection(int index)
{
    if(d->model->isValidIndex(index))
    {
        if(d->model->isEllipse(index))
            showDialog();
        else
        {
            d->comboPos = index;
            emit categoriesSelected(selectedCategories());
            emit filterSelected(selectedFilter());
        }

        d->comboBox->hidePopup();
    }
}

/*!
  \internal
  An item on the list view was toggled.
*/
void QCategorySelector::listActivated(const QModelIndex &idx)
{
    if(d->model->isValidIndex(idx.row()))
    {
        d->model->toggleItem(idx.row());
        emit categoriesSelected(selectedCategories());
        emit filterSelected(selectedFilter());
    }
}

/*!
  \internal
  Sets the frame style for the listview of the category selector
  to \a style.
  Note that this only works if the QCategorySelector is displayed as a list view.
*/
void QCategorySelector::setListFrameStyle(int style)
{
    if(d->listView)
        d->listView->setFrameStyle(style);
}

/*!
  Destroys a QCategorySelector.
*/
QCategorySelector::~QCategorySelector()
{
}

/*!
  Selects the categories with identifiers in the comma-separated list \a categoryids.
*/
void QCategorySelector::selectCategories(const QString &categoryids)
{
    selectCategories(categoryids.split(QChar(','), QString::SkipEmptyParts));
}

/*!
  Selects the categories with identifiers in list \a categoryids.
*/
void QCategorySelector::selectCategories(const QStringList &categoryids)
{
    QStringList appliedCategories;
    if (categoryids.count() == 0) {
        selectUnfiled();
        emit categoriesSelected(selectedCategories());
        return;
    } else if ((d->flags & SingleSelection) != 0)
        appliedCategories.append(categoryids[0]);
    else
        appliedCategories = categoryids;

    bool wasManuallyUpdating = d->manuallyUpdating;
    d->manuallyUpdating = true;
    if(d->listView)
    {
        CategoryItemList* list = (CategoryItemList*)d->listView->model();

        for(int i = 0; i < list->rowCount(); i++)
            if((appliedCategories.indexOf(list->itemId(i)) > -1) != list->isChecked(i))
                list->toggleItem(i);
    }
    else if(d->comboBox)
    {
        CategoryItemList* list = (CategoryItemList*)d->comboBox->model();

        if (appliedCategories.count() > 1) {
            // set to ellipse position.
            d->comboBox->setCurrentIndex(d->comboPos = list->ellipsePos());
            d->comboMultiSelection = appliedCategories;
        } else if (appliedCategories.count() == 1) {
            QString cat = appliedCategories.first();
            for (int i = 0; i < list->rowCount(); i++) {
                if (cat == list->itemId(i)) {
                    d->comboBox->setCurrentIndex(d->comboPos = i);
                    break;
                }
            }
        } else {
            d->comboBox->setCurrentIndex(d->comboPos = -1);
        }

    }
    if( d->dialogButton )
    {
        d->selectedCategories = QCategoryFilter( appliedCategories );;
        if( appliedCategories.count() > 1 )
            d->dialogButton->setText( QCategoryManager::multiLabel() );
        else
            d->dialogButton->setText( d->cats->label( appliedCategories.first() ) );
    }

    emit categoriesSelected(selectedCategories());
    emit filterSelected(selectedFilter());

    d->manuallyUpdating = wasManuallyUpdating;
}

/*!
  Selects the categories specified by \a filter.
*/
void QCategorySelector::selectFilter(const QCategoryFilter &filter)
{
    if (filter.acceptAll())
        selectAll();
    else if (filter.acceptUnfiledOnly())
        selectUnfiled();
    else
        selectCategories(filter.requiredCategories());
}

/*!
  Returns the list of selected categories.

  Note that this function does not indicate if \c All or \c Unfiled is selected. Use
  selectedFilter() to determine if these items are selected.
*/
QStringList QCategorySelector::selectedCategories() const
{
    //
    //  Deal with All / Unfiled responses
    //

    switch(d->selectedFilterType())
    {
        case QCategoryFilter::All:
            return d->cats->categoryIds();

        case QCategoryFilter::Unfiled:
            return QStringList();

        case QCategoryFilter::List:
            break;
    }

    //
    //  Deal with List responses.
    //

    if(d->comboBox)
    {
        CategoryItemList* list = (CategoryItemList*)d->comboBox->model();

        if(d->comboPos == -1)
            return QStringList();
        else if (list->isEllipse(d->comboPos))
            return d->comboMultiSelection;
        else
            return QStringList(list->itemId(d->comboPos));
    }
    else if(d->listView)
    {
        CategoryItemList* list = (CategoryItemList*)d->listView->model();

        QStringList cats;

        for(int i = 0; i < list->rowCount(); i++)
            if(list->isChecked(i))
                cats.append(list->itemId(i));

        return cats;
    }
    else if(d->dialogButton)
    {
        return d->selectedCategories.requiredCategories();
    }

    return QStringList();
}

/*!
  Returns the QCategoryFilter that indicates the selected categories.
  \sa QCategoryFilter::requiredCategories()
*/
QCategoryFilter QCategorySelector::selectedFilter() const
{
    QCategoryFilter::FilterType type = d->selectedFilterType();

    if(type == QCategoryFilter::All || type == QCategoryFilter::Unfiled)
        return QCategoryFilter(type);
    else
        return QCategoryFilter(selectedCategories());
}

/*!
  \reimp
*/
QSize QCategorySelector::sizeHint () const
{
    return QWidget::sizeHint();
}

/*!
  Selects the All entry (all categories are selected).
*/
void QCategorySelector::selectAll()
{
    int pos = d->model->allPos();

    if( pos > -1 )
    {
        if( d->comboBox ) {
            d->comboBox->setCurrentIndex( pos );
        } else if( d->listView ) {
            d->listView->setCurrentIndex( d->model->index( pos, 0 ) );
            d->model->toggleItem( pos );
        } else if( d->dialogButton ) {
            d->selectedCategories = QCategoryFilter( QCategoryFilter::All );
            d->dialogButton->setText( QCategoryManager::allLabel() );
        }
    }
}

/*!
  Selects the Unfiled entry (no categories are selected).
*/
void QCategorySelector::selectUnfiled()
{
    d->selectUnfiled();
}

/*!
  \internal
  Shows a dialog that allows editing of the current category.
*/
void QCategorySelector::showDialog()
{
    if (!d->selectorDialog) {
        d->selectorDialog = new QCategoryDialog(d->scope, QCategoryDialog::Editor, this);
        d->selectorDialog->setWindowModality(Qt::WindowModal);

        connect(d->selectorDialog, SIGNAL(accepted()), this, SLOT(dialogAccepted()));
    }

    switch (d->selectedFilterType()) {
    case QCategoryFilter::All:
        d->selectorDialog->selectAll();
        break;
    case QCategoryFilter::List:
        {
            QStringList catids = selectedCategories();
            if (!catids.isEmpty()) {
                d->selectorDialog->selectCategories(catids);
                break;
            }
        }
    default:
        d->selectorDialog->selectUnfiled();
    }

    QtopiaApplication::showDialog(d->selectorDialog);
}

/*!
    \internal
*/
void QCategorySelector::dialogAccepted()
{
    selectCategories(d->selectorDialog->selectedCategories());
}

//////////////////////////////////////////////
// Dialog - uses same abstract model
//////////////////////////////////////////////

class QCategoryDialogData : public QObject
{
    Q_OBJECT
public:
    QCategoryDialogData(const QString &s, QCategorySelector::ContentFlags f, QCategoryDialog* parent);

    QCategorySelector* categorySelect;
    QLabel* label;
};

QCategoryDialogData::QCategoryDialogData(const QString &s,
    QCategorySelector::ContentFlags f, QCategoryDialog* parent)
    : QObject(parent), label(0)
{
    QVBoxLayout *vbl = new QVBoxLayout(parent);
    parent->setLayout(vbl);
    vbl->setSpacing(0);
    vbl->setMargin(0);

    label = new QLabel(parent);
    label->setWordWrap(true);
    vbl->addWidget(label);
    label->hide();

    categorySelect = new QCategorySelector(s, f);
    categorySelect->setListFrameStyle(QFrame::NoFrame);
    vbl->addWidget(categorySelect);
}

/*!
  \class QCategoryDialog
    \inpublicgroup QtBaseModule

  \ingroup categories
  \brief The QCategoryDialog widget allows users to select Categories with a
  dialog interface.

  QCategoryDialog behaves in the same way as QCategorySelector using a list view.

  \sa Categories
*/

/*!
  \enum QCategoryDialog::ContentFlag

  \value IncludeAll Include the All categories option.
  \value IncludeUnfiled Include the Unfiled categories option.
  \value SingleSelection Allow selection of a single category. The default is to allow combinations of categories.
  \value Filter Alias for IncludeAll|IncludeUnfiled.
  \value Editor Alias for IncludeUnfiled.
*/

/*!
  Constructs as a category dialog type defined by flags \a f with parent \a parent and
  categories scope \a scope.
*/
QCategoryDialog::QCategoryDialog(const QString &scope, ContentFlags f, QWidget *parent)
    : QDialog(parent)
{
    QCategorySelector::ContentFlags sf(0);
    if (f & IncludeAll)
        sf |= QCategorySelector::IncludeAll;
    if (f & IncludeUnfiled)
        sf |= QCategorySelector::IncludeUnfiled;
    if (f & SingleSelection)
        sf |= QCategorySelector::SingleSelection;

    sf |= QCategorySelector::ListView;

    d = new QCategoryDialogData(scope, sf, this);

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);

    setWindowTitle(tr("Select Category"));
}

/*!
  Destroys the QCategoryDialog.
*/
QCategoryDialog::~QCategoryDialog()
{
    delete d;
}

/*!
  \internal
  Handles the key event \a e.
*/
void QCategoryDialog::keyPressEvent(QKeyEvent* e)
{
    if(e->key() == Qt::Key_Select)
        accept();
    else if(e->key() == Qt::Key_Back)
        accept();
    else if (e->key() == Qt::Key_Hangup)
        e->ignore();
}

/*!
  \reimp
*/
QSize QCategoryDialog::sizeHint() const
{
    QDesktopWidget *desktop = QApplication::desktop();
    if(!d->label->text().isEmpty())
        return desktop->availableGeometry(desktop->screenNumber(this)).size();
    else
        return d->categorySelect->sizeHint();
}

/*!
  Displays \a text above the category selection list.  \a text may be used
  to clarify the category selection purpose.
  \sa text()
*/
void QCategoryDialog::setText(const QString &text)
{
    d->label->setText(text);
    if(text.isEmpty())
        d->label->hide();
    else
        d->label->show();
}

/*!
  Returns the dialog text, as set by a previous call to \l setText().
 */
QString QCategoryDialog::text() const
{
    return d->label->text();
}

/*!
  Returns the list of selected categories.

  Note that this function does not indicate if \c All or \c Unfiled is selected. Use
  selectedFilter() to determine if these items are selected.
*/
QList<QString> QCategoryDialog::selectedCategories() const
{
    return d->categorySelect->selectedCategories();
}

/*!
  Returns the QCategoryFilter that indicates the selected categories.
  \sa QCategoryFilter::requiredCategories()
*/
QCategoryFilter QCategoryDialog::selectedFilter() const
{
    return d->categorySelect->selectedFilter();
}

/*!
  Selects the categories with identifiers in the comma-separated list \a id.
*/
void QCategoryDialog::selectCategories(const QString &id)
{
    d->categorySelect->selectCategories(id);
}

/*!
  Selects the categories with identifiers in list \a categoryids.
*/
void QCategoryDialog::selectCategories(const QList<QString> &categoryids)
{
    d->categorySelect->selectCategories(categoryids);
}

/*!
  Selects the categories specified by \a filter.
*/
void QCategoryDialog::selectFilter(const QCategoryFilter &filter)
{
    d->categorySelect->selectFilter(filter);
}

/*!
  Selects the All entry (all categories are selected).
*/
void QCategoryDialog::selectAll()
{
    d->categorySelect->selectAll();
}

/*!
  Selects the Unfiled entry (no categories are selected).
*/
void QCategoryDialog::selectUnfiled()
{
    d->categorySelect->selectUnfiled();
}

#include "qcategoryselector.moc"

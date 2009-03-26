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

#include <qcontactview.h>
#include <QTextEntryProxy>

#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QAction>
#include <QDebug>
#include <QPainter>
#include <QMenu>
#include <QTextDocument>
#include <QTextFrame>
#include <QAbstractTextDocumentLayout>
#include <QTimer>

#include <QKeyEvent>

#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <QtopiaItemDelegate>

/*!
  \class QContactDelegate
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactDelegate class provides drawing of QContactModel items (\l {QContact}{QContacts}).

  By using QContactDelegate, applications dealing with QContacts can achieve a consistent
  look and feel.

  QContacts are drawn with four major sections per item.  There are icons on
  the left and right sides of the rendered item, and top and bottom lines of text.
  The data drawn is fetched from the model (which is assumed to be a QContactModel),
  using some additional Qt::ItemDataRole values defined in QContactModel.  The
  following image illustrates a list of \l{QContact}{QContacts} being displayed
  in a QContactListView using a QContactDelegate:

  \image qcontactview.png "List of QContacts"

  QContactModel::QContactModelRole defines the additional roles used to draw the items:

  \table 80 %
   \header
    \o Role
    \o Data Type
    \o Description
   \row
    \o QContactModel::PortraitRole
    \o QPixmap
    \o Drawn vertically centered on the leading side (left for LTR languages) of the item.
   \row
    \o Qt::DisplayRole
    \o QString
    \o Plain unformatted text drawn at the top of the item, between any icons.
   \row
    \o QContactModel::StatusIconRole
    \o QPixmap
    \o Optional. Drawn vertically centered on the trailing side (right for LTR languages) of the item.
   \row
    \o QContactModel::SubLabelRole
    \o QString
    \o Drawn as plain text below the label text, if space is available.
  \endtable

  The first four contacts shown in the picture above have the following data in the QContactModel:
  \table 80 %
   \header
    \o PortraitRole
    \o DisplayRole
    \o StatusIconRole
    \o SubLabelRole
   \row
    \o Pixmap of a person
    \o Adam Zucker
    \o <empty pixmap>
    \o 12345
   \row
    \o Pixmap of a SIM card
    \o Adam Zucker/h
    \o <empty pixmap>
    \o 12345
   \row
    \o Pixmap of a SIM card
    \o Adam Zucker/m
    \o <empty pixmap>
    \o 24685
   \row
    \o Pixmap of a person
    \o Bradley Young
    \o Pixmap of a briefcase
    \o 48759
  \endtable

  \sa QContact, QContactListView, QContactModel, QPimDelegate, {Pim Library}
*/

/*!
  Constructs a QContactDelegate with the given \a parent.
*/
QContactDelegate::QContactDelegate( QObject * parent )
    : QPimDelegate(parent)
{
}

/*!
  \reimp
  Returns a list consisting of a single string, that of the QContactModel::SubLabelRole for the
  supplied \a index.  Ignores \a option.
*/
QList<StringPair> QContactDelegate::subTexts(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    QList< StringPair > subList;
    QString subLabel = index.data(QContactModel::SubLabelRole).toString();
    subList.append(qMakePair(QString(), subLabel));
    return subList;
}

/*!
  \reimp
  Returns 1 (the QContactModel::SubLabelRole text).

  Ignores \a index and \a option.
 */
int QContactDelegate::subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // We usually have one row, and no harm comes from this if we have zero
    return 1;
}

/*!
  \reimp
  Draws the portrait picture (QContactModel::PortraitRole) for the contact
  specified by \a index on the leading side of the item, and the pixmap
  specified by QContactModel::StatusIconRole on the trailing side of the
  item.  The leading and trailing sides are determined by the \a rtl parameter.
  The pixmaps are drawn using the painter \a p.  The rectangle taken up by the
  portrait is added to \a leadingFloats, while the rectangle taken up by the
  status icon is added to \a trailingFloats.

  Ignores \a option.
*/
void QContactDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index,
                                      QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const
{
    QPixmap decoration = qvariant_cast<QPixmap>(index.data(QContactModel::PortraitRole));
    QPixmap trailingdecoration = qvariant_cast<QPixmap>(index.data(QContactModel::StatusIconRole));

    QRect drawRect;
    QSize ths;

    if (!decoration.isNull()) {
        drawRect = option.rect;
        ths = QContact::thumbnailSize();
        if (rtl)
            drawRect.setLeft(drawRect.right() - ths.width() - 4);
        else
            drawRect.setWidth(ths.width() + 4);

        // Center the thumbnail
        QPoint drawOffset = QPoint(drawRect.left() + (drawRect.width() - ths.width())/2, drawRect.top() + (drawRect.height() - ths.height())/2);

        p->drawPixmap(drawOffset, decoration);

        leadingFloats.append(drawRect);
    }


    if (!trailingdecoration.isNull()) {
        drawRect = option.rect;
        ths = trailingdecoration.size();

        if (rtl)
            drawRect.setWidth(ths.width() + 4);
        else
            drawRect.setLeft(drawRect.right() - ths.width() - 4);

        // Center the thumbnail
        QPoint drawOffset = QPoint(drawRect.left() + (drawRect.width() - ths.width())/2, drawRect.top() + (drawRect.height() - ths.height())/2);

        p->drawPixmap(drawOffset, trailingdecoration);

        trailingFloats.append(drawRect);
    }
}

/*!
  \reimp
  Always returns QPimDelegate::Independent, ignoring \a option and \a index.
*/
QPimDelegate::SubTextAlignment QContactDelegate::subTextAlignment(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QPimDelegate::Independent;
}

/*!
  \reimp
  Always returns QPimDelegate::SelectedOnly, ignoring \a option and \a index.
 */
QPimDelegate::BackgroundStyle QContactDelegate::backgroundStyle(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QPimDelegate::SelectedOnly;
}

/*!
  \reimp
  Returns the size hint for the delegate, given the \a option and \a index parameters.
  The \a textSize size hint is expanded to accommodate the portrait icon width and height.
*/
QSize QContactDelegate::decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // Note - this ignores the secondaryIconRole pixmap, since it is of variable size
    QSize ths = QContact::thumbnailSize();
    QPixmap trailingdecoration = qvariant_cast<QPixmap>(index.data(QContactModel::StatusIconRole));
    return QSize(ths.width() + 4 + textSize.width() + trailingdecoration.width() + 4, qMax(qMax(ths.height(), trailingdecoration.height()) + 4, textSize.height()));
}

/*!
  Destroys a QContactDelegate.
*/
QContactDelegate::~QContactDelegate() {}


class QContactListViewPrivate
{
public:
    QContactListViewPrivate()
        : proxy(0), searchTimer(0) {}

    QTextEntryProxy *proxy;
    QTimer *searchTimer;
};

/*!
  \class QContactListView
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactListView class provides a list view widget for use with QContactModel.

  The convenience functions provided by QContactListView include functions for interpreting
  the view's model, delegate and current item as the corresponding QContactModel, QContactDelegate and
  QContact objects.  In addition, QContactListView enforces using a QContactModel (or a derivative)
  as the model.

  Upon construction, QContactListView automatically sets itself to use a QContactDelegate for drawing,
  sets \c Batch layout mode (\l setLayoutMode()), and sets the resize mode to \c Adjust
  (\l setResizeMode()).

  The following image displays a QContactListView, using the
  default QContactDelegate to render QContacts from a QContactModel.

  \image qcontactview.png "List of QContacts"

  \sa QContact, QContactModel, QContactDelegate, {Pim Library}
*/

/*!
    \fn QContact QContactListView::currentContact() const

    Returns the QContact for the currently selected index.
*/

/*!
  \fn QContactModel *QContactListView::contactModel() const

  Returns the QContactModel set for the view.
*/

/*!
  \fn QContactDelegate *QContactListView::contactDelegate() const

  Returns the QContactDelegate set for the view.  During
  construction, QContactListView  will automatically create
  a QContactDelegate to use as the delegate, but this can be
  overridden with a different delegate derived from
  QContactDelegate if necessary.
*/


/*!
  Constructs a QContactListView with the given \a parent.

  This also sets the layout mode to \c Batched for performance,
  the resize mode to \c Adjust, and creates a \l QContactDelegate
  to use as the delegate.
*/
QContactListView::QContactListView(QWidget *parent)
    : QListView(parent)
{
    d = new QContactListViewPrivate();
    setItemDelegate(new QContactDelegate(this));
    setResizeMode(Adjust);
    setLayoutMode(Batched);
    setSelectionMode(QAbstractItemView::SingleSelection);
    d->searchTimer = new QTimer(this);
    d->searchTimer->setInterval(100);
    d->searchTimer->setSingleShot(true);
    connect(d->searchTimer, SIGNAL(timeout()),
            this, SLOT(setFilterText()));
}

/*!
  Destroys the QContactListView.
*/
QContactListView::~QContactListView()
{
    delete d;
    d = 0;
}

/*!
  Sets a QTextEntryProxy for the list view to \a proxy.  This allows the list
  view to accept text and InputMethod events, which it will pass to \a proxy.
  The text of the proxy is used for filtering the list of contacts in the view.
*/
void QContactListView::setTextEntryProxy(QTextEntryProxy *proxy)
{
    if(style()->inherits("QThumbStyle")) {
        d->proxy = 0;
        return;
    }
    if (d->proxy)
        disconnect(d->proxy, SIGNAL(textChanged(QString)), d->searchTimer, SLOT(start()));

    d->proxy = proxy;
    d->proxy->setTarget(this);

    if (d->proxy)
        connect(d->proxy, SIGNAL(textChanged(QString)), d->searchTimer, SLOT(start()));
}

/*!
  Returns the QTextEntryProxy for the list view.  If there is no QTextEntryProxy set returns
  0.

  \sa setTextEntryProxy()
*/
QTextEntryProxy *QContactListView::textEntryProxy() const
{
    return d->proxy;
}

/*!
  \overload

  Sets the model for the view to \a model.
*/
void QContactListView::setModel( QAbstractItemModel *model )
{
    QListView::setModel(model);
    /* connect the selectionModel (which is created by setModel) */
    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentContactChanged(QModelIndex)));
}

/*!
  Returns the list of complete contacts selected from the view.  If a large number of contacts
  might be selected this function can be expensive, and selectedContactIds() should be used
  instead.

  \sa selectedContactIds()
*/
QList<QContact> QContactListView::selectedContacts() const
{
    QContactModel *model = contactModel();
    if (!model)
        return QList<QContact>();

    QList<QContact> res;
    for (int i = 0; i < model->count(); ++i) {
        if (selectionModel()->isSelected(model->index(i)))
            res.append(model->contact(i));
    }
    return res;
}

/*!
  Returns the list of ids for contacts selected in the view.

  \sa selectedContacts()
*/
QList<QUniqueId> QContactListView::selectedContactIds() const
{
    QContactModel *model = contactModel();
    if (!model)
        return QList<QUniqueId>();

    QList<QUniqueId> res;
    for (int i = 0; i < model->count(); ++i) {
        if (selectionModel()->isSelected(model->index(i)))
            res.append(model->id(i));
    }
    return res;
}

/*!
  \internal
   Try to make sure we select the current item \a newIndex, whenever it
   changes.
   */
void QContactListView::currentContactChanged(const QModelIndex& newIndex)
{
    if (newIndex.isValid()) {
        selectionModel()->select(newIndex, QItemSelectionModel::Select);
    }
}

/*
   This code used to be necesary, but is not any more.  Binary compatibility
   requires it here, though.
*/

/*!
  \reimp
*/
void QContactListView::focusInEvent(QFocusEvent *)
{
    if (selectionModel() && !currentIndex().isValid()) {
        selectionModel()->setCurrentIndex(
            moveCursor(MoveNext, Qt::NoModifier), // first visible index
            QItemSelectionModel::NoUpdate);
    }
    // Avoid unnecesary repaint
}

/*!
  \reimp
*/
void QContactListView::focusOutEvent(QFocusEvent *)
{
    // Avoid unnecesary repaint
}

void QContactListView::setFilterText()
{
    QString text = d->proxy ? d->proxy->text() : QString();
    if (contactModel()) {
        contactModel()->setFilter(text, contactModel()->filterFlags());
    }
    if (text.isEmpty())
        QSoftMenuBar::clearLabel(this, Qt::Key_Back);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
}


class QSmoothContactListViewPrivate
{
public:
    QSmoothContactListViewPrivate()
        : proxy(0), searchTimer(0) {}

    QTextEntryProxy *proxy;
    QTimer *searchTimer;
};

/*!
  \class QSmoothContactListView
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QSmoothContactListView class provides a list view widget for use with QContactModel.

  The convenience functions provided by QSmoothContactListView include functions for interpreting
  the view's model, delegate and current item as the corresponding QContactModel, QContactDelegate and
  QContact objects.  In addition, QSmoothContactListView enforces using a QContactModel (or a derivative)
  as the model.

  Upon construction, QSmoothContactListView automatically sets itself to use a QContactDelegate for drawing.

  The following image displays a QSmoothContactListView, using the
  default QContactDelegate to render QContacts from a QContactModel.

  The API of this class is a subset of the QContactListView API.

  \image qcontactview.png "List of QContacts"

  \sa QContact, QContactModel, QContactDelegate, {Pim Library}
*/

/*!
    \fn QContact QSmoothContactListView::currentContact() const

    Returns the QContact for the currently selected index.
*/

/*!
  \fn QContactModel *QSmoothContactListView::contactModel() const

  Returns the QContactModel set for the view.
*/

/*!
  \fn QContactDelegate *QSmoothContactListView::contactDelegate() const

  Returns the QContactDelegate set for the view.  During
  construction, QSmoothContactListView  will automatically create
  a QContactDelegate to use as the delegate, but this can be
  overridden with a different delegate derived from
  QContactDelegate if necessary.
*/


/*!
  Constructs a QSmoothContactListView with the given \a parent.

  This also sets the layout mode to \c Batched for performance,
  the resize mode to \c Adjust, and creates a \l QContactDelegate
  to use as the delegate.
*/
QSmoothContactListView::QSmoothContactListView(QWidget *parent)
    : QSmoothList(parent)
{
    d = new QSmoothContactListViewPrivate();
    setItemDelegate(new QContactDelegate(this));
    d->searchTimer = new QTimer(this);
    d->searchTimer->setInterval(100);
    d->searchTimer->setSingleShot(true);
    connect(d->searchTimer, SIGNAL(timeout()),
            this, SLOT(setFilterText()));
}

/*!
  Destroys the QSmoothContactListView.
*/
QSmoothContactListView::~QSmoothContactListView()
{
    delete d;
    d = 0;
}

/*!
  Sets a QTextEntryProxy for the list view to \a proxy.  This allows the list
  view to accept text and InputMethod events, which it will pass to \a proxy.
  The text of the proxy is used for filtering the list of contacts in the view.
*/
void QSmoothContactListView::setTextEntryProxy(QTextEntryProxy *proxy)
{
    if(style()->inherits("QThumbStyle")) {
        d->proxy = 0;
        return;
    }
    if (d->proxy)
        disconnect(d->proxy, SIGNAL(textChanged(QString)), d->searchTimer, SLOT(start()));

    d->proxy = proxy;
    d->proxy->setTarget(this);

    if (d->proxy)
        connect(d->proxy, SIGNAL(textChanged(QString)), d->searchTimer, SLOT(start()));
}

/*!
  Returns the QTextEntryProxy for the list view.  If there is no QTextEntryProxy set returns
  0.

  \sa setTextEntryProxy()
*/
QTextEntryProxy *QSmoothContactListView::textEntryProxy() const
{
    return d->proxy;
}

/*!
  \overload

  Sets the model for the view to \a model.
*/
void QSmoothContactListView::setModel( QAbstractItemModel *model )
{
    QSmoothList::setModel(model);
    connect(this, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentContactChanged(QModelIndex)));
}

/*!
  \internal
   Try to make sure we select the current item \a newIndex, whenever it
   changes.
   */
void QSmoothContactListView::currentContactChanged(const QModelIndex& newIndex)
{
    Q_UNUSED(newIndex);
}

void QSmoothContactListView::setFilterText()
{
    QString text = d->proxy ? d->proxy->text() : QString();
    if (contactModel()) {
        contactModel()->setFilter(text, contactModel()->filterFlags());
    }
    if (text.isEmpty())
        QSoftMenuBar::clearLabel(this, Qt::Key_Back);
    else
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
}

/***************************
* QContactSelector
***********************/
class QContactSelectorPrivate
{
public:
    QContactSelectorPrivate()
        : view(0), mType(RejectType), mTextSelectable(false),
        newAction(0), proxy(0), mModel(0)
        {}
    QSmoothContactListView *view;
    enum AcceptType {
        RejectType,
        ContactType,
        NewType,
        TextType
    };

    AcceptType mType;
    bool mTextSelectable;

    QAction *newAction;
    QTextEntryProxy *proxy;
    QContactModel *mModel;
};

/*!
  \class QContactSelector
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactSelector class provides a way of selecting a single contact from a QContactModel.

  In addition, the user can optionally be allowed to indicate they want to create a new contact,
  if none of the existing contacts are suitable.

  The following image displays a QContactSelector with the option to
  create a new QContact highlighted.

  \image qcontactselector.png "QContactSelector with the option to create a new contact highlighted"

  \sa {Pim Library}
*/

/*!
  Constructs a QContactSelector with parent \a parent.  If \a showCreateNew is true an action will
  be include to allow the user to create new contacts.

  \sa setCreateNewContactEnabled()
*/
QContactSelector::QContactSelector(bool showCreateNew, QWidget *parent)
    : QDialog(parent)
{
    init();
    setCreateNewContactEnabled(showCreateNew);
}
/*!
  Contructs a QContactSelector with parent \a parent.
*/
QContactSelector::QContactSelector(QWidget *parent)
    : QDialog(parent)
{
    init();
}


/*!
    Destroys a QContactSelector.
    */
QContactSelector::~QContactSelector()
{
    delete d;
}

/*!
  \internal
  Does the work of constructing a QContactSelector
*/
void QContactSelector::init()
{
    d = new QContactSelectorPrivate();
    setWindowTitle( tr("Select Contact") );
    QVBoxLayout *l = new QVBoxLayout;
    l->setMargin(0);
    l->setSpacing(2);

    d->view = new QSmoothContactListView;

    if(!style()->inherits("QThumbStyle")) {
        QtopiaApplication::setInputMethodHint(d->view, "text");
        d->view->setAttribute(Qt::WA_InputMethodEnabled);
    }
    d->view->installEventFilter(this);

    connect( d->view, SIGNAL(activated(QModelIndex)), this, SLOT(setSelected(QModelIndex)) );

    l->addWidget( d->view );

    if(!style()->inherits("QThumbStyle")) {
        d->proxy = new QTextEntryProxy(this, d->view);
        int mFindHeight = d->proxy->sizeHint().height();
        QLabel *findIcon = new QLabel;
        findIcon->setPixmap(QIcon(":icon/find").pixmap(mFindHeight-2, mFindHeight-2));
        findIcon->setMargin(2);

        QHBoxLayout *findLayout = new QHBoxLayout;
        findLayout->addWidget(findIcon);
        findLayout->addWidget(d->proxy);

        l->addLayout( findLayout );

        connect( d->proxy, SIGNAL(textChanged(QString)),
             this, SLOT(filterList(QString)) );
    } else
        d->proxy = 0;

    setLayout(l);

    QMenu *menu = QSoftMenuBar::menuFor( this );
    d->newAction = menu->addAction( QIcon(":icon/new"), tr("New"), this, SLOT(setNewSelected()) );
    d->newAction->setVisible(false);

    connect(this, SIGNAL(accepted()), this, SLOT(completed()));

    QtopiaApplication::setMenuLike( this, true );
}

/*!
  If \a enable is true an action to create new contacts will be visible in the context menu.
  Otherwise the action to create new contacts will be hidden.
*/
void QContactSelector::setCreateNewContactEnabled(bool enable)
{
    d->newAction->setVisible(enable);
}

/*!
  If \a enable is true pressing select while filter text will accept the dialog even if
  no contact from the list is selected.
*/
void QContactSelector::setAcceptTextEnabled(bool enable)
{
    d->mTextSelectable = enable;
}

/*!
   \overload
   Filters events if this object has been installed as an event filter for the \a watched object.
   Returns true if the \a event is filtered. Otherwise returns false.
*/
bool QContactSelector::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->view) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = (QKeyEvent *)event;
            if (ke->key() == Qt::Key_Select && !d->view->currentIndex().isValid()) {
                d->mType = QContactSelectorPrivate::TextType;
                event->accept();
                accept();
                return true;
            }
        }
    }
    return false;
}

/*!
  \internal
   Model resets don't generate current index changed message, so
   force the first contact to be selected.
*/
void QContactSelector::contactModelReset()
{
    /* we know our selection is invalid.. */
    QModelIndex newSel = d->mModel->index(0,0);
    if (newSel.isValid()) {
        d->view->setCurrentIndex(newSel);
    }
}


/*!
  \internal
  Accepts the dialog and indicates that a new contact should be created.
*/
void QContactSelector::setNewSelected()
{
    d->mType = QContactSelectorPrivate::NewType;
    accept();
}

/*!
  \internal
  Accepts the dialog and indicates that a contact at \a idx in the model was selected.
*/
void QContactSelector::setSelected(const QModelIndex& idx)
{
    if (idx.isValid())
    {
        d->view->setCurrentIndex(idx);
        d->mType = QContactSelectorPrivate::ContactType;
    }

    QContactModel *m = qobject_cast<QContactModel *>(d->view->model());
    emit contactSelected(m->contact(idx));

    accept();
}

/*!
  \internal
  Given text proxy input \a str, filter the list of contacts to those that match.
*/
void QContactSelector::filterList(const QString& str)
{
    if (d->mModel) {
        d->mModel->setFilter(str, d->mModel->filterFlags());
    }
}

/*!
  Sets the model providing the choice of contacts to \a model.
*/
void QContactSelector::setModel(QContactModel *model)
{
    QAbstractItemModel *m = d->mModel;
    d->mModel = model;
    d->view->setModel(model);
    if (m != model) {
        if (m)
            disconnect(m, SIGNAL(modelReset()), this, SLOT(contactModelReset()));
        if (model) {
            if(!style()->inherits("QThumbStyle"))
                d->mModel->setFilter(d->proxy->text(), d->mModel->filterFlags());
            connect(model, SIGNAL(modelReset()), this, SLOT(contactModelReset()));
        }
    }
}

/*!
  Returns true if the dialog was accepted via the option to
  create a new contact.
  Otherwise returns false.

  \sa contactSelected()
*/
bool QContactSelector::newContactSelected() const
{
    if (result() == Rejected)
        return false;
    return d->mType == QContactSelectorPrivate::NewType;
}

/*!
  Returns true if the dialog was accepted with an existing contact selected.
  Otherwise returns false.

  \sa newContactSelected()
*/
bool QContactSelector::contactSelected() const
{
    return d->mType == QContactSelectorPrivate::ContactType;
}

/*!
  Returns true if the dialog was accepted with filter text entered but no contact
  selected.
*/
bool QContactSelector::textSelected() const
{
    return d->mType == QContactSelectorPrivate::TextType;
}

/*!
  Returns the contact that was selected.  If no contact was selected returns a null contact.

  \sa contactSelected(), newContactSelected()
*/
QContact QContactSelector::selectedContact() const
{
    QContactModel *m = qobject_cast<QContactModel *>(d->view->model());
    if (!m || d->mType != QContactSelectorPrivate::ContactType
            || result() == Rejected || !d->view->currentIndex().isValid())
        return QContact();

    return m->contact(d->view->currentIndex());
}

/*!
  Returns the text entered when the dialog was accepted.  Requires that accept on
  text is enabled.

  \sa setAcceptTextEnabled()
*/
QString QContactSelector::selectedText() const
{
    if (!textSelected() || result() == Rejected)
        return QString();
    if (d->view->textEntryProxy())
        return d->view->textEntryProxy()->text();
    else if (d->proxy)
        return d->proxy->text();
    return QString();
}

/*! \internal */
void QContactSelector::completed()
{
    if (textSelected()) {
        if (d->view->textEntryProxy())
            emit textSelected(d->view->textEntryProxy()->text());
        else if (d->proxy)
            emit textSelected(d->proxy->text());
    }
}

/*!
    \fn void QContactSelector::contactSelected(const QContact& contact)

    When selection of a contact occurs, this signal is emitted with the \a contact selected.

    \sa selectedContact()
*/

/*!
    \fn void QContactSelector::textSelected(const QString& text)

    When the dialog is accepted with text entered, this signal is emitted with the \a text previously entered.

    \sa selectedText()
*/


// A QListWidget with a useful size hint.
class TightListWidget : public QListWidget {
public:
    TightListWidget(QWidget* parent) : QListWidget(parent) {}
    QSize sizeHint() const
    {
        // Assumes all rows/columns similar enough to #0.
        return QSize(sizeHintForColumn(0)*model()->columnCount(),
                     sizeHintForRow(0)*model()->rowCount());
    }
};

/***************************
* Phone type Selector
***********************/

class QPhoneTypeSelectorPrivate
{
public:
    QPhoneTypeSelectorPrivate(const QContact &cnt, const QString &number, QList<QContact::PhoneType> allowedTypes)
        : mToolTip(0), mContact( cnt ), mNumber(number), mAllowedPhoneTypes(allowedTypes) {}

    QLabel *mLabel, *mToolTip;
    TightListWidget *mPhoneType;
    QMap<QListWidgetItem *, QContact::PhoneType> mItemToPhoneType;
    const QContact mContact;
    const QString mNumber;
    QList<QContact::PhoneType> mAllowedPhoneTypes;
};

/*!
  \class QPhoneTypeSelector
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPhoneTypeSelector class provides a way of selecting a single type of phone number.

  The phone number types correspond to those defined in \l QContact::PhoneType.

  It can be used in one of two ways - either to allow the selection of one of a
  QContact's existing phone numbers, or to allow the user to pick the type of a
  new phone number (for example, to save a phone number as a 'work mobile' number).

  \table 80 %
  \row
  \o \inlineimage qphonetypeselector-existing.png "Picking an existing phone number"
  \o Picking an existing number from a QContact
  \row
  \o \inlineimage qphonetypeselector-new.png "Picking a new phone number type"
  \o Choosing a phone number type for a new phone number.  Note that the existing
     phone numbers for the QContact are displayed.
  \endtable

  \sa QContact, {Pim Library}
*/

/*!
  \fn void QPhoneTypeSelector::selected(QContact::PhoneType type)

    This signal is emitted when the user selects a phone number \a type.
*/

/*!
  Constructs a QPhoneTypeSelector dialog with parent \a parent.

  The dialog will show phone numbers and phone number types from the given \a contact.

  The dialog can be used in two ways:
  \list
  \o To choose an existing phone number from a contact, pass an empty string as the \a number argument.
  \o To choose a phone number type for a new phone number, pass the phone number as a string as the \a number argument.
  \endlist

  In the second case, the dialog will show any existing phone numbers for the
  contact in addition to the available phone number types.

  \sa updateContact()
*/
QPhoneTypeSelector::QPhoneTypeSelector( const QContact &contact, const QString &number,
        QWidget *parent )
    : QDialog( parent )
{
    d = new QPhoneTypeSelectorPrivate(contact, number, QList<QContact::PhoneType>());

    init();
}

/*!
  Constructs a QPhoneTypeSelector dialog with parent \a parent.

  The dialog will show phone numbers and phone number types from the given \a contact.

  The dialog can be used in two ways:
  \list
  \o To choose an existing phone number from a contact, pass an empty string as the \a number argument.
  \o To choose a phone number type for a new phone number, pass the phone number as a string as the \a number argument.
  \endlist

  In the second case, the dialog will show any existing phone numbers for the
  contact in addition to the available phone number types, restricted by the
  set \a allowedTypes.  If \a allowedTypes has zero length, the entire
  set of possible types is allowed.

  \sa updateContact()

 */
QPhoneTypeSelector::QPhoneTypeSelector( const QContact &contact, const QString& number, QList<QContact::PhoneType> allowedTypes, QWidget *parent)
    : QDialog( parent )
{
    d = new QPhoneTypeSelectorPrivate(contact, number, allowedTypes);

    init();
}


/*!
    Destroys a QPhoneTypeSelector
    */
QPhoneTypeSelector::~QPhoneTypeSelector()
{
    delete d;
}

/*!
  If \a number is empty returns the translation of "(empty)".  Otherwise returns \a number.

  \internal
 */
QString QPhoneTypeSelector::verboseIfEmpty( const QString &number )
{
    if( number.isEmpty() )
        return tr("(empty)");
    return number;
}

/*!
  Returns the QContact::PhoneType that is selected in the dialog.
  If no phone number type is selected, this will return \c OtherPhone.
*/
QContact::PhoneType QPhoneTypeSelector::selected() const
{
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        return QContact::OtherPhone;
    return d->mItemToPhoneType[item];
}

/*!
  Returns the contact's phone number that corresponds to the phone number type
  selected in the dialog.
*/
QString QPhoneTypeSelector::selectedNumber() const
{
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( !item )
        return QString();
    return item->text();
}

/*!
  \internal
  Initializes state of the QPhoneTypeSelector
*/
void QPhoneTypeSelector::init()
{
    QVBoxLayout *l = new QVBoxLayout( this );
    l->setMargin(0);
    l->setSpacing(0);
    d->mLabel = new QLabel( this );
    d->mLabel->setWordWrap( true );
    l->addWidget(d->mLabel);
    d->mPhoneType = new TightListWidget( this );
    d->mPhoneType->setFrameStyle(QFrame::NoFrame);
    l->addWidget(d->mPhoneType);
    d->mPhoneType->setItemDelegate(new QtopiaItemDelegate(this));
    d->mPhoneType->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    setWindowTitle(tr("Phone Type"));

    QListWidgetItem *item = 0;

    if (!d->mNumber.isEmpty()) //assume an empty number means we want to select a number
    {
        bool first = true;

        d->mLabel->setText( "<qt>"+tr("Please select the phone type for '%1'.", "%1 = mobile number or land line number").arg( d->mNumber )+"</qt>" );

        QList<QContact::PhoneType> phoneTypes = d->mAllowedPhoneTypes;
        if (phoneTypes.count() == 0)
            phoneTypes = QContact::phoneTypes();

        foreach(QContact::PhoneType type, phoneTypes) {
            QString phoneNumber = d->mContact.phoneNumber(type);
            item =new QListWidgetItem(verboseIfEmpty(phoneNumber), d->mPhoneType, type);
            item->setIcon(QContact::phoneIcon(type));
            d->mItemToPhoneType[item] = type;
            if (first) {
                d->mPhoneType->setCurrentItem( item );
                first = false;
            }
        }
    } else {
        bool haveNumber = false;
        bool first = true;
        setWindowTitle(tr("Phone Number"));
        d->mLabel->hide();

        QList<QContact::PhoneType> phoneTypes = d->mAllowedPhoneTypes;
        if (phoneTypes.count() == 0)
            phoneTypes = QContact::phoneTypes();

        foreach(QContact::PhoneType type, phoneTypes) {
            QString number = d->mContact.phoneNumber(type);
            if (!number.isEmpty()) {
                item = new QListWidgetItem( number, d->mPhoneType );
                item->setIcon(QContact::phoneIcon(type));
                d->mItemToPhoneType[item] = type;
                haveNumber = true;
                if (first) {
                    d->mPhoneType->setCurrentItem( item );
                    first = false;
                }
            }
        }

        if (!haveNumber) {
            d->mToolTip = new QLabel(tr("No phone number for this contact"), this);
            d->mToolTip->setWordWrap( true );
            d->mToolTip->setAlignment(Qt::AlignCenter);
            d->mToolTip->show();
        }
    }
    connect( d->mPhoneType, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(accept()) );

    d->mPhoneType->setFocus();
    if( !Qtopia::mousePreferred() ) {
    if( d->mPhoneType->hasEditFocus() )
        d->mPhoneType->setEditFocus( true );
    }
    QtopiaApplication::setMenuLike(this, true);
}

/*!
  \reimp
*/
void QPhoneTypeSelector::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (d->mToolTip)
        d->mToolTip->move((width()-d->mToolTip->width())/2, (height()-d->mToolTip->height())/2);
}

/*!
  Updates \a contact to have the given \a number for the dialog's selected phone number type.

  \sa selected()
*/
void QPhoneTypeSelector::updateContact(QContact &contact, const QString &number) const {
    contact.setPhoneNumber(selected(), number);
}

/*!
  \reimp
*/
void QPhoneTypeSelector::accept()
{
    QDialog::accept();
    QListWidgetItem *item = d->mPhoneType->currentItem();
    if( item )
        emit selected( d->mItemToPhoneType[item] );
}

/*!
  \reimp
*/
void QPhoneTypeSelector::keyPressEvent(QKeyEvent *ke)
{
    /* If we're choosing a number, then allow call to accept
       the number.. it's very rare you'd want Call History to be
       displayed, for example */
    if (ke->key() == Qt::Key_Call && d->mNumber.isEmpty()) {
        ke->setAccepted(true);
        accept();
        return;
    }
    QDialog::keyPressEvent(ke);
}

/*!
  \class QContactItem
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactItem class provides a QStandardItem based class representing a \l QContact.

  By using QContactItem (and \l QContactItemModel), views of arbitrary collections of
  contacts can be created.  This can be useful when choosing arbitrary contacts (for
  example, selecting recipients for a message), or when you would like to manually
  filter a \l QContactModel.

  This class is designed to operate with \l QContactDelegate.

  \sa QContactItemModel, QStandardItem
*/

/*!
  Create an empty QContactItem, with no label, portrait,
  secondary label, or status icon.
*/
QContactItem::QContactItem()
    : QStandardItem()
{
}

/*!
  Create a QContactItem based on the supplied \a contact and secondary label \a subLabel, and
  secondary decoration \a statusIcon.

  The \a contact's portrait will be used as the decoration for this item, and if QContactItem
  is being rendered by \l QContactDelegate, the additional \a subLabel and \a statusIcon will
  also be rendered.

  If \a subLabel is not empty, the supplied string will be rendered with \l QContactDelegate
  underneath the \a contact label. If \a subLabel is empty, a default value will be generated
  from \a contact in a similar manner to QContactModel::SubLabelRole.

  If it is valid, the \a statusIcon pixmap will be rendered on the trailing edge of the item.
*/
QContactItem::QContactItem(const QContact &contact, const QString &subLabel, const QPixmap &statusIcon)
    : QStandardItem()
{
    setLabel(contact.label());
    if (subLabel.isEmpty()) {
        if (!contact.defaultPhoneNumber().isEmpty())
            setSubLabel( Qt::escape(contact.defaultPhoneNumber()) );
        if (!contact.defaultEmail().isEmpty())
            setSubLabel( Qt::escape(contact.defaultEmail()) );
        if (contact.label() != contact.company())
            setSubLabel( Qt::escape(contact.company()) );
    } else {
        setSubLabel(subLabel);
    }
    setPortrait(contact.thumbnail());
    setStatusIcon(statusIcon);
}

/*!
  Destroys the QContactItem.
*/
QContactItem::~QContactItem()
{}

/*!
  Return the contact's portrait (QContactModel::PortraitRole) as
  a pixmap.

  \sa setPortrait()
*/
QPixmap QContactItem::portrait() const
{
    return qvariant_cast<QPixmap>(data(QContactModel::PortraitRole));
}

/*!
  Return the contact's additional status icon (QContactModel::StatusIconRole) as
  a pixmap.

  \sa setStatusIcon()
*/
QPixmap QContactItem::statusIcon() const
{
    return qvariant_cast<QPixmap>(data(QContactModel::StatusIconRole));
}

/*!
  Set the text displayed by this item to \a text.
  This corresponds to Qt::DisplayRole and QContactModel::LabelRole.

  \sa label()
*/
void QContactItem::setLabel(const QString &text)
{
    setData(text, Qt::DisplayRole);
    setData(text, QContactModel::LabelRole);
}

/*!
  Set the pixmap displayed by this item to \a image.
  This corresponds to Qt::DecorationRole and QContactModel::PortraitRole.

  \sa portrait()
*/
void QContactItem::setPortrait(const QPixmap &image)
{
    setData(image, Qt::DecorationRole);
    setData(image, QContactModel::PortraitRole);
}

/*!
  Set the secondary label displayed by this item to \a text.
  This corresponds to QContactModel::SubLabelRole

  \sa subLabel()
*/
void QContactItem::setSubLabel(const QString &text)
{
    setData(text, QContactModel::SubLabelRole);
}

/*!
  Set the additional status icon displayed by this item to \a image.
  This corresponds to QContactModel::StatusIconRole and
  Qtopia::AdditionalDecorationRole.

  \sa statusIcon()
*/
void QContactItem::setStatusIcon(const QPixmap &image)
{
    setData(image, Qtopia::AdditionalDecorationRole);
    setData(image, QContactModel::StatusIconRole);
}

/*!
  Returns the label displayed by this item.
  \sa setLabel(), subLabel()
*/
QString QContactItem::label() const
{
    return data(QContactModel::LabelRole).toString();
}

/*!
  Returns the secondary label displayed by this item.
  \sa setSubLabel(), label()
*/
QString QContactItem::subLabel() const
{
    return data(QContactModel::SubLabelRole).toString();
}

/*!
  \class QContactItemModel
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactItemModel class provides a QStandardItemModel based class representing a list of \l{QContact}s.

  By using QContactItemModel and (\l QContactItem), views of arbitrary collections of contacts can be
  created.  This class provides convenience functions that operate on \l {QContact}s on top of the QStandardItemModel API.

  \sa QContactItem, QStandardItemModel
*/

/*!
  Create a QContactItemModel with the specified \a parent.
*/
QContactItemModel::QContactItemModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

/*!
  Destroys this QContactItemModel.
*/
QContactItemModel::~QContactItemModel()
{
}

/*!
  This is a convenience function.
  Add a contact to this model.  The \a contact, \a subLabel and \a statusIcon
  parameters are used to create a \l QContactItem, and the item is then
  added to this model.

  \sa QContactItem
*/
void QContactItemModel::appendRow(const QContact &contact, const QString &subLabel, const QPixmap &statusIcon)
{
    QContactItem *item = new QContactItem(contact, subLabel, statusIcon);
    appendRow(item);
}

/*!
  Returns a list of the labels of the items in this model.

  This corresponds to the item's QContactModel::LabelRole data.

  \sa subLabels()
*/
QStringList QContactItemModel::labels() const
{
    QStringList result;
    for(int i = 0; i < rowCount(); ++i)
        result.append(item(i)->data(QContactModel::LabelRole).toString());
    return result;
}

/*!
  Returns a list of the secondary labels of the items in this model.

  This corresponds to the item's QContactModel::SubLabelRole data.

  \sa labels()
*/
QStringList QContactItemModel::subLabels() const
{
    QStringList result;
    for(int i = 0; i < rowCount(); ++i)
        result.append(item(i)->data(QContactModel::SubLabelRole).toString());
    return result;
}

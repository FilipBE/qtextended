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

#include "addressselectorwidget_p.h"
#include "qcollectivenamespace.h"
#include <QContactModel>
#include <QContactListView>
#include <QFieldDefinition>
#ifdef QTOPIA_HOMEUI
#include <private/homewidgets_p.h>
#endif
#include <QVBoxLayout>
#include <QtopiaItemDelegate>
#include <QtopiaApplication>
#include <QMailAddress>
#include <QKeyEvent>
#include <QSmoothContactListView>
#include <QSmoothList>
#include <QSoftMenuBar>
#include <QLabel>
#include <QLineEdit>
#include <QTextEntryProxy>
#include <QToolButton>

typedef QPair<QString,QString> NameAddressPair;
static QString URISchemePattern("^(([^:/?#]+):)");
static QString absoluteURIPattern(URISchemePattern + "(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?");

#ifdef QTOPIA_HOMEUI
class CheckableContactDelegate : public QtopiaItemDelegate
{
    Q_OBJECT;
public:
    CheckableContactDelegate(QWidget *parent);

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    virtual void getInfo(const QModelIndex& index, QPixmap&, QColor&, QString&, QString&) const;
    QFont mNameFont;
    QWidget *mParent;
};

CheckableContactDelegate::CheckableContactDelegate(QWidget *parent)
    :
    QtopiaItemDelegate(parent),
    mParent(parent)
{
    QFont f = parent->font();
    mNameFont = f;
    mNameFont.setWeight(80);
}

void CheckableContactDelegate::getInfo(const QModelIndex& index, QPixmap& pm, QColor& frameColor, QString& mainText, QString& subText) const
{
    const QAbstractItemModel *m = index.model();
    if (m) {
        mainText = m->data(index, Qt::DisplayRole).toString();
        subText = m->data(index.sibling(index.row(), QContactModel::Company), Qt::DisplayRole).toString();
        QIcon i = qvariant_cast<QIcon>(m->data(index, Qt::DecorationRole));
        pm = i.pixmap(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight);
        QPresenceTypeMap map = m->data(index.sibling(index.row(), QContactModel::PresenceStatus), Qt::DisplayRole).value<QPresenceTypeMap>();
        frameColor = QtopiaHome::presenceColor(QtopiaHome::chooseBestPresence(map));
    } else {
        mainText.clear();
        subText.clear();
        pm = QPixmap();
        frameColor = Qt::transparent;
    }
}

QSize CheckableContactDelegate::sizeHint(const QStyleOptionViewItem& , const QModelIndex& index) const
{
    QString name;
    QString company;
    QPixmap pm;
    QColor presence;

    getInfo(index, pm, presence, name, company);
    QSize iconSize(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight);
    QSize infoSize = ContactLabelPainter::sizeHint(iconSize, name, mNameFont, company, mParent->font());
    return infoSize;
}

void CheckableContactDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);
    QString name;
    QString company;
    QPixmap pm;
    QColor presence;
    getInfo(index, pm, presence, name, company);

    //checkmark
    QRect checkRect = option.rect;
    QRect textRect = option.rect;
    QFontMetrics mainMetrics(option.font);
    const int checkReduction = 4;
    const int checkSpacing = 2;
    const int checkSize = mainMetrics.lineSpacing() - checkReduction;
    checkRect.setTop(checkRect.top() + ((checkRect.height() - checkSize) / 2));
    checkRect.setHeight(checkSize);
    if (rightToLeftMode) {
        checkRect.setLeft(checkRect.right() - checkSize);
        textRect.setRight(checkRect.left() - checkSpacing);
    } else {
        checkRect.setRight(checkRect.left() + checkSize);
        textRect.setLeft(checkRect.right() + checkSpacing);
    }

    QtopiaItemDelegate::drawBackground(painter, option, index);
    QtopiaItemDelegate::drawCheck(painter, option, checkRect, static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt()));
    QSize iconSize(ContactLabelPainter::kIconWidth, ContactLabelPainter::kIconHeight);
    ContactLabelPainter::paint(*painter,textRect, pm, iconSize, presence, name, mNameFont, company, mParent->font(), checkSpacing, checkSize+checkSpacing);
}
#endif //QTOPIA_HOMEUI

class CheckableContactModel : public QContactModel
{
public:
    CheckableContactModel(AddressSelectorWidget* parent);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    QList<NameAddressPair> selectedRows() const;
    QStringList explictAddressList() const;
    void setExplicitAddressList(const QStringList& addresses);
    void clearCheckedRows();

private:
    Qt::CheckState determineCheckState(const QModelIndex& index) const;

private:
    mutable QMap<int,NameAddressPair> m_checkedRows;
    mutable QStringList m_explicitAddressList;
    AddressSelectorWidget* m_parent;
};

CheckableContactModel::CheckableContactModel(AddressSelectorWidget* parent)
    :
    QContactModel(parent),
    m_parent(parent)
{
}

QVariant CheckableContactModel::data(const QModelIndex& index, int role) const
{
    if(role == Qt::CheckStateRole && index.isValid())
        return determineCheckState(index);
    else
        return QContactModel::data(index,role);
}

static QStringList extractIMAddresses(const QStringList& source)
{
    QStringList results;
    foreach(const QString& addressString, source)
    {
        QMailAddress address(addressString);
        if(address.isChatAddress())
            results.append(address.address());
    }
    return results;
}

static QString extractIMAddress(const QContact& c)
{
    QStringList fields = QContactFieldDefinition::fields("chat");
    foreach (const QString& field, fields) {
        QContactFieldDefinition def(field);
        QString address = def.value(c).toString();
        if (!address.isEmpty())
            return QCollective::encodeUri(def.provider(), address);
    }

    return QString();
}

bool CheckableContactModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(role == Qt::CheckStateRole && index.isValid())
    {
        if(value.toUInt() == Qt::Checked && !m_checkedRows.contains(index.row()))
        {
            NameAddressPair info;
            QContact contact = this->contact(index.row());
            info.first = contact.label();
            switch(m_parent->selectionMode())
            {
                case AddressSelectorWidget::EmailSelection:
                    info.second = contact.defaultEmail();
                break;
                case AddressSelectorWidget::InstantMessageSelection:
                    info.second = extractIMAddress(contact);
                break;
                case AddressSelectorWidget::PhoneSelection:
                    info.second = contact.defaultPhoneNumber();
                break;
            }
            if(!info.second.isEmpty())
                m_checkedRows.insert(index.row(),info);
        }
        else
            m_checkedRows.remove(index.row());

        emit dataChanged(index,index);
        return true;
    }
    else return QContactModel::setData(index,value,role);
}

bool CheckableContactModel::removeRows(int row, int count, const QModelIndex& parent)
{
    for(int index = row; index <= row+count; index++)
        m_checkedRows.remove(index);
    return QContactModel::removeRows(row,count,parent);
}

QList<NameAddressPair> CheckableContactModel::selectedRows() const
{
    QList<NameAddressPair> results;

    QMap<int,NameAddressPair>::const_iterator itr = m_checkedRows.begin();
    while(itr != m_checkedRows.end())
    {
        results.append(itr.value());
        itr++;
    }
    return results;
}

QStringList CheckableContactModel::explictAddressList() const
{
    return m_explicitAddressList;
}

void CheckableContactModel::setExplicitAddressList(const QStringList& addresses)
{
    clearCheckedRows();
    m_explicitAddressList = addresses;
}

void CheckableContactModel::clearCheckedRows()
{
    m_checkedRows.clear();
    reset();
}

Qt::CheckState CheckableContactModel::determineCheckState(const QModelIndex& index) const
{
    if (m_checkedRows.contains(index.row())) {
        return Qt::Checked;
    } else {
        QContact contact = QContactModel::contact(index);

        QString address;
        switch(m_parent->selectionMode())
        {
            case AddressSelectorWidget::EmailSelection:
                address = contact.defaultEmail();
            break;
            case AddressSelectorWidget::InstantMessageSelection:
                address = extractIMAddress(contact);
            break;
            case AddressSelectorWidget::PhoneSelection:
                address = contact.defaultPhoneNumber();
            break;
        }
        if(m_explicitAddressList.contains(address,Qt::CaseInsensitive))
        {
            //add the item into the map
            NameAddressPair info;
            info.first = contact.label();
            info.second = address;
            m_checkedRows.insert(index.row(),info);
            m_explicitAddressList.removeAll(address);
            return Qt::Checked;
        }
    }
    return Qt::Unchecked;
}

class AddressListWidget : public QSmoothList
{
    Q_OBJECT
public:
    AddressListWidget(QWidget* parent = 0):QSmoothList(parent){};

protected:
    void keyPressEvent(QKeyEvent* e);
};

void AddressListWidget::keyPressEvent(QKeyEvent* e)
{
    switch( e->key() ) {
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Select:
    case Qt::Key_Enter:
    {
        if (currentIndex().isValid())
            emit clicked(currentIndex());
    }
    break;
    default:  QSmoothList::keyPressEvent( e );
    }
}

class AddressFilterWidget : public QWidget
{
    Q_OBJECT

public:
    AddressFilterWidget(AddressListWidget* listWidget);

signals:
    void filterTextChanged(const QString& text);

public slots:
    void clear();

private slots:
    void updateClearButton(const QString& text);

private:
    QToolButton* m_clearButton;
    QTextEntryProxy* m_filterEntry;
    QLineEdit* m_filterEdit;
};

AddressFilterWidget::AddressFilterWidget(AddressListWidget* listWidget)
    :
    QWidget(listWidget),
    m_clearButton(0),
    m_filterEntry(0),
    m_filterEdit(0)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    if(!style()->inherits("QThumbStyle"))
    {
        QtopiaApplication::setInputMethodHint(listWidget, "text");
        listWidget->setAttribute(Qt::WA_InputMethodEnabled);

        m_filterEntry = new QTextEntryProxy(this, listWidget);

        int mFindHeight = m_filterEntry->sizeHint().height();
        QLabel *findIcon = new QLabel;
        findIcon->setPixmap(QIcon(":icon/find").pixmap(mFindHeight-2, mFindHeight-2));
        findIcon->setMargin(2);
        findIcon->setFocusPolicy(Qt::NoFocus);

        layout->addWidget(findIcon);
        layout->addWidget(m_filterEntry);
        connect(m_filterEntry, SIGNAL(textChanged(QString)), this, SIGNAL(filterTextChanged(QString)));
    }
    else
    {
        m_filterEdit = new QLineEdit(this);
        m_filterEdit->setFocus();
        m_filterEdit->selectAll();
        connect(m_filterEdit, SIGNAL(textChanged(QString)), this, SIGNAL(filterTextChanged(QString)));

        int mFindHeight = m_filterEdit->sizeHint().height();
        QLabel *findIcon = new QLabel;
        findIcon->setPixmap(QIcon(":icon/find").pixmap(mFindHeight-2, mFindHeight-2));
        findIcon->setMargin(2);
        findIcon->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(findIcon);
        layout->addWidget(m_filterEdit);

        m_clearButton = new QToolButton(this);
        m_clearButton->setText("Clear");
        m_clearButton->setVisible(false);
        layout->addWidget(m_clearButton);
        connect(m_clearButton,SIGNAL(clicked()),this,SLOT(clear()));

        connect(this,SIGNAL(filterTextChanged(QString)),this,SLOT(updateClearButton(QString)));
    }
}

void AddressFilterWidget::clear()
{
    if(m_filterEdit) m_filterEdit->clear();
    else if(m_filterEntry) m_filterEntry->clear();
}

void AddressFilterWidget::updateClearButton(const QString& text)
{
    bool showClearButton = !text.isEmpty();
    m_clearButton->setVisible(showClearButton);
}

AddressSelectorWidget::AddressSelectorWidget(SelectionMode mode, QWidget* parent)
    :
    QWidget(parent),
    m_selectionMode(mode),
    m_contactListView(0),
    m_contactModel(0),
    m_filterWidget(0)
{
    init();
}

AddressSelectorWidget::SelectionMode AddressSelectorWidget::selectionMode() const
{
    return m_selectionMode;
}

void AddressSelectorWidget::setSelectionMode(SelectionMode mode)
{
    m_selectionMode = mode;

    switch(mode)
    {
        //TODO filter contacts with IM address types, for now just add the whole list as is
        case InstantMessageSelection:
            m_contactModel->setFilter(QString(),QContactModel::ContainsChat);
        break;
        case PhoneSelection:
            m_contactModel->setFilter(QString(),QContactModel::ContainsPhoneNumber);
        break;
        case EmailSelection:
            m_contactModel->setFilter(QString(),QContactModel::ContainsEmail);
        break;
    }
    m_contactModel->clearCheckedRows();
}

QStringList AddressSelectorWidget::selectedAddresses() const
{
    //Because the checkedRows map is lazily populated from the explict list,
    //there may be explict addresses not yet checked that otherwise would be when painted,
    //therefore return both the explict list contatenated with checkedRows addresses.

    QStringList addresses = m_contactModel->explictAddressList();

    foreach(const NameAddressPair& p, m_contactModel->selectedRows())
        addresses.append(QMailAddress(p.first, p.second).toString());

    return addresses;
}

static QStringList extractAddresses(const QStringList& source, const QString& pattern)
{
    QRegExp regExp(pattern,Qt::CaseInsensitive);
    QStringList addressList;
    foreach(const QString& s, source)
    {
        if(s.contains(regExp))
        {
            //only extract the address
            regExp.indexIn(s);
            addressList.append(regExp.cap(0));
        }
    }
    return addressList;
}

void AddressSelectorWidget::setSelectedAddresses(const QStringList& addressString)
{
    reset();
    QString pattern;
    switch(m_selectionMode)
    {
        case EmailSelection:
            pattern = QMailAddress::emailAddressPattern();
        break;
        case PhoneSelection:
            pattern = QMailAddress::phoneNumberPattern();
        break;
        case InstantMessageSelection:
            //pattern = absoluteURIPattern;
            m_contactModel->setExplicitAddressList(extractIMAddresses(addressString));
            return;
        break;
    }
    m_contactModel->setExplicitAddressList(extractAddresses(addressString,pattern));
}

void AddressSelectorWidget::reset()
{
    m_contactModel->clearCheckedRows();
}

bool AddressSelectorWidget::showingFilter() const
{
    return m_filterWidget->isVisible();
}

void AddressSelectorWidget::setShowFilter(bool val)
{
    m_filterWidget->setVisible(val);
}

void AddressSelectorWidget::currentChanged(const QModelIndex& current, const QModelIndex&)
{
    if (current.isValid()) {
        m_currentIndex = current;
        updateLabel();
    }
}

void AddressSelectorWidget::indexClicked(const QModelIndex& index)
{
    bool checked(static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt()) == Qt::Checked);
    m_contactModel->setData(index, static_cast<int>(checked ? Qt::Unchecked : Qt::Checked), Qt::CheckStateRole);

    updateLabel();
}

void AddressSelectorWidget::filterTextChanged(const QString& filterText)
{
    QContactModel::FilterFlags f = QContactModel::ContainsEmail;
    switch(m_selectionMode)
    {
        case EmailSelection:
        f = QContactModel::ContainsEmail;
        break;
        case InstantMessageSelection:
        f = QContactModel::ContainsChat;
        break;
        case PhoneSelection:
        f = QContactModel::ContainsPhoneNumber;
        break;
    }
    m_contactModel->setFilter(filterText,f);
}

void AddressSelectorWidget::init()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    m_contactModel = new CheckableContactModel(this);

    m_contactListView = new AddressListWidget(this);
    connect(m_contactListView,SIGNAL(currentChanged(QModelIndex, QModelIndex)),this,SLOT(currentChanged(QModelIndex, QModelIndex)));
    connect(m_contactListView,SIGNAL(clicked(QModelIndex)),this,SLOT(indexClicked(QModelIndex)));

    m_contactListView->setModel(m_contactModel);
#ifdef QTOPIA_HOMEUI
    m_contactListView->setItemDelegate(new CheckableContactDelegate(m_contactListView));
    m_contactListView->setFocusPolicy(Qt::NoFocus);
#endif
    m_contactListView->setEmptyText(tr("No valid contacts"));
    layout->addWidget(m_contactListView);

    m_filterWidget = new AddressFilterWidget(m_contactListView);
    connect(m_filterWidget,SIGNAL(filterTextChanged(QString)),this,SLOT(filterTextChanged(QString)));
    layout->addWidget(m_filterWidget);

    setShowFilter(true);
    setSelectionMode(m_selectionMode);
}

void AddressSelectorWidget::showEvent(QShowEvent* e)
{
    updateLabel();
    QWidget::showEvent(e);
}

void AddressSelectorWidget::updateLabel()
{
    if (m_currentIndex.isValid()) {
        bool checked(static_cast<Qt::CheckState>(m_currentIndex.data(Qt::CheckStateRole).toInt()) == Qt::Checked);
        QSoftMenuBar::setLabel(this, Qt::Key_Select, (checked ? QSoftMenuBar::Deselect : QSoftMenuBar::Select));
    }
}

#include <addressselectorwidget.moc>


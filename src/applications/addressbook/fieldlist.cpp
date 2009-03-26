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

#include "fieldlist.h"
#include "qfielddefinition.h"
#include <QIconSelector>
#include <QGridLayout>
#include <QtopiaApplication>
#include <QSoftMenuBar>
#include <QAction>
#include <QMenu>
#include <QDebug>

typedef QPair< FieldIconSelector *, FieldLineEdit * > FieldPair;

/*
  Returns true if the given \a list contains an equivalent field to the
  given \a field.
*/
class QContactFieldListData
{
public:
    QGridLayout *layout;

    QContact entry;
    QList< FieldPair > activeFields;

    // TODO too much info stored.  Memory usage could be better.
    QStringList commonFields;
    QStringList allowedFields;
    QStringList selectableFields;
    QStringList suggestedFields;

    QAction *actionRemoveField;
    QMap<QString, QAction *> actionMap;
};

/*
  \class QContactFieldList
    \inpublicgroup QtPimModule
  \brief The QContactFieldList widget displays a list of editors that is adjusted
  to show the most relevant fields for a contact.

  The editors displayed are pairs of QIconSelectors and QLineEdits.  
  The type of fields that can be edited is set using the setAllowedFields()
  function.  The type of Fields that can be selected for new fields is determined
  by the existing set of fields already set for the contact given by setEntry() that
  are both non empty and within the set of allowed fields.  This set of selectable
  fields is extended by using the setCommonFields() functions to provide a set
  of fields the user is likely to want to set.

  Initially, all of the suggestedFields() will be shown, regardless of if they are empty.
  As long not all allowed fields are already shown, an empty line edit will be added
  any time the existing empty line edit gains text.
  If an editor loses focus while empty, it will be removed for the list of editors unless it
  is the last empty line edit or one of the suggestedFields().

  // TODO need graphic/table to explain the relationship of allowed, entry, and common fields.
*/

/*!
  Constructs a new field list
*/
QContactFieldList::QContactFieldList(QWidget *parent)
    : QWidget(parent), d(new QContactFieldListData)
{
    d->layout = new QGridLayout;
    setLayout(d->layout);

    d->actionRemoveField = new QAction(QIcon(":icon/trash"), tr("Remove"), this);
    connect(d->actionRemoveField, SIGNAL(triggered()), this, SLOT(removeCurrentEdit()));
}

/*!
  Destroys the field list.
*/
QContactFieldList::~QContactFieldList()
{
}

/*!
  Set the suggested set of fields for the field list to the given list
  of \a fields.  Any fields not already in the list of allowed fields
  or common fields for the contact will be added to those lists.

  The suggested fields are always shown, even if they are empty.

  \sa setEntry()
*/
void QContactFieldList::setSuggestedFields(const QStringList &fields)
{
    d->suggestedFields = fields;
    foreach(QString field, fields)
    {
        if (!d->allowedFields.contains(field))
            d->allowedFields.append(field);
        if (!d->commonFields.contains(field))
            d->commonFields.append(field);
    }
    updateSelectableFields();
}

/*!
  Set the common set of fields for the field list to the given list
  of \a fields.  Any fields not already in the list of allowed fields
  for the contact will be added to the list of allowed fields.

  The set of options displayed when selecting field types in the widget
  is the combined set of common fields, and those fields that are
  allowed and non-null in the current contact entry.

  \sa setAllowedFields(), setEntry()
*/
void QContactFieldList::setCommonFields(const QStringList &fields)
{
    d->commonFields = fields;
    foreach(QString field, fields)
    {
        if (!d->allowedFields.contains(field))
            d->allowedFields.append(field);
    }
    updateSelectableFields();
}

/*!
  Sets the allowed set of fields for the field list to the given list of 
  \a fields.  Any fields in the list of common fields that are not in the
  given list of \a fields will be removed from the list of common fields.

  The set of options displayed when selecting field types in the widget
  is the combined set of common fields, and those fields that are
  allowed and non-null in the current contact entry.

  \sa setCommonFields(), setEntry()
*/
void QContactFieldList::setAllowedFields(const QStringList &fields)
{
    d->allowedFields = fields;
    QMutableListIterator<QString> it(d->commonFields);
    while(it.hasNext())
    {
        QString field = it.next();
        if (!d->allowedFields.contains(field))
            it.remove();
    }
    updateSelectableFields();
}

/*!
  Returns the set of fields set as suggested to the field list.
*/
QStringList QContactFieldList::suggestedFields() const
{
    return d->suggestedFields;
}

/*!
  Returns the set of fields set as common to the field list.
*/
QStringList QContactFieldList::commonFields() const
{
    return d->commonFields;
}

/*!
  Returns the set of fields allowed to be displayed in the field list.
*/
QStringList QContactFieldList::allowedFields() const
{
    return d->allowedFields;
}

/*!
  Returns the set of fields displayed when selecting field types in the
  widget.

  The set of options displayed when selecting field types in the widget
  is the combined set of common fields, and those fields that are
  allowed and non-null in the current contact entry.
*/
QStringList QContactFieldList::selectableFields() const
{
    return d->selectableFields;
}

void QContactFieldList::setEntry(const QContact &entry, bool newEntry)
{
    Q_UNUSED(newEntry);
    d->entry = entry;
    updateSelectableFields();
}

/*!
  Updates the set of selectable fields.

  \sa selectableFields()
*/
void QContactFieldList::updateSelectableFields()
{
    // TODO.  This is expensive, and called more often than needed.
    // Its quite likely that this will need to be set up to delay
    // until show or some other similar 'just-in-time' calculation.
    clearEdits();

    // Set set of selectable fields.  Need to do this before
    // adding edits so that the icon selectors are complete.
    d->selectableFields = d->commonFields;
    foreach(QString field, d->allowedFields)
    {
        QContactFieldDefinition def(field);
        QString value = def.value(d->entry).toString();
        if (!value.isEmpty())
            if (!d->selectableFields.contains(field))
                d->selectableFields.append(field);
    }

    bool emptydone = false;

    // Add edits for those fields already set in the contact.
    foreach(QString field, d->allowedFields)
    {
        QContactFieldDefinition def(field);
        QString value = def.value(d->entry).toString();
        if (!value.isEmpty() || d->suggestedFields.contains(field)) {
            addEdit(field, value);
            if (value.isEmpty() && d->suggestedFields.contains(field))
                emptydone = true;
        }
    }

    if (!emptydone)
        addEmptyEdit();

    d->layout->activate();
}

/*!
  If the last line edit has non empty content adds a new edit pair
  using the first unshown selectable field.
*/
void QContactFieldList::addEmptyEdit()
{
    QStringList candidates = d->selectableFields;
    foreach (FieldPair pair, d->activeFields)
        candidates.removeAll(pair.second->field());

    if (!candidates.isEmpty())
        addEdit(candidates.first(), QString());
}

/*!
  \internal

  Removes all editor pairs from the widget, including the empty pair.
*/
void QContactFieldList::clearEdits()
{
    foreach(FieldPair pair, d->activeFields)
    {
        d->layout->removeWidget(pair.first);
        d->layout->removeWidget(pair.second);
        pair.first->hide();
        pair.first->deleteLater();
        pair.second->hide();
        pair.second->deleteLater();

        emit fieldChanged(pair.second->field(), QString());
    }
    d->activeFields.clear();
}

bool QContactFieldList::isEmpty() const
{
    foreach(FieldPair pair, d->activeFields)
    {
        if (!pair.second->text().isEmpty())
            return false;
    }
    return true;
}

/*!
  \internal

  Adds a new editor pair for the given \a field.  It will have the text for
  the line editor set to the given \value. This new editor pair will be at
  the bottom of the list of editors.
*/
void QContactFieldList::addEdit(const QString &field, const QString &value)
{
    FieldPair pair;
    pair.first = new FieldIconSelector;
    pair.second = new FieldLineEdit(field);
    pair.first->setFields(d->selectableFields);
    pair.first->setCurrentField(field);
    pair.second->setText(value);

    QMenu *contextMenu = QSoftMenuBar::menuFor(pair.second, QSoftMenuBar::AnyFocus);
    contextMenu->addAction(tr("Type"), pair.first, SIGNAL(clicked()));
    contextMenu->addAction(d->actionRemoveField);

    QSet<QString> actionIds;
    foreach (QString possibleField, d->allowedFields)
    {
        QContactFieldDefinition def(possibleField);
        actionIds |= QSet<QString>::fromList(def.editActions());
    }

    foreach(QString actionId, actionIds)
    {
        QAction *action;
        if (!d->actionMap.contains(actionId)) {
            QString label = QContactFieldDefinition::actionLabel(actionId);
            QIcon icon = QContactFieldDefinition::actionIcon(actionId);
            action = new QAction(icon, label, this);
            connect(action, SIGNAL(triggered()),
                    this, SLOT(forwardFieldAction()));
            d->actionMap.insert(actionId, action);
        } else {
            action = d->actionMap[actionId];
        }
        contextMenu->addAction(action);
    }


    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

    // grid layout never 'shrinks'.  Hence it never loses rows.
    // hence even if we want screen row 1, we have to get grid
    // layout row 12 if there has been some previous removes that
    // bumps the row count up that high.
    int rowCount = d->layout->rowCount();
    d->layout->addWidget(pair.first, rowCount, 0);
    d->layout->addWidget(pair.second, rowCount, 1);
    // and back to the real row count without grid layout's nuttiness
    rowCount = d->activeFields.count();
    if (rowCount > 0)
        setTabOrder(d->activeFields[rowCount-1].second, pair.second);

    connect(pair.first, SIGNAL(fieldActivated(QString)),
        this, SLOT(changeEditField(QString)));

    connect(pair.second, SIGNAL(textChanged(QString,QString)),
            this, SIGNAL(fieldChanged(QString,QString)));

    connect(pair.second, SIGNAL(textChanged(QString)),
            this, SLOT(maintainVisibleEdits()));

    d->activeFields.append(pair);
}

void QContactFieldList::updateContextMenu()
{
    FieldLineEdit *le = qobject_cast<FieldLineEdit *>(focusWidget());
    if (le) {
        d->actionRemoveField->setVisible(!le->text().isEmpty());
        // and the tr.
        QContactFieldDefinition def(le->field());

        QStringList actionIds = def.editActions();

        QMapIterator<QString, QAction *> it(d->actionMap);
        while(it.hasNext()) {
            it.next();
            it.value()->setVisible(actionIds.contains(it.key()));
        }

        if (def.inputHint() == "phone" || def.inputHint() == "number")
        {
            d->actionRemoveField->setText(tr("Remove Number"));
        } else {
            d->actionRemoveField->setText(tr("Remove Field"));
        }
        // also a number of field specific items?
    } else {
        d->actionRemoveField->setVisible(false);
        QMapIterator<QString, QAction *> it(d->actionMap);
        while(it.hasNext()) {
            it.next();
            it.value()->setVisible(false);
        }
    }
}

/*!
  If last edit pair is non empty and any selectable fields are
  not currently shown, add a new empty edit pair with a new field
  definition drawn from the selectable fields.
*/
void QContactFieldList::maintainVisibleEdits()
{
    if (d->activeFields.count() && !d->activeFields.last().second->text().isEmpty())
    {
        addEmptyEdit();
        d->layout->activate();
    }
}


/*!
  \internal

  Removes the editor pair at the given \a position from the list of editor pairs.
*/
void QContactFieldList::removeEdit(int position)
{
    FieldPair pair = d->activeFields.at(position);

    d->layout->removeWidget(pair.first);
    d->layout->removeWidget(pair.second);
    pair.first->hide();
    pair.first->deleteLater();
    pair.second->hide();
    pair.second->deleteLater();

    emit fieldChanged(pair.second->field(), QString());

    d->activeFields.removeAt(position);
}

void QContactFieldList::removeCurrentEdit()
{
    foreach (FieldPair pair, d->activeFields) {
        if (pair.second == QApplication::focusWidget())
        {
            d->layout->removeWidget(pair.first);
            d->layout->removeWidget(pair.second);
            pair.first->hide();
            pair.first->deleteLater();
            pair.second->hide();
            pair.second->deleteLater();

            emit fieldChanged(pair.second->field(), QString());

            d->activeFields.removeAll(pair);
            break;
        }
    }
    maintainVisibleEdits();
}

/*!
  Applies the set values for the allowed fields of the field list
  to a copy of the given \a entry and returns the modify contact.
*/
QContact QContactFieldList::updateEntry(const QContact &entry) const
{
    // TODO, badly named function. doesn't set this entry, doesn't update
    // the passed entry.  
    QContact result = entry;
    foreach(QString field, d->allowedFields)
    {
        QString value;
        foreach(FieldPair pair, d->activeFields)
            if (pair.second->field() == field) {
                value = pair.second->text();
                break;
            }
        QContactFieldDefinition def(field);
        def.setValue(result, value);
    }
    return result;
}

/*!
  \internal
  Changes the field type for the editor pair determined by the sender()
  to the given \a field.  If a separate editor pair already have the
  same field, swaps that field with the old field for the pair determined
  by sender().
*/
void QContactFieldList::changeEditField(const QString &field)
{
    // s will be originating  QIconSelector.  
    QObject *s = sender();

    FieldLineEdit *target = 0;
    FieldLineEdit *swapLineEdit = 0;
    FieldIconSelector *swapIconSelector = 0;
    foreach(FieldPair pair, d->activeFields)
    {
        if (pair.first == s)
            target = pair.second;
        if (pair.second->field() == field) {
            swapIconSelector = pair.first;
            swapLineEdit = pair.second;
        }
    }
    // assert target exists;
    QString oldField = target->field();

    target->setField(field);
    if (swapIconSelector)
    {
        swapLineEdit->setField(oldField);
        swapIconSelector->blockSignals(true);
        swapIconSelector->setCurrentField(oldField);
        swapIconSelector->blockSignals(false);

        emit fieldChanged(oldField, swapLineEdit->text());
    } else {
        emit fieldChanged(oldField, QString());
    }

    emit fieldChanged(field, target->text());
}

/*!
  If there is already a visible line editor for the given \a field,
  set the text for that line editor to the given \a value.

  If no visible line editors have the given \a field and the last
  editor is empty, set that editor to have the given \a field and \a value.

  If the last line editor is set to non empty text and there are unshown
  selectable field types, this function will add a new empty line editor.

  \sa selectableFields()
*/
void QContactFieldList::setField(const QString &field, const QString &value)
{
    if (!d->allowedFields.contains(field))
        return;

    bool selectable = d->selectableFields.contains(field);
    if (!selectable)
        d->selectableFields.append(field);

    for (int i = 0; i < d->activeFields.count(); ++i)
    {
        FieldPair pair = d->activeFields[i];
        if (selectable)
        {
            if (pair.second->field() == field) {
                pair.second->setText(value);
                break;
            }
        } else {
            pair.first->addField(field);
        }

        if (i == d->activeFields.count()-1 && pair.second->text().isEmpty()) {
            pair.first->blockSignals(true);
            pair.second->blockSignals(true);

            pair.first->setCurrentField(field);
            pair.second->setField(field);
            pair.second->setText(value);

            pair.first->blockSignals(false);
            pair.second->blockSignals(false);

            addEmptyEdit();
            break;
        }
    }
}

/*!
  Returns the line editor text for the \a given field.
*/
QString QContactFieldList::field(const QString &field) const
{
    foreach(FieldPair pair, d->activeFields)
    {
        if (pair.second->field() == field)
            return pair.second->text();
    }
    return QString();
}


void QContactFieldList::forwardFieldAction()
{
    foreach(FieldPair pair, d->activeFields)
    {
        if (pair.second == focusWidget())
        {
            QAction *a = qobject_cast<QAction *>(sender());
            if (a) {
                QContactFieldDefinition def(pair.second->field());
                QString value = pair.second->text();
                QString actionId = d->actionMap.key(a);

                emit fieldActivated(actionId, value);
            }
            break;
        }
    }
}

//-------------------------------------------------------------

/*
  \class FieldLineEdit
    \inpublicgroup QtPimModule
  \brief The FieldLineEdit widget associates a field with a QLineEdit.

  It extends QLineEdit by adding signals that also provide the associated
  field and uses the field to determine the appropriate input hint to use.
*/

/*!
  Constructs a new FieldLineEdit associated with the given \a field with
  the given \a parent.
*/
FieldLineEdit::FieldLineEdit(const QString &field, QWidget *parent)
    : QLineEdit(parent), mField(field)
{
    connect(this, SIGNAL(textChanged(QString)),
            this, SLOT(q_textChanged(QString)));
    connect(this, SIGNAL(textEdited(QString)),
            this, SLOT(q_textEdited(QString)));
    QContactFieldDefinition def(field);
    QtopiaApplication::setInputMethodHint(this, def.inputHint());
}

/*!
  Destroys the FieldLineEdit.
*/
FieldLineEdit::~FieldLineEdit()
{
}

/*!
  Returns the field definition associated with the line edit.
*/
QString FieldLineEdit::field() const
{
    return mField;
}

/*!
  Associates the given \a field definition with the line edit.
*/
void FieldLineEdit::setField(const QString &field)
{
    mField = field;
    QContactFieldDefinition def(field);
    QtopiaApplication::setInputMethodHint(this, def.inputHint());
}

/*!
  \internal
  Forwards the textChanged signal with the additional field parameter.
*/
void FieldLineEdit::q_textChanged(const QString &value)
{
    emit textChanged(mField, value);
}

/*!
  \internal
  Forwards the textEdited signal with the additional field parameter.
*/
void FieldLineEdit::q_textEdited(const QString &value)
{
    emit textEdited(mField, value);
}

/*!
  If the field definition of the line edit is equal to the given
  \a field, set the text of the line edit to the given \a value.
*/
void FieldLineEdit::updateText(const QString &field, const QString &value)
{
    if (field == FieldLineEdit::field() && value != text())
        setText(value);
}

//-------------------------------------------------------------

/*
  \class FieldIconSelector
    \inpublicgroup QtPimModule
  \brief The FieldIconSelector widget adds field definitions to the items of
  a QIconSelector.

*/

/*!
  Constructs a new FieldIconSelector.
*/
FieldIconSelector::FieldIconSelector(QWidget *parent)
    : QIconSelector(parent)
{
    connect(this, SIGNAL(activated(int)),
            this, SLOT(q_fieldActivated(int)));
    setFocusPolicy(Qt::NoFocus);
}

/*!
  Destroys the FieldIconSelector.
*/
FieldIconSelector::~FieldIconSelector()
{
}

/*!
  Removes all items.
  Note that no icon will be visible
*/
void FieldIconSelector::clear()
{
    QIconSelector::clear();
    mFields.clear();
}

/*!
  Returns the list of field definitions displayed in the
  field icon selector.
*/
QStringList FieldIconSelector::fields() const
{
    return mFields;
}

/*!
  Sets the list of field definitions displayed in the
  field icon selector to the given list of \a fields.
*/
void FieldIconSelector::setFields(const QStringList fields)
{
    QString lastField = currentField();
    mFields = fields;
    QIconSelector::clear();
    int lastIndex = -1;
    for(int i = 0; i < mFields.count(); ++i) {
        QContactFieldDefinition def(mFields[i]);
        insertItem(def.icon(), def.label());
        if (def.id() == lastField)
            lastIndex = i;
    }
    if (lastIndex != -1)
        setCurrentIndex(lastIndex);
}

/*!
  Appends the given \a field to the list of field definitions
  displayed in the field icon selector.
*/
void FieldIconSelector::addField(const QString &field)
{
    if (!contains(field))
    {
        mFields.append(field);
        QContactFieldDefinition def(field);
        insertItem(def.icon(), def.label());
    }
}

/*!
  Returns true if the list of field definitions displayed in the 
  field icon selector include the given \a field.
*/
bool FieldIconSelector::contains(const QString &field) const
{
    for (int i= 0; i < mFields.count(); ++i) {
        if (field == mFields[i])
            return true;
    }
    return false;
}

/*!
  Returns the current field selected.  If no field is selected returns
  a null field definition.
*/
QString FieldIconSelector::currentField() const
{
    int index = currentIndex();

    if (index != -1 && index < int(count()))
        return mFields[currentIndex()];
    return QString();
}

/*!
  Sets the current field selected for the field icon selector to the given
  \a field.  If the given field is not in the set of displayed fields
  this function has no effect.

  \sa fields()
*/
void FieldIconSelector::setCurrentField(const QString &field)
{
    // set current index doesn't check, so we must.
    if (currentField() == field)
        return;

    // slightly harder.  Need to determine the right index first.
    for (int i = 0; i < mFields.count(); ++i) {
        if (field == mFields[i])
        {
            setCurrentIndex(i);
            break;
        }
    }
}

/*!
  \internal
  forwards the fieldActivated signal with the field definition for the
  displayed item at the given \a index.
*/
void FieldIconSelector::q_fieldActivated(int index)
{
    if (index < int(count()))
        emit fieldActivated(mFields[index]);
}

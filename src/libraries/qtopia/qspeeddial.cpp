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

#include "qspeeddial.h"

#include <qtopiaapplication.h>
#include <qexpressionevaluator.h>
#include <qsoftmenubar.h>
#include <QtopiaSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <qtranslatablesettings.h>
#include <QMessageBox>
#include <QInputDialog>
#include <QString>
#include <qtopiaserviceselector.h>
#include <QAbstractListModel>
#include <QtopiaItemDelegate>
#include <QDialog>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QDebug>
#include <QObject>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QKeyEvent>
#include <QtopiaStyle>
#include <QTimer>
#include <QWaitWidget>

class QSpeedDialModel : public QAbstractListModel
{
    Q_OBJECT
    friend class QSpeedDialList;
public:
    QSpeedDialModel(QObject* parent = 0);
    ~QSpeedDialModel();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    void setSelector(int startPos, const QString &label, const QString &icon);
    void moveSelector(int newPos);
    
private slots:
    void change();
private:
    mutable bool changed;
    mutable bool countChanged;
    mutable int rowCountCache;
    mutable QList<QString> displayCache;
    mutable QList<QString> iconCache;
    mutable QList<QString> inputCache;
    QString selectorLabel;
    QString selectorIcon;
    mutable int selectorPos;//Set to 0 if no selector item
    mutable int selectorRow;
    mutable bool shadowing;
};

class QSpeedDialItemDelegate : public QtopiaItemDelegate
{
    Q_OBJECT
public:
    QSpeedDialItemDelegate(QSmoothList* parent);
    ~QSpeedDialItemDelegate();

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:
    QSmoothList* parentList;
    int iconSize;
    int textHeight;
    int itemHeight;

    QIcon selIcon;
    QString selText;
};

class  QSpeedDialDialog : public QDialog
{
    Q_OBJECT
public:
    QSpeedDialDialog(QWidget *parent);
    QString inputChoice() const {return m_inputChoice;}
private slots:
    void store(const QModelIndex &selected);
private:
    QString m_inputChoice;
    QSpeedDialModel *model;
    QSpeedDialItemDelegate *delegate;
    QSmoothList *list;
};

class QSpeedDialAddDialog : public QDialog
{
    Q_OBJECT
public:
    QSpeedDialAddDialog(const QtopiaServiceRequest &req,
            const QString &label, const QString &icon, int input, QWidget *parent);
    QString inputChoice() const {return m_inputChoice;}
private slots:
    void store();
    void undoRemove();
private:
    QString m_inputChoice;
    QString m_prevInput;
    QtopiaServiceDescription m_desc;
    QSpeedDialModel *model;
    QSpeedDialList *list;
};

class QSpeedDialListPrivate
{
public:
    QSpeedDialListPrivate(QSpeedDialList* parent):selectorPos(0),input(0),
        parentList(parent), model(new QSpeedDialModel()),
        delegate(new QSpeedDialItemDelegate(parent)), serviceSelector(0)
        {
            inputTimer = new QTimer();
            QObject::connect(inputTimer,SIGNAL(timeout()), parent, SLOT(clearKeyInput()));
        }

    ~QSpeedDialListPrivate()
        {
            delete inputTimer;
        }

    int selectorPos;
    int input;
    QSpeedDialList *parentList;
    QTimer *inputTimer;
    QSpeedDialModel *model;
    QSpeedDialItemDelegate *delegate;
    QtopiaServiceSelector *serviceSelector;
};
/*!
  \internal
  Finds the first available input number
*/
int QSpeedDial::firstAvailableSlot()
{
    // possible slots - 1 ~ 99
    QList<QString> notAvailableStrings = QSpeedDial::assignedInputs();
    QList<int> notAvailable;
    bool ok=false;
    foreach(QString str, notAvailableStrings){
        int num = str.toInt(&ok);
        if(ok&&num)
            notAvailable<<num;
    }
    qSort(notAvailable);
    int acc=0;
    foreach(int i, notAvailable){
        if(i != ++acc)
            return acc;
    }
    //No gaps, but not all used
    if(++acc<100)
        return acc;
    // all slots are used.
    return 0;
}

/*!
  \class QSpeedDial
    \inpublicgroup QtBaseModule

  \brief The QSpeedDial class provides access to the Speed Dial settings.

  The QSpeedDial class includes a set of static functions that give access to the
  Speed Dial settings. This class should not be instantiated.

  The Speed Dial actions are actions from the Favorite Services, with an associated
  input for faster access.

  The input range is from 1 to 99. To allow the user to select
  an input for a given action, use addWithDialog().
  \image qspeeddial.png "Add with Dialog"

  To directly modify the Speed Dial settings, use remove() and set().
  Use find() to retrieve the assigned action for a given input or position.

  \sa QSpeedDialList, QFavoriteServicesModel

  \ingroup userinput
*/

/*!
  Provides a dialog that allows the user to select an input for action \a action,
  using \a label and \a icon as the display label and icon respectively.
  The dialog has the given \a parent.

  Returns the input that the user selected, and also assigns the input to the action.

  If the user cancels the dialog, a null string is returned.

  \sa set()
*/
QString QSpeedDial::addWithDialog(const QString& label, const QString& icon,
    const QtopiaServiceRequest& action, QWidget* parent)
{
    QString ret = QString();
    QSpeedDialAddDialog dlg(action, label,icon,qMax(firstAvailableSlot(),1),parent);
    dlg.setObjectName("speeddial");
    dlg.setWindowModality(Qt::WindowModal);
    if ( QtopiaApplication::execDialog(&dlg) ){
        ret = dlg.inputChoice();
        QSpeedDial::set(ret,QtopiaServiceDescription(action,label,icon));
    }
    return ret;
}

/*!
  Provides a dialog that allows the user to select an existing speed dial item.
  The dialog has the given \a parent.

  Similar to selectServiceWithDialog(), but returns the speed dial input selected
  instead of the service.

  If the user cancels the dialog, a null pointer is returned.
*/
QString QSpeedDial::selectWithDialog(QWidget* parent)
{
    QSpeedDialDialog dlg(parent);
    dlg.setObjectName("speeddial");
    dlg.setWindowModality(Qt::WindowModal);
    if ( QtopiaApplication::execDialog(&dlg) )
        return dlg.inputChoice();
    else
        return 0;
}


/*!
  Returns a QtopiaServiceDescription for the given Speed Dial \a input. If the
  input is not assigned, returns 0. The caller must \i not delete the
  QtopiaServiceDescription returned, nor rely on pointer validity after subsequent
  calls to QSpeedDial::find().
*/
QtopiaServiceDescription* QSpeedDial::find(const QString& input)
{
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if(!q.prepare("SELECT label,icon,service,message,arguments,optionalMap FROM favoriteservices WHERE speedDial=:input")){
        qWarning() << "Prepare Find speeddial failed:" << q.lastError().text();
        return 0;
    }
    q.bindValue("input",input);
    if(!q.exec()){
        qWarning() << "Exec Find speeddial failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
        return 0;
    }
    if(q.next()){
        QString label = q.value(0).toString();
        QString icon = q.value(1).toString();
        QString service = q.value(2).toString();
        QString message = q.value(3).toString();
        QByteArray args = q.value(4).toByteArray();

        QtopiaServiceRequest req(service, message);
        QtopiaServiceRequest::deserializeArguments(req, args);
        QByteArray map = q.value(5).toByteArray();
        QVariantMap optionalMap;
        if(!map.isEmpty()){
            QDataStream in(map);
            in >> optionalMap;
        }

        // Needs to be a pointer for compatibility with previous API.
        static QtopiaServiceDescription *desc= new QtopiaServiceDescription;
        *desc = QtopiaServiceDescription(req, label, icon, optionalMap);

        return desc;
    }
    return 0;
}

/*!
  Removes the action currently associated with the given Speed Dial \a input.
  If the action is in the favorites list, it will be deleted from there as well.
*/
void QSpeedDial::remove(const QString& input)
{
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if(!q.prepare(QLatin1String("DELETE FROM favoriteservices WHERE speedDial = :input"))){
        qWarning() << "Prepare remove speeddial failed:" << q.lastError().text();
        return;
    }
    q.bindValue("input",input);
    if(!q.exec()){
        qWarning() << "Exec remove speeddial failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
    }

    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
}

/*!
  Assigns the given QtopiaServiceDescription, \a r, as the action to perform when the given
  Speed Dial \a input, is detected. Will remove any previous services assigned to that \a input
  from the Speed Dial.
*/
void QSpeedDial::set(const QString& input, const QtopiaServiceDescription& r)
{

    if(input.isEmpty())
        return;

    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if(!q.prepare(QLatin1String("SELECT id FROM favoriteservices WHERE ") +
                QLatin1String("speedDial = :input"))){
        qWarning() << "Prepare set speeddial failed:" << q.lastError().text();
        return;
    }
    q.bindValue("input", input);
    if(!q.exec()){
        qWarning() << "Exec set speeddial failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
    }
    while(q.next()){
        QSqlQuery q2(db);
        if(!q2.prepare(QLatin1String("UPDATE favoriteservices SET speedDial = NULL WHERE id = :id"))){
            qWarning() << "Prepare set speeddial failed:" << q2.lastError().text();
            return;
        }
        q2.bindValue(QLatin1String("id"), q.value(0).toInt());
        if (!q2.exec()) {
            qWarning() << "Exec Insert speed dial failed:" << q2.lastError().text();
            qLog(Sql) << q2.executedQuery();
            return;
        }
    }
    //If no desc, don't put a null item in the list
    if(r.isNull()){
        QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
        tempAdaptor.send(MESSAGE(change()));
        return;
    }
    if(!q.prepare(QLatin1String("SELECT id FROM favoriteservices WHERE ") +
                QLatin1String("label = :label AND icon = :icon AND service = :service") +
                QLatin1String(" AND message = :message AND arguments = :arguments"))){
        qWarning() << "Prepare set speeddial failed:" << q.lastError().text();
        return;
    }
    q.bindValue(QLatin1String("label"), r.label());
    q.bindValue(QLatin1String("icon"), r.iconName());
    q.bindValue(QLatin1String("service"), r.request().service());
    q.bindValue(QLatin1String("message"), r.request().message());
    QByteArray args = QtopiaServiceRequest::serializeArguments(r.request());
    q.bindValue(QLatin1String("arguments"), args);
    if(!q.exec()){
        qWarning() << "Exec set speeddial failed:" << q.lastError().text();
        qLog(Sql) << q.executedQuery();
    }

    if(q.next()){
    //If the description is already in the list, set it's input
        QSqlQuery q2(db);
        if(!q2.prepare(QLatin1String("UPDATE favoriteservices SET speedDial = :input WHERE id = :id"))){
            qWarning() << "Prepare set speeddial failed:" << q2.lastError().text();
            return;
        }
        q2.bindValue(QLatin1String("input"), input);
        q2.bindValue(QLatin1String("id"), q.value(0).toInt());
        if (!q2.exec()) {
            qWarning() << "Exec Insert speed dial failed:" << q2.lastError().text();
            qLog(Sql) << q2.executedQuery();
            return;
        }
    } else {
        //Else add it (with the input)
        QSqlQuery q2(db);
        if(!q2.exec("SELECT COUNT(id) FROM favoriteservices")){
            qWarning() << "Insert speed dial failed:" << q2.lastError().text();
            qLog(Sql) << q2.executedQuery();
        }
        q2.first();
        int nextIndex = q2.value(0).toInt();

        if (!q2.prepare(QLatin1String("INSERT INTO favoriteservices ") +
                    QLatin1String("(sortIndex, speedDial, label, icon, service, message, arguments, optionalMap) ") +
                    QLatin1String("VALUES (:index, :input, :label, :icon, :service, :message, :arguments, :optionalMap)"))) {
            qWarning() << "Prepare insert speeddial failed:" << q2.lastError().text();
            return;
        }
        q2.bindValue(QLatin1String("index"), nextIndex);
        q2.bindValue(QLatin1String("input"), input);
        q2.bindValue(QLatin1String("label"), r.label());
        q2.bindValue(QLatin1String("icon"), r.iconName());
        q2.bindValue(QLatin1String("service"), r.request().service());
        q2.bindValue(QLatin1String("message"), r.request().message());
        //Reuse args from above
        q2.bindValue(QLatin1String("arguments"), args);
        if(r.optionalProperties().isEmpty()){
            q2.bindValue(QLatin1String("optionalMap"),QVariant());
        }else{
            QByteArray map;
            QDataStream ds(&map, QIODevice::WriteOnly);
            ds << r.optionalProperties();
            q2.bindValue(QLatin1String("optionalMap"),map);
        }

        if (!q2.exec()) {
            qWarning() << "Exec Insert speed dial failed:" << q2.lastError().text();
            qLog(Sql) << q2.executedQuery();
            return;
        }
    }
    QtopiaIpcAdaptor tempAdaptor("QPE/FavoriteServices");
    tempAdaptor.send(MESSAGE(change()));
}

/*!
  Returns a list of the currently assigned Speed Dial inputs.
  \since 4.3
  \sa possibleInputs()
*/
QList<QString> QSpeedDial::assignedInputs()
{

    QList<QString> ret;
    QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
    QSqlQuery q(db);
    if(!q.exec(QLatin1String("SELECT DISTINCT speedDial FROM favoriteservices WHERE speedDial IS NOT NULL"))){
        qWarning() << "select inputs from speeddial failed" << q.lastError().text();
        return ret;
    }
    while(q.next())
        ret << q.value(0).toString();
    return ret;
}

/*!
  Returns a list of possible Speed Dial inputs, some of which
  may be assigned already.

  \since 4.3
  \sa assignedInputs()
*/
QList<QString> QSpeedDial::possibleInputs()
{
    QList<QString> ret;

    for (int i = 1; i < 100; i++) {
        ret << QString::number(i);
    }
    return ret;
}

//QSpeedDialList Implementation
/*!
  \class QSpeedDialList
    \inpublicgroup QtBaseModule

  \brief The QSpeedDialList class provides a list widget for editing Speed Dial entries.

  If you need a dialog that allows the user to select a spot to insert an already selected
  action (for example, adding a QContact's phone number to Speed Dial list), use
  QSpeedDial::addWithDialog().

  Use editItem() to edit selected entry.
  This will open QtopiaServiceSelector which provides a list of predefined services.

  Use clearItem() to remove the entry.

  \image qspeeddiallist.png "Editing Speed Dial Entries"

  \sa QSpeedDial, QtopiaServiceSelector

  \ingroup userinput
*/

/*!
  \fn QSpeedDialList::currentRowChanged(int row)

  This signal is emitted whenever the user selects
  a different \a row (either with the keypad or the mouse).
*/
/*!
  \fn QSpeedDialList::rowClicked(int row)

  This signal is emitted whenever the user
  either clicks on a different \a row with the mouse, or presses the keypad
  Select key while a row is selected.
*/

/*!
  Constructs a QSpeedDialList object with the given \a parent.
*/

QSpeedDialList::QSpeedDialList(QWidget* parent) : QSmoothList(parent)
{
    d = new QSpeedDialListPrivate(this);
    setItemDelegate(d->delegate);
    setModel(d->model);
    setCurrentIndex(d->model->index(0,0));
    setWindowTitle(  tr( "Speed Dial" ) );
    connect(this,SIGNAL(activated(QModelIndex)),
            this,SLOT(select(QModelIndex)));
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);
    QAction *a_add = new QAction( QIcon( ":icon/edit" ),  tr("Add", "Add action"), this);
    connect( a_add, SIGNAL(triggered()), this, SLOT(addItem()) );
    contextMenu->addAction(a_add);
    QAction *a_del = new QAction( QIcon( ":icon/trash" ),  tr("Remove", "Remove action"), this);
    connect( a_del, SIGNAL(triggered()), this, SLOT(clearItem()) );
    contextMenu->addAction(a_del);
    QAction *a_edit = new QAction( QIcon( ":icon/edit" ),  tr("Edit", "Edit action"), this);
    connect( a_edit, SIGNAL(triggered()), this, SLOT(editItem()) );
    contextMenu->addAction(a_edit);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/*!
  \internal
  Removes the list's edit menu.
  There is not currently a method to get it back.
*/
void QSpeedDialList::disableEditMenu()
{
    delete QSoftMenuBar::menuFor(this);
    QSoftMenuBar::menuFor(this);
}

/*!
  Destroys this QSpeedDialList
*/
QSpeedDialList::~QSpeedDialList()
{
    delete d;
}

/*!
  \fn void QSpeedDialList::setCurrentInput(const QString& input)

  Selects the row from the list that corresponds to the Speed Dial \a input.
  If there is no such row then it will go the the one before where it would be.
*/
void QSpeedDialList::setCurrentInput(const QString& sd)
{
    int i;
    for(i = 0; i < d->model->rowCount(); ++i)
    {
        if(d->model->data(d->model->index(i,0,QModelIndex()),Qt::UserRole).toInt() == sd.toInt())
        {
            setCurrentRow(i);
            return;
        }
        else if(d->model->data(d->model->index(i,0,QModelIndex()),Qt::UserRole).toInt() > sd.toInt())
        {
            break;
        }
    }
    if(i==0)
        ++i;
    setCurrentRow(--i);
    return;
}

/*!
  \fn void QSpeedDialList::reload(const QString& input)

  Forces the entry for Speed Dial at \a input to be refreshed from the source.
*/
void QSpeedDialList::reload(const QString& input)
{
    //Q_UNUSED(input);
    /*Easiest to just say something's changed, if that's too slow
    then the model could just be told that specific row changed*/
    d->model->change();
    setCurrentInput(input);
}
/*!
  \internal

  Catches selection events and emits an event more meaningful to outside this class
  (emits a row instead of a QModelIndex)
*/
void QSpeedDialList::sendRowChanged()
{
    emit currentRowChanged(currentRow());
}

/*!
  \internal

  Catches selection events and emits an event more meaningful to outside this class
  (emits a row instead of a QModelIndex). Also attempts the selected service request.
*/
void QSpeedDialList::select(const QModelIndex& index)
{
    if(d->selectorPos){
        d->selectorPos = rowInput(index.row()).toInt();
        d->model->moveSelector(d->selectorPos);
        setCurrentInput(QString::number(d->selectorPos));
        emit currentRowChanged(index.row());
        emit rowClicked(index.row());
        return;//When in selector mode don't activate, but move Selector
    }
    emit currentRowChanged(index.row());
    emit rowClicked(index.row());
    QtopiaServiceDescription *desc = QSpeedDial::find(rowInput(index.row()));
    if(desc)
        desc->request().send();
}

/*!
  \internal

  Catches click events and emits an event more meaningful to outside this class
  (emits a row instead of a QModelIndex).
*/
void QSpeedDialList::click(const QModelIndex& index)
{
    emit currentRowChanged(index.row());
    emit rowClicked(index.row());
}

/*!
  Returns the input required to trigger the Speed Dial entry at \a row.
*/
QString QSpeedDialList::rowInput(int row) const
{
    if(row >= 0 && row < count())
        return d->model->data(d->model->index(row,0,QModelIndex()),Qt::UserRole).toString();
    else
        return QString();
}

/*!
  \property QSpeedDialList::currentInput
  \brief the input required for the currently selected Speed Dial entry if exists;
  otherwise returns an empty string
*/
QString QSpeedDialList::currentInput() const
{
    return d->model->data(currentIndex(),Qt::UserRole).toString();
}

/*!
  \internal

  Does nothing as QSmoothList doesn't support it.
  Preserved for binary compatibility.
*/
void QSpeedDialList::scrollContentsBy(int /*dx*/, int /*dy*/)
{
}

/*!
  \property QSpeedDialList::count
  \brief the number of rows in the list
*/
int QSpeedDialList::count() const
{
    return d->model->rowCount();
}

/*!
  \property QSpeedDialList::currentRow
  \brief the currently selected row number
*/
int QSpeedDialList::currentRow() const
{
    return currentIndex().row();
}

/*!
  Sets the current row to \a row.
*/
void QSpeedDialList::setCurrentRow(int row)
{
    setCurrentIndex(d->model->index(row,0,QModelIndex()));
}

/*!
  Adds a service to the Speed Dial

  Presents the a QtopiaServiceSelector for the user
  to select a service performed, and also asks for an input to associate with it.

  \sa QtopiaServiceSelector
*/
void QSpeedDialList::addItem()
{
    if(!d->serviceSelector){
        QWaitWidget *waitWidget = new QWaitWidget(this);
        waitWidget->show();
        QtopiaApplication::processEvents(QEventLoop::AllEvents,1000);

        d->serviceSelector = new QtopiaServiceSelector(this);
        d->serviceSelector->addApplications();
        QtopiaApplication::setMenuLike(d->serviceSelector,true);

        delete waitWidget;
    }
    QtopiaServiceDescription desc;
    if(d->serviceSelector->edit(tr("Speed Dial"),desc)){
        if(!desc.isNull()){
            QString ok = QSpeedDial::addWithDialog(desc.label(),desc.iconName(),
                            desc.request(),this);
            if(ok.isNull())
                return;
            reload(ok);
        }
     }
}

/*!
  Edits the Speed Dial entry at \a row.

  Presents the a QtopiaServiceSelector for the user
  to select a service performed for the same input.

  \sa QtopiaServiceSelector
*/
void QSpeedDialList::editItem(int row)
{
    if(!d->serviceSelector){
        QWaitWidget *waitWidget = new QWaitWidget(this);
        waitWidget->show();
        QtopiaApplication::processEvents(QEventLoop::AllEvents,1000);

        d->serviceSelector = new QtopiaServiceSelector(this);
        d->serviceSelector->addApplications();
        QtopiaApplication::setMenuLike(d->serviceSelector,true);

        delete waitWidget;
    }
    QString inp = rowInput(row);
    QtopiaServiceDescription *desc;
    desc = QSpeedDial::find(inp);
    if(desc && d->serviceSelector->edit(tr("Speed Dial"),*desc)){
        QSpeedDial::remove(inp);
        QSpeedDial::set(inp,*desc);
        reload(inp);
    }
}

/*!

  Edits the Speed Dial entry at the currently selected row.
*/

void QSpeedDialList::editItem()
{
    editItem(currentRow());
}

/*!
  Removes the Speed Dial entry at the given \a row. The action
  is not removed from Favorite Services.
*/
void QSpeedDialList::clearItem(int row)
{
    QSpeedDial::remove(rowInput(row));
    d->model->change();
    if(row >= count())
        row--;
    setCurrentRow(row);
}

/*!
  Removes the Speed Dial entry at the currently selected row. The action
  is not removed from Favorite Services.
*/
void QSpeedDialList::clearItem()
{
    clearItem(currentRow());
}

/*!
  \internal
  Sets the list to 'selection mode' where it shows where the given
  action would end up. Must have a QSpeedDial Model already set 
*/
void QSpeedDialList::setSelector(int startPos, const QString &label, const QString &icon)
{
    d->selectorPos = startPos;
    d->model->setSelector(startPos, label, icon);
    setCurrentInput(QString::number(startPos));
}

/*!
  \internal
*/
QString QSpeedDialList::selectorInput()
{
    return QString::number(d->selectorPos);
}

/*!
  \internal
*/
void QSpeedDialList::clearKeyInput()
{
    d->input=0;
}

/*!\internal
  Preserved for Binary Compatibility, no longer used in implementation
*/
void QSpeedDialList::timerEvent(QTimerEvent*e)
{
    QSmoothList::timerEvent(e);
}

/*!
  \internal
*/
void QSpeedDialList::keyPressEvent(QKeyEvent* e)
{
    int k = e->key();
    if(!d->selectorPos){
        if( k >= Qt::Key_0 && k <= Qt::Key_9 ){
            d->input = d->input * 10 + k - Qt::Key_0;
            if ( d->input ){
                setCurrentInput(QString::number(d->input));
                if ( d->input < 10 ){
                    d->inputTimer->start(800);
                }else{
                    d->input = 0;
                    d->inputTimer->stop();
                }
            }
        }else{
            QSmoothList::keyPressEvent(e);
        }
        return;
    }
    if( k >= Qt::Key_0 && k <= Qt::Key_9 ){
        d->input = d->input * 10 + k - Qt::Key_0;
        if ( d->input ){
            d->model->moveSelector(d->input);
            setCurrentInput(QString::number(d->input));
            d->selectorPos = d->input;
            if ( d->input < 10 ){
                d->inputTimer->start(800);
            }else{
                d->input = 0;
                d->inputTimer->stop();
            }
        }
    }else if(k == Qt::Key_Up){
        if(d->selectorPos>1){
            d->selectorPos--;
            d->model->moveSelector(d->selectorPos);
            setCurrentInput(QString::number(d->selectorPos));
        }
    }else if(k == Qt::Key_Down){
        if(d->selectorPos<99){
            d->selectorPos++;
            d->model->moveSelector(d->selectorPos);
            setCurrentInput(QString::number(d->selectorPos));
        }
    }else if( k == Qt::Key_Select ){
        if(currentRow() > -1)
            emit rowClicked(currentRow());
    }else{
        QSmoothList::keyPressEvent(e);
    }
}
//QSpeedDialDialog Implementation

QSpeedDialDialog::QSpeedDialDialog(QWidget *parent):QDialog(parent)
{
    setModal(true);
    setWindowState(windowState() | Qt::WindowMaximized);

    model = new QSpeedDialModel(this);
    list = new QSmoothList(this);
    delegate = new QSpeedDialItemDelegate(list);
    list->setModel(model);
    list->setItemDelegate(delegate);
    //list->setFrameStyle(QFrame::NoFrame);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);
    vb->addWidget(list);
    m_inputChoice = QString();
    QObject::connect(list, SIGNAL(activated(QModelIndex)),
            this, SLOT(store(QModelIndex)));
    setWindowTitle(tr("Select Speed Dial"));
}
void QSpeedDialDialog::store(const QModelIndex &selected){
    m_inputChoice = model->data(selected,Qt::UserRole).toString();
    accept();
}

//QSpeedDialModel Implementation
/*!
    \internal
    This class accesses the SQL Database directly for efficiency.
    Care must be taken to keep it in sync with QFavoritesList
*/
QSpeedDialModel::QSpeedDialModel(QObject* parent) : QAbstractListModel(parent)
{
    QtopiaIpcAdaptor *adaptor = new QtopiaIpcAdaptor("QPE/FavoriteServices", this);
    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(change()),
                              this, SLOT(change()));
    changed = true;
    countChanged = true;
    selectorPos = 0;
}
QSpeedDialModel::~QSpeedDialModel()
{
}
void QSpeedDialModel::change()
{
    changed = true;
    countChanged = true;
    reset();
}
/*!
  \internal
  The Selector item is not really there, it is there to show what would happen
  if that item was added. The way this is implemented is that it's details are
  stored, and it is treated as another item from outside the model. Internally
  it doesn't enter the cache, values like rowCount and the cache lists indexes
  are modified to go around it or to it.
*/
void QSpeedDialModel::setSelector(int startPos, const QString &label, const QString &icon)
{
    shadowing=false;
    selectorLabel = label;
    selectorIcon = icon[0] == '/' || icon[0] == ':' ? icon : QLatin1String(":icon/") + icon;
    selectorRow = -1;
    moveSelector(startPos);
    reset();
}
void QSpeedDialModel::moveSelector(int newPos)
{
    //Iff there are other entries, inputcache must be loaded
    if(rowCount())
        data(index(0,0,QModelIndex()));

    bool oldShadowing=shadowing;
    int oldRow = selectorRow;
    selectorPos = newPos;
    Q_ASSERT(selectorPos);
    int i;

    for(i = 0; i<inputCache.size(); i++)
    {
        if(inputCache.at(i).toInt()>=selectorPos){
            break;
        }
    }
    selectorRow = i;
    shadowing = (i==inputCache.size() ? false : inputCache.at(i).toInt()==selectorPos);
    if(shadowing!=oldShadowing){
        //Either it went up and isn't shadowing, which is redraw it and what it was covering but don't move,
        //or it went down and they swap places, and need redraws. This can't happen if it was the last row
        emit dataChanged(index(oldRow,0,QModelIndex()), index(oldRow+1,0,QModelIndex()));
        return;
    }
    if(oldRow<0)
        oldRow = selectorRow;
    emit dataChanged(index(oldRow,0,QModelIndex()), index(selectorRow,0,QModelIndex()));
}
int QSpeedDialModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    if(countChanged){
        QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
        QSqlQuery q(db);
        if(!q.exec("SELECT COUNT(speedDial) FROM favoriteServices WHERE speedDial IS NOT NULL")){
            qWarning() << "Count speed dial failed:" << q.lastError().text();
            return 0;
        }
        q.first();
        rowCountCache=q.value(0).toInt();
        countChanged = false;
    }
    if(selectorPos)
        return rowCountCache+1;
    return rowCountCache;
}
QVariant QSpeedDialModel::data(const QModelIndex & index, int role) const
{
    if(!index.isValid())
        return QVariant();
    
    if(changed){
        QSqlDatabase db = QtopiaSql::instance()->systemDatabase();
        QSqlQuery q(db);
        if(!q.exec(QLatin1String("SELECT label,icon,speedDial FROM favoriteservices WHERE speedDial IS NOT NULL ORDER BY speedDial asc"))){
            qWarning() << "Select from speed dial failed:" << q.lastError().text();
            return QVariant(tr("An Error Has Occured"));
        }
        iconCache.clear();
        displayCache.clear();
        inputCache.clear();
        while (q.next()) {
            QString input = q.value(2).toString();
            QString label = q.value(0).toString();
            QString icon = q.value(1).toString();
            if ( icon[0] != '/' && icon[0] != ':' )
                icon = QLatin1String(":icon/") + icon;
            displayCache << label;
            iconCache << icon;
            inputCache << input;
        }
        changed = false;
    }
    int offset=0;
    if(selectorPos){
        if(index.row() == selectorRow){
            if(role==Qt::DisplayRole){
                return QVariant(selectorLabel);
            }else if(role==Qt::DecorationRole){
                return QVariant(QIcon(selectorIcon));
            }else if(role==Qt::UserRole){
                return QVariant(selectorPos);
            }else if(role==Qt::UserRole+1){
                return QVariant(false);
            }
            return QVariant();
        }else if (index.row()>selectorRow){
            offset=-1;
        }
    }
    if(role==Qt::DisplayRole){
        return QVariant(displayCache[index.row()+offset]);
    }else if(role==Qt::DecorationRole){
        return QVariant(QIcon(iconCache[index.row()+offset]));
    }else if(role==Qt::UserRole){
        return QVariant(inputCache[index.row()+offset]);
    }else if(role==Qt::UserRole+1){
        if(selectorPos && index.row() == selectorRow+1 && shadowing)
            return QVariant(true);
        return QVariant(false);
    }
    return QVariant();
}

//QSpeedDialItemDelegate implmentation
QSpeedDialItemDelegate::QSpeedDialItemDelegate(QSmoothList* parent)
    : QtopiaItemDelegate(parent)
{
    parentList = parent;

    iconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
    QFontMetrics fm(parent->font());
    textHeight = fm.height();

    itemHeight = qMax(iconSize,textHeight);
}

QSpeedDialItemDelegate::~QSpeedDialItemDelegate()
{
}

void QSpeedDialItemDelegate::paint(QPainter * painter,
    const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QSpeedDialModel* model = (QSpeedDialModel*)index.model();
    if(model)
    {
        QFontMetrics fm(option.font);
        int x = option.rect.x();
        int y = option.rect.y();
        int width = option.rect.width();
        int height = option.rect.height()-1;
        bool selected = option.state & QStyle::State_Selected;

        if(selected)
        {
            painter->setPen(option.palette.highlightedText().color());
            painter->fillRect(option.rect, option.palette.highlight());
        }
        else
        {
            painter->setPen(option.palette.text().color());
        }

        QString input = model->data(index,Qt::UserRole).toString();
        QPixmap pixmap = model->data(index,Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize);
        QString label = model->data(index,Qt::DisplayRole).toString();
        bool overwrite = model->data(index,Qt::UserRole+1).toBool();

        if(overwrite)
            pixmap =model->data(index,Qt::DecorationRole).value<QIcon>().pixmap(option.decorationSize,QIcon::Disabled);

        QTextOption to;
        to.setAlignment( QStyle::visualAlignment(qApp->layoutDirection(),
                    Qt::AlignLeft) | Qt::AlignVCenter);
        bool rtl = qApp->layoutDirection() == Qt::RightToLeft;
        if(!overwrite)
            painter->drawText(QRect(x, y+1, width, height), input, to);
        if (!rtl )
            x += fm.width(QLatin1String("00"));
        width -= fm.width(QLatin1String("00"));
        if(!overwrite)
            painter->drawText(QRect(x,y+1,width,height), QLatin1String(":"), to);
        if (!rtl )
            x += fm.width(QLatin1String(": "));
        width -= fm.width(QLatin1String(": "));
    
        if(!pixmap.isNull())
        {
            if ( rtl ) {
                painter->drawPixmap(QRect(x+width-pixmap.width(), y+1, height-1,height-1), pixmap);
                width -= (height - 1) + fm.width(QLatin1String(" "));
            } else {
                painter->drawPixmap(QRect(x, y+1, height-1, height-1), pixmap);
                x += height - 1  + fm.width(QLatin1String(" "));
                width -= (height - 1) + fm.width(QLatin1String(" "));
            }
        }
        label = elidedText( fm, width, Qt::ElideRight, label );
        if(overwrite){
            QFont struckFont(painter->font());
            struckFont.setStrikeOut(true);
            painter->setFont(struckFont);
        }
        painter->drawText(QRect(x, y+1, width, height), label, to);
        if(overwrite){
            QFont unstruckFont(painter->font());
            unstruckFont.setStrikeOut(false);
            painter->setFont(unstruckFont);
        }
    }
}

QSize QSpeedDialItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(parentList->width(), itemHeight+2);
}

//QSpeedDialAddDialog Implementation

QSpeedDialAddDialog::QSpeedDialAddDialog(const QtopiaServiceRequest &req, const QString &label,
        const QString &icon, int input, QWidget *parent): QDialog(parent),
    m_prevInput(QString())
{
    m_inputChoice = QString();
    setModal(true);
    setWindowModality(Qt::WindowModal);
    setWindowState(windowState() | Qt::WindowMaximized);
    QtopiaApplication::setMenuLike(this,true);
    QSoftMenuBar::menuFor(this);

    m_desc = QtopiaServiceDescription(req,label,icon);
    QFavoriteServicesModel *favoriteModel = new QFavoriteServicesModel(this);
    QModelIndex descIndex = favoriteModel->indexOf(m_desc);
    if(descIndex.isValid()){
        m_prevInput = favoriteModel->speedDialInput(descIndex);
        if(!m_prevInput.isEmpty()){
            QSpeedDial::remove(m_prevInput);
            input = m_prevInput.toInt();
        }
    }

    list = new QSpeedDialList(this);
    list->setSelector(input, label, icon);
    list->disableEditMenu();

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);
    vb->addWidget(list);
    connect(list, SIGNAL(rowClicked(int)),
            this, SLOT(store()));
    connect(this, SIGNAL(rejected()),
            this, SLOT(undoRemove()));
    setWindowTitle(tr("Set Speed Dial"));

}

void QSpeedDialAddDialog::undoRemove()
{
    if(!m_prevInput.isEmpty())
        QSpeedDial::set(m_prevInput, m_desc);
}

void QSpeedDialAddDialog::store()
{
    m_inputChoice = list->selectorInput();
    accept();
}

#include "qspeeddial.moc"

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

#include "qtopiaserviceselector.h"

#include <QBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QCloseEvent>
#include <qtopianamespace.h>
#include <qsoftmenubar.h>
#include <qtranslatablesettings.h>
#include <qexpressionevaluator.h>
#include <qcontentset.h>
#include <QMenu>
#include <QDebug>
#include <qsmoothlistwidget_p.h>
#include <Qtopia>

class QtopiaServiceDescriptionPrivate {
public:
    QtopiaServiceDescriptionPrivate() {}
    QtopiaServiceDescriptionPrivate(const QtopiaServiceRequest& req,
            const QString& label, const QString& icon,
            const QMap<QString, QVariant>& goop)
        : _request(req), _label(label), _icon(icon), _goop(goop) {}
    QtopiaServiceRequest _request;
    QString _label;
    QString _icon;
    QMap<QString, QVariant> _goop;
};

/*!
  \class QtopiaServiceDescription
    \inpublicgroup QtBaseModule

  \brief The QtopiaServiceDescription class describes a service request in user terms.

  This data includes what action to undertake when activated, and a display name
  and icon.  It can also include extra information about the request that is not
  present in the request itself.

  \ingroup ipc
  \sa QtopiaServiceSelector, QtopiaServiceRequest
*/

/*!
  Constructs a null QtopiaServiceDescription.
*/
QtopiaServiceDescription::QtopiaServiceDescription()
    : d(new QtopiaServiceDescriptionPrivate())
{
}

/*!
  Constructs a QtopiaServiceDescription, which describes an action called \a label, with the
  display icon found in the file specified by \a icon, which initiates
  QtopiaServiceRequest \a request when activated.  The optional properties in the constructed
  object are initialized from \a optionalProperties.
*/
QtopiaServiceDescription::QtopiaServiceDescription(const QtopiaServiceRequest& request, const QString& label,
        const QString& icon, const QVariantMap& optionalProperties)
    : d(new QtopiaServiceDescriptionPrivate(request, label, icon, optionalProperties))
{
}

/*!
  Create a copy of \a other.
*/
QtopiaServiceDescription::QtopiaServiceDescription(const QtopiaServiceDescription& other)
    : d(new QtopiaServiceDescriptionPrivate(other.d->_request, other.d->_label, other.d->_icon, other.d->_goop))
{
}

/*!
  Assign \a other to this object.
*/
QtopiaServiceDescription& QtopiaServiceDescription::operator=(const QtopiaServiceDescription& other)
{
    if (&other != this) {
        d->_request = other.d->_request;
        d->_label = other.d->_label;
        d->_icon = other.d->_icon;
        d->_goop = other.d->_goop;
    }
    return *this;
}

/*!
  Returns true if \a other is equal to this QtopiaServiceDescription.
  Two service descriptions are considered equal if they have the same
  icon, label, service request. Optional properties are not checked.
*/
bool QtopiaServiceDescription::operator==(const QtopiaServiceDescription& other) const
{
	return (d->_request==other.d->_request&&d->_label==other.d->_label
			&&d->_icon==other.d->_icon);
}

/*!
  Returns true if either the label, icon or request for this 
  QtopiaServiceDescription is null.
*/
bool QtopiaServiceDescription::isNull() const
{
	return(d->_request.isNull() || d->_label.isNull()
			|| d->_icon.isNull());
}

/*!
  Destroys this QtopiaServiceDescription.
*/
QtopiaServiceDescription::~QtopiaServiceDescription()
{
    delete d;
}

/*!
  Returns the QtopiaServiceRequest described.

  \sa setRequest()
*/
QtopiaServiceRequest QtopiaServiceDescription::request() const
{
    return d->_request;
}

/*!
  Returns the display label describing the request.

  \sa setLabel()
*/
QString QtopiaServiceDescription::label() const
{
    return d->_label;
}

/*!
  Returns the icon name describing the request.

  \sa setIconName()
*/
QString QtopiaServiceDescription::iconName() const
{
    return d->_icon;
}

/*!
  Sets the request to \a request.

  \sa request()
*/
void QtopiaServiceDescription::setRequest(const QtopiaServiceRequest& request)
{
    d->_request = request;
}

/*!
  Sets the display label to \a label.

  \sa label()
*/
void QtopiaServiceDescription::setLabel(const QString& label)
{
    d->_label = label;
}

/*!
  Sets the display icon to \a iconName.

  \sa iconName()
*/
void QtopiaServiceDescription::setIconName(const QString& iconName)
{
    d->_icon = iconName;
}

/*!
  Returns an optional property with the given \a name that describes the service request.

  \sa setOptionalProperty(), removeOptionalProperty()
*/
QVariant QtopiaServiceDescription::optionalProperty(const QString& name) const
{
    return d->_goop.value(name);
}

/*!
  Sets an optional property describing this service request with the given \a name and \a value.

  \sa removeOptionalProperty(), optionalProperty()
*/
void QtopiaServiceDescription::setOptionalProperty(const QString& name, const QVariant& value)
{
    d->_goop.insert(name, value);
}

/*!
  Removes any optional properties with the given \a name describing this service request.

  \sa setOptionalProperty(), optionalProperty()
*/
void QtopiaServiceDescription::removeOptionalProperty(const QString& name)
{
    d->_goop.remove(name);
}

/*!
  Returns the entire collection of optional properties describing this service request.

  \sa setOptionalProperties()
*/
QVariantMap QtopiaServiceDescription::optionalProperties() const
{
    return d->_goop;
}

/*!
  Sets the entire collection of optional properties describing this service request to \a properties.

  \sa optionalProperties()
*/
void QtopiaServiceDescription::setOptionalProperties(QVariantMap properties)
{
    d->_goop = properties;
}

/*!
    \internal
    \fn void QtopiaServiceDescription::serialize(Stream &stream) const
*/
template <typename Stream> void QtopiaServiceDescription::serialize(Stream &stream) const
{
    stream << d->_request;
    stream << d->_label;
    stream << d->_icon;
    stream << d->_goop;
}

/*!
    \internal
    \fn void QtopiaServiceDescription::deserialize(Stream &stream)
*/
template <typename Stream> void QtopiaServiceDescription::deserialize(Stream &stream)
{
    stream >> d->_request;
    stream >> d->_label;
    stream >> d->_icon;
    stream >> d->_goop;
}

Q_IMPLEMENT_USER_METATYPE(QtopiaServiceDescription)

/*!
  \class QtopiaServiceSelector
    \inpublicgroup QtBaseModule

  \brief The QtopiaServiceSelector class implements a list dialog for selecting a service.

  \ingroup ipc
  \sa QtopiaServiceDescription, QtopiaServiceRequest
*/

class QListView;

#define SRV_ROLE        Qt::UserRole
#define ICON_ROLE       Qt::UserRole + 1
#define ACTION_ROLE     Qt::UserRole + 2
#define ARGS_ROLE       Qt::UserRole + 3

/*!
    Construct a Qt Extended service selector dialog owned by \a parent.
*/
QtopiaServiceSelector::QtopiaServiceSelector(QWidget* parent) : QDialog(parent)
{
    setModal(true);
    setWindowModality(Qt::WindowModal);
    setWindowTitle("Select Service");

    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->setSpacing(0);
    vbl->setMargin(0);

    QHBoxLayout *hbl = new QHBoxLayout();
    hbl->setSpacing(0);
    hbl->setMargin(0);
    vbl->addLayout(hbl);

    label = new QLabel();
    hbl->addWidget(label);

    actionlist = new QSmoothListWidget();
    //Below are not yet supported for QSmoothList
    //actionlist->setUniformItemSizes(true);
    //actionlist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vbl->addWidget(actionlist);

    (void)QSoftMenuBar::menuFor(actionlist);

    QSmoothListWidgetItem* item = new QSmoothListWidgetItem(tr("No action"), actionlist);
    item->setIcon(QIcon(":icon/reset"));

    populateActionsList();

    connect(actionlist, SIGNAL(itemActivated(QSmoothListWidgetItem*)), this, SLOT(selectAction(QSmoothListWidgetItem*)));
}

/*!
    Add all installed applications (and games and settings) to the list of services displayed in this selector.
    By default, only service actions are shown.

    This will not add any application which has the same name and icon as another application or a service already in the list.
*/
void QtopiaServiceSelector::addApplications()
{
    // Copy all of the AppLnks to our own list
    QContentSet lnkSet = QContentSet(QContentFilter(QContent::Application));
    QContentList apps = lnkSet.items();
    foreach(const QContent &it, apps )
    {
        if ( it.type() != "Separator" && it.property("Builtin") != "1") // No tr
        {
            //Do not add apparently duplicate items which may confuse the user
            QList<QSmoothListWidgetItem*> sameItems = actionlist->findItems(
                    Qtopia::dehyphenate(it.name()),
                    Qt::MatchFixedString);
            bool duplicate = false;
            foreach(QSmoothListWidgetItem *item, sameItems){
                if(item->data(ICON_ROLE).toString() == it.iconName()){
                    duplicate = true;
                    break;
                }
            }
            if(duplicate){
                continue;
            }

            //Note that in the list names will not be split over multiple lines
            QSmoothListWidgetItem *item = new QSmoothListWidgetItem(Qtopia::dehyphenate(it.name()), actionlist);
            QList<QVariant> args;
            args << it.executableName();
            item->setData(SRV_ROLE, "Launcher");
            item->setData(ICON_ROLE, it.iconName());
            item->setData(ACTION_ROLE, "execute(QString)");
            item->setData(ARGS_ROLE, args);
            item->setIcon(QIcon(":icon/"+it.iconName()));
        }
    }
}

void QtopiaServiceSelector::populateActionsList()
{
    QStringList srvList = QtopiaService::list();
    QStringList::ConstIterator itService;

    for(itService = srvList.begin(); itService != srvList.end(); ++itService)
    {
        if ( !QtopiaService::app(*itService).isEmpty() ) { // only show available services (eg. not 'Beam Business Card' if no Contacts)
            QTranslatableSettings srv(QtopiaService::config(*itService), QSettings::IniFormat);
            if( srv.status()==QSettings::NoError )
            {
                srv.beginGroup("Service");
                populateActionsList(*itService, srv);
            }
        }

        QTranslatableSettings srvapp(QtopiaService::appConfig(*itService, QString()), QSettings::IniFormat);
        if( srvapp.status()==QSettings::NoError )
        {
            srvapp.beginGroup("Extensions");
            populateActionsList(*itService, srvapp);
        }
    }
}

void QtopiaServiceSelector::populateActionsList(const QString& srv, QTranslatableSettings& cfg)
{
    QStringList actions = cfg.value("Actions").toString().split( ';');
    QString name;
    QString icon;
    QSmoothListWidgetItem* item;

    cfg.endGroup();

    for(QStringList::ConstIterator ait = actions.begin(); ait != actions.end(); ++ait)
    {
        if((*ait).right(2) == "()")
        {
            cfg.beginGroup(*ait);
            name = cfg.value("Name").toString();
            if(!name.isEmpty())
            {
                QByteArray r = cfg.value(QLatin1String("Requires")).toByteArray();
                QExpressionEvaluator expr(r);
                if ( r.isEmpty() || expr.isValid() && expr.evaluate() && expr.result().toBool() ) {
                    icon = cfg.value("Icon").toString();

                    item = new QSmoothListWidgetItem(name, actionlist);
                    QList<QVariant> args;
                    item->setData(SRV_ROLE, QVariant(srv));
                    item->setData(ICON_ROLE, QVariant(icon));
                    item->setData(ACTION_ROLE, QVariant(*ait));
                    item->setData(ARGS_ROLE, args);
                    item->setIcon(QIcon(":icon/"+icon));
                }
            }
            cfg.endGroup();
        }
    }
}

/*!
    \reimp
*/
void QtopiaServiceSelector::closeEvent(QCloseEvent *e)
{
    e->accept();
}

/*!
    Displays this Qt Extended service selector dialog, to allow the user to select
    a service to associate with \a targetlabel.  The selected service description
    is returned in \a item.

    This function is typically used to edit the service associated with a device button
    or similar action.  The contents of the service selector are not being edited; rather
    the button or action is being edited.

    Returns true if an item was selected; or false if the dialog was canceled.
*/
bool QtopiaServiceSelector::edit(const QString& targetlabel, QtopiaServiceDescription& item)
{
    bool rowset = false;

    //
    //  Find the currently selected entry, and put the cursor there.
    //

    label->setText(tr("<p>Action for <b>%1</b>...").arg(targetlabel));

    int count = actionlist->count();
    for(int i = 0; i < count; i++)
    {
        QSmoothListWidgetItem* actionitem = actionlist->item(i);

        if(item.label() == actionitem->text() &&
            item.request().service() == actionitem->data(SRV_ROLE) &&
            item.iconName() == actionitem->data(ICON_ROLE) &&
            item.request().message() == actionitem->data(ACTION_ROLE) &&
            item.request().arguments() == actionitem->data(ARGS_ROLE))
        {
            actionlist->setCurrentRow(i);
            selection = i;
            rowset = true;
            break;
        }
    }

    if(!rowset)
    {
        actionlist->setCurrentRow(0);
        selection = -1;
    }

    showMaximized();
    if ( exec() != QDialog::Accepted )
        return false;

    bool chose = false;
    if( selection > -1 )
    {
        int ch = actionlist->currentRow();
        if( ch == 0 )
        {
            chose = true;
            item.setRequest(QtopiaServiceRequest());
        }
        else if( ch > 0 )
        {
            QSmoothListWidgetItem* actionitem = actionlist->item(ch);
            if( actionitem )
            {
                chose = true;
                item = descFor(actionitem);
            }
        }
    }

    return chose;
}

QtopiaServiceDescription QtopiaServiceSelector::descFor(QSmoothListWidgetItem* item) const
{
    QString service = item->data(SRV_ROLE).toString();
    QString message = item->data(ACTION_ROLE).toString();
    QString name = item->text();
    QString icon = item->data(ICON_ROLE).toString();
    QList<QVariant> args = item->data(ARGS_ROLE).toList();

    QtopiaServiceRequest sr(service, message);
    sr.setArguments(args);
    return QtopiaServiceDescription(sr, name, icon);
}

/*!
    Returns the description of the service request \a req.  Descriptions include
    the service request, a user-visible name, and an icon name.

    \sa QtopiaServiceDescription
*/
QtopiaServiceDescription QtopiaServiceSelector::descriptionFor(const QtopiaServiceRequest& req) const
{
    QString srv = req.service();

    const int count = actionlist->count();

    for (int tryapp=0; tryapp<=1; ++tryapp) {
        if ( tryapp )
            srv = QtopiaService::app(req.service());
        for (int i = 0; i < count; i++)
        {
            QSmoothListWidgetItem* actionitem = actionlist->item(i);

            if( srv == actionitem->data(SRV_ROLE) &&
                req.message() == actionitem->data(ACTION_ROLE) &&
                req.arguments() == actionitem->data(ARGS_ROLE))
            {
                return descFor(actionitem);
            }
        }
    }

    return QtopiaServiceDescription();
}

void QtopiaServiceSelector::selectAction(QSmoothListWidgetItem *i)
{
    int a = actionlist->row(i);
    if ( a >= 0 )
        selectAction(a);
}

void QtopiaServiceSelector::selectAction(int a)
{
    selection = a;
    accept();
}

/*!
    \reimp
*/
void QtopiaServiceSelector::keyPressEvent(QKeyEvent* e)
{
    int k = e->key();
    if ( k == Qt::Key_Back )
        reject();
    else if( k >= Qt::Key_Select )
        if(actionlist->currentItem())
            selectAction(actionlist->currentRow());
}


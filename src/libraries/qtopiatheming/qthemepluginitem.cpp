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

#include <QThemePluginItem>
#include <QPluginManager>
#include <private/themedviewinterface_p.h>

/*!
  \class QThemePluginItem
    \inpublicgroup QtBaseModule
  \since 4.4

  \brief The QThemePluginItem class manages the interaction between a ThemedItemInterface implementation and a QThemedView.

  The QThemePluginItem class implements the \l{Themed View Elements#themepluginelement}{plugin element} from the theme XML.

  You can explicitly assign a new plug-in interface using the setPlugin() or setBuiltin() functions.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

  \ingroup qtopiatheming
*/
class QThemePluginItemPrivate
{
public:
    QThemePluginItemPrivate()
            : iface(0), builtin(false) {
    }
//    QPluginManager *loader;
    ThemedItemPlugin *iface;
    bool builtin;
};

/*!
  Constructs a QThemePluginItem.
  \a parent is passed to the base class constructor.
*/
QThemePluginItem::QThemePluginItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemePluginItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemePluginItem.
*/
QThemePluginItem::~QThemePluginItem()
{
    releasePlugin();
//    delete d->loader;
    delete d;
}

/*!
  Loads a new plug-in using QPluginManager based on \a name.
*/
void QThemePluginItem::setPlugin(const QString &name)
{
    Q_UNUSED(name);
//     if (!d->loader)
//         d->loader = new QPluginManager(QLatin1String("themedview"));
    releasePlugin();
    d->iface = 0;//qobject_cast<ThemedItemPlugin*>(d->loader->instance(name));
    d->builtin = false;
//    d->iface->resize(boundingRect().width(), boundingRect().height());
//    if (isVisible())
//        update();
}

/*!
    Sets the plug-in for the item to be the already constructed plug-in pointed to by \a plugin.
*/
void QThemePluginItem::setBuiltin(ThemedItemPlugin *plugin)
{
    releasePlugin();
    d->iface = plugin;
    if (d->iface) {
        d->builtin = true;
        d->iface->resize(qRound(boundingRect().width()), qRound(boundingRect().height()));
    }
    if (isVisible())
        update();
}

/*!
  \reimp
*/
int QThemePluginItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemePluginItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (d->iface)
        d->iface->paint(painter, boundingRect().toRect());
}

/*!
  \reimp
  Lays out the plug-in item.
  The plug-in item resizes its plug-in to match the geometry specified for this item in the themed view XML.
*/
void QThemePluginItem::layout()
{
    QSize oldSize = boundingRect().size().toSize();
    QThemeItem::layout();
    if (d->iface && boundingRect().size() != oldSize) {
        d->iface->resize(qRound(boundingRect().width()), qRound(boundingRect().height()));
    }
}

void QThemePluginItem::releasePlugin()
{
    delete d->iface;
    d->iface = 0;
}

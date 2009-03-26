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

#include <themedview.h>
#include "themedviewinterface_p.h"
#include "qtagmap_p.h"
#include <qtopianamespace.h>
#include <qpluginmanager.h>
#include <qexpressionevaluator.h>
#include <qtopiaipcenvelope.h>
#include <qtopialog.h>

#include <qxml.h>
#include <QFile>
#include <QPainter>
#include <QStack>
#include <QBitmap>
#include <QStringList>
#include <QLineEdit>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QTextLayout>
#include <QStylePainter>
#include <QAbstractItemDelegate>
#include <QDesktopWidget>
#include <QDebug>
#include <QListView>
#include <QUuid>
#include <QVariant>
#include <QtopiaService>
#include <QSvgRenderer>
#include <QPixmapCache>
#include <QPicture>

#include <limits.h>

#include <QXmlStreamReader>

//===================================================================
/* declare ThemeListDelegate */

struct ThemeListDelegatePrivate;
class ThemeListDelegate : public QItemDelegate
{
public:
    ThemeListDelegate(QListView *listview, ThemedView *view, QObject *parent = 0);
    virtual ~ThemeListDelegate();

    void paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int height(ThemeListModelEntry* entry, const QModelIndex& index) const;

private:
    ThemeListDelegatePrivate* d;
};

class ThemeFactory;
struct ThemedViewPrivate
{
    ThemedViewPrivate(ThemedView *view);
    ~ThemedViewPrivate();

    QString themeName;
    ThemeItem *root;
    ThemedView *view;
    bool needLayout;

    QString themeSource;
    QMap<QExpressionEvaluator*, ThemeItem*> expressionToThemeItemMap;
    bool sourceLoaded;

    QWidget *target;
    ThemeItem *pressedItem;

    ThemeFactory* factory;
};

class ThemeAttributes
{
public:
    ThemeAttributes(const QXmlStreamAttributes &atts) : m_atts(atts) {}
    QString value(const QString &key) const { return m_atts.value(key).toString(); }
    QString value(const QLatin1String &key) const { return m_atts.value(key).toString(); }

private:
    QXmlStreamAttributes m_atts;
};

struct ThemeItemPrivate
{
    ThemeItemPrivate( ThemeItem *parent, ThemedView *view )
        : parent(parent), view(view), rmode(ThemeItem::Rect), actv(true), visible(true), press(false), focus(false), interactive(false), activeExpr(0), isDataExpression(false)
    {
    }
    ThemeItem *parent;
    ThemedView *view;
    QString name;
    QRectF sr;
    QRect br;
    ThemeItem::RMode rmode;
    ThemeItem::Unit runit[4];
    bool actv;
    bool transient;
    bool visible;
    QList<ThemeItem*> chldn;
    bool press;
    bool focus;
    bool interactive;
    QTagMap<int> intattributes[3];
    QTagMap<QString> strattributes[3];
    QExpressionEvaluator* activeExpr;
    QList<ThemeMessageData> message;
    bool isDataExpression;
};

class ThemeFactory : public QXmlStreamReader
{
public:

    ThemeFactory(ThemedView *view)
      : QXmlStreamReader(), m_root(0), m_view(view) { }

    bool readThemedView(QIODevice *device)
    {
        Q_ASSERT(device && m_view);

        m_root = 0;
        clear();
        setDevice(device);
        while (!atEnd()) {
            readNext();
            if (isStartElement() && name() == "page")
                readPage();
        }
        return !error();
    }

    bool readTemplate(const QString &data, ThemeItem *parent, const QString &uid)
    {
        Q_ASSERT(parent && m_view);

        m_root = 0;
        clear();
        addData(data);
        while (!atEnd()) {
            readNext();
            if (isStartElement())
                readTemplateInstance(parent, uid);
        }
        return !error();
    }

    ThemeItem *root() const { return m_root; }

private:

    void readTemplateInstance(ThemeItem *parent, const QString &uid)
    {
        Q_ASSERT(parent && m_view && isStartElement());

        ThemeAttributes atts(attributes());
        m_root = new ThemeTemplateInstanceItem(parent, m_view, atts);
        m_root->setVSPath(uid);

        while (!atEnd()) {
            readNext();
            if (isEndElement())
                break;
            if (isStartElement())
                startElement(m_root);
        }
        parent->addChild(m_root);
        m_root->constructionComplete();
    }

    void readPage()
    {
        Q_ASSERT(isStartElement() && name() == "page");

        ThemeAttributes atts(attributes());
        m_root = new ThemePageItem(m_view, atts);

        while (!atEnd()) {
            readNext();
            if (isEndElement())
                break;
            if (isStartElement()) {
                startElement(m_root);
            }
        }
        m_root->constructionComplete();
    }

    void startElement(ThemeItem *parent)
    {
        Q_ASSERT(isStartElement());

        if (!filter(attributes())) {
            readElementText();
            return;
        }

        ThemeAttributes atts(attributes());
        ThemeItem *item = 0;
        QStringRef iname = name();
        if (iname == "image") {
            item = new ThemeImageItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "level") {
            item = new ThemeLevelItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "anim") {
            item = new ThemeAnimationItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "status") {
            item = new ThemeStatusItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "rect") {
            item = new ThemeRectItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "text") {
            item = new ThemeTextItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "layout") {
            item = new ThemeLayoutItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "group") {
            item = new ThemeGroupItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "plugin") {
            item = new ThemePluginItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "widget") {
            item = new ThemeWidgetItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "exclusive") {
            item = new ThemeExclusiveItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "list") {
            item = new ThemeListItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "template") {
            ThemeTemplateItem *templateItem = new ThemeTemplateItem(parent, m_view, atts);
            item = templateItem;
            readTemplateItem(templateItem);
        } else if (iname == "line") {
            item = new ThemeLineItem(parent, m_view, atts);
            readElement(item);
        } else if (iname == "tr") {
            readTr(parent);
        }

        if (item && parent) {
            parent->addChild(item);
            item->constructionComplete();
        }
    }

    void readElement(ThemeItem *item)
    {
        while (!atEnd()) {
            readNext();
            if (isCharacters() && !text().isEmpty()) {
               item->addCharacters(text().toString());
            }
            if (isEndElement())
                break;
            if (isStartElement())
                startElement(item);
        }
    }

    void readTr(ThemeItem *parent)
    {
        Q_ASSERT(isStartElement() && name() == "tr");

        QString trtext;
        QStringList trargs;

        while (!(isEndElement() && name() == "tr")) {
            readNext();
            if (isStartElement() && name() == "trtext")
                trtext = readElementText();
            if (isStartElement() && name() == "trarg")
                trargs += readTrArg();
        }
        QString name = m_view->themeName();
        name.replace(QChar(' '), QLatin1String(" "));
        name.prepend( QLatin1String("Theme-") );
        QString translated = Qtopia::translate(name, m_view->pageName(), trtext);
        QRegExp numregex("%[0-9]");
        QStringList split;
        int curIdx = 0, idx = 0;
        while (idx != -1) {
            idx = translated.indexOf(numregex, curIdx);
            if (idx >= 0) {
                if (idx-curIdx > 0) {
                    split.append(translated.mid(curIdx, idx-curIdx));
                    curIdx = idx;
                }
                split.append(translated.mid(curIdx, 2));
                curIdx += 2;
                idx = curIdx;
            } else if (curIdx < translated.length()) {
                split.append(translated.right(translated.length()-curIdx));
            }
        }
        QString expanded;
        int ii = 0;
        while (ii < split.count()) {
            QString cur = split[ii];
            if (numregex.exactMatch(cur)) {
                if (ii > 0)
                    expanded += " . ";
                expanded += cur;
            } else {
                if (ii > 0)
                    expanded += " . ";
                if (cur.isEmpty())
                    expanded += "\"\"";
                else
                    expanded += (QString("\"") + cur + QString("\""));
            }
            ++ii;
        }
        for (QStringList::Iterator it = trargs.begin(); it != trargs.end() ; ++it)
            expanded = expanded.arg(*it);
        if (expanded.isEmpty())
            expanded += "\"\"";
        if (expanded.contains(numregex))
            qWarning("trtext has more variable markers than corresponding trarg elements.");
        parent->addCharacters(expanded);
        parent->setDataExpression(true);
    }

    QString readTrArg()
    {
        while (!(isEndElement() && name() == "trarg")) {
            readNext();
            if (isEndElement())
                break;
            if (isCharacters())
                return text().toString().trimmed();
        }
        return QString();
    }

    void readTemplateItem(ThemeTemplateItem *t)
    {
        Q_ASSERT(isStartElement() && name() == "template");

        QString data;
        data += "<template ";
        foreach ( QXmlStreamAttribute a, attributes())
            data += a.name().toString() + "='" + a.value().toString() + "' ";
        data += '>';

        while (!(isEndElement() && name() == "template")) {
            readNext();
            if (isStartElement()) {
                data += '<' + name().toString() + ' ';
                foreach ( QXmlStreamAttribute a, attributes()) {
                    data += a.name().toString() + "='" + a.value().toString() + "' ";
                }
                data += '>';
            }
            else if (isCharacters())
                data += text().toString().trimmed();
            else if (isEndElement())
                data += "</" + name().toString() + '>';
        }
        t->setData(data);
    }

    bool filter(const QXmlStreamAttributes &atts)
    {
        bool keypad = atts.value(QLatin1String("keypad")) != QLatin1String("no");
        bool touchscreen = atts.value(QLatin1String("touchscreen")) != QLatin1String("no");
        if (!keypad && !Qtopia::mousePreferred())
            return false;
        if (!touchscreen && Qtopia::mousePreferred())
            return false;
        return true;
     }

private:
    ThemeItem   *m_root;
    ThemedView  *m_view;
};


/*!
  \class ThemeTemplateInstanceItem
    \inpublicgroup QtBaseModule

  \brief The ThemeTemplateInstanceItem class represents a template instance in a ThemedView.

  You can create new template instance items using ThemeTemplateItem::createInstance().
  Template instances are parented to an item when created and automatically removed from that item's children
  when deleted.

    \ingroup appearance
*/
// TODO  more details

/*!
  Constructs a ThemeTemplateItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeTemplateInstanceItem::ThemeTemplateInstanceItem( ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts )
    : ThemeTemplateItem(parent, view, atts)
{
}

/*!
  Destroys a theme template instance item.
  The item is removed from its parent item's children.
  */
ThemeTemplateInstanceItem::~ThemeTemplateInstanceItem()
{
    if ( parentItem() )
        parentItem()->removeChild( this );
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::TemplateInstance.
*/
int ThemeTemplateInstanceItem::rtti() const
{
    return ThemedView::TemplateInstance;
}

struct ThemeTemplateItemPrivate
{
    QString data;
};

/*!
  \class ThemeTemplateItem
    \inpublicgroup QtBaseModule


  \brief The ThemeTemplateItem class represents a theme template in a ThemedView.

  The ThemeTemplateItem class implements the \l{Themed View Elements#themetemplateelement}{template element} from the theme XML.

  A theme template item records the type, structure and attributes of its child theme items.
  Using this information, it can then create instances of these theme items dynamically through createInstance().
  Theme templates are used to describe theme visuals, without creating any specific instance of that visual.
  Therefore, instances of the visual descriptions can be created as necessary, and drawn to custom locations.
  ThemeListModel makes use of templates to theme items in a QListView.

  \sa ThemeListModel, {Themed View Elements#themetemplateelement}{template element}

  \ingroup appearance
*/

/*!
  Constructs a ThemeTemplateItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
  */
ThemeTemplateItem::ThemeTemplateItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeItem(parent, view, atts)
{
    d = new ThemeTemplateItemPrivate;
}

/*!
  Destroys the ThemeTemplateItem.
*/
ThemeTemplateItem::~ThemeTemplateItem()
{
    delete d;
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Template.
*/
int ThemeTemplateItem::rtti() const
{
    return ThemedView::Template;
}

/*!
  Creates a new ThemeTemplateInstanceItem from this ThemeTemplateItem with the given \a uid.
  The template instance item is parented to the parent of this template item.
*/
ThemeTemplateInstanceItem* ThemeTemplateItem::createInstance( const QString& uid ) {
    ThemeFactory* factory = view()->d->factory;
    Q_ASSERT(factory != 0);

    factory->readTemplate(d->data, parentItem(), uid);
    ThemeItem* item = factory->root();

    Q_ASSERT(item->rtti() == ThemedView::TemplateInstance);
    view()->d->needLayout = true;
    return static_cast<ThemeTemplateInstanceItem*>(item);
}


void ThemeTemplateItem::setData(const QString& data)
{
    d->data = data;
}

ThemedViewPrivate::ThemedViewPrivate(ThemedView *view)
    : root(0), view(view), needLayout(false),
      sourceLoaded(false), pressedItem(0), factory(0)
{
    factory = new ThemeFactory(view);
}

ThemedViewPrivate::~ThemedViewPrivate()
{
    delete root;
    delete factory;
}

struct ThemeMessageData
{
    ThemeMessageData() {
    }

    ThemeMessageData( const ThemeMessageData& other ) {
        (*this) = other;
    }

    ThemeMessageData& operator=( const ThemeMessageData& other ) {
        channel = other.channel;
        msg = other.msg;
        type = other.type;
        variant = other.variant;
        return *this;
    }

    QString channel;
    QString msg;
    QString type;
    QVariant variant;
};

static const struct {
    const char *name;
    QPalette::ColorRole role;
} colorTable[] = {
    { "Foreground", QPalette::Foreground },
    { "Button", QPalette::Button },
    { "Light", QPalette::Light },
    { "Midlight", QPalette::Midlight },
    { "Dark", QPalette::Dark },
    { "Mid", QPalette::Mid },
    { "Text", QPalette::Text },
    { "BrightText", QPalette::BrightText },
    { "ButtonText", QPalette::ButtonText },
    { "Base", QPalette::Base },
    { "Background", QPalette::Background },
    { "Shadow", QPalette::Shadow },
    { "Highlight", QPalette::Highlight },
    { "HighlightedText", QPalette::HighlightedText },
    { "None", QPalette::NColorRoles },
    { 0, QPalette::Foreground }
};

static int stateToIndex( ThemeItem::State st )
{
    switch( st ) {
        case ThemeItem::Default:
            return 0;
        case ThemeItem::Focus:
            return 1;
        case ThemeItem::Pressed:
            return 2;
        default:
            qFatal("BUG : stateToIndex passed invalid st %d", st);
            return -1;
    }
}

static ThemeItem::State indexToState( int idx )
{
    switch( idx ) {
        case 0:
            return ThemeItem::Default;
        case 1:
            return ThemeItem::Focus;
        case 2:
            return ThemeItem::Pressed;
        default:
            qFatal("BUG : indexToState passed invalid index %d", idx);
            return ThemeItem::Default;
    }
}

/*!
  \class ThemeItem
    \inpublicgroup QtBaseModule


  \brief The ThemeItem class is the base class of all items in a ThemedView.

  A ThemedView manages the theme item tree, which is run-time representation of
  a loaded themed view XML document.

  ThemeItem encapsulates functionality common to all theme item tree nodes.
  It manages common attributes such as rect, whether the item is active, transient, visible, layout attributes,
  user interactivity and so on.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

  \sa ThemedView, {Themed View Elements#themecommonattributes}{common theme attributes}
  \ingroup appearance
*/

/*!
    \enum ThemeItem::State
    State describes the different states of a theme element.

    \value Default The default state of an element.
    \value Focus The element has the focus.
    \value Pressed The element is pressed.
    \value All All the states of an element.
*/

/*!
  Constructs a ThemeItem with parent \a parent, view \a view and attributes \a atts.
*/
ThemeItem::ThemeItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : d(new ThemeItemPrivate(parent, view))
{
    d->name = atts.value(QLatin1String("name"));
    d->sr = parseRect(atts, d->rmode);
    d->transient = atts.value(QLatin1String("transient")) == QLatin1String("yes");
    QString actvdata = atts.value(QLatin1String("active"));
    bool isliteral = !isStringExpression(actvdata);
    if ( isliteral ) {
        d->actv = actvdata != QLatin1String("no");
    } else {
        setAttribute("activeExpression", stripStringExpression(actvdata));
        d->actv = false;
    }

    d->interactive = atts.value(QLatin1String("interactive")) == QLatin1String("yes");

    setAttribute(QLatin1String("expanding"), atts.value(QLatin1String("expanding")).toLower() == QLatin1String("yes"));
    setVSPath( atts.value(QLatin1String("vspath")) );

    QMap<QString,QString> onClickAtts = parseSubAtts( atts.value(QLatin1String("onclick")) );
    if ( onClickAtts.contains( "message" ) ) {
        bool ok = false;
        ThemeMessageData data( parseMessage( onClickAtts["message"], &ok ) );
        if ( ok )
            d->message.append( data );
    }
}

/*!
  Called by ThemedView when this item has been clicked.
  A click is a mouse press followed by a mouse release.
*/
void ThemeItem::clickedEvent()
{
    for( int i = 0 ; i < d->message.count() ; ++i ) {
        QString msg;
        msg = d->message[i].msg + "(";
        if ( !d->message[i].type.isEmpty() )
            msg += d->message[i].type;
        msg += ")";

        // look for a service with this name, and get the channel from that if it exists
        QtopiaService service;
        bool isService = service.channel( d->message[i].channel ).length() != 0;
        if ( isService ) {
            QString channel = d->message[i].channel;
            QtopiaServiceRequest req( d->message[i].channel, msg );
            if ( !d->message[i].type.isEmpty() )
                req.addVariantArg(d->message[i].variant); // QtopiaServiceRequest will serialize the variant's value
            req.send();
        } else {
            QtopiaIpcEnvelope e( d->message[i].channel, msg );
            if ( !d->message[i].type.isEmpty() )
                QMetaType::save( e, d->message[i].variant.type(), d->message[i].variant.data() );
        }
    }
}

ThemeMessageData ThemeItem::parseMessage(const QString &message, bool *ok)
{
    //channel,msg([param])[,param1]
    if ( ok )
        *ok = true;
    ThemeMessageData data;
    QStringList tokens = message.split( QRegExp("[,()]" ), QString::SkipEmptyParts );
    if ( tokens.count() != 2 && tokens.count() != 4 ) {
        *ok = false;
        qWarning("Error parsing message for theme element with name '%s'", itemName().toAscii().constData());
        return ThemeMessageData();
    }

    data.channel = tokens[0]; // could be a service or channel, resolved at send time

    data.msg = tokens[1];

    if ( tokens.count() == 4 ) {
        // verify that the passed parameter type is the same type as
        data.type = tokens[2];
        if ( tokens[3].length() == 1 ) { // special case, passing a single character, store its unicode value so conversion to char is valid
            data.variant.setValue( tokens[3].at(0) );
        } else {
            data.variant.setValue( tokens[3] );
        }
        QVariant::Type t = QVariant::nameToType( data.type.toAscii().constData() );
        if ( !data.variant.canConvert( t ) ) {
            qWarning("Error parsing message for theme element with name '%s' - Cannot convert parameter '%s' with data type '%s' to data type '%s'", itemName().toAscii().constData(), tokens[3].toAscii().constData(), data.variant.typeName(), QVariant::typeToName(t));
            *ok = false;
            return ThemeMessageData();
        }
        bool ok = data.variant.convert( t );
        Q_ASSERT(ok == true);
        Q_UNUSED(ok);
    }
    return data;
}

/*!
  Destroys the ThemeItem.
  All child nodes are deleted.
 */
ThemeItem::~ThemeItem()
{
    foreach( ThemeItem *child, d->chldn )
        delete child;
    if ( d->activeExpr ) {
        delete d->activeExpr;
        d->activeExpr = 0;
    }
    delete d;
}

void ThemeItem::setVSPath( const QString &path )
{
    setAttribute( "vspath", path.trimmed() );
}


/*!
    Returns the \c vspath attribute for this item.
    This can be the attribute as it was specified for this item in the themed view XML,
    or in the case of ThemeTemplateInstanceItem, it will be a dynamically assigned
    uid.
    \sa ThemeTemplateInstanceItem
*/
QString ThemeItem::vsPath()
{
    return strAttribute("vspath");
}

/*!
  Returns the full vspath for this item.
  The full vspath is this item's vspath appended to the parent item's full vspath.
  A trailing slash is always included.
 */
QString ThemeItem::fullVSPath()
{
    QString parentPath;
    QString path = vsPath();
    if ( parentItem() != 0 ) {
        parentPath = parentItem()->fullVSPath();
    }
    if ( !parentPath.endsWith("/") )
        parentPath += "/"; // parent path always ends in a /, even if it is empty

    if ( path.startsWith("/") )
        path = path.remove(0,1); // so remove a leading / in our path

    if ( !path.isEmpty() && !path.endsWith("/") )
        path += "/"; // and add one incase we are the deepest caller

    return parentPath + path;
}

/*!
  \internal
*/
void ThemeItem::removeChild( ThemeItem* item )
{
    d->chldn.removeAll( item );
}

/*!
  \internal
 */
void ThemeItem::addChild( ThemeItem* item )
{
    d->chldn.append( item );
}


/*!
  \internal
*/
bool ThemeItem::isDataExpression() const {

    return d->isDataExpression;
}

/*!
  \internal
*/
void ThemeItem::setDataExpression(bool b)
{
    d->isDataExpression = b;
}

/*!
  Returns true if \a str is an expression from the themed view XML, false otherwise.
*/
bool ThemeItem::isStringExpression(const QString &str)
{
    QString s = str.trimmed();
    if ( s.startsWith( "expr:" ) )
        return true;
    return false;
}

/*!
  Strips the expression indicators from the expression \a str.
  isStringExpression(str) must return true, or this function will abort.
*/
QString ThemeItem::stripStringExpression(const QString &str)
{
    Q_ASSERT(isStringExpression(str));
    QString s = str.trimmed();
    s = s.remove( 0, QString("expr:").length() );
    return s;
}

/*!
   Called by ThemedView for a QExpressionEvaluator instance when it emits the Expression::termsChanged() signal.
   \a expression is an expression registered through a call to ThemedView::registerExpression().
   The Expression \a expression will be an expression that belongs to this item.
   Subclasses should make sure they call the base class version of this function should they not know about \a expression.
*/
void ThemeItem::expressionChanged(QExpressionEvaluator *expression)
{
    Q_ASSERT(d->activeExpr == expression);
    if (d->activeExpr == expression) {
        Q_ASSERT(d->activeExpr == expression);
        QVariant result = getExpressionResult(expression, QVariant::Bool);
        if (!result.canConvert(QVariant::Bool)) {
            qWarning() << "ThemeItem::expressionChanged() - Cannot convert value to Bool";
        } else {
            setActive(result.toBool());
        }
    }
}


/*!
Called by ThemedView when the item has finished being constructed from the themed view XML.
This call-back allows an item to perform any required calculations on its data with the knowledge that the data it has is complete.
Subclasses should always call the base class version of this function so that proper construction occurs at all levels.
 */
void ThemeItem::constructionComplete()
{
    Q_ASSERT(d->activeExpr == 0);
    QString expr = strAttribute("activeExpression");
    if ( !expr.isEmpty() ) {
        d->activeExpr = createExpression( expr );
        if ( d->activeExpr != 0 )
            ThemeItem::expressionChanged( d->activeExpr );
        setAttribute("activeExpression", QString());
    }
}

/*!
  Retreives a QVariant from \a expression that can be converted to the specified type \a type.
  It performs error checking to ensure \a expression is a valid expression and \c expression->result() returns a type that can be converted to \a type.
  If an error occurs, returns a QVariant that cannot be converted to type \a type.
  Returns a QVariant that can be converted to type \a type otherwise.
 */
QVariant ThemeItem::getExpressionResult(QExpressionEvaluator *expression, const QVariant::Type &type)
{
    Q_ASSERT(expression != 0);
    QVariant result;
    Q_ASSERT(expression->isValid());
    if (!(expression->evaluate())) {
        qWarning("Error: ThemeItem(name='%s')::getExpressionResult(): Error evaluating expression",
                 d->name.toAscii().data());
    } else {
        // a null result isn't error
        result = expression->result();
        if (result.isNull())
            result.setValue(QString(""));
        if (!result.canConvert(type)) {
            qWarning("Error: ThemeItem(name='%s')::getExpressionResult():  Variant returned by expression could not be converted to required variant type '%d'.", d->name.toAscii().data(), type);
        }
    }
    return result;
}

/*!
  Creates a new QExpressionEvaluator instance based on \a data.
  Any valuespace keys are expanded based on this item's fullVSPath().
  Performs error checking to ensure the expression given by \a data is valid.
  If no error occurs, registers the expression with ThemedView by calling ThemedView::registerExpression()
  and returns a pointer to the new expression, otherwise returns 0.
  */
QExpressionEvaluator* ThemeItem::createExpression(const QString &data)
{
    QExpressionEvaluator* expr = 0;
    if ( !data.isEmpty() ) {
        // prepend this item's fullVSPath to any vs terms in expression
        QString expandedData = data;
        expandedData.replace(QRegExp("'"), "\"");
        QString fp = fullVSPath();
        if ( !fp.isEmpty() && fp != "/") {
            expandedData = QString(data).replace( "@", "@"+fp );
            //qWarning() << "ThemeItem::createExpression: expanded:" << expandedData << " orig:" << data;
        }
        expr = new QExpressionEvaluator( expandedData.toUtf8() );
        if ( expr->isValid() )
            view()->registerExpression( this, expr );
        else {
            qWarning("ThemeItem(name='%s')::createExpression(): Invalid expression given", d->name.toAscii().data());
            delete expr;
            expr = 0;
        }
    }
    return expr;
}


/*!
  Stores the integer \a value for the state \a state based on the given storage \a key.
*/
void ThemeItem::setAttribute(const QString &key, const int &value, ThemeItem::State state)
{
    int _st = stateToIndex(state);
    QTagMap<int> &dataset = d->intattributes[_st];
    if (dataset.contains(key))
        dataset.remove(key);
    dataset.insert(key, value);
}

/*!
  Stores the integer \a value for the state \a state based on the given storage \a key.

  \since 4.3.1
*/
void ThemeItem::setAttribute(const QLatin1String &key, const int &value, ThemeItem::State state)
{
    int _st = stateToIndex(state);
    QTagMap<int> &dataset = d->intattributes[_st];
    if (dataset.contains(key))
        dataset.remove(key);
    dataset.insert(key, value);
}

/*!
  Returns an integer value for state \a state based on the given storage \a key.
*/
int ThemeItem::attribute(const QString &key, ThemeItem::State state) const
{
    int _st = stateToIndex(state);
    const QTagMap<int> &dataset = d->intattributes[_st];
    return dataset[key];
}

/*!
  Returns an integer value for state \a state based on the given storage \a key.

  \since 4.3.1
*/
int ThemeItem::attribute(const QLatin1String &key, ThemeItem::State state) const
{
    int _st = stateToIndex(state);
    const QTagMap<int> &dataset = d->intattributes[_st];
    return dataset[key];
}

/*!
  Stores the string \a value for state \a state based on the given storage \a key.
*/
void ThemeItem::setAttribute(const QString &key, const QString &value, ThemeItem::State state)
{
    int _st = stateToIndex(state);
    QTagMap<QString> &dataset = d->strattributes[_st];
    if (dataset.contains(key))
        dataset.remove(key);
    dataset.insert(key, value);
}

/*!
  Stores the string \a value for state \a state based on the given storage \a key.

  \since 4.3.1
*/
void ThemeItem::setAttribute(const QLatin1String &key, const QString &value, ThemeItem::State state)
{
    int _st = stateToIndex(state);
    QTagMap<QString> &dataset = d->strattributes[_st];
    if (dataset.contains(key))
        dataset.remove(key);
    dataset.insert(key, value);
}

/*!
  Returns the string value for state \a state based on the given storage \a key.
*/
QString ThemeItem::strAttribute(const QString &key, ThemeItem::State state)
{
    int _st = stateToIndex(state);
    const QTagMap<QString> &dataset = d->strattributes[_st];
    return dataset[key];
}

/*!
  Returns the string value for state \a state based on the given storage \a key.

  \since 4.3.1
*/
QString ThemeItem::strAttribute(const QLatin1String &key, ThemeItem::State state)
{
    int _st = stateToIndex(state);
    const QTagMap<QString> &dataset = d->strattributes[_st];
    return dataset[key];
}

/*!
  \internal
  */
void ThemeItem::paletteChange(const QPalette &)
{
}

/*!
  Returns the current state of this ThemeItem.
  \sa ThemeItem::State
*/
ThemeItem::State ThemeItem::state() const
{
    if (pressed()) {
        return ThemeItem::Pressed;
    } else if (hasFocus()) {
        return ThemeItem::Focus;
    } else {
        return ThemeItem::Default;
    }
}

/*!
  \internal
*/
void ThemeItem::setFocus(bool focus)
{
    if ( !isInteractive() )
        return;

    if ( focus != d->focus )
    {
        if ( active() && (rtti() != ThemedView::Status || (!d->focus || ((ThemeStatusItem *)this)->isOn())) )
        {
            ThemeItem::State oldState = state();
            d->focus = focus;
            if ( !d->focus )
                setPressed(false);

            if (oldState != state())
                stateChanged(state());
            if ( isVisible() )
                update();
        }
    }
}

/*!
  Called when an item's state is changed to \a state.
  The default implementation does nothing.
*/
void ThemeItem::stateChanged(const ThemeItem::State &state)
{
    Q_UNUSED(state);
}

/*!
  \internal
*/
void ThemeItem::setPressed(bool pressed)
{
    if ( !isInteractive() )
        return;

    if ( pressed != d->press ) // only allow presses if
    {
        if ( active() && (rtti() != ThemedView::Status || (!pressed || ((ThemeStatusItem *)this)->isOn())) )
        {
            d->press = pressed;
            if ( d->press )
                d->focus = true;
            if ( isVisible() )
                update();
        }
    }
}

/*!
  Returns the name attribute for this item.
  This is the value given for the name attribute in the themed view XML.
  */
QString ThemeItem::itemName() const
{
    return d->name;
}

/*!
  Returns the geometry of this item relative to the parent ThemedView.
  rect().x() and rect.y() are the ThemedView widget coordinates where
  this item is currently positioned.
*/
QRect ThemeItem::rect() const
{
    int x = d->br.x();
    int y = d->br.y();
    ThemeItem *item = d->parent;
    while (item) {
        x += item->d->br.x();
        y += item->d->br.y();
        item = item->parentItem();
    }
    return QRect(x, y, d->br.width(), d->br.height());
}

/*!
  Returns the geometry of this item, relative to its parent.
*/
QRect ThemeItem::geometry() const
{
    return d->br;
}

/*!
  Returns the rect for this item as literally specified in the themed view XML.
  */
QRectF ThemeItem::geometryHint()const
{
    return d->sr;
}

/*!
  Sets this item's run-time geometry to \a rect.
  This controls the position and size of the item in the ThemedView.
*/
void ThemeItem::setGeometry(const QRect& rect)
{
    d->br = rect;
}

/*!
  Returns true if this item has the themed view XML attribute interactive="yes", otherwise returns false.
*/
bool ThemeItem::isInteractive() const {
    return d->interactive;
}

/*!
  Returns true if the item is currently pressed, otherwise returns false.
 */
bool ThemeItem::pressed() const { return d->press; }

/*!
  Returns true if the item currently has focus, otherwise returns false.
*/
bool ThemeItem::hasFocus() const { return d->focus; }

/*!
  Returns the parent item of this item.
  Should always return an item, except for the ThemePageItem
  which will return 0.
*/
ThemeItem *ThemeItem::parentItem() const { return d->parent; }

/*!
  Returns the ThemedView this item belongs to.
*/
ThemedView* ThemeItem::view() const { return d->view; }

/*!
  Returns the children of this item.
*/
QList<ThemeItem*> ThemeItem::children() const { return d->chldn; }


ThemeItem* ThemeItem::parentLayout() const
{
    ThemeItem *parent = parentItem();
    while( parent != 0 ) {
        if ( parent->rtti() == ThemedView::Layout )
            return parent;
        parent = parent->parentItem();
     }
     return 0;
 }

/*!
  Returns true if this item is currently active, otherwise returns false.
  */
bool ThemeItem::active() const
{
    return d->actv;
}

/*!
  Sets this item to be active if \a active is true, otherwise sets the item to be inactive.
*/
void ThemeItem::setActive(bool active)
{
    if (active != d->actv) {
        d->actv = active;
        if ( !d->actv )
            d->press = false; //can't be pressed when not active
        view()->layout();
        // update me or my parent, position controlling layout?
        ThemeItem* item = this;
        ThemeItem *plhelp = 0;
        while( (plhelp = item->parentLayout()) )
            item = plhelp;
        item->update();
    }
}

/*!
  Returns true if this item has the themed view XML attribute transient="yes", otherwise returns false.
*/
bool ThemeItem::transient() const
{
    return d->transient;
}

/*!
  Returns true if the item is currently visible, otherwise returns false.
*/
bool ThemeItem::isVisible() const
{
    if ((!d->transient || d->actv) && d->visible) {
        if (parentItem())
            return parentItem()->isVisible();
        else
            return true;
    }

    return false;
}

/*!
  Sets this item to be visible if \a visible is true, otherwise hides the item.
*/
void ThemeItem::setVisible( bool visible )
{
    if ( visible != d->visible ) {
        d->visible = visible;
        view()->visChanged(this, visible);
    }
}

/*!
  \internal
  */
int ThemeItem::resolveUnit(qreal value, int bound, ThemeItem::Unit unit) const
{
    switch (unit) {
        case ThemeItem::Percent:
            return qRound(value * bound / 100.0);
        case ThemeItem::Point:
            return qRound(value * view()->physicalDpiY() / 72);
        default:
            return qRound(value);
    }
}

/*!
  Lays out the item.
  The item calcuates its actual position and size based on the abstract geometry information specified for the rect attribute in the themed view XML.
*/
void ThemeItem::layout()
{
    Q_ASSERT(parentItem() != 0);
    bool rtl = view()->layoutDirection() == Qt::RightToLeft;
    int pw = parentItem()->d->br.width();
    int ph = parentItem()->d->br.height();

    int c = resolveUnit(d->sr.left(), pw, d->runit[0]);
    d->br.setLeft(d->sr.left() < 0 ? pw + c : c);
    c = resolveUnit(d->sr.top(), ph, d->runit[1]);
    d->br.setTop(d->sr.top() < 0 ? ph + c : c);

    if (d->rmode == Coords) {
        c = resolveUnit(d->sr.right(), pw, d->runit[2]);
        d->br.setRight(d->sr.right() <= 0 ? pw + c : c);
        c = resolveUnit(d->sr.bottom(), ph, d->runit[3]);
        d->br.setBottom(d->sr.bottom() <= 0 ? ph + c : c);
    } else {
        d->br.setWidth(!d->sr.width() ? pw - d->br.x() : resolveUnit(d->sr.width(), pw, d->runit[2]));
        d->br.setHeight(!d->sr.height() ? ph - d->br.y() : resolveUnit(d->sr.height(), ph, d->runit[3]));
    }

    if ( rtl && d->br.width() < parentItem()->d->br.width() )
        d->br.moveRight( parentItem()->d->br.width() - d->br.left() - 1 );
}

/*!
  Updates this item.
  Causes this item to be repainted immediately.
*/
void ThemeItem::update()
{
    update(d->br.x(), d->br.y(), d->br.width(), d->br.height());
}

/*!
  \overload
  Causes the area given by \a x, \a y, \a w, \a h of this item to be repainted immediately.
*/
void ThemeItem::update(int x, int y, int w, int h)
{
    ThemeItem *item = parentItem();
    while (item) {
        x += item->d->br.x();
        y += item->d->br.y();
        item = item->parentItem();
    }

    view()->update(x, y, w, h);
}

/*!
  Run-time type information.
  Returns ThemedView::Item.
  */
int ThemeItem::rtti() const
{
    return ThemedView::Item;
}

/*!
    Paints the \a rect portion of the item using the painter \a painter.
    Normally overridden by sub-classes.
*/
void ThemeItem::paint(QPainter *painter, const QRect &rect)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
}

QRectF ThemeItem::parseRect(const ThemeAttributes &atts, RMode &mode, const QString &name)
{
    mode = ThemeItem::Rect;
    QString rn(name.isEmpty() ? QString(QLatin1String("rect")) : name);
    QString val = atts.value(rn);
    QRectF rect;
    if (!val.isEmpty()) {
        const QChar percent('%');
        const QString pt("pt");
        qreal coords[4];
        QStringList args = atts.value(rn).split(',');
        if (args.count() == 4) {
            mode = Coords;
            for (int i = 0; i < 4; i++) {
                if (args[i].contains(percent)) {
                    d->runit[i] = ThemeItem::Percent;
                    coords[i] = args[i].remove(percent).toFloat();
                } else if (args[i].contains(pt)) {
                    d->runit[i] = ThemeItem::Point;
                    coords[i] = args[i].remove(pt).toFloat();
                } else {
                    d->runit[i] = ThemeItem::Pixel;
                    coords[i] = args[i].toFloat();
                }
            }
            rect.setCoords(coords[0], coords[1], coords[2], coords[3]);
        } else if (args.count() == 3 && args[2].contains('x')) {
            QString size = args[2];
            args.removeLast();
            args += size.split('x');
            for (int i = 0; i < 4; i++) {
                if (args[i].contains(percent)) {
                    d->runit[i] = ThemeItem::Percent;
                    coords[i] = args[i].remove(percent).toFloat();
                } else if (args[i].contains(pt)) {
                    d->runit[i] = ThemeItem::Point;
                    coords[i] = args[i].remove(pt).toFloat();
                } else {
                    d->runit[i] = ThemeItem::Pixel;
                    coords[i] = args[i].toFloat();
                }
            }
            rect.setRect(coords[0], coords[1], coords[2], coords[3]);
        }
    } else {
        // No rect means use all available space
        rect.setRect(0, 0, 0, 0);
        for (int i = 0; i < 4; i++)
            d->runit[i]= Pixel;
    }
    return rect;
}

/*!
  \internal
*/
/*
  Parses sub-attributes into key => value pairs from the attribute value \a subatts specified in the themed view XML.
  Sub attributes are attributes specified within themed view XML attributes.  For example if \a subatts contained
  "active=no;transient=yes", then this function would return a map containing "active" => "no", "transient" => "yes".
  */
QMap<QString,QString> ThemeItem::parseSubAtts( const QString &subatts ) const
{
    QMap<QString,QString> subAtts;
    if ( subatts.trimmed().isEmpty() )
        return subAtts;
    QStringList assignments = subatts.split(";");
    for( int i = 0 ; i < assignments.count() ; ++i )
    {
        QStringList assignment = assignments[i].split("=");
        if ( assignment.count() == 2 )
            subAtts.insert( assignment[0], assignment[1] );
    }
    return subAtts;
}

/*!
  \internal
*/
int ThemeItem::parseAlignment(const ThemeAttributes &atts, const QString &name)
{
    int align = Qt::AlignLeft | Qt::AlignTop;

    QString val = atts.value(name.isEmpty() ? QString(QLatin1String("align")) : name);
    if (!val.isEmpty()) {
        QStringList list = val.split(',');
        QStringList::Iterator it;
        for (it = list.begin(); it != list.end(); ++it) {
            val = *it;
            if (val == QLatin1String("right"))
                align = (align & Qt::AlignVertical_Mask) | Qt::AlignRight;
            else if (val == QLatin1String("hcenter"))
                align = (align & Qt::AlignVertical_Mask) | Qt::AlignHCenter;
            else if (val == QLatin1String("bottom"))
                align = (align & Qt::AlignHorizontal_Mask) | Qt::AlignBottom;
            else if (val == QLatin1String("vcenter"))
                align = (align & Qt::AlignHorizontal_Mask) | Qt::AlignVCenter;
            else if (val == QLatin1String("center"))
                align = Qt::AlignHCenter | Qt::AlignVCenter;
        }
    }

    return align;
}

/*!
  Adds the characters \a characters to this item.
  Normally overridden by subclasses.
*/
void ThemeItem::addCharacters(const QString &characters)
{
    Q_UNUSED(characters);
}

//---------------------------------------------------------------------------

/*!
  \class ThemeGroupItem
    \inpublicgroup QtBaseModule


  \brief The ThemeGroupItem class groups its children into a single item in a ThemedView.

  The ThemeGroupItem class implements the \l{Themed View Elements#themegroupelement}{group element} from the theme XML.

  This is useful when a single logical item is made up of multiple graphical components.
  Eg. a ThemeGroupItem item may represent a single button by grouping multiple text and image items
  together.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

  \sa {Themed View Elements#themegroupelement}{group element}

  \ingroup appearance
*/

/*!
  Constructs a ThemeGroupItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeGroupItem::ThemeGroupItem( ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts )
    : ThemeItem(parent, view, atts)
{
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Group.
*/
int ThemeGroupItem::rtti() const
{
    return ThemedView::Group;
}

/*!
    \reimp
*/
void ThemeGroupItem::setPressed( bool p )
{
    ThemeItem::setPressed( p );
    QList<ThemeItem*> c = children();
    foreach( ThemeItem *item, c )
        item->setPressed( p );
}


//---------------------------------------------------------------------------

/*!
  \class ThemePluginItem
    \inpublicgroup QtBaseModule


  \brief The ThemePluginItem class manages the interaction between a ThemedItemInterface implementation and a ThemedView.

  The ThemePluginItem class implements the \l{Themed View Elements#themepluginelement}{plugin element} from the theme XML.

  You can explicitly assign a new plug-in interface using the setPlugin() or setBuiltin() functions.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

    \sa {Themed View Elements#themepluginelement}{plugin element}
  \ingroup appearance
*/
struct ThemePluginItemPrivate
{
    ThemePluginItemPrivate()
        : loader(0), iface(0), builtin(false)
    {
    }
    QPluginManager *loader;
    ThemedItemPlugin *iface;
    bool builtin;
};

/*!
  Constructs a ThemePluginItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemePluginItem::ThemePluginItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeItem(parent, view, atts)
{
    d = new ThemePluginItemPrivate;
}

/*!
  Destroys the ThemePluginItem.
*/
ThemePluginItem::~ThemePluginItem()
{
    releasePlugin();
    delete d->loader;
    delete d;
}

/*!
  Loads a new plug-in using QPluginManager based on \a name.
*/
void ThemePluginItem::setPlugin(const QString &name)
{
    if (!d->loader)
        d->loader = new QPluginManager(QLatin1String("themedview"));
    releasePlugin();
    d->iface = qobject_cast<ThemedItemPlugin*>(d->loader->instance(name));
    d->builtin = false;
    d->iface->resize(geometry().width(), geometry().height());
    if (isVisible())
        update();
}

/*!
    Sets the plug-in for the item to be the already constructed plug-in pointed to by \a plugin.
*/
void ThemePluginItem::setBuiltin(ThemedItemPlugin *plugin)
{
    releasePlugin();
    d->iface = plugin;
    if (d->iface) {
        d->builtin = true;
        d->iface->resize(geometry().width(), geometry().height());
    }
    if (isVisible())
        update();
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Plugin.
*/
int ThemePluginItem::rtti() const
{
    return ThemedView::Plugin;
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a painter.
*/
void ThemePluginItem::paint(QPainter *painter, const QRect &r)
{
    if (d->iface)
        d->iface->paint(painter, r);
}

/*!
  \reimp
  Lays out the plug-in item.
  The plug-in item resizes its plug-in to match the geometry specified for this item in the themed view XML.
*/
void ThemePluginItem::layout()
{
    QSize oldSize = geometry().size();
    ThemeItem::layout();
    if (d->iface && geometry().size() != oldSize) {
        d->iface->resize(geometry().width(), geometry().height());
    }
}

void ThemePluginItem::releasePlugin()
{
    delete d->iface;
    d->iface = 0;
}

//---------------------------------------------------------------------------

/*!
  \class ThemeExclusiveItem
    \inpublicgroup QtBaseModule


  \brief The ThemeExclusiveItem class allows only one of its children to be active in a ThemedView.

  The ThemeExclusiveItem class implements the \l{Themed View Elements#themeexclusiveelement}{exclusive element} from the theme XML.

  This is useful for a number of items where only one can logically be active/visible at a time.
  When a new child item becomes active, all other items are set to be inactive.
  If there are two child items, and one becomes inactive, the other is set to be active.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

  \sa {Themed View Elements#themeexclusiveelement}{exclusive element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeExclusiveItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeExclusiveItem::ThemeExclusiveItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeItem(parent, view, atts)
{
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Exclusive.
*/
int ThemeExclusiveItem::rtti() const
{
    return ThemedView::Exclusive;
}

/*!
  \reimp
  Lays out the exclusive item.
  The exclusive item iterates through its children in reverse order, sets the first active found item to be visible, and sets all other items to not be visible.
  \sa setVisible(), isVisible()
*/
void ThemeExclusiveItem::layout()
{
    ThemeItem::layout();
    QList<ThemeItem*> c = children();
    if ( c.count() != 0 ) {
        bool found = false;
        for( int j = c.count()-1 ; j >= 0 ; --j ) {
            ThemeItem *item = c[j];
            if (item->active() && !found) {
                item->setVisible(true);
                found = true;
            } else {
                item->setVisible(false);
            }
        }
    }
}

//---------------------------------------------------------------------------

struct ThemeLayoutItemPrivate
{
    ThemeLayoutItemPrivate()
        : spacing(0), align(0)
    {
    }
    Qt::Orientation orient;
    int spacing;
    int align;
};

/*!
  \class ThemeLayoutItem
    \inpublicgroup QtBaseModule


  \brief The ThemeLayoutItem class positions and resizes its child items in a ThemedView.

  The ThemeLayoutItem class implements the \l{Themed View Elements#themelayoutelement}{layout element} from the theme XML.

  ThemeLayoutItem does not display anything to the user - it simply manages the
  geometry of any child items.

  \sa {Themed View Elements#themelayoutelement}{layout element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeLayoutItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeLayoutItem::ThemeLayoutItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeItem(parent, view, atts)
{
    d = new ThemeLayoutItemPrivate;
    d->orient = atts.value(QLatin1String("orientation")) == QLatin1String("vertical") ? Qt::Vertical : Qt::Horizontal;
    QString val = atts.value(QLatin1String("spacing"));
    if (!val.isEmpty())
        d->spacing = val.toInt();
    d->align = parseAlignment(atts);
    //stretch = atts.value(QLatin1String("stretch")) == QLatin1String("yes");
}


/*!
  Destroys the ThemeLayoutItem.
*/
ThemeLayoutItem::~ThemeLayoutItem()
{
    delete d;
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Layout.
*/
int ThemeLayoutItem::rtti() const
{
    return ThemedView::Layout;
}

/*!
  \reimp
*/
void ThemeLayoutItem::paint(QPainter *p, const QRect &r)
{
    Q_UNUSED(p);
    Q_UNUSED(r);
    qLog(UI) << "ThemeLayoutItem::paint has been called, a ThemeLayoutItem subclass is missing an implementation of paint?";
}

/*!
  \reimp
  Lays out this item.
  The layout item positions and resizes its children.
*/
void ThemeLayoutItem::layout()
{
    ThemeItem::layout();
    int fixedSize = 0;
    int visCount = 0;
    int expandingCount = 0;
    int w, h;
    QList<ThemeItem*> c = children();
    foreach ( ThemeItem *item, c ) {
        if ((!item->transient() || item->active()) && item->isVisible()) {
            visCount++;
            item->layout(); // need to layout child items now, in order to get their width/height
            if (d->orient == Qt::Horizontal) {
                w = qRound(item->d->sr.width());
                if ( item->attribute("expanding") != 0 || w == 0)
                    ++expandingCount;
                else
                    fixedSize += resolveUnit(w, geometry().width(), item->d->runit[2]);
            } else {
                h = qRound(item->d->sr.height());
                if ( item->attribute("expanding") != 0 || h == 0)
                    ++expandingCount;
                else {
                    fixedSize += resolveUnit(h, geometry().height(), item->d->runit[3]);
                }
            }
        }
    }
    if (!visCount)
        return;

    int offs = 0;
    int expansionSize = 0;
    int size = d->orient == Qt::Horizontal ? geometry().width() : geometry().height();
    if (expandingCount > 0) {
        expansionSize = (size - fixedSize - (visCount-1)*d->spacing)/expandingCount;
    } else {
        if (d->align & Qt::AlignHCenter || d->align & Qt::AlignVCenter)
            offs = (size - fixedSize - (visCount-1)*d->spacing)/2;
        else if (d->align & Qt::AlignRight)
            offs = size - fixedSize - (visCount-1)*d->spacing;
    }
    foreach ( ThemeItem *item, c ) {
        if ((!item->transient() || item->active()) && item->isVisible()) {
            if (d->orient == Qt::Horizontal) {
                w = qRound(item->d->sr.width());
                if ( item->attribute("expanding") != 0 || w == 0) {
                    w = expansionSize;
                    item->d->runit[2] = Pixel;
                } else {
                    w = item->geometry().width();
                }

                item->setGeometry(QRect(offs,0,w,geometry().height()));
                offs += qAbs(resolveUnit(item->geometry().width(), geometry().width(), item->d->runit[2]));
            } else {
                h = qRound(item->d->sr.height());
                if (item->attribute("expanding") || h == 0) {
                    h = expansionSize;
                    item->d->runit[3] = Pixel;
                } else {
                    h = item->geometry().height();
                }

                item->setGeometry(QRect(0,offs,geometry().width(),h));
                offs += qAbs(resolveUnit(item->geometry().height(), geometry().height(), item->d->runit[3]));
            }
            offs += d->spacing;
        }
    }
}

//---------------------------------------------------------------------------

struct ThemePageItemPrivate
{
    ThemePageItemPrivate()
        : stretch(false)
    {
    }
    QPixmap bg;
    QString bd;
    bool stretch;
    QString maskImg;
    int offs[2];
    Qt::Orientation sorient;
};
/*!
  \class ThemePageItem
    \inpublicgroup QtBaseModule


  \brief The ThemePageItem class is the root item in a ThemedView.

  The ThemePageItem class implements the \l{Themed View Elements#themepageelement}{page element} from the theme XML.

  The ThemePageItem contains all other items in the theme item tree.
  It is responsible for loading and displaying a background image.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

  \sa {Themed View Elements#themepageelement}{page element}
  \ingroup appearance
*/


/*!
  Constructs a ThemePageItem.
  \a view and \a atts are passed to the base class constructor.
  */
ThemePageItem::ThemePageItem(ThemedView *view, const ThemeAttributes &atts)
    : ThemeItem(0, view, atts)
{
    d = new ThemePageItemPrivate;
    d->bd = atts.value(QLatin1String("base"));
    if (d->bd[d->bd.length()-1] != '/')
        d->bd.append(QLatin1String("/"));
    QString val = atts.value(QLatin1String("background"));
    if (!val.isEmpty()) {
        d->bg = QPixmap(QLatin1String(":image/")+d->bd+val);
        if ( d->bg.isNull() )
            d->bg = QPixmap(QLatin1String(":image/")+view->defaultPics()+val);
    }
    view->clearMask();
    d->maskImg = atts.value(QLatin1String("mask"));
    val = atts.value(QLatin1String("stretch"));
    if (!val.isEmpty()) {
        QStringList ol = val.split(',');
        if (ol.count() == 2) {
            d->stretch = true;
            d->offs[0] = ol[0].toInt();
            d->offs[1] = ol[1].toInt();
        }
        d->sorient = atts.value(QLatin1String("orientation")) == QLatin1String("vertical") ? Qt::Vertical : Qt::Horizontal;
    }
    if (atts.value(QLatin1String("transparent")) == QLatin1String("yes")) {
        QPalette pal = view->palette();
        pal.setColor(QPalette::All, QPalette::Window, QColor(0,0,0,0));
        view->setPalette(pal);
    }
}

/*!
  Destroys the ThemePageItem.
*/
ThemePageItem::~ThemePageItem()
{
    delete d;
}

/*!
  Returns the base directory of for the ThemedView.
  This is the theme-specific directory where theme resources such as images are located.
*/
const QString ThemePageItem::base() const { return d->bd; }

void ThemePageItem::applyMask()
{
    if (!geometry().width() || !geometry().isValid())
        return;

    if (!d->maskImg.isEmpty()) {
        QImage maskImage = QImage(QLatin1String(":image/")+d->bd+d->maskImg);
        if (maskImage.isNull())
            maskImage = QImage(QLatin1String(":image/")+view()->defaultPics());

        QBitmap mask;
        if (!maskImage.isNull()) {
            if (maskImage.depth() == 1) {
                mask = QBitmap::fromImage(maskImage);
            } else if (maskImage.hasAlphaChannel()) {
                QPixmap pm = QPixmap::fromImage(maskImage);
                if (!pm.mask().isNull())
                    mask = pm.mask();
            }
        }

        QRegion maskRgn;
        if (d->stretch) {
            QRegion r = QRegion(mask);
            QRegion rgn[3];
            rgn[0] = r & QRegion(0,0,d->offs[0],mask.height());
            rgn[1] = r & QRegion(d->offs[0],0,d->offs[1]-d->offs[0],mask.height());
            rgn[2] = r & QRegion(d->offs[1],0,mask.width()-d->offs[1],mask.height());

            QRegion trgn;
            trgn = rgn[0].isEmpty() ? QRect(0, 0, d->offs[0], mask.height()) : rgn[0];
            maskRgn = trgn;

            int w = geometry().width() - d->offs[0] - (mask.width()-d->offs[1]);
            if (!rgn[1].isEmpty()) {
                QRegion tmp(rgn[1]);
                int sw = d->offs[1] - d->offs[0];
                for (int x = 0; x < w; x+=sw) {
                    trgn += tmp;
                    tmp.translate(sw, 0);
                }
                trgn &= QRect(d->offs[0], 0, w, mask.height());
            } else {
                trgn = QRect(d->offs[0], 0, w, mask.height());
            }
            maskRgn += trgn;

            trgn = rgn[2].isEmpty() ? QRect(d->offs[1], 0, mask.width()-d->offs[1], mask.height()) : rgn[2];
            trgn.translate(geometry().width()-mask.width(),0);
            maskRgn += trgn;
        } else {
            maskRgn = QRegion(mask);
        }
        view()->setMask(maskRgn);
    }
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Page.
*/
int ThemePageItem::rtti() const
{
    return ThemedView::Page;
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a painter.
*/
void ThemePageItem::paint(QPainter *painter, const QRect &rect)
{
    painter->save();
    painter->setClipRect(rect);
    if (!d->bg.isNull()) {
        if (!d->stretch) {
            painter->drawPixmap(0, 0, d->bg);
        } else if (d->sorient == Qt::Horizontal) {
            int h = d->bg.height();
            painter->drawPixmap(0, 0, d->bg, 0, 0, d->offs[0], h);
            int w = geometry().width() - d->offs[0] - (d->bg.width()-d->offs[1]);
            int sw = d->offs[1]-d->offs[0];
            int x = 0;
            if (sw) {
                for (; x < w-sw; x+=sw)
                    painter->drawPixmap(d->offs[0]+x, 0, d->bg, d->offs[0], 0, sw, h);
            }
            painter->drawPixmap(d->offs[0]+x, 0, d->bg, d->offs[0], 0, w-x, h);
            painter->drawPixmap(geometry().width()-(d->bg.width()-d->offs[1]), 0, d->bg, d->offs[1], 0, d->bg.width()-d->offs[1], h);
        } else {
            int w = d->bg.width();
            painter->drawPixmap(0, 0, d->bg, 0, 0, w, d->offs[0]);
            int h = geometry().height() - d->offs[0] - (d->bg.height()-d->offs[1]);
            int sh = d->offs[1]-d->offs[0];
            int y = 0;
            if (d->offs[1]-d->offs[0]) {
                for (; y < h-sh; y+=sh)
                    painter->drawPixmap(0, d->offs[0]+y, d->bg, 0, d->offs[0], w, sh);
            }
                painter->drawPixmap(0, d->offs[0]+y, d->bg, 0, d->offs[0], w, h-y);
            painter->drawPixmap(0, geometry().height()-(d->bg.height()-d->offs[1]), d->bg, 0, d->offs[1], w, d->bg.height()-d->offs[1]);
        }
    }
    painter->restore();
}

QRectF ThemePageItem::calcPageGeometry(const QSize &defSize) const
{
    QSize ps = defSize;
    if (view()->isWindow()) {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect desktopRect(desktop->screenGeometry(desktop->screenNumber(view())));
        if (ThemeItem::d->runit[2] == ThemeItem::Percent)
            ps.setWidth(desktopRect.width());
        if (ThemeItem::d->runit[3] == ThemeItem::Percent)
            ps.setHeight(desktopRect.height());
    } else {
        if (ThemeItem::d->runit[2] == ThemeItem::Percent)
            ps.setWidth(view()->parentWidget()->width());
        if (ThemeItem::d->runit[3] == ThemeItem::Percent)
            ps.setHeight(view()->parentWidget()->height());
    }

    QRectF pgr = geometryHint();
    if (!pgr.width()) {
        pgr.setWidth(ps.width());
    } else if (pgr.width() < 0) {
        pgr.setX(ps.width()+pgr.width()-pgr.x());
        pgr.setWidth(-pgr.width());
    } else {
        pgr.setWidth(resolveUnit(pgr.width(), ps.width(), ThemeItem::d->runit[2]));
    }
    if (!pgr.height()) {
        pgr.setHeight(ps.height());
    } else if (pgr.height() < 0) {
        pgr.setY(ps.height()+pgr.height()-pgr.y());
        pgr.setHeight(-pgr.height());
    } else {
        pgr.setHeight(resolveUnit(pgr.height(), ps.height(), ThemeItem::d->runit[3]));
    }

    return pgr;
}

/*!
  \reimp
  Lays out the page item.
*/
void ThemePageItem::layout()
{
    int oldWidth = geometry().width();
    QSize ps = view()->size();

    QRectF pgr = calcPageGeometry(ps);

    setGeometry( pgr.toRect() );
    if (!d->maskImg.isEmpty() && pgr.width() != oldWidth)
        applyMask();
}

/*!
Returns a suggested size for the page.
This is directly returned by ThemedView::sizeHint().
*/
QSize ThemePageItem::sizeHint() const
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect(desktop->screenGeometry(desktop->screenNumber(view())));
    QSize ps = view()->isWindow() ? desktopRect.size()
                                : view()->parentWidget()->size();

    QSize sh = calcPageGeometry(ps).toRect().size();

    return sh;
}

//---------------------------------------------------------------------------

struct ThemeGraphicItemPrivate
{
    QTagMap<QColor> colors[3];
    QTagMap<QFont> fonts[3];
};

/*!
  \class ThemeGraphicItem
    \inpublicgroup QtBaseModule


  \brief The ThemeGraphicItem class contains common functionality for graphics based items in a ThemedView.

  Many items share common graphics related functionality including ThemeRectItem, ThemeTextItem and ThemeLineItem.
  ThemeGraphicItem provides functionality and data storage for graphics related items.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.
  \ingroup appearance
*/

/*!
  Constructs a ThemeGraphicItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeGraphicItem::ThemeGraphicItem( ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts )
    : ThemeItem(parent, view, atts)
{
    d = new ThemeGraphicItemPrivate;
}

/*!
  Destroys the ThemeGraphicItem
*/
ThemeGraphicItem::~ThemeGraphicItem()
{
    delete d;
}

/*!
  Stores the given color \a color based on the given storage \a key and state \a state.
*/
void ThemeGraphicItem::setupColor( const QString &key, const QString &color, ThemeItem::State state )
{
    QColor _color;
    int role = parseColor( color, _color );
    if ( role == QPalette::NColorRoles && state != ThemeItem::Default )
    {
        role = attribute( key + QLatin1String("Role") );
        _color = this->color( key );
    }
    setAttribute( key + QLatin1String("Role"), role, state );
    setColor( key, _color, state );
}

/*!
  Stores the given color \a color based on the given storage \a key role key \a roleKey and state \a state.

  \since 4.3.1
*/
void ThemeGraphicItem::setupColor( const QLatin1String &key, const QLatin1String& roleKey, const QString &color, ThemeItem::State state )
{
    QColor _color;
    int role = parseColor( color, _color );
    if ( role == QPalette::NColorRoles && state != ThemeItem::Default )
    {
        role = attribute( roleKey );
        _color = this->color( key );
    }
    setAttribute( roleKey, role, state );
    setColor( key, _color, state );
}

/*!
  Stores the alpha attribute \a alpha for this item based on the given storage \a key and state \a state.
*/
void ThemeGraphicItem::setupAlpha( const QString &key, const QString &alpha, ThemeItem::State state )
{
    bool ok;
    int _alpha = alpha.toInt(&ok);
    Q_UNUSED(_alpha);
    if (ok) {
        //still set a string version, so can easily check for 'emptiness' in usage case
        setAttribute( key, alpha, state );
    }
}

/*!
  Stores the alpha attribute \a alpha for this item based on the given storage \a key and state \a state.

  \since 4.3.1
*/
void ThemeGraphicItem::setupAlpha( const QLatin1String &key, const QString &alpha, ThemeItem::State state )
{
    bool ok;
    int _alpha = alpha.toInt(&ok);
    Q_UNUSED(_alpha);
    if (ok) {
        //still set a string version, so can easily check for 'emptiness' in usage case
        setAttribute( key, alpha, state );
    }
}

/*!
  Stores the color \a value based on the given storage \a key and state \a state.
*/
void ThemeGraphicItem::setColor( const QString &key, const QColor &value, ThemeItem::State state )
{
    int _st = stateToIndex( state );
    QTagMap<QColor> &dataset = d->colors[_st];
    if ( dataset.contains( key ) )
        dataset.remove( key );
    dataset.insert( key, value );
}

/*!
  Stores the color \a value based on the given storage \a key and state \a state.

  \since 4.3.1
*/
void ThemeGraphicItem::setColor( const QLatin1String &key, const QColor &value, ThemeItem::State state )
{
    int _st = stateToIndex( state );
    QTagMap<QColor> &dataset = d->colors[_st];
    if ( dataset.contains( key ) )
        dataset.remove( key );
    dataset.insert( key, value );
}

/*!
  Returns the color for the given storage key \a key and state \a state.
*/
QColor ThemeGraphicItem::color( const QString &key, ThemeItem::State state ) const
{
    int _st = stateToIndex( state );
    const QTagMap<QColor> &dataset = d->colors[_st];
    return dataset[key];
}

/*!
  Returns the color for the given storage key \a key and state \a state.

  \since 4.3.1
*/
QColor ThemeGraphicItem::color( const QLatin1String &key, ThemeItem::State state ) const
{
    int _st = stateToIndex( state );
    const QTagMap<QColor> &dataset = d->colors[_st];
    return dataset[key];
}

/*!
  Stores the font \a value based on the given storage \a key and state \a state.
*/
void ThemeGraphicItem::setFont( const QString &key, const QFont &value, ThemeItem::State state )
{
    int _st = stateToIndex( state );
    QTagMap<QFont> &dataset = d->fonts[_st];
    if ( dataset.contains( key ) )
        dataset.remove( key );
    dataset.insert( key, value );
}

/*!
  Stores the font \a value based on the given storage \a key and state \a state.

  \since 4.3.1
*/
void ThemeGraphicItem::setFont( const QLatin1String &key, const QFont &value, ThemeItem::State state )
{
    int _st = stateToIndex( state );
    QTagMap<QFont> &dataset = d->fonts[_st];
    if ( dataset.contains( key ) )
        dataset.remove( key );
    dataset.insert( key, value );
}

/*!
  Returns the font for the given storage \a key and state \a state.
*/
QFont ThemeGraphicItem::font( const QString &key, ThemeItem::State state ) const
{
    int _st = stateToIndex( state );
    const QTagMap<QFont> &dataset = d->fonts[_st];
    return dataset[key];
}

/*!
  Returns the font for the given storage \a key and state \a state.

  \since 4.3.1
*/
QFont ThemeGraphicItem::font( const QLatin1String &key, ThemeItem::State state ) const
{
    int _st = stateToIndex( state );
    const QTagMap<QFont> &dataset = d->fonts[_st];
    return dataset[key];
}

/*!
  Returns \a defFont modified to be the point size given by \a size, and to be bold if \a bold is "yes" or normal weight if \a bold is
  any other non-empty string.
 */
QFont ThemeGraphicItem::parseFont(const QFont &defFont, const QString &size, const QString &bold)
{
    QFont font(defFont);
    if (!size.isEmpty()) {
        int pSize = size.toInt();
        if (pSize <= 0) {
            qWarning() << "Font size is invalid:" << size;
            pSize = 1;
        }
        font.setPointSize(pSize);
    }
    if (!bold.isEmpty())
    {
        bool b = bold == QLatin1String("yes");
        font.setWeight(b ? QFont::Bold : QFont::Normal);
    }

    return font;
}

/*!
  Parses the color given by the color name \a value.
  \a value can either be a palette color name, eg. HighlightedText, or a name
  suitable for being passed to QColor::setNamedColor().
  If \a value references the palette, this function returns the QPalette::ColorRole
  for the color and does not modify \a color.
  Otherwise, \a value is passed to QColor::setNamedColor(), the result is stored in \a color,
  and the function returns QPalette::NColorRoles+1.
*/
int ThemeGraphicItem::parseColor(const QString &value, QColor &color)
{
    int role = QPalette::NColorRoles;
    if (!value.isEmpty()) {
        if (value.startsWith(QChar('#'))) {
            role = QPalette::NColorRoles+1;
            color.setNamedColor(value);
        } else {
            int i = 0;
            while (colorTable[i].name) {
                if (QString(colorTable[i].name).toLower() == value.toLower()) {
                    role = colorTable[i].role;
                break;
            }
                i++;
            }
            if (!colorTable[i].name) {
                role = QPalette::NColorRoles+1;
                color.setNamedColor(value);
            }
        }
    }

    return role;
}

/*!
  Returns a QColor object for either \a color or \a role.
  If role is a valid QPalette::ColorRole, the corresponding
  QColor object for the role is returned.
  Otherwise, \a color is returned.
*/
QColor ThemeGraphicItem::getColor(const QColor &color, int role) const
{
    if (role < QPalette::NColorRoles)
        return view()->palette().color((QPalette::ColorRole)role);
    else
        return color;
}

//---------------------------------------------------------------------------

struct ThemeTextItemPrivate {
    ThemeTextItemPrivate()
        : shortLbl(false), shadow(0), shadowImg(0), format(Qt::AutoText), richText(false), elidedMode(-1),
        textExpr(0)
    {}
    QString displayText;
    bool shortLbl;
    int shadow;
    QImage *shadowImg;
    int align;
    Qt::TextFormat format;
    bool richText;
    int elidedMode;
    QString themeText;
    QExpressionEvaluator* textExpr;
};

/*!
  \class ThemeTextItem
    \inpublicgroup QtBaseModule

  \brief The ThemeTextItem class represents a text in a ThemedView.

  The ThemeTextItem class implements the \l{Themed View Elements#themetextelement}{text element} from the theme XML.

    The text displayed can be set manually by calling setText() or accessed through text().
    The format of the text can be set explicitly using the setTextFormat() function.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant data for this item in the themed view XML or in the valuespace.

    \sa {Themed View Elements#themetextelement}{text element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeTextItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeTextItem::ThemeTextItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeGraphicItem(parent, view, atts)

{
    d = new ThemeTextItemPrivate;

    QMap<QString,QString> onClickAtts, onFocusAtts;
    if (isInteractive()) {
        onClickAtts = parseSubAtts(atts.value(QLatin1String("onclick")));
        onFocusAtts = parseSubAtts(atts.value(QLatin1String("onfocus")));
    }
    setupFont(view->font(), atts.value(QLatin1String("size")), atts.value(QLatin1String("bold")), atts.value(QLatin1String("color")), atts.value(QLatin1String("outline")));
    setupFont(font(QLatin1String("font")), onClickAtts[QLatin1String("size")], onClickAtts[QLatin1String("bold")], onClickAtts[QLatin1String("color")], onClickAtts[QLatin1String("outline")], ThemeItem::Pressed);
    setupFont(font(QLatin1String("font")), onFocusAtts[QLatin1String("size")], onFocusAtts[QLatin1String("bold")], onFocusAtts[QLatin1String("color")], onFocusAtts[QLatin1String("outline")], ThemeItem::Focus);

    QString val = atts.value(QLatin1String("shadow"));
    if (!val.isEmpty())
        d->shadow = qMin(val.toInt(), 255);
    d->align = parseAlignment(atts);
    d->shortLbl = atts.value(QLatin1String("short")) == QLatin1String("yes");
    val = atts.value(QLatin1String("format"));
    if (!val.isEmpty()) {
        if (val == QLatin1String("RichText")) {
            d->format = Qt::RichText;
            d->richText = true;
        } else if (val == QLatin1String("PlainText")) {
            d->format = Qt::PlainText;
        }
    }
    val = atts.value(QLatin1String("elided"));
    if (!val.isEmpty()) {
        if (val == QLatin1String("left"))
            d->elidedMode = Qt::ElideLeft;
        else if (val == QLatin1String("right"))
            d->elidedMode = Qt::ElideRight;
        else if (val == QLatin1String("middle"))
            d->elidedMode = Qt::ElideMiddle;
    }
}

/*!
  Destroys the ThemeTextItem.
*/
ThemeTextItem::~ThemeTextItem()
{
    delete d->shadowImg;
    if (d->textExpr) {
        delete d->textExpr;
        d->textExpr = 0;
    }
    delete d;
}

/*!
  \internal
  Stores the font related data given by \a deffont, \a size, \a bold, \a col and \a outline for the given state \a state.
*/
void ThemeTextItem::setupFont(const QFont &deffont, const QString &size, const QString &bold, const QString &col, const QString &outline, ThemeItem::State state)
{
    setFont(QLatin1String("font"), parseFont(deffont, size, bold), state);
    QColor colour;
    int role = parseColor(col, colour);
    if (role == QPalette::NColorRoles && state != ThemeItem::Default)
        role = attribute(QLatin1String("colorRole"));
    if (!colour.isValid() && state != ThemeItem::Default)
        colour = color(QLatin1String("color"));
    setAttribute(QLatin1String("colorRole"), role, state);
    setColor(QLatin1String("color"), colour, state);
    role = parseColor( outline, colour);
    if (role == QPalette::NColorRoles && state != ThemeItem::Default)
        role = attribute(QLatin1String("outlineRole"));
    if (!colour.isValid() && state != ThemeItem::Default)
        colour = color(QLatin1String("outline"));
    setAttribute(QLatin1String("outlineRole"), role, state);
    setColor(QLatin1String("outline"), colour, state);
}

/*!
  \internal
  */
void ThemeTextItem::setupThemeText()
{
    if (!d->themeText.isEmpty()) { // should only occur once
        QString newtext = d->themeText;
        d->themeText = QString(); // setText() expects this to be empty
        if (isDataExpression() && !newtext.trimmed().startsWith("expr:"))
            newtext = newtext.prepend("expr:");
        if (isStringExpression(newtext)) {
            Q_ASSERT(d->textExpr == 0);
            d->textExpr = createExpression(stripStringExpression(newtext));
            if (d->textExpr != 0)
                expressionChanged(d->textExpr); // Force initial update
        } else {
            setText(newtext);
        }
        setTextFormat(d->format); // reset now that the initial text has been set.
    }
}

/*!
  Sets the content for this text item. \a text will be drawn to the screen.
  */
void ThemeTextItem::setText(const QString &text)
{
    Q_ASSERT(d->themeText.isEmpty()); // no theme text should be left to process
    if (text != d->displayText) {
        d->displayText = text;
        if (d->format == Qt::AutoText)
            d->richText = Qt::mightBeRichText(d->displayText);
        delete d->shadowImg;
        d->shadowImg = 0;
        if (isVisible())
            update();
    }
}

/*!
  Returns the content of this text item.
*/
QString ThemeTextItem::text() const { return d->displayText; }

/*!
  Returns the short label for this item, as specified in the themed view XML.
*/
bool ThemeTextItem::shortLabel() const  { return d->shortLbl; }

/*!
  Returns the current text format of the text item.
*/
Qt::TextFormat ThemeTextItem::textFormat() const { return d->format; }

/*!
  Sets the text format of the item explicitly to \a format.
  If the value is Qt::AutoText, the item tries to determine the correct format.
*/
void ThemeTextItem::setTextFormat(Qt::TextFormat format)
{
    d->format = format;
    switch (d->format) {
        case Qt::AutoText:
            d->richText = Qt::mightBeRichText(d->displayText);
            break;
        case Qt::RichText:
            d->richText = true;
            break;
        default:
            d->richText = false;
    }
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Text.
*/
int ThemeTextItem::rtti() const
{
    return ThemedView::Text;
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a painter.
*/
void ThemeTextItem::paint(QPainter *painter, const QRect &rect)
{
    Q_UNUSED(rect);
    QFont defaultFnt = font(QLatin1String("font"), state());
    if (!d->displayText.isEmpty()) {
        QFont fnt(defaultFnt);
        //if (itemName() == QLatin1String("time")) {
            // (workaround) use helvetica for time strings or we run into trouble
            // when the default font is not scalable
            // assumption!!??!: time string can always be displayed
            // in helvetica no matter what language we have
            //fnt.setFamily(QLatin1String("helvetica"));
        //}

        QString text(d->displayText);
        int x = 0;
        int y = 0;
        int w = geometry().width();
        int h = geometry().height();
        QTextDocument *doc = 0;
        if (d->richText) {
            if (d->align & Qt::AlignHCenter)
                text = "<center>"+text+"</center>";
            doc = new QTextDocument();
            doc->setHtml(text);
            doc->setPageSize(QSize(w, INT_MAX));
            QAbstractTextDocumentLayout *layout = doc->documentLayout();
            QSizeF bound(layout->documentSize());
            w = (int)bound.width();
            h = (int)bound.height();
            if (d->align & Qt::AlignRight)
                x += geometry().width()-w-1;
            if (d->align & Qt::AlignBottom)
                y += geometry().height()-h-1;
            else if (d->align & Qt::AlignVCenter)
                y += (geometry().height()-h-1)/2;
        } else if (d->elidedMode != -1) {
            QFontMetrics fm(fnt, painter->device());
            text = fm.elidedText (text, (Qt::TextElideMode)d->elidedMode, rect.width());
        }

        QPalette pal(view()->palette());
        if (d->shadow) {
            if (!d->shadowImg) {
                QPixmap spm(qMin(w+2,geometry().width()), qMin(h+2, geometry().height()));
                spm.fill();
                QPainter sp(&spm);
                if (d->richText) {
                    QAbstractTextDocumentLayout *layout = doc->documentLayout();
                    doc->setPageSize(QSize(w, INT_MAX));
                    QAbstractTextDocumentLayout::PaintContext context;
                    context.palette = view()->palette();
                    context.palette.setColor(QPalette::Text, Qt::black);
                    layout->draw(&sp, context);
                } else {
                    sp.setPen(Qt::black);
                    sp.setFont(fnt);
                    sp.drawText(QRect(1,1,w,h), d->align, text);
                }
                QImage img = spm.toImage();
                d->shadowImg = new QImage(w+2, h+2, QImage::Format_ARGB32);
                d->shadowImg->fill(0x00000000);
                int sv = (d->shadow/5) << 24;
                for (int i = 1; i < img.height()-1; i++) {
                    QRgb *r0 = (QRgb *)d->shadowImg->scanLine(i-1);
                    QRgb *r1 = (QRgb *)d->shadowImg->scanLine(i);
                    QRgb *r2 = (QRgb *)d->shadowImg->scanLine(i+1);
                    for (int j = 1; j < img.width()-1; j++, r0++, r1++, r2++) {
                        if (!(img.pixel(j, i) & 0x00ffffff)) {
                            *r0 += sv;
                            *r1 += sv;
                            *(r1+1) += sv;
                            *(r1+2) += sv;
                            *r2 += sv;
                            *(r2+1) += sv;
                        }
                    }
                }
            }
            painter->drawImage(x, y-1, *d->shadowImg);
        }
        QColor col = color(QLatin1String("color"), state());
        int role = attribute(QLatin1String("colorRole"), state()),
            outlineRole = attribute(QLatin1String("outlineRole"), state());
        if (d->richText) {
            if (outlineRole != QPalette::NColorRoles) {
                pal.setColor(QPalette::Text, getColor(col, role));
                drawOutline(painter, QRect(x,y,geometry().width(),geometry().height()), pal, doc->documentLayout());
            } else {
                QAbstractTextDocumentLayout *layout = doc->documentLayout();
                doc->setPageSize(QSize(w, INT_MAX));
                QAbstractTextDocumentLayout::PaintContext context;
                context.palette = view()->palette();
                context.palette.setColor(QPalette::Text, getColor(col, role));
                QRect cr = QRect(0,0,geometry().width(),geometry().height());
                painter->translate(x, y);
                painter->setClipRect(cr);
                layout->draw(painter, context);
                painter->setClipping(false); // clear our call to setClipRect()
                painter->translate(-x, -y);
            }
        } else {
            painter->setPen(getColor(col, role));
            painter->setFont(fnt);
            if (outlineRole != QPalette::NColorRoles)
                drawOutline(painter, QRect(x,y,w,h), d->align, text);
            else {
                painter->drawText(QRect(x,y,w,h), d->align, text);
            }
        }
        delete doc;
    }
}

/*!
  \reimp
  */
void ThemeTextItem::expressionChanged(QExpressionEvaluator *expression)
{
    if (d->textExpr == expression) {
        QVariant result = getExpressionResult(expression, QVariant::String);
        if (!result.canConvert(QVariant::String)) {
            qWarning("ThemeTextItem::expressionChanged() - Expression '%s' could not be coerced to string type, ignoring value.", expression->expression().constData());
        } else {
            setText(result.toString());
        }
    } else {
        ThemeItem::expressionChanged(expression);
    }
}

/*!
  \reimp
  */
void ThemeTextItem::constructionComplete()
{
    ThemeGraphicItem::constructionComplete();
    setupThemeText();
}


void ThemeTextItem::drawOutline(QPainter *painter, const QRect &rect, int flags, const QString &text)
{
    QColor outlineColor = color(QLatin1String("outline"), state());
    int outlineRole = attribute(QLatin1String("outlineRole"), state());

    // Cheaper to paint into pixmap and blit that four times than
    // to draw text four times.

    // First get correct image size for DPI of target.
    QImage img(QSize(1,1), QImage::Format_ARGB32_Premultiplied);
    QPainter ppm(&img);
    ppm.setFont(painter->font());
    QFontMetrics fm(ppm.font());
    QRect br = fm.boundingRect(rect, flags, text);
    ppm.end();

    // Now create proper image and paint.
    img = QImage(br.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(qRgba(0,0,0,0));
    ppm.begin(&img);
    ppm.setFont(painter->font());
    ppm.setPen(getColor(outlineColor, outlineRole));
    ppm.drawText(QRect(QPoint(0,0), br.size()), flags, text);

    QPoint pos(br.topLeft());
    pos += QPoint(-1,0);
    painter->drawImage(pos, img);
    pos += QPoint(2,0);
    painter->drawImage(pos, img);
    pos += QPoint(-1,-1);
    painter->drawImage(pos, img);
    pos += QPoint(0,2);
    painter->drawImage(pos, img);

    img.fill(qRgba(0,0,0,0));
    ppm.setPen(painter->pen());
    ppm.drawText(QRect(QPoint(0,0), br.size()), flags, text);

    pos += QPoint(0,-1);
    painter->drawImage(pos, img);
}

void ThemeTextItem::drawOutline(QPainter *p, const QRect &r, const QPalette &pal, QAbstractTextDocumentLayout *layout)
{
    QColor outlineColor = color(QLatin1String("outline"), state());
    int outlineRole = attribute(QLatin1String("outlineRole"), state());
    QAbstractTextDocumentLayout::PaintContext context;
    context.palette = view()->palette();
    context.palette.setColor(QPalette::Text, getColor(outlineColor, outlineRole));

    p->translate(r.x()-1,0);
    layout->draw(p, context);
    p->translate(2,0);
    layout->draw(p, context);
    p->translate(-1,-1);
    layout->draw(p, context);
    p->translate(0,2);
    layout->draw(p, context);

    context.palette.setColor(QPalette::Text, pal.color(QPalette::Text));
    p->translate(0,-1);
    layout->draw(p, context);
}

/*!
  \reimp
*/
void ThemeTextItem::addCharacters(const QString &ch)
{
    d->themeText.append(ch);
    Q_ASSERT(d->textExpr == 0); // shouldn't be setup already
}

//---------------------------------------------------------------------------

/*!
  \class ThemeRectItem
    \inpublicgroup QtBaseModule


  \brief The ThemeRectItem class represents a visual rectangle in a ThemedView.

  The ThemeRectItem class implements the \l{Themed View Elements#themerectelement}{rect element} from the theme XML.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

    \sa {Themed View Elements#themerectelement}{rect element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeRectItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeRectItem::ThemeRectItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeGraphicItem(parent, view, atts)
{
    QMap<QString,QString> onClickAtts, onFocusAtts;
    if ( isInteractive() ) {
        onClickAtts = parseSubAtts( atts.value(QLatin1String("onclick")) );
        onFocusAtts = parseSubAtts( atts.value(QLatin1String("onfocus")) );
    }

    setupColor( QLatin1String("fg"), QLatin1String("fgRole"), atts.value( QLatin1String("color") ), ThemeItem::Default );
    setupColor( QLatin1String("fg"), QLatin1String("fgRole"), onClickAtts[QLatin1String("color")], ThemeItem::Pressed );
    setupColor( QLatin1String("fg"), QLatin1String("fgRole"), onFocusAtts[QLatin1String("color")], ThemeItem::Focus );

    setupColor( QLatin1String("bg"), QLatin1String("bgRole"), atts.value( QLatin1String("brush") ), ThemeItem::Default );
    setupColor( QLatin1String("bg"), QLatin1String("bgRole"), onClickAtts[QLatin1String("brush")], ThemeItem::Pressed );
    setupColor( QLatin1String("bg"), QLatin1String("bgRole"), onFocusAtts[QLatin1String("brush")], ThemeItem::Focus );

    setupAlpha( QLatin1String("alpha"), atts.value( QLatin1String("alpha") ), ThemeItem::Default );
    setupAlpha( QLatin1String("alpha"), onClickAtts[QLatin1String("alpha")], ThemeItem::Pressed );
    setupAlpha( QLatin1String("alpha"), onFocusAtts[QLatin1String("alpha")], ThemeItem::Focus );
}

/*!
  Returns the color used for the rectangle's background for the given state \a state.
*/
QColor ThemeRectItem::brushColor( ThemeItem::State state ) const
{
    return getColor( color( QLatin1String("bg"), state ), attribute( QLatin1String("bgRole"), state ) );
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Rect.
*/
int ThemeRectItem::rtti() const
{
    return ThemedView::Rect;
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a p.
*/
void ThemeRectItem::paint(QPainter *p, const QRect &rect)
{
    Q_UNUSED(rect);
    QBrush oldBrush = p->brush(); // change back what we change. faster than using Painter save/restore()
    QPen oldPen = p->pen();
    int fgRole = attribute( QLatin1String("fgRole"), state() );
    if (fgRole == QPalette::NColorRoles)
        p->setPen(Qt::NoPen);
    else {
        QPen newPen = oldPen;
        newPen.setColor( getColor(color(QLatin1String("fg"), state()), fgRole) );
        newPen.setWidth( 1 );
        p->setPen(newPen);
    }
    int bgRole = attribute( QLatin1String("bgRole"), state() );
    if (bgRole == QPalette::NColorRoles) {
        p->setBrush(Qt::NoBrush);
    } else {
        QColor col = getColor(color(QLatin1String("bg"), state()), bgRole);
        QString al = strAttribute("alpha", state());
        if ( !al.isEmpty() ) {
            bool ok = false;
            if (!al.isEmpty() && al != "255")
                col.setAlpha(al.toInt(&ok));
            Q_ASSERT(ok == true);
        }
        p->setBrush(col);
    }
    p->drawRect(0, 0, geometry().width()-p->pen().width(), geometry().height()-p->pen().width());
    p->setPen(oldPen);
    p->setBrush(oldBrush);
}

//---------------------------------------------------------------------------

/*!
  \class ThemeLineItem
    \inpublicgroup QtBaseModule


  \brief The ThemeLineItem class represents a visual line in a ThemedView.

  The ThemeLineItem class implements the \l{Themed View Elements#themelineelement}{line element} from the theme XML.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

  \sa {Themed View Elements#themelineelement}{line element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeLineItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeLineItem::ThemeLineItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeGraphicItem(parent, view, atts)
{
    QColor colour;
    setAttribute( QLatin1String("colorRole"), parseColor(atts.value(QLatin1String("color")), colour) );
    setColor( QLatin1String("color"), colour );
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Line.
*/
int ThemeLineItem::rtti() const
{
    return ThemedView::Line;
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a p.
*/
void ThemeLineItem::paint(QPainter *p, const QRect &)
{
    p->setPen(getColor(color( QLatin1String("color") ), attribute( QLatin1String("colorRole"))));
    p->drawLine(0, 0, geometry().width()-1, geometry().height()-1);
}

//---------------------------------------------------------------------------

struct Image
{
    QString filename;
    QPixmap pixmap;
};

struct ThemePixmapItemPrivate
{
    ThemePixmapItemPrivate()
    : hscale(false), vscale(false)
    {}
    QTagMap<Image> images[3];
    bool hscale;
    bool vscale;
};

/*!
  \class ThemePixmapItem
    \inpublicgroup QtBaseModule


  \brief The ThemePixmapItem class contains common functionality for image based items in a ThemedView.

  As several items such as ThemeImageItem, ThemeLevelItem and ThemeStatusItem are based on images,
  a ThemePixmapItem provides the convenient setPixmap() and pixmap() functions to access and store images
  based on a name and state pair of values.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.
  \ingroup appearance
*/

/*!
  Constructs a ThemePixmapItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
  */
ThemePixmapItem::ThemePixmapItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeGraphicItem(parent, view, atts)
{
    d = new ThemePixmapItemPrivate;

    QString val = atts.value(QLatin1String("scale"));
    if (!val.isEmpty()) {
        if (val == QLatin1String("yes")) {
            d->vscale = d->hscale = (val == QLatin1String("yes"));
        } else {
            if (val.contains(QLatin1String("vertical")))
                d->vscale = true;
            if (val.contains(QLatin1String("horizontal")))
                d->hscale = true;
        }
    }
}

/*!
  Destroys the ThemePixmapItem.
*/
ThemePixmapItem::~ThemePixmapItem()
{
    delete d;
}

/*!
  Returns the pixmap for the given \a key and state \a state.
  \a state  has a default value of ThemeItem::Default.
*/
QPixmap ThemePixmapItem::pixmap( const QString &key, ThemeItem::State state ) const
{
    int _st = stateToIndex( state );
    const QTagMap<Image> &dataset = d->images[_st];
    return dataset[key].pixmap;
}

/*!
  Returns the pixmap for the given \a key and state \a state.
  \a state  has a default value of ThemeItem::Default.

  \since 4.3.1
*/
QPixmap ThemePixmapItem::pixmap( const QLatin1String &key, ThemeItem::State state ) const
{
    int _st = stateToIndex( state );
    const QTagMap<Image> &dataset = d->images[_st];
    return dataset[key].pixmap;
}

/*!
  Sets the pixmap \a value for the given \a key and state \a state from \a filename.
  \a state has a default value of ThemeItem::Default.
*/
void ThemePixmapItem::setPixmap( const QString &key, const QPixmap &value, ThemeItem::State state, const QString &filename )
{
    int _st = stateToIndex( state );
    QTagMap<Image> &dataset = d->images[_st];
    if ( dataset.contains( key ) ) {
        dataset[key].pixmap = value;
        dataset[key].filename = filename;
    }
    else {
        Image image;
        image.pixmap = value;
        image.filename = filename;
        dataset.insert( key, image );
    }
}

/*!
  Sets the pixmap \a value for the given \a key and state \a state from \a filename.
  \a state has a default value of ThemeItem::Default.

  \since 4.3.1
*/
void ThemePixmapItem::setPixmap( const QLatin1String &key, const QPixmap &value, ThemeItem::State state, const QString &filename )
{
    int _st = stateToIndex( state );
    QTagMap<Image> &dataset = d->images[_st];
    if ( dataset.contains( key ) ) {
        dataset[key].pixmap = value;
        dataset[key].filename = filename;
    }
    else {
        Image image;
        image.pixmap = value;
        image.filename = filename;
        dataset.insert( key, image );
    }
}

static void yuv_to_rgb(int Y, int U, int V, int& R, int& G, int&B)
{
    R = int(Y +  ((92242 * (V - 128)) >> 16));
    G = int(Y - (((22643 * (U - 128)) >> 16)) - ((46983 * (V - 128)) >> 16));
    B = int(Y + ((116589 * (U - 128)) >> 16));
}

static void rgb_to_yuv(int R, int G, int B, int& Y, int& U, int& V)
{
    Y = int(R *  19595 + G *  38470 + B *  7471) >> 16;
    U = int(R * -11076 + G * -21758 + (B << 15) + 8388608) >> 16;
    V = int(R *  32768 + G * -27460 + B * -5328 + 8388608) >> 16;
}

static inline QRgb blendYuv(QRgb rgb, int /*sr*/, int /*sg*/, int /*sb*/, int sy, int su, int sv, int alpha, bool colorroles)
{
    int a = (rgb >> 24) & 0xff;
    int r = (rgb >> 16) & 0xff;
    int g = (rgb >> 8) & 0xff;
    int b = rgb & 0xff;
    if (colorroles) {
        int y,u,v;
        rgb_to_yuv(r,g,b,y,u,v);
        y = (y*2+sy)/3;
        u = (u+su*2)/3;
        v = (v+sv*2)/3;
        yuv_to_rgb(y,u,v,r,g,b);
        if (r>255) r = 255; if (r<0) r=0;
        if (g>255) g = 255; if (g<0) g=0;
        if (b>255) b = 255; if (b<0) b=0;
    }
    if (alpha != 255)
        a = (a*alpha)/255;
    return qRgba(r, g, b, a);
}

/*!
  Blend the color \a col and the alpha value \a alpha with the image \a img if \a blendColor is true (the default).
  If \a blendColor is false, only the alpha value \a alpha is blended.
  This function modifies \a img directly.
*/
void ThemePixmapItem::colorizeImage( QImage& img, const QColor& col, int alpha, bool blendColor )
{
    QColor colour = col;
    Q_ASSERT( !img.isNull() );
    Q_ASSERT( col.isValid() );
    int count;
    int sr, sg, sb;
    colour.getRgb(&sr, &sg, &sb);
    int sy, su, sv;
    rgb_to_yuv(sr,sg,sb,sy,su,sv);

    if ( alpha != 255 &&
         img.format() != QImage::Format_ARGB32 &&
         img.format() != QImage::Format_ARGB32_Premultiplied ) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }
    if (img.depth() == 32) {
        const QImage &cimg = img; // avoid QImage::detach().
        QRgb *rgb = (QRgb*)cimg.bits();
        count = img.bytesPerLine()/sizeof(QRgb)*img.height();
        for (int i = 0; i < count; i++, rgb++)
            *rgb = blendYuv(*rgb, sr, sg, sb, sy, su, sv, alpha,
                blendColor);
    } else {
        QVector<QRgb> ctable = img.colorTable();
        for (int i = 0; i < ctable.count(); i++)
            ctable[i] = blendYuv(ctable[i], sr, sg, sb, sy, su, sv, alpha,
                    blendColor);
        img.setColorTable(ctable);
    }
}

/*!
    \internal
    Replaces pixels with color \a before with color \a after in the \a image.
*/
bool ThemePixmapItem::replaceColor(QImage &image, const QColor &before, const QColor &after)
{
    QRgb b = before.rgb();
    QRgb a = after.rgb();
    bool modified = false;

    for (int j = 0; j < image.height(); j++) {
        for (int i = 0; i < image.width(); i++) {
            if (image.pixel(i, j) == b) {
                image.setPixel(QPoint(i, j), a);
                modified = true;
            }
        }
    }
    return modified;
}

/*
   We cannot use QtopiaResource translation lookup
   because we use absolute addressing with two subdirs. QtopiaResource only looks up
   one subdir down.

   Note that there is no guarantee that the returned filename actually exists
   */
QString imageTr( const QString& path, const QString& image, bool i18n )
{
    if ( !i18n )
        return QString (path+image);

    static QStringList langs;
    if ( langs.empty() ) {
        langs = Qtopia::languageList();
        langs.append(QLatin1String("en_US"));
    }

    QFileInfo fi;
    QString file;
    foreach ( QString l, langs )
    {
        file = path + QLatin1String("i18n/")+ l + QLatin1String("/") + image;
        fi.setFile( file );
        if ( fi.exists() ) //we have to do the lookup to determine whether we have to change to alternative language
            return file;
    }
    //if we cannot find anything we just return the en_US version which is last in the language list

    return file;
}

/*!
  Loads the image given by \a filename for the given \a width and \a height.
  If \a colorRole is a valid index to QPalette, or if the color \a col is valid,
  and \a alpha is less than 255, the image is passed through ThemePixmapItem::colorizeImage().
  Returns the loaded image.
*/
QPixmap ThemePixmapItem::loadImage(const QString &filename, int colorRole, const QColor &col, int alpha, int width, int height)
{
    qLog(Resource) << "ThemePixmapItem::loadImage" << filename << colorRole << col << alpha << width << height << "geometry=" << geometry();

    QPixmap pm;
    static QString dflt_path(QLatin1String("default/"));

    if (filename.isEmpty())
        return pm;

    QString imgName = filename;

    bool i18n = false;
    if ( imgName.startsWith(QLatin1String("i18n/")) ) {
        i18n = true;
        imgName.remove(0, 5 /*strlen("i18n/") */ );
    }

    if (filename.endsWith(".svg")) {
        int w = width ? width : geometry().width();
        int h = height ? height : geometry().height();
        if ( !w || !h )
            return pm;
        QColor colour;
        if (colorRole != QPalette::NColorRoles || alpha != 255)
            colour = getColor(col, colorRole);
        QString key("QTV_%1_%2_%3_%4");
        key = key.arg(filename).arg(w).arg(h).arg(colour.name());
        if (QPixmapCache::find(key, pm))
            return pm;
        QImage buffer(w, h, QImage::Format_ARGB32_Premultiplied);
        buffer.fill(0);
        QPainter painter(&buffer);

#ifndef QT_NO_PICTURE

        QString picFile(imgName);
        picFile.replace(imgName.length()-3, 3, QLatin1String("pic"));
        QFileInfo fi;
        if (picFile.startsWith(dflt_path)) {
            picFile = picFile.mid(dflt_path.length());
            fi.setFile(imageTr(QLatin1String(":image/")+view()->defaultPics(),picFile,i18n));
        } else {

            if (picFile.startsWith(":"))
                fi.setFile(picFile);
            else {
            fi.setFile(imageTr(QLatin1String(":image/")+view()->base(),picFile,i18n));
            if (!fi.exists())
                fi.setFile(imageTr(QLatin1String(":image/")+view()->defaultPics(),picFile,i18n));
            }
        }
        if (fi.exists()) {
            QPicture picture;
            QRectF viewRect;
            picture.load(fi.filePath());
            QRect br = picture.boundingRect();
            painter.setViewport(0,0,w,h);
            painter.translate(-br.topLeft());
            painter.scale(qreal(w)/br.width(), qreal(h)/br.height());
            painter.drawPicture(0, 0, picture);
        } else {
#endif
            painter.setViewport(0, 0, w, h);
            QSvgRenderer doc;
            if (!imgName.startsWith(dflt_path))
                doc.load(imageTr(QLatin1String(":image/")+view()->base(),imgName, i18n));
            else
                imgName = imgName.mid(dflt_path.length());
            if(buffer.isNull())
                doc.load(imageTr(QLatin1String(":image/")+view()->defaultPics(),imgName,i18n));
            doc.render(&painter);
#ifndef QT_NO_PICTURE
        }
#endif

        // If we replace a color we need to regenerate a key for QPixmapCache
        if (replaceColor(buffer, QColor(255, 0, 255), view()->palette().color(QPalette::Highlight))) {
            key = "QTV_%1_%2_%3_%4";
            key = key.arg(filename).arg(w).arg(h).arg(view()->palette().color(QPalette::Highlight).name());
        }

        if ( colour.isValid() ) // only call colorizeImage if the colour isValid
            colorizeImage( buffer, colour, alpha, colorRole != QPalette::NColorRoles );
        pm = QPixmap::fromImage(buffer);
        QPixmapCache::insert(key, pm);
        return pm;
    }

    if (colorRole != QPalette::NColorRoles || alpha != 255) {
        QColor colour = getColor(col, colorRole);
        QImage img;
        if (imgName.startsWith(":icon")) {
            int w = width ? width : geometry().width();
            int h = height ? height : geometry().height();
            if ( !w || !h )
                return pm;
            QIcon icon(imgName);
            img = icon.pixmap(w, h).toImage();
        } else {
            if (itemName() == "background") {
                QSettings config("Trolltech", "qpe");
                config.beginGroup( "Appearance" );
                QString s = config.value("BackgroundImage").toString();
                img = QImage(QLatin1String(":image/") + view()->base() + s);
            }
            if (img.isNull()) {
                if (!imgName.startsWith(dflt_path))
                    img = QImage(imageTr(QLatin1String(":image/")+view()->base(),imgName,i18n));
                else
                    imgName = imgName.mid(dflt_path.length());
                if( img.isNull() )
                    img = QImage(imageTr(QLatin1String(":image/")+view()->defaultPics(),imgName,i18n));
                if( img.isNull() )
                    img = QImage(imgName);
            }
        }
        if ( img.isNull() )
        if ( img.isNull() )
            return pm;
        if ( colour.isValid() ) // only call colorizeImage if the colour isValid
            colorizeImage( img, colour, alpha, colorRole != QPalette::NColorRoles );
        pm = pm.fromImage( img );
    } else {
        if (imgName.startsWith(":icon")) {
            int w = width ? width : geometry().width();
            int h = height ? height : geometry().height();
            if ( !w || !h )
                return pm;
            QIcon icon(imgName);
            if (!icon.isNull())
                pm = icon.pixmap(w, h);
        } else {
            if (itemName() == "background") {
                QSettings config("Trolltech", "qpe");
                config.beginGroup( "Appearance" );
                QString s = config.value("BackgroundImage").toString();
                pm = QPixmap(QLatin1String(":image/") + view()->base() + s);
            }
            if (pm.isNull()) {
                if (!imgName.startsWith(dflt_path))
                    pm = QPixmap(imageTr(QLatin1String(":image/")+view()->base(),imgName,i18n));
                else
                    imgName = imgName.mid(dflt_path.length());
                if( pm.isNull() )
                    pm = QPixmap(imageTr(QLatin1String(":image/")+view()->defaultPics(),imgName,i18n));
            }
        }
        if( pm.isNull() )
            pm = QPixmap(imgName);
    }
    return pm;
}

/*!
    \internal
*/
QPixmap ThemePixmapItem::scalePixmap( const QPixmap &pix, int width, int height )
{
    if ( pix.isNull() || (width <= 0 || height <= 0) || (pix.width() == width && pix.height() == height) )
        return pix;
    return pix.scaled(width, height);
}

/*!
    \internal
    Set horizontal scaling if \a enable is true, otherwise the pixmap
    will not be scaled in the horizontal direction.
*/
void ThemePixmapItem::setHorizontalScale( bool enable )
{
    d->hscale = enable;
}

/*!
    \internal
    Set vertical scaling if \a enable is true, otherwise the pixmap
    will not be scaled in the vertical direction.
*/
void ThemePixmapItem::setVerticalScale( bool enable )
{
    d->vscale = enable;
}

/*!
    \internal
    Returns true if the pixmap will be scaled horizontally.
*/
bool ThemePixmapItem::horizontalScale() const
{
    return d->hscale;
}


/*!
    \internal
    Returns true if the pixmap will be scaled vertically.
*/
bool ThemePixmapItem::verticalScale() const
{
    return d->vscale;
}

/*!
    \internal
*/
void ThemePixmapItem::scaleImage( const QString &key, int width, int height )
{
    // We are not visible anymore.. do not scale the image as we might want to use it again
    if (width == 0 && height == 0) {
        return;
    }

    for ( int i = 0; i < 3; i++ ) {
        QTagMap<Image> &map = d->images[i];
        QString filename = map[key].filename;

        if (map[key].pixmap.width() == width && map[key].pixmap.height() == height)
            continue;

        if ( filename.endsWith(".svg") ) {
            QColor colour = color( QLatin1String("color") );
            int alpha = attribute( QLatin1String("alpha") );
            map[key].pixmap = loadImage( map[key].filename, QPalette::NColorRoles, colour, alpha, width, height );

        } else if (!filename.isEmpty() && !map[key].pixmap.isNull()) {
            map[key].pixmap = scalePixmap( map[key].pixmap, width, height );
        }
    }
}

/*!
    \internal
*/
void ThemePixmapItem::scaleImages( int count = 1 )
{
    int width = 0, height = 0;
    for ( int i = 0; i < 3; i++ ) {
        QTagMap<Image> &dataset = d->images[i];
        QTagMap<Image>::iterator it;
        for ( it = dataset.begin(); it != dataset.end(); ++it ) {
            QPixmap pm = it.value().pixmap;
            if ( horizontalScale() && pm.width() != geometry().width() * count || verticalScale() && pm.height() != geometry().height() ) {
                width = horizontalScale() ? geometry().width() * count : pm.width();
                height = verticalScale() ? geometry().height() : pm.height();
                scaleImage(it.key().toString(), width, height);
            }
        }
    }
}

//---------------------------------------------------------------------------

class ThemeAnimationFrameInfo : public QObject
{
public:
    ThemeAnimationFrameInfo(ThemeAnimationItem *a, int p)
        : QObject(), anim(a), tid(0), period(p) {
    }

    void setPeriod(int p) {
        period = p;
    }
    void start() {
        stop();
        tid = startTimer(period);
    }
    void stop() {
        if (tid) {
            killTimer(tid);
            tid = 0;
        }
    }

protected:
    void timerEvent(QTimerEvent *) {
        anim->advance();
    }

private:
    ThemeAnimationItem *anim;
    int tid;
    int period;
    QPixmap pm;
};


struct ThemeAnimationItemPrivate
{
    ThemeAnimationItemPrivate() : currFrame(0), inc(1), frameExpr(0) {
        fi[0] = 0;
        fi[1] = 0;
        fi[2] = 0;
        playExpr[0] = 0; playExpr[1] = 0; playExpr[2] = 0;
    }
    ThemeAnimationFrameInfo *fi[3];
    int currFrame;
    int inc;
    QExpressionEvaluator* frameExpr;
    QString text;
    QExpressionEvaluator* playExpr[3];
};

/*!
  \class ThemeAnimationItem
    \inpublicgroup QtBaseModule


  \brief The ThemeAnimationItem class represents a pixmap-based animation in a ThemedView.

  The ThemeAnimationItem class implements the \l{Themed View Elements#themeanimelement}{anim element} from the theme XML.

  It gets its frames from a single image which has frames at predefined intervals side-by-side.
  You can monitor and control the playing of the animation using the frame(), setFrame(), stop()
  and start() functions.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

\sa {Themed View Elements#themeanimelement}{anim element}
  \ingroup appearance
  */

/*!
  Constructs a ThemeAnimationitem
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeAnimationItem::ThemeAnimationItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemePixmapItem(parent, view, atts)
{
    d = new ThemeAnimationItemPrivate;

    QMap<QString,QString> onClickAtts, onFocusAtts;
    if ( isInteractive() ) {
        onClickAtts = parseSubAtts( atts.value(QLatin1String("onclick")) );
        onFocusAtts = parseSubAtts( atts.value(QLatin1String("onfocus")) );
    }

    setupAnimation(view->base(), atts.value( QLatin1String("src") ), atts.value( QLatin1String("color") ), atts.value( QLatin1String("alpha") ), atts.value( QLatin1String("count") ), atts.value( QLatin1String("width") ),
            atts.value( QLatin1String("loop") ), atts.value( QLatin1String("looprev") ), atts.value( QLatin1String("delay") ), atts.value(QLatin1String("play")));
    QString filename;
    filename = onClickAtts[QLatin1String("src")];
    /*
    if ( filename.isEmpty() )
        filename = atts.value( QLatin1String("src") );
        */
    setupAnimation(view->base(), filename, onClickAtts[QLatin1String("color")], onClickAtts[QLatin1String("alpha")], onClickAtts[QLatin1String("count")], onClickAtts[QLatin1String("width")],
            onClickAtts[QLatin1String("loop")], onClickAtts[QLatin1String("looprev")], onClickAtts[QLatin1String("delay")], onClickAtts[QLatin1String("play")], ThemeItem::Pressed);
    filename = onFocusAtts[QLatin1String("src")];
    /*
    if ( filename.isEmpty() )
        filename = atts.value( QLatin1String("src") );
        */
    setupAnimation(view->base(), filename, onFocusAtts[QLatin1String("color")], onFocusAtts[QLatin1String("alpha")], onFocusAtts[QLatin1String("count")], onFocusAtts[QLatin1String("width")],
            onFocusAtts[QLatin1String("loop")], onFocusAtts[QLatin1String("looprev")], onFocusAtts[QLatin1String("delay")], onFocusAtts[QLatin1String("play")], ThemeItem::Focus);

    for( int i = 0 ; i < 3 ; ++i )
        if ( attribute( QLatin1String("delay"), indexToState(i) ) > 0 )
            d->fi[i] = new ThemeAnimationFrameInfo(this, attribute(QLatin1String("delay"), indexToState(i) ));
        else
            d->fi[i] = 0;



    d->currFrame = 0;
    d->inc = 1;
}

/*!
  Destroys a ThemeAnimationItem
*/
ThemeAnimationItem::~ThemeAnimationItem()
{
    for( int i = 0 ; i < 3 ; ++i )
        if ( d->fi[i] )
            delete d->fi[i];

    QExpressionEvaluator* firstexpr = d->playExpr[0];
    for(int i = 0 ; i < 3 ; ++i)
        if (i == 0 || d->playExpr[i] != firstexpr)
            delete d->playExpr[i];

    if ( d->frameExpr ) {
        delete d->frameExpr;
        d->frameExpr = 0;
    }
    delete d;
}

/*!
  \reimp
  */
void ThemeAnimationItem::addCharacters( const QString& ch )
{
    d->text += ch;
}

/*!
  Returns the index of the current frame.
  Returns 0 if frameCount() returns 0.
*/
int ThemeAnimationItem::frame() const { return d->currFrame; }

/*!
  Returns the total number of frames in this animation.
*/
int ThemeAnimationItem::frameCount() const { return attribute("count", state()); }

void ThemeAnimationItem::setupAnimation( const QString &, const QString &src, const QString &col, const QString &alpha, const QString &count,
const QString &width, const QString &loop, const QString &looprev, const QString &delay, const QString& play, ThemeItem::State st )
{
    QColor colour;
    int role = QPalette::NColorRoles;
    QString c = col;
    int idx = stateToIndex(st);
    if ( c.isEmpty() && st != ThemeItem::Default ) {
        colour = color( QLatin1String("color") );
        role = attribute( QLatin1String("colorRole") );
    } else if ( !c.isEmpty() )
        role = parseColor( c, colour );
    int al;
    if ( alpha.isEmpty() && st != ThemeItem::Default )
        al = attribute( QLatin1String("alpha") );
    else if ( !alpha.isEmpty() )
        al = alpha.toInt();
    else
        al = 255;
    QString s = src;
    if ( s.isEmpty() && st != ThemeItem::Default ) {
        s = strAttribute( QLatin1String("src") );
    }
    setAttribute( QLatin1String("src"), s, st );
    QPixmap pm = loadImage( s, role, colour, al );
    setPixmap( QLatin1String("src"), pm, st, s );
    setAttribute( QLatin1String("colorRole"), role, st );
    setColor( QLatin1String("color"), colour, st );
    setAttribute( QLatin1String("alpha"), al, st );

    if ( !count.isEmpty() )
        setAttribute( QLatin1String("count"), count.toInt(), st );
    else if ( st != ThemeItem::Default )
        setAttribute( QLatin1String("count"), attribute( QLatin1String("count") ), st );
    else
        setAttribute( QLatin1String("count"), 0 );
    if ( !width.isEmpty() )
        setAttribute( QLatin1String("width"), width.toInt(), st );
    else if ( st != ThemeItem::Default )
        setAttribute( QLatin1String("width"), attribute( QLatin1String("width") ), st );
    else
    {
        int count = attribute(QLatin1String("count"));
        if ( count <= 0 )
            count = 1;
        setAttribute( QLatin1String("width"), pixmap( QLatin1String("src") ).width() / count );
    }
    if ( !loop.isEmpty() )
        setAttribute( QLatin1String("loop"), loop.toInt(), st );
    else if ( st != ThemeItem::Default )
        setAttribute( QLatin1String("loop"), attribute( QLatin1String("loop") ), st );
    else
        setAttribute( QLatin1String("loop"), -1, st );
    if ( !looprev.isEmpty() )
        setAttribute( QLatin1String("looprev"), looprev.toInt(), st );
    else if ( st != ThemeItem::Default )
        setAttribute( QLatin1String("looprev"), attribute( QLatin1String("looprev") ), st );
    else
        setAttribute( QLatin1String("looprev"), 0 );
    if ( !delay.isEmpty() )
        setAttribute( QLatin1String("delay"), delay.toInt(), st );
    else if ( st != ThemeItem::Default )
        setAttribute( QLatin1String("delay"), attribute( QLatin1String("delay") ), st );
    else
        setAttribute( QLatin1String("delay"), 0 );
    if ( !play.isEmpty() ) {
        if (isStringExpression(play)) {
            d->playExpr[idx] = createExpression(stripStringExpression(play));
            if ( d->playExpr[idx] )
                expressionChanged(d->playExpr[idx]);
        } else if (play != "no") {
            if (!attribute("playing"))
                start();
        }
    } else if ( st != ThemeItem::Default ) {
        d->playExpr[stateToIndex(st)] = d->playExpr[stateToIndex(ThemeItem::Default)];
    } else {
        d->playExpr[stateToIndex(st)] = 0;
    }
}

/*!
  Sets the current frame to \a frame.
  If the animation is currently playing, the animation will jump to the frame at \a frame.
  Otherwise, the animation will begin playing from the frame at \a frame.
*/
void ThemeAnimationItem::setFrame(int frame)
{
    if (frame >= 0 && frame < attribute(QLatin1String("count"), state()) && d->currFrame != frame) {
        d->currFrame = frame;
        if (isVisible())
            update();
    }
}

/*!
  Starts the animation.
  If a call to setFrame has been made, the animation will start from that position, otherwise
  starts playing from frame 0.
*/
void ThemeAnimationItem::start()
{
    int cst = stateToIndex( state() );
    //set all variables, even if there's no animation for this state
    if ( rtti() != ThemedView::Level ) {
        d->currFrame = 0;
        d->inc = 1;
    } //don't reset current frame for levels
    QPixmap pm = pixmap( QLatin1String("src"), state() );
    if ( d->fi[cst] && attribute(QLatin1String("delay"),state()) > 0 && !pm.isNull() ) {
        d->fi[cst]->start();
        setAttribute("playing", 1);
    }
}

/*!
  Stops the animation.
  The current frame is not reset, so calling play will continue from the frame the animation was stopped at.
*/
void ThemeAnimationItem::stop()
{
    for( int i = 0 ; i < 3 ; ++i )
        if (d->fi[i]) {
            d->fi[i]->stop();
        }
    setAttribute("playing", 0);
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a p.
*/
void ThemeAnimationItem::paint(QPainter *p, const QRect &)
{
    if ( !active() )
        return;

    QPixmap pm = pixmap( QLatin1String("src"), state() );
    if (!pm.isNull()) {
        p->drawPixmap((geometry().width()-attribute(QLatin1String("width"),state()))/2, (geometry().height()-pm.height())/2,
                pm, d->currFrame*attribute(QLatin1String("width"),state()), 0, attribute(QLatin1String("width"),state()), pm.height());
    }
}

/*!
  \reimp
  Lays out the animation item.
*/
void ThemeAnimationItem::layout()
{
    int count = attribute(QLatin1String("count"));
    ThemeItem::layout();
    scaleImages( count );
    setAttribute( QLatin1String("width"), pixmap( QLatin1String("src") ).width() / count );
}

/*!
  Advances the animation by one frame.
  If the current frame is the last frame in the animation but a loop point has been set, the animation jumps to that frame.
*/
void ThemeAnimationItem::advance()
{
    d->currFrame += d->inc;
    bool rev = attribute(QLatin1String("looprev"), state());
    if (d->currFrame >= attribute(QLatin1String("count"),state())) {
        if (attribute(QLatin1String("loop"),state()) >= 0) {
            if (rev) {
                d->inc = -d->inc;
                d->currFrame = attribute(QLatin1String("count"),state())-2;
            } else {
                d->currFrame = attribute(QLatin1String("loop"),state());
            }
        } else {
            stop();
        }
    } else if (d->currFrame < attribute(QLatin1String("loop"),state()) && d->inc < 0) {
        d->currFrame = attribute(QLatin1String("loop"),state())+1;
        d->inc = -d->inc;
    }
    if (isVisible())
        update();
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Animation.
*/
int ThemeAnimationItem::rtti() const
{
    return ThemedView::Animation;
}

/*!
  \reimp
*/
void ThemeAnimationItem::stateChanged( const ThemeItem::State& /*st*/ )
{
    if (attribute("playing") != 0) {
        stop();
        start();
    }
}

/*!
  \reimp
*/
void ThemeAnimationItem::constructionComplete()
{
    if ( !d->text.isEmpty() ) {
        if ( isStringExpression( d->text ) ) {
            d->frameExpr = createExpression( stripStringExpression(d->text));
            if ( d->frameExpr != 0 )
                expressionChanged( d->frameExpr );
        } else {
            setFrame( d->text.toInt() );
        }
        d->text = QString();
    }
    ThemePixmapItem::constructionComplete();
}

/*!
  \reimp
*/
void ThemeAnimationItem::expressionChanged( QExpressionEvaluator* expr )
{
    if ( d->frameExpr == expr ) {
        QVariant result = getExpressionResult( expr, QVariant::Int );
        if ( !result.canConvert(QVariant::Int) ) {
            qWarning() << "ThemeAnimationItem::expressionChanged() - Cannot convert value to Int.";
        } else {
            setFrame( result.toInt() );
        }
    } else if ( d->playExpr[stateToIndex(state())] == expr) {
        // play expr for current state
        QVariant result = getExpressionResult(expr, QVariant::Bool);
        if (result.canConvert(QVariant::Bool)) {
            bool b = result.toBool();
            if (b) {
                if (attribute("playing") == 0)
                    start();
            } else {
                if (attribute("playing") != 0)
                    stop();
            }
        }
    } else {
        ThemeItem::expressionChanged( expr );
    }
}

//---------------------------------------------------------------------------

struct ThemeLevelItemPrivate
{
    ThemeLevelItemPrivate()
        : minVal(0), maxVal(0), val(0)
    {
    }

    int minVal;
    int maxVal;
    int val;
};

/*!
  \class ThemeLevelItem
    \inpublicgroup QtBaseModule


  \brief The ThemeLevelItem class represents a level indicator in a ThemedView.

  The ThemeLevelItem class implements the \l{Themed View Elements#themelevelelement}{level element} from the theme XML.

  Examples of level indicators are the battery level or signal quality indicators in Qtopia.
  A ThemeLevelItem has a range given by minValue() and maxValue(). It displays an indicator
  showing a current value within that range.
  The range can be manually set using setRange(), and the current value using setValue().

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

    \sa {Themed View Elements#themelevelelement}{level element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeLevelItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeLevelItem::ThemeLevelItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeAnimationItem(parent, view, atts)
{
    d = new ThemeLevelItemPrivate;
    d->maxVal = attribute(QLatin1String("count"),state());

    if ( !atts.value( "max" ).isEmpty() )
        d->maxVal = atts.value( "max" ).toInt();
    if ( !atts.value( "min" ).isEmpty() )
        d->minVal = atts.value( "min" ).toInt();
}

/*!
  Destroys the ThemeLevelItem
*/
ThemeLevelItem::~ThemeLevelItem()
{
    delete d;
}

/*!
  \reimp
  */
void ThemeLevelItem::stop()
{
    // when this level stops playing, reset the level's value
    ThemeAnimationItem::stop();
    updateValue(d->val);
}

/*!
  Returns the current value of the level.
 */
int ThemeLevelItem::value() const { return d->val; }

/*!
  Returns the minimum value of the level, as set through the min attribute in the theme XML or setRange().
*/
int ThemeLevelItem::minValue() const { return d->minVal; }

/*!
  Returns the maximum value of the level, as set through the max attribute in the theme XML or setRange().
*/
int ThemeLevelItem::maxValue() const { return d->maxVal; }

/*!
  \reimp
*/
void ThemeLevelItem::setFrame(int v)
{
    setValue(v+d->minVal);
}

/*!
  Sets the current level of the level item to \a value.
*/
void ThemeLevelItem::setValue(int value)
{
    if (d->val != value)
        updateValue(value);
}

void ThemeLevelItem::updateValue(int v)
{
    d->val = v;
    int idx = 0;
    if (d->maxVal-d->minVal != 0)
        idx = attribute(QLatin1String("count"),state())*(d->val-d->minVal)/(d->maxVal-d->minVal);
    if (idx < 0)
        idx = 0;
    if (idx >= attribute(QLatin1String("count"),state()))
        idx = attribute(QLatin1String("count"),state())-1;
    ThemeAnimationItem::setFrame(idx);
}

/*!
  Sets the minimum and maximum value of the level to be \a min and \a max respectively.
*/
void ThemeLevelItem::setRange(int min, int max)
{
    d->minVal = min;
    d->maxVal = max;
}

/*!
  \reimp
*/
void ThemeLevelItem::layout()
{
    int oldFrame = frame();
    ThemeAnimationItem::layout();
    ThemeAnimationItem::setFrame(oldFrame);
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Level.
*/
int ThemeLevelItem::rtti() const
{
    return ThemedView::Level;
}


//---------------------------------------------------------------------------

struct ThemeStatusItemPrivate
{
    ThemeStatusItemPrivate()
        : isOn(false), onExpr(0)
        {}
    bool isOn;
    QExpressionEvaluator* onExpr;
};

/*!
  \class ThemeStatusItem
    \inpublicgroup QtBaseModule


  \brief The ThemeStatusItem class represents a status indicator in a ThemedView.

  The ThemeStatusItem class implements the \l{Themed View Elements#themestatuselement}{status element} from the theme XML.

  A ThemeStatusItem has 2 states on or off.
  You can see if a ThemeStatusItem is currently on using isOn() and set its state using setOn().
  Whether the item is on or off determines the way the item looks to the user. Eg. the item may be
  greyed out when it is off or not appear at all.

    \sa {Themed View Elements#themestatuselement}{status element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeStatusItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
  */
ThemeStatusItem::ThemeStatusItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemePixmapItem(parent, view, atts)
{
    d = new ThemeStatusItemPrivate;

    QMap<QString,QString> onClickAtts, onFocusAtts;
    if ( isInteractive() ) {
        onClickAtts = parseSubAtts( atts.value(QLatin1String("onclick")) );
        onFocusAtts = parseSubAtts( atts.value(QLatin1String("onfocus")) );
    }

    /* If specified, pass the value of the 'state' attribute to an Expression instance.
       The result of the expression controls whether this status item is on or not.
       Expressions can contain valuespace keys, so it can source its terms from elsewhere
       in the system */
    QString expr = atts.value( QLatin1String( "on" ) );
    bool isliteral = !isStringExpression(expr);
    if ( isliteral ) {
        d->isOn = expr != QLatin1String("no");
    } else {
        d->isOn = false;
        setAttribute("onExpression", stripStringExpression(expr));
    }

    QString filename = atts.value( QLatin1String("imageon") );
    createImage( QLatin1String("on"), filename, atts.value( QLatin1String("coloron") ), atts.value( QLatin1String("alphaon") ) );
    filename = onClickAtts[QLatin1String("imageon")];
    if ( filename.isEmpty() && onClickAtts[QLatin1String("coloron")].length() || onClickAtts[QLatin1String("alphaon")].length() )
        filename = atts.value( QLatin1String("imageon") );
    createImage( QLatin1String("on"), filename, onClickAtts[QLatin1String("coloron")], onClickAtts[QLatin1String("alphaon")], ThemeItem::Pressed );
    filename = onFocusAtts[QLatin1String("imageon")];
    if ( filename.isEmpty() && onFocusAtts[QLatin1String("coloron")].length() || onFocusAtts[QLatin1String("alphaon")].length() )
        filename = atts.value( QLatin1String("imageon") );
    createImage( QLatin1String("on"), filename, onFocusAtts[QLatin1String("coloron")], onFocusAtts[QLatin1String("alphaon")], ThemeItem::Focus );
    filename = atts.value( QLatin1String("imageoff") );
    createImage( QLatin1String("off"), filename, atts.value( QLatin1String("coloroff") ), atts.value( QLatin1String("alphaoff") ) );
}

/*!
  Destroys a ThemeStatusItem.
*/
ThemeStatusItem::~ThemeStatusItem()
{
    if ( d->onExpr ) {
        delete d->onExpr;
        d->onExpr = 0;
    }
    delete d;
}

/*!
   \reimp
*/
void ThemeStatusItem::constructionComplete()
{
    QString expr = strAttribute("onExpression");
    if ( !expr.isEmpty() ) {
        d->onExpr = createExpression( expr );
        if ( d->onExpr != 0 )
            expressionChanged( d->onExpr ); // Force initial update
        setAttribute("onExpression", QString());
    }
    ThemePixmapItem::constructionComplete();
}

/*!
  \reimp
 */
void ThemeStatusItem::expressionChanged( QExpressionEvaluator* expr )
{
    if ( d->onExpr == expr ) {
        QVariant result = getExpressionResult( expr, QVariant::Bool );
        if ( !result.canConvert(QVariant::Bool) ) {
            qWarning() << "ThemeStatusItem::expressionChanged() - Cannot convert value to Bool";
        } else {
            setOn( result.toBool() );
        }
    } else {
        ThemeItem::expressionChanged( expr );
    }
}

void ThemeStatusItem::createImage( const QString &key, const QString &filename, const QString &col, const QString &alpha, ThemeItem::State st )
{
    //if the user has specified a file, then col and alpha apply even if they're null
    //else copy deafult state's values
    QColor colour;
    int role = QPalette::NColorRoles;
    int al = 255;
    QPixmap pm;
    QString name = filename;
    if ( !filename.isEmpty() ) {
        role = parseColor( col, colour );
        if ( !alpha.isEmpty() )
            al = alpha.toInt();
    } else if ( st != ThemeItem::Default ) {
        role = attribute( QLatin1String("colorRole") + key );
        name = strAttribute(QLatin1String("image") + key);
        colour = color( QLatin1String("color") + key );
        al = attribute( QLatin1String("alpha") + key );
    }
    setAttribute(QLatin1String("image") + key, name, st);
    setAttribute( QLatin1String("colorRole") + key, role, st );
    setColor( QLatin1String("color") + key , colour, st );
    setAttribute( QLatin1String("alpha") + key, al, st );
}

void ThemeStatusItem::updateImage(const QString &key, ThemeItem::State st)
{
    if (strAttribute(QLatin1String("image") + key, st).isEmpty())
        return;

    int width = 0, height = 0;
    QPixmap pm = pixmap(QLatin1String("image") + key);
    if ( horizontalScale() && pm.width() != geometry().width() ||
             verticalScale() && pm.height() != geometry().height() ) {
        width = horizontalScale() ? geometry().width() : pm.width();
        height = verticalScale() ? geometry().height() : pm.height();
    }

    int alpha = attribute(QLatin1String("alpha") + key,st);

    pm = loadImage( strAttribute(QLatin1String("image") + key, st),
                    attribute(QLatin1String("colorRole") + key, st),
                    color(QLatin1String("color") + key,st), alpha, width, height );
    setPixmap( QLatin1String("image") + key, pm, st,
                strAttribute(QLatin1String("image") + key, st ) );
}

/*!
  Returns true if the status item is currently on, false otherwise.
  */
bool ThemeStatusItem::isOn() const { return d->isOn; }

/*!
  Sets whether the item is on or off based on the flag \a on.
  The status item is displayed to the user differently depending
  on whether it is on or off.
*/
void ThemeStatusItem::setOn(bool on)
{
    if (on != d->isOn) {
        d->isOn = on;
        if ( !d->isOn )
            setPressed(false); // if disabled can't be pressed
        if (isVisible())
            update();
    }
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Status.
*/
int ThemeStatusItem::rtti() const
{
    return ThemedView::Status;
}

/*!
  \reimp
    Paints the \a rect portion of the status item using the painter \a p.
*/
void ThemeStatusItem::paint(QPainter *p, const QRect &rect)
{
    Q_UNUSED(rect);
    QPixmap pm = d->isOn ? pixmap( QLatin1String("imageon"), state() ) : pixmap( QLatin1String("imageoff") );
    if (pm.isNull()) {
        if (d->isOn)
            updateImage(QLatin1String("on"), state());
        else
            updateImage(QLatin1String("off"), ThemeItem::Default);
        pm = d->isOn ? pixmap( QLatin1String("imageon"), state() ) : pixmap( QLatin1String("imageoff") );
    }

    if (!pm.isNull())
        p->drawPixmap((geometry().width()-pm.width())/2, (geometry().height()-pm.height())/2, pm);
}

/*!
  \reimp
  Lays out the status item.
*/
void ThemeStatusItem::layout()
{
    ThemeItem::layout();
}

//---------------------------------------------------------------------------

struct ThemeImageItemPrivate
{
    ThemeImageItemPrivate()
        : stretch(false), sorient(Qt::Horizontal)
        {}
    int offs[2];
    bool stretch;
    bool tile;
    Qt::Orientation sorient;
    QExpressionEvaluator* expressions[3];
};

/*!
  \class ThemeImageItem
    \inpublicgroup QtBaseModule


  \brief The ThemeImageItem class represents an image in a ThemedView.

  The ThemeImageItem class implements the \l{Themed View Elements#themeimageelement}{image element} from the theme XML.

  You can manually set an image at run-time using setImage()
  and retrieve the current image with image().

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML.

    \sa {Themed View Elements#themeimageelement}{image element}
  \ingroup appearance
  */

/*!
  Constructs a ThemeImageItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeImageItem::ThemeImageItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemePixmapItem(parent, view, atts)
{
    d = new ThemeImageItemPrivate;

    QMap<QString,QString> onClickAtts, onFocusAtts;
    if ( isInteractive() ) {
        onClickAtts = parseSubAtts( atts.value(QLatin1String("onclick")) );
        onFocusAtts = parseSubAtts( atts.value(QLatin1String("onfocus")) );
    }

    setAttribute( "src", atts.value(QLatin1String("src")));
    QString imgName = onClickAtts[QLatin1String("src")];
    if ( imgName.isEmpty() && (!onClickAtts[QLatin1String("alpha")].isEmpty() || !onClickAtts[QLatin1String("color")].isEmpty()) )
        imgName = atts.value("src");
    setAttribute( "src", imgName, ThemeItem::Pressed );
    imgName = onFocusAtts[QLatin1String("src")];
    if ( imgName.isEmpty() && (!onFocusAtts[QLatin1String("alpha")].isEmpty() || !onFocusAtts[QLatin1String("color")].isEmpty()) )
        imgName = atts.value("src");
    setAttribute( "src", imgName, ThemeItem::Focus );

    setupColor( QLatin1String("color"), QLatin1String("colorRole"), atts.value( QLatin1String("color") ), ThemeItem::Default );
    setupColor( QLatin1String("color"), QLatin1String("colorRole"), onClickAtts[QLatin1String("color")], ThemeItem::Pressed );
    setupColor( QLatin1String("color"), QLatin1String("colorRole"), onFocusAtts[QLatin1String("color")], ThemeItem::Focus );
    setupAlpha( QLatin1String("alpha"), atts.value( QLatin1String("alpha") ), ThemeItem::Default );
    setupAlpha( QLatin1String("alpha"), onClickAtts[QLatin1String("alpha")], ThemeItem::Pressed );
    setupAlpha( QLatin1String("alpha"), onFocusAtts[QLatin1String("alpha")], ThemeItem::Focus );

    QString val = atts.value(QLatin1String("stretch"));
    if (!val.isEmpty()) {
        QStringList ol = val.split(',');
        if (ol.count() == 2) {
            d->stretch = true;
            d->offs[0] = ol[0].toInt();
            d->offs[1] = ol[1].toInt();
        }
        d->sorient = atts.value(QLatin1String("orientation")) == QLatin1String("vertical") ? Qt::Vertical : Qt::Horizontal;
    }

    d->tile = atts.value(QLatin1String("tile")) == QLatin1String("yes");

    d->expressions[0] = 0;
    d->expressions[1] = 0;
    d->expressions[2] = 0;
}

/*!
  Destroys the ThemeImageItem.
*/
ThemeImageItem::~ThemeImageItem()
{
    for( int i = 0 ; i < 3 ; ++i )
        if ( d->expressions[i] != 0 )
            delete d->expressions[i];
    delete d;
}

/*!
  \reimp
*/
void ThemeImageItem::expressionChanged( QExpressionEvaluator* expr )
{
    // FIXME : implement expression handling for images
    Q_ASSERT(expr != 0);
    QString result = getExpressionResult(expr, QVariant::String).toString();
    if ( d->expressions[stateToIndex(ThemeItem::Default)] == expr ) {
        if ( strAttribute("src", ThemeItem::Default) != result ) {
            setAttribute( "src", result, ThemeItem::Default );
            updateImage( ThemeItem::Default );
        }
    } else if ( d->expressions[stateToIndex(ThemeItem::Pressed)] == expr ) {
        if ( strAttribute("src", ThemeItem::Pressed) != result ) {
            setAttribute( "src", result, ThemeItem::Pressed);
            updateImage( ThemeItem::Pressed );
        }
    } else if ( d->expressions[stateToIndex(ThemeItem::Focus)] == expr ) {
        if ( strAttribute("src", ThemeItem::Focus) != result ) {
            setAttribute( "src", result, ThemeItem::Focus );
            updateImage( ThemeItem::Focus );
        }
    } else {
        ThemeItem::expressionChanged( expr );
    }
}

/*!
  \reimp
  */
void ThemeImageItem::constructionComplete()
{
    // construct images now
    QString imgName = strAttribute("src", ThemeItem::Default);
    if ( isStringExpression(imgName) ) {
        d->expressions[stateToIndex(ThemeItem::Default)] = createExpression( stripStringExpression(imgName) );
        if ( d->expressions[stateToIndex(ThemeItem::Default)] != 0 )
            expressionChanged( d->expressions[stateToIndex(ThemeItem::Default)] );
    } else if ( !imgName.isEmpty() ) {
        updateImage( ThemeItem::Default );
    }
    imgName = strAttribute("src", ThemeItem::Pressed);
    if ( isStringExpression(imgName) ) {
        d->expressions[stateToIndex(ThemeItem::Pressed)] = createExpression( stripStringExpression(imgName) );
        if ( d->expressions[stateToIndex(ThemeItem::Pressed)] != 0 )
            expressionChanged( d->expressions[stateToIndex(ThemeItem::Pressed)] );
    } else if ( !imgName.isEmpty() ) {
        updateImage( ThemeItem::Pressed );
    }
    imgName = strAttribute("src", ThemeItem::Focus);
    if ( isStringExpression(imgName) )  {
        d->expressions[stateToIndex(ThemeItem::Focus)] = createExpression( stripStringExpression(imgName) );
        if ( d->expressions[stateToIndex(ThemeItem::Focus)] != 0 )
            expressionChanged( d->expressions[stateToIndex(ThemeItem::Focus)] );
    } else if ( !imgName.isEmpty() ) {
        updateImage( ThemeItem::Focus );
    }
    ThemePixmapItem::constructionComplete();
}

void ThemeImageItem::updateImage( ThemeItem::State st )
{
    Q_ASSERT(!isStringExpression(strAttribute("src", st)));
    //createImage( strAttribute("src", st), strAttribute(QLatin1String("color"), st), strAttribute(QLatin1String("alpha"), st), st );

    int alpha = 255;
    QPixmap pm;

    if ( !strAttribute("alpha",st).isEmpty() )
        alpha = strAttribute("alpha",st).toInt();
    pm = loadImage( strAttribute("src", st), attribute("colorRole", st), color("color",st), alpha );
    setPixmap( QLatin1String("src"), pm, st, strAttribute("src", st) );
    if( isVisible() ) {
        scaleImages();  //updated image might be different size. scalePixmap internally verifies whether a scale is needed.
        update();
    }
    /*
    setAttribute( QLatin1String("colorRole"), colorRole, st );
    setColor( QLatin1String("color"), colour, st );
    setAttribute( QLatin1String("alpha"), alpha, st );
    */
}

/*!
  Sets \a pixImage to be the image for this item for the state \a state.
*/
void ThemeImageItem::setImage(const QPixmap &pixImage, ThemeItem::State state)
{
    if (pixmap(QLatin1String("src"), state).serialNumber() != pixImage.serialNumber()) {
        setPixmap( QLatin1String("src"), pixImage, state );
        if ( isVisible() )
            update();
    }
}

/*!
  Returns the QPixmap for this item for the state \a state.
*/
QPixmap ThemeImageItem::image( ThemeItem::State state ) const
{
    return pixmap( QLatin1String("src"), state );
}

/*!
  \reimp
  Lays out the image item.
  If the geometry of the item has changed, the images are scaled according to the scale attribute.
*/
void ThemeImageItem::layout()
{
    int width = 0, height = 0;

    ThemeItem::layout();
    QPixmap pm = image();
    if ( horizontalScale() && pm.width() != geometry().width() ||
             verticalScale() && pm.height() != geometry().height() ) {
        width = horizontalScale() ? geometry().width() : pm.width();
        height = verticalScale() ? geometry().height() : pm.height();
    }
    if (d->stretch) {
        qreal ratio = 1.0;
        if ( d->sorient == Qt::Horizontal ) {
            if (geometry().height() && pm.height()) {
                ratio = (qreal)geometry().height() / pm.height();
                width = qRound( pm.width() * ratio );
                height = geometry().height();
            }
        }
        else {
            if (geometry().width() && pm.width()) {
                ratio = (qreal)geometry().width() / pm.width();
                width = geometry().width();
                height = qRound( pm.height() * ratio );
            }
        }
        d->offs[0] = qRound( ratio * d->offs[0] );
        d->offs[1] = qRound( ratio * d->offs[1] );
    }
    scaleImage( QLatin1String("src"), width, height );
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Image.
  */
int ThemeImageItem::rtti() const
{
    return ThemedView::Image;
}

/*!
  \reimp
    Paints the rect \a r portion of the image item using the painter \a p.
*/
void ThemeImageItem::paint(QPainter *p, const QRect &r)
{
    QPixmap pix = pixmap( QLatin1String("src"), state() );
    if ( pix.isNull() ) {
        pix = image();
    }

    if ( pix.isNull() )
        return;

    if (d->tile) {
        int dx = r.x() % pix.width();
        int dy = r.y() % pix.height();
        p->drawTiledPixmap(r.x()-dx, r.y()-dy,
                r.width()+dx, r.height()+dy, pix);
    } else if (!d->stretch) {
        QPoint off((geometry().width()-pix.width())/2, (geometry().height()-pix.height())/2);
        QRect cr = r;
        cr.translate(-off.x(), -off.y());
        cr &= QRect(0, 0, pix.width(), pix.height());
        p->drawPixmap(cr.topLeft()+off, pix, cr);
    } else {
        if (d->sorient == Qt::Horizontal) {
            int h = pix.height();
            p->drawPixmap(0, 0, pix, 0, 0, d->offs[0], h);
            int w = geometry().width() - d->offs[0] - (pix.width()-d->offs[1]);
            int sw = d->offs[1]-d->offs[0];
            int x = 0;
            if (sw) {
                for (; x < w-sw; x+=sw)
                    p->drawPixmap(d->offs[0]+x, 0, pix, d->offs[0], 0, sw, h);
            }
            p->drawPixmap(d->offs[0]+x, 0, pix, d->offs[0], 0, w-x, h);
            p->drawPixmap(geometry().width()-(pix.width()-d->offs[1]), 0, pix, d->offs[1], 0, pix.width()-d->offs[1], h);
        } else {
            int w = pix.width();
            p->drawPixmap(0, 0, pix, 0, 0, w, d->offs[0]);
            int h = geometry().height() - d->offs[0] - (pix.height()-d->offs[1]);
            int sh = d->offs[1]-d->offs[0];
            int y = 0;
            if (d->offs[1]-d->offs[0]) {
                for (; y < h-sh; y+=sh)
                    p->drawPixmap(0, d->offs[0]+y, pix, 0, d->offs[0], w, sh);
            }
                p->drawPixmap(0, d->offs[0]+y, pix, 0, d->offs[0], w, h-y);
            p->drawPixmap(0, geometry().height()-(pix.height()-d->offs[1]), pix, 0, d->offs[1], w, pix.height()-d->offs[1]);
        }
    }
}

/*!
    \internal
*/

void ThemeImageItem::paletteChange(const QPalette &)
{
    if ( attribute( QLatin1String("colorRole") ) != QPalette::NColorRoles)
        setImage( loadImage(strAttribute("src",ThemeItem::Default), attribute( QLatin1String("colorRole") ), color( QLatin1String("color") ), attribute( QLatin1String("alpha") ) ) );
    if ( attribute( QLatin1String("colorRole"), ThemeItem::Pressed ) != QPalette::NColorRoles )
        setPixmap( QLatin1String("src"), loadImage( strAttribute("src", ThemeItem::Pressed), attribute( QLatin1String("colorRole"), ThemeItem::Pressed ),
        color( QLatin1String("color"), ThemeItem::Pressed ), attribute( QLatin1String("alpha"), ThemeItem::Pressed ) ), ThemeItem::Pressed );
    if ( attribute( QLatin1String("colorRole"), ThemeItem::Focus ) != QPalette::NColorRoles )
        setPixmap( QLatin1String("src"), loadImage( strAttribute("src",ThemeItem::Focus), attribute( QLatin1String("colorRole"), ThemeItem::Focus ),
        color( QLatin1String("color"), ThemeItem::Focus ), attribute( QLatin1String("alpha"), ThemeItem::Focus ) ), ThemeItem::Focus );
}

class ThemeWidgetItemPrivate : public QObject
{
    Q_OBJECT
public:
    ThemeWidgetItemPrivate( ThemeWidgetItem* p )
        : item(p), widget(0), autoDelete(true)
        {}

    ~ThemeWidgetItemPrivate() {
        if ( widget && autoDelete )
            delete widget;
    }

    void setWidget( QWidget* w ) {
        if ( widget == w )
            return;
        if ( widget != 0 && autoDelete )
            delete widget;
        widget = w;
        widget->setParent(item->view());
        if( !item->active() ) widget->hide();
        else widget->show();
        // trying to base active semantic on show/hide events doesn't seem to work, too complicated, just always enforce widget visibility in paint()
        //widget->installEventFilter(this);
        item->parseColorGroup( colorGroupAtts );
        if ( widget != 0 )
            widget->setFont( item->parseFont( widget->font(), size, font ) );
    }


    ThemeWidgetItem* item;
    QWidget* widget;
    bool autoDelete;
    QMap<QString,QString> colorGroupAtts;
    QString font;
    QString size;

protected:
    /*
    virtual bool eventFilter( QObject* watched, QEvent* event ) {
        Q_ASSERT(watched == widget);
        if ( widget->isVisible() != parent->active() )
            parent->setActive( widget->isVisible() );
        if ( event->type() == QEvent::Hide ) {
            parent->setActive(false);
        } else if ( event->type() == QEvent::Show ) {
            parent->setActive(true);
        }
        return false;
    }
*/
};

/*!
  \class ThemeWidgetItem
    \inpublicgroup QtBaseModule


  \brief The ThemeWidgetItem class represents a widget in a ThemedView.

  The ThemeWidgetItem class implements the \l{Themed View Elements#themewidgetelement}{widget element} from the theme XML.

  The item can be assigned a widget by overriding ThemedView::newWidget() for the parent
  ThemedView for this item, or be calling setWidget() directly.
  ThemedView::newWidget() is called by this item once it has been completed constructed from the themed view XML.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML, or overload ThemedView::newWidget
  to provide a widget that way.

  \sa {Themed View Elements#themewidgetelement}{widget element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeWidgetItem.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeWidgetItem::ThemeWidgetItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts)
    : ThemeGraphicItem(parent, view, atts)
{
    d = new ThemeWidgetItemPrivate(this);
    if ( !atts.value(QLatin1String("colorGroup")).isEmpty() )
        d->colorGroupAtts = parseSubAtts(atts.value(QLatin1String("colorGroup")));
    d->size = atts.value(QLatin1String("size"));
    d->font = atts.value(QLatin1String("bold"));
}

/*!
  Destroys this ThemeWidgetItem.
  If a widget was set, and autoDelete() is true, the widget is deleted.
*/
ThemeWidgetItem::~ThemeWidgetItem()
{
    delete d;
}

/*!
  \reimp
  Calls show() and raise() on the set widget if active() returns true, otherwise calls hide().
*/
void ThemeWidgetItem::paint( QPainter* p, const QRect& r)
{
    Q_UNUSED(r);
    Q_UNUSED(p);
}

/*!
  \internal
*/
void ThemeWidgetItem::parseColorGroup( const QMap<QString,QString> &cgatts )
{
    if ( !d->widget )
        return;
    QPalette pal = view()->palette();
    for( int i = 0 ; colorTable[i].role != QPalette::NColorRoles ; ++i )
    {
        const QString curColorName = QString(colorTable[i].name).toLower();
        QColor colour;
        for( QMap<QString,QString>::ConstIterator it = cgatts.begin() ;
                                                    it != cgatts.end() ; ++it )
        {
            if ( it.key().toLower() == curColorName ) {
                colour = getColor( *it, parseColor( *it, colour ) );
                break;
            }
        }
        if ( colour.isValid() ) {
            pal.setColor( QPalette::Active, colorTable[i].role, colour );
            pal.setColor( QPalette::Inactive, colorTable[i].role, colour );
            pal.setColor( QPalette::Disabled, colorTable[i].role, colour );
        }
    }
    d->widget->setPalette( pal );
}

/*!
    \reimp
    Sets the geometry of the set widget to be \a geom.
*/
void ThemeWidgetItem::setGeometry( const QRect& geom )
{
    ThemeItem::setGeometry(geom);
    if ( !geom.isValid() )
        return;
    if ( d->widget ) {
        QRect r = rect();
        d->widget->setFixedSize(r.width(), r.height());
        d->widget->move(r.x(), r.y());
    }
}

/*!
  \reimp
  If \a f is true the widget is shown, otherwise the widget is hidden.
*/
void ThemeWidgetItem::setActive( bool f )
{
    ThemeItem::setActive( f );
    updateWidget();
}

void ThemeWidgetItem::updateWidget()
{
    if (d->widget != 0) {
        if (active()) {
            d->widget->show();
            d->widget->raise();
            d->widget->update();
        } else {
            d->widget->hide();
        }
    }
}

/*!
  \reimp
*/
void ThemeWidgetItem::constructionComplete()
{
    const QString in = itemName().toLower();

    QWidget* w = view()->newWidget(this,in); // done after virtual function table has been created, eg. for rtti()
    if ( w )
        setWidget( w );
    ThemeGraphicItem::constructionComplete();
}

/*!
  Sets the auto-delete flag for this widget item.
  If \a autodelete is true (the default), subsequent calls to setWidget()
  will cause any existing widget to be deleted.
  If you don't want ThemeWidgetItem to take memory ownership
  of the widgets you set on it, then call setAutoDelete(false) to disable
  this behaviour.
*/
void ThemeWidgetItem::setAutoDelete( bool autodelete )
{
    d->autoDelete = autodelete;
}

/*!
  Sets the widget \a widget on this item.
*/
void ThemeWidgetItem::setWidget( QWidget* widget )
{
    d->setWidget( widget );
    updateWidget();
}

/*!
  Returns the widget for this item.
*/
QWidget* ThemeWidgetItem::widget() const
{
    return d->widget;
}

/*!
  \internal
*/
void ThemeWidgetItem::paletteChange(const QPalette &)
{
    parseColorGroup( d->colorGroupAtts );
}

/*!
  \reimp
  Lays out the widget item.
  The widget is positioned and resized according to the theme.
*/
void ThemeWidgetItem::layout()
{
    if ( !d->widget )
        return;

    if( ThemeItem::d->rmode == Rect && ThemeItem::d->sr.width() < 0 )
        ThemeItem::d->sr.setWidth( d->widget->sizeHint().width() );
    if( ThemeItem::d->rmode == Rect && ThemeItem::d->sr.height() < 0 )
        ThemeItem::d->sr.setHeight( d->widget->sizeHint().height() );

    ThemeItem::layout();

    if ( d->widget != 0 ) {
        if ( !parentItem() || parentItem()->isVisible()) {
            d->widget->setFixedSize(rect().width(), rect().height());
            d->widget->move(rect().x(), rect().y());
        }
    }
}


/*!
  \reimp
  Run-time type information.
  Returns ThemedView::Widget.
*/
int ThemeWidgetItem::rtti() const
{
    return ThemedView::Widget;
}

//---------------------------------------------------------------------------

struct ThemeListItemPrivate
{
    ThemeListItemPrivate() : model(0) {}
    ThemeListModel* model;
};
/*!
  \class ThemeListItem
    \inpublicgroup QtBaseModule


  \brief The ThemeListItem class represents a graphical list in a themed view.

  The ThemeListItem class implements the \l{Themed View Elements#themelistelement}{list element} from the theme XML.

    As an extension of ThemeWidgetItem, ThemeListItem has the same functionality
    but only works with a QListView derived widget, and extra functionality needed
    to work with a ThemeListModel.

  Normally you do not want to call this item's functions directly, but rather
  specify the relevant attributes and data for this item in the themed view XML, and
  just pass it as a parameter when constructing a ThemeListModel. The model installs itself
  automatically using ThemeListItem::setModel().

    \sa ThemeListModel, {Themed View Elements#themelistelement}{list element}
  \ingroup appearance
*/

/*!
  Constructs a ThemeListItem object.
  \a parent, \a view and \a atts are passed to the base class constructor.
*/
ThemeListItem::ThemeListItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes & atts)
    : ThemeWidgetItem(parent, view, atts)
{
    d = new ThemeListItemPrivate;
    /*
    if ( 0 == widget() ) {
        setWidget( new QListView( ir ) );
    }
    */
}

/*!
  Destroys the ThemeListItem object.
*/
ThemeListItem::~ThemeListItem()
{
    delete d;
}

/*!
  Returns the ThemeListModel object associated with this ThemeListItem.
*/
ThemeListModel* ThemeListItem::model() const
{
    return d->model;
}

/*!
  Sets \a model to be the ThemeListModel associated with this ThemeListItem.
  If this item currently has a QListView widget set, this model is installed
  immediately on the view using QListView::setModel().
  If a call is made to QWidget::setWidget() in the future, this model
  will be installed in the same way.
*/
void ThemeListItem::setModel( ThemeListModel* model )
{
    if ( d->model != model ) {
        d->model = model;
        QListView* view = listView();
        if ( view != 0 && view->model() != d->model )
            view->setModel(d->model);
    }
}

/*!
  \reimp
  Run-time type information.
  Returns ThemedView::List.
*/
int ThemeListItem::rtti() const
{
    return ThemedView::List;
}

/*!
  \reimp
  Sets the QListView \a w as this ThemeListItem's widget.
  \a w must be non-zero and inherit QListView, otherwise this function will abort.
  An internal implementation of QItemDelegate is installed on the view to handle
  communication between itself and a ThemeListModel.
  If a ThemeListModel object has already been set using setModel(), it is installed immediately
  using QListView::setModel(). If a call is made to setModel() in the future, that model
  will be installed on this widget in the same way.
*/
void ThemeListItem::setWidget( QWidget* w )
{
    ThemeWidgetItem::setWidget( w );
    Q_ASSERT(widget() == 0 || widget()->inherits("QListView") == true);
    if ( !widget() )
        return;
    QListView* v = listView();
    v->setItemDelegate( new ThemeListDelegate( v, view(), view() ) );
    if (d->model != 0 && v->model() != d->model)
        v->setModel( d->model );
}

/*!
  Returns the QListView for this item.
  Equivalent to qobject_cast<QListView*>(widget())
*/
QListView* ThemeListItem::listView() const
{
    if ( !widget() )
        return 0;
    Q_ASSERT(widget()->inherits("QListView") == true);
    return qobject_cast<QListView*>(widget());
}

//----------------------------------------------------------
struct ThemeListModelEntryPrivate
{
    ThemeListModelEntryPrivate() : vsObject("/"), vsItem("/")
    {}
    QString uid;
    ThemeListModel* model;
    QValueSpaceObject vsObject;
    QValueSpaceItem vsItem;
    ThemeTemplateInstanceItem* templateInstance;
};

/*!
    \class ThemeListModelEntry
    \inpublicgroup QtBaseModule


    \brief The ThemeListModelEntry class implements a single theme list entry in a ThemeListModel.

    The ThemeListModelEntry has a uid() and a type(), which are used by templateInstance() to associate
    a single template instance to this entry. The ThemeListModelEntry stores this template instance
    internally and can be retrieved using templateInstance().

    This class must be subclassed and the pure virtual type() function reimplemented to return the appropriate
    value.

    \sa ThemeListModel, ThemeTemplateItem, ThemeTemplateInstanceItem
  \ingroup appearance
*/

/* define ThemeListModelEntry */
/*!
  Constructs a ThemeListModelEntry an associates it with the given \a model.
*/
ThemeListModelEntry::ThemeListModelEntry( ThemeListModel* model )
{
    d = new ThemeListModelEntryPrivate;
    d->uid = QUuid::createUuid().toString();
    d->model = model;
    d->templateInstance = 0;
}

/*!
    Destroys the ThemeListModelEntry.
*/
ThemeListModelEntry::~ThemeListModelEntry() {
    delete d;
}

/*!
    \fn QString ThemeListModelEntry::type() const = 0

    Returns the type of this entry.
    Subclasses must overrride this pure virtual function and return a value that corresponds
    to the name attribute of a template item defined under the associated list item in the themed view XML.
    templateInstance() calls this function to lookup and associate this entry to a ThemeTemplateItem.
    If the value that this function returns changes, either the subclass or the parent ThemeListModel implementation should
    instruct the view to repaint this entry, probably by calling ThemeListModel::triggerUpdate().
    The internal QItemDelegate implementation will call templateInstance() to create and associate a ThemeTemplateInstanceItem
    object with this entry, based on the new return value.
*/

/*!
  Returns a globally unique identifier for this entry.
  The return value of this function is used by templateInstance()
  in order to associate this entry with a theme template instance.
*/
QString ThemeListModelEntry::uid()
{
    return d->uid;
}

/*!
  Sets the given \a value in the valuespace based on the given \a key.
  The actual key set will be templateInstance()->fullVSPath() + \a key.
*/
void ThemeListModelEntry::setValue( const QString& key, const QVariant& value ) {
    d->vsObject.setAttribute( valuespacePath()+key, value );
}

/*!
  Retrives a value from the valuespace based on the given \a key.
  The actual valuespace key used to get the value will be templateInstance()->fullVSPath() + \a key.
*/
QVariant ThemeListModelEntry::value( const QString& key ) {
    return d->vsItem.value( valuespacePath()+key );
}

/*!
  Returns the parent ThemeListModel instance associated with this entry.
*/
ThemeListModel* ThemeListModelEntry::model() const {
    return d->model;
}

/*!
  \internal
*/
QString ThemeListModelEntry::valuespacePath() {
    Q_ASSERT(d->model != 0);
    if (!templateInstance() )
        return QString();
    return d->templateInstance->fullVSPath();
}

/*!
  Creates and returns a ThemeTemplateInstanceItem object for this entry.
  This function searches all template items defined under the list item in the themed view XML file
  looking for an item that has a name attribute matching the return value of type().
  When it finds one it calls ThemeTemplateItem::createInstance() passing the value of uid(), and stores the return value in this entry.
  If a template instance item already exists but its name attribute does not match the value currently returned by type(),
  then it is deleted and a new template instance is created.
  If no template can be found for this entry 0 is returned, otherwise a pointer to the associated ThemeTemplateInstanceItem is returned.
  \sa ThemeTemplateInstanceItem
*/
ThemeTemplateInstanceItem* ThemeListModelEntry::templateInstance()
{
    getTemplateInstance();
    return d->templateInstance;
}

void ThemeListModelEntry::getTemplateInstance() {
    Q_ASSERT(d->model != 0);
    if ( d->templateInstance == 0 ) {
        // FIXME : should only search under theme list item
        //qWarning("ThemeListModelEntry::getTemplateInstance() - Type returned '%s'", type().toAscii().data());
        ThemeTemplateItem* ti = static_cast<ThemeTemplateItem*>(d->model->themedView()->findItem( /*li,*/ type(), ThemedView::Template ));
        if ( !ti ) {
            qWarning("ThemeListModelEntry::getTemplateInstance() - Cannot find template item with name '%s'", type().toAscii().data());
            return;
        }
        d->templateInstance = ti->createInstance( uid() );
        if ( !d->templateInstance ) {
            qWarning("ThemeListModelEntry::getTemplateInstance() - Could not create template instance.");
            return;
        }
    } else if ( d->templateInstance->itemName() != type() ) {
       Q_ASSERT(d->templateInstance->itemName() != type());
       delete d->templateInstance;
       d->templateInstance = 0;
       getTemplateInstance(); // call again
    } // else the same
}


//-----------------------------------------------------------
/* Define ThemeListModel */
struct ThemeListModelPrivate
{
    QList<ThemeListModelEntry*> items;
    ThemeListItem* listItem;
    ThemedView* themedView;
};

/*!
    \class ThemeListModel
    \inpublicgroup QtBaseModule


    \brief The ThemeListModel class provides a list model that is used for list functionality in Qt Extended theming.

    List functionality in theming is implemented using Qt's model-view architecture and theme templates.

    To use a ThemeListModel you pass in the associated ThemeListItem and ThemedView objects during construction, and then
    populate it with ThemeListModelEntry items using the addEntry() and removeEntry() functions.
    The ThemeListModel installs itself as the model for the ThemeListItem's QListView object during construction.
    From that point on communication between the QListView and the ThemeListModel occurs through Qt's model-view architecture.

    Theme templates are used to describe the visual look of items in the ThemeListModel.
    As a list's items are added dynamically at runtime, they cannot be specified in the themed view XML.
    However, using theme templates, the visual appearance of a list item is defined instead, which is used to draw items as appropriate.
    Instances of these theme templates can be created at anytime using ThemeTemplateItem::createInstance().

    An internal implementation of QItemDelegate called ThemeListDelegate exists to handle communication between the QListView
    of a ThemeListItem, and associated ThemeListModel/ThemeListModelEntry objects.
    It works completely under the hood, and so this information is given only to help the developer understand how the system works.

    The ThemeListDelegate is installed using QListView::setItemDelegate() on the ThemeListItem's QListView. Qt's model-view architecture
    then asks the delegate to paint items in the QListView as required.
    When requested to repaint a particular QModelIndex in the QListView, the ThemeItemDelegate performs the following functions:
    \list
    \o The index is used to look up the associated ThemeListModelEntry through the associated ThemeListModel. The ThemeItemDelegate asserts
        that the return value of QListView::model() is an object that inherits from ThemeListModel.
    \o If no template instance has been created for the ThemeListModelEntry object, a new one is created using by calling ThemeListModelEntry::templateInstance()
        which calls ThemeTemplateItem::createInstance().
    \o If a template instance exists but the value returned by its itemName() function is not equal to the value returned by the ThemeListModelEntry's
        type() function, then the delegate deletes the existing template instance and calls ThemeTemplateItem::templateInstance() to get a new one.
    \o A pointer to the template instance is stored in the ThemeListModelEntry object.
    \o The template instance for the ThemeListModelEntry object is painted to the QListView at the index.
    \endlist

    See the ThemeTemplateItem and ThemeListModelEntry documentation for more information.

    The valuespace is used to communicate data from the system to items in the themed list.
    Normally a valuespace path can be assigned in the themed view XML to items using the vspath attribute of ThemeItem.
    However, as template definitions representing a theme list items' visual appearance are not actual instances,
    their vspath is dynamically determined and set at runtime when they are created using ThemeTemplateItem::createInstance().
    ThemeTemplateItem::createInstance() takes a uid which is actually set to the vspath of the returned ThemeTemplateInstance.
    For template instances created by ThemeListModelEntry::templateInstance() the uid passed is the return value of ThemeListModelEntry::uid(). This acts as a
    unique association between a theme template instance and the ThemeListModelEntry, as well as a unique valuespace path for both to communicate under.
    A call to ThemeListModelEntry::setValue() set keys under this unique vspath.
    For example, given the following themed view XML definition:
    \code
    <list vspath="/UserInterface/MyList/">
        <template name="myListItem">
            <text name="titleMessage">@TitleMessage</text>
        </template>
    </list>
    \endcode

    you could set the text of the element 'titleMessage' with the following:
    \code
    ThemeListModelEntry* myEntry; // assuming that myEntry->type() returns "myListItem"
    ..
    myEntry->setValue("TitleMessage", "Hello, World"); // expands to /UserInterface/MyList/<myEntry->uid()>/TitleMessage
    \endcode

    \sa {Themed View Elements}, ThemeTemplateItem, ThemeTemplateInstanceItem, ThemeListModelEntry
  \ingroup appearance
*/


/*!
  Constructs a ThemeListModel.
  \a parent is passed to QAbstractListModel, \a listItem is the ThemeListItem associated with this model and \a view
  is the ThemedView associated with this model.
  The model is installed on the ThemeListItem \a listItem using the ThemeListItem::setModel() function.
*/
ThemeListModel::ThemeListModel( QObject* parent, ThemeListItem* listItem, ThemedView *view )
    : QAbstractListModel(parent)
{
    d = new ThemeListModelPrivate;
    d->listItem = listItem;
    d->themedView = view;
    Q_ASSERT(listItem != 0);
    listItem->setModel(this);
}

/*!
  Destroys the model. All items in the model are deleted.
*/
ThemeListModel::~ThemeListModel()
{
    clear();
    delete d;
}

/*!
   Returns a QList of all items in the model.
*/
QList<ThemeListModelEntry*> ThemeListModel::items() const
{
    return d->items;
}

/*!
  Returns the ThemedView instance associated with this model.
*/
ThemedView* ThemeListModel::themedView() const
{
    return d->themedView;
}

/*!
   \reimp
   Returns the number of items below \a parent in this model.
*/
int ThemeListModel::rowCount(const QModelIndex &parent ) const
{
    Q_UNUSED(parent);
    return d->items.count();
}

/*!
  Returns the ThemeListItem instance associated with this model.
*/
ThemeListItem* ThemeListModel::listItem() const
{
    return d->listItem;
}

/*!
  \reimp
*/
QVariant ThemeListModel::data(const QModelIndex &index, int ) const
{
    QVariant ret;
    if (index.isValid() && (index.row() < d->items.count()))
        ret = QVariant::fromValue( d->items.at(index.row()) );
    return ret;
}


/*!
  Returns the index of the item \a entry in the model.
*/
QModelIndex ThemeListModel::entryIndex( const ThemeListModelEntry* entry ) const
{
    int idx = d->items.indexOf( const_cast<ThemeListModelEntry*>(entry) );
    return index( idx, 0 );
}

/*!
  Returns a pointer to the ThemeListModelEntry at \a index.
*/
ThemeListModelEntry* ThemeListModel::themeListModelEntry(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= d->items.count())
        return 0;
    return d->items.at(index.row());
}

/*!
  Adds \a item to the end of this model.
  ThemeListModel takes ownership of \a item and will delete it when it is removed.
*/
void ThemeListModel::addEntry( ThemeListModelEntry* item )
{
    /* calling addEntry transfers ownership of item to this model ie. removeEntry and clear will delete it*/
    beginInsertRows(QModelIndex(), d->items.count(), d->items.count());
    d->items.append(item);
    endInsertRows();
}

/*!
  Removes the item at \a index from the mdoel.
  The item is deleted.
*/
void ThemeListModel::removeEntry( const QModelIndex &index )
{
    if (!index.isValid())
        return;

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    ThemeListModelEntry* entry = d->items.takeAt(index.row());
    Q_ASSERT(entry != 0);
    delete entry;
    endRemoveRows();
}

/*!
  Clears all items from the model.
  The items are deleted.
*/
void ThemeListModel::clear()
{
    if ( !rowCount() )
        return;
    beginRemoveRows(QModelIndex(), 0, rowCount() );
    while( d->items.count() != 0 ) {
        ThemeListModelEntry* entry = d->items.takeFirst();
        Q_ASSERT(entry != 0);
        delete entry;
    }
    endRemoveRows();
}

/*!
  Triggers an update in the view for all items in the model.
 */
void ThemeListModel::triggerUpdate()
{
    emit dataChanged(index(0), index(rowCount()-1));
}

//===================================================================
/* Define ThemeListDelegate */

struct ThemeListDelegatePrivate
{
    QListView* listView;
    ThemedView* themedView;
};

ThemeListDelegate::ThemeListDelegate(QListView* listview, ThemedView* tv, QObject *parent)
    : QItemDelegate(parent)
{
    d = new ThemeListDelegatePrivate;
    d->listView = listview;
    d->themedView = tv;
}

ThemeListDelegate::~ThemeListDelegate()
{
    delete d;
}

void ThemeListDelegate::paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    const ThemeListModel* model = qobject_cast<const ThemeListModel*>(index.model());
    ThemeListModelEntry* entry = model->themeListModelEntry(index);
    if (entry == 0) {
        qWarning("ThemeListDelegate::paint(): invalid index passed ");
        return;
    }
    Q_ASSERT(d->themedView != 0);
    Q_ASSERT(d->listView != 0);

    if ( !entry->templateInstance() )
        return;

    // paint that item into this rect
    QSize listSize = sizeHint(option,index);
    p->save();
    p->setClipRect(option.rect);
    p->translate(option.rect.x(), option.rect.y());

    ThemeItem *bgItem = d->themedView->findItem("background", ThemedView::Item);
    if (bgItem)
        d->themedView->paint(p, d->listView->visualRect(index), bgItem);

    QSize itemSize = sizeHint(option, index);
    d->themedView->paint(p, QRect(0,0,itemSize.width(),itemSize.height()), entry->templateInstance());

    p->restore();
}

QSize ThemeListDelegate::sizeHint( const QStyleOptionViewItem&,
            const QModelIndex& index) const
{
    const ThemeListModel* model = qobject_cast<const ThemeListModel*>(index.model());
    ThemeListModelEntry* entry = model->themeListModelEntry(index);
    int w = d->listView->width();
    if (!entry ) {
        qWarning("ThemeListDelegate::sizeHint(): invalid index passed");
        return QSize(w, 0);
    }
    return QSize(w,height(entry,index));
}

int ThemeListDelegate::height(ThemeListModelEntry* entry, const QModelIndex& ) const {
    Q_ASSERT(entry != 0);
    if ( !entry->templateInstance() )
        return 0;
    return entry->templateInstance()->geometry().height();
}

//---------------------------------------------------------------------------

/*!
  \class ThemedView
    \inpublicgroup QtBaseModule


  \brief The ThemedView widget constructs, manages and displays themed views in Qtopia.

  A Qt Extended theme is made up of multiple themed views, each described by an XML document called the themed view XML.
  You construct a new ThemedView by passing a themed view XML file to the ThemedView::loadSource() function.
  The ThemedView widget parses the XML, creates the theme item tree and renders it to the widget.

  The theme item tree is the run-time representation of a themed view. Each node in the tree derives from ThemeItem
  and implements some specific functionality eg. ThemeTextItem displays text, ThemeRectItem draws rectangles, ThemeLayoutItem
  positions and sizes other items etc. The ThemedView widget manages the theme item tree, instructing items to layout and paint themselves
  at the appropriate times.

  As the ThemedView is basically a customizeable user interface component, a core part of a themed view is its ability to display data to the user.
  There are 2 different methods of getting data into a themed view for display to the user.
  1. Using the ThemedView::findItem() function, you can get access to theme items in the tree and set data on them directly. Eg.
  \code
    ThemedView *view;
    ..
    ThemeTextItem* myTextItem = static_cast<ThemeTextItem*>(view->findItem("myTextItem", ThemedView::Text);
    if (myTextItem != 0)
        myTextItem->setText("Hello, World!");
  \endcode
  The above example finds the item with name "myTextItem" and a ThemeItem::rtti() value of ThemedView::Text and
  sets its contents to be "Hello, World!".

  2. ThemeLevelItem, ThemeStatusItem, ThemeTextItem and ThemeImageItem can source their data through a QExpressionEvaluator.
     This method allows items to source data from the system without any tight-coupling.  See the item-specific documentation for details.

  The ThemedView is also responsible for handling user interactivity with theme items. You can monitor user interactivity
  programatically by connecting to the ThemedView::itemPressed(), ThemedView::itemClicked() and ThemedView::itemReleased() signals.
  Alternatively, theme items can send IPC messages to services or a specific channel when clicked. This allows themed views to effect
  the system without tight coupling. See the documentation of ThemeItem for details.

  \ingroup appearance
*/

/*!
    \enum ThemedView::Type

    \value Item A generic item.
    \value Page Top-level item for all themed views.
    \value Animation  An item for displaying pixmap collage animations.
    \value Level An item for displaying a graphical level indicator.
    \value Status An item for displaying an on/off indicator.
    \value Image An item for displaying images
    \value Text An item for displaying text
    \value Rect An item for displaying rectangles.
    \value Line An item for displaying lines.
    \value Plugin An item for adding a plug-in as input.
    \value Exclusive A container item that allows only 1 of its children to be active at once.
    \value Layout A container item that positions and sizes its children.
    \value Group A container that groups its children together
    \value Widget An item that allows a widget to be displayed in a themed view.
    \value List An item that displays a list based on QListView.
    \value Template An item that interprets its child hierarchy declratively and can create instances of its hierarchy dynamically.
    \value TemplateInstance An instance of a Template

    \sa ThemeItem::rtti()
*/

/*!
  \fn void ThemedView::itemPressed(ThemeItem* item)

  Emitted when the interactive item \a item is pressed.
*/

/*!
  \fn void ThemedView::itemReleased(ThemeItem* item)

  Emitted when the interactive item \a item is released.
*/

/*!
  \fn void ThemedView::itemClicked(ThemeItem* item)

  Emitted when the interactive item \a item is clicked.
  A click is a press and a release.
*/

/*!
  \fn void ThemedView::visibilityChanged(ThemeItem* item, bool visible)

  Emitted when the visibility of \a item changes.
  The \a visible parameter is true if \a item is visible or false otherwise.
*/

/*!
  \fn void ThemedView::loaded()

  Emitted when the theme is successfully loaded.
*/

/*!
  Constructs a ThemedView object passing the \a parent widget and widget flags \a f to the QWidget constructor.
*/
ThemedView::ThemedView(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
{
    d = new ThemedViewPrivate(this);
}

/*!
  Destroys the ThemedView object and all of its data.
*/
ThemedView::~ThemedView()
{
    delete d;
}

/*!
    Called by the ThemeWidgetItem \a item when it wishes to construct the widget corresponding to the specified \a name.
    \a name is the name attribute from the themed view XML.
    This function must be overriden by subclasses for themed views that have a ThemeWidgetItem in them.
    The default implementation always returns 0.
    \sa ThemeWidgetItem
*/
QWidget *ThemedView::newWidget(ThemeWidgetItem* item, const QString& name)
{
    Q_UNUSED(item);
    Q_UNUSED(name);
    return 0;
}

/*!
  Registers the expression \a expression for the given theme \a item.
  The ThemedView will unregister the expression when \a expression is deleted, and call
  the expressionChanged() call-back on \a item whenever the QExpressionEvaluator::termsChanged()
  signal is emitted.
*/
void ThemedView::registerExpression(ThemeItem *item, QExpressionEvaluator *expression)
{
    if ( d->expressionToThemeItemMap.contains( expression ) ) {
        qWarning("ThemedView::registerExpression - Already registered expression for this item");
        return;
    }
    d->expressionToThemeItemMap.insert( expression, item );
    connect( expression, SIGNAL(termsChanged()), this, SLOT(notifyExpressionChanged()) );
    connect( expression, SIGNAL(destroyed(QObject*)), this, SLOT(expressionDestroyed(QObject*)) );
}

void ThemedView::notifyExpressionChanged()
{
    QExpressionEvaluator* expr = (QExpressionEvaluator *)sender();
    Q_ASSERT(d->expressionToThemeItemMap.contains(expr));
    d->expressionToThemeItemMap[expr]->expressionChanged( expr );
}

void ThemedView::expressionDestroyed( QObject* obj )
{
    QExpressionEvaluator* expr = (QExpressionEvaluator *)obj;
    Q_ASSERT(d->expressionToThemeItemMap.contains(expr));
    d->expressionToThemeItemMap.remove( expr );
}

/*!
  \internal
  */
void ThemedView::mousePressEvent( QMouseEvent *e )
{
    QWidget::mousePressEvent( e );
    QPoint p = e->pos();
    //find the closest item to p
    d->pressedItem = itemAt( p );
    if ( d->pressedItem ) {
        d->pressedItem->setPressed( true );
        if ( !d->pressedItem->pressed() )
            d->pressedItem = 0;
        else
            emit itemPressed( d->pressedItem );
    }
}

/*!
  \reimp
*/
void ThemedView::mouseMoveEvent( QMouseEvent *e )
{
    ThemeItem *item = itemAt( e->pos() );
    //no item under mouse or an item that is not the initially pressed item
    if ( !item || item != d->pressedItem )
        while( (item = findItem( QString(), ThemedView::Item, ThemeItem::Pressed)) )
            item->setPressed( false );
    //item that was initially pressed, set to be pressed if not already
    else if ( !item->pressed() )
        item->setPressed( true );
    //else initially pressed item already pressed
}

/*!
  \reimp
  */
void ThemedView::mouseReleaseEvent( QMouseEvent *e )
{
    ThemeItem *item = itemAt( e->pos() );
    bool ic = false;
    if ( item && item == d->pressedItem )
        ic = true;
    ThemeItem *pitem;
    while( (pitem = findItem( QString(), ThemedView::Item, ThemeItem::Pressed )) )
        pitem->setPressed( false );
    if (item)
        emit itemReleased( item );
    if ( ic )
    {
        d->pressedItem->clickedEvent(); // deliver the pressed event to the item
        emit itemClicked( d->pressedItem );
        d->pressedItem = 0;
    }
}

/*!
  If the rectangle \a rect is different to the current rect(),
  sets the new geometry to \a rect and calls layout() immediately.
*/
void ThemedView::setGeometryAndLayout(const QRect &rect)
{
    setGeometryAndLayout(rect.x(), rect.y(), rect.width(), rect.height());
}

/*!
  \overload
  Equivalent to calling ThemedView::setGeometryAndLayout(QRect(\a gx, \a gy, \a w, \a h))
*/
void ThemedView::setGeometryAndLayout(int gx, int gy, int w, int h)
{
    if ( w!=width() || h!=height() || gx!=x() || gy!=y() ) {
        setGeometry(gx,gy,w,h);
        layout();
    }
}

/*!
   Returns the ThemeItem located at the point \a pos in this ThemedView.
   \a pos is in local coordinates.
*/
ThemeItem *ThemedView::itemAt(const QPoint &pos) const
{
    if ( !d->root )
        return 0;
    return itemAt( pos, d->root );
}

ThemeItem *ThemedView::itemAt(const QPoint &pos, ThemeItem *item) const
{
    if ( !item->children().count() )
        return 0;
    ThemeItem *bestMatch = 0; //
    QList<ThemeItem*> c = item->children();
    foreach(ThemeItem *cur, c) {
        //br is local coordinates, ThemeItem::rect sums the x,y offsets back to top parent
        if ( cur->rect().contains( pos )
                && (!item->transient() || item->active()))
        {
            if ( cur->rtti() == ThemedView::Group && cur->isInteractive() && cur->active() && isOn( cur ) ) {
                bestMatch = cur; // current is a group and interactive
            } else
            {
                ThemeItem *f = itemAt( pos, cur );
                if ( f && f->isInteractive() && f->active() && isOn( f ) )
                    bestMatch = f; // an item, deeper than cur
                else if ( cur->isInteractive() && cur->active() && isOn( cur ) )
                    bestMatch = cur; //cur item is deepest
                else if ( f && f->active() && isOn( f ) && (!bestMatch || !bestMatch->isInteractive()) )
                    bestMatch = f; // not user items, but some kind of items under the cursor. only set if don't already have an interactive match
                else if ( cur->active() && isOn( cur ) && (!bestMatch || !bestMatch->isInteractive()) )
                    bestMatch = cur;
                //if it's not a user item, just a rect or a line or something, keep looking for a better match
            }
        }
    }

    return bestMatch;
}

/*!
   Returns true if a themed view XML document has been loaded using ThemedView::loadSource() for this ThemedView, otherwise returns false.
*/
bool ThemedView::sourceLoaded() const
{
    return d->sourceLoaded;
}

/*!
  Set the themed view XML file to be \a fileName.
  If ThemedView::loadSource() is called with an empty argument (the default) the themed view XML file \a fileName will be loaded.
*/
void ThemedView::setSourceFile(const QString &fileName)
{
    d->themeSource = fileName;
}

/*!
  Loads the themed view XML specified by \a fileName.
  If \a fileName is empty (the default) but ThemedView::setSourceFile() has been called, the \a fileName value from that call will be used.
  Returns true if a source was successfully loaded, otherwise returns false.
*/
bool ThemedView::loadSource(const QString &fileName)
{
    if ( !fileName.isEmpty() )
        d->themeSource = fileName;
    if ( d->themeSource.isEmpty() ) {
        qWarning("ThemedView::loadSource() - No theme file to set.");
        themeLoaded(QString());
        emit loaded();
        return false;
    }

    if ( d->root ) { // delete before reset
        delete d->root;
        d->root = 0;
    }
    QFile file(d->themeSource);
    if (file.exists()){
        if (file.open(QFile::ReadOnly | QFile::Text))
            d->factory->readThemedView(&file);
        file.close();
        d->root = d->factory->root();
        if (d->root && isVisible()) {
            layout();
            update();
        } else {
            d->needLayout = true;
        }
        d->sourceLoaded = true;
    } else {
        qWarning() << "Unable to open " << file.fileName();
        themeLoaded(QString());
        emit loaded();
        return false;
    }
    themeLoaded(d->themeSource);
    emit loaded();
    return true;
}

/*!
    Paints \a item using the painter \a painter within the given \a clip rect.
    You normally don't want to call this function directly, but it is useful
    for painting template items to a user-specified painter.
*/
void ThemedView::paint(QPainter *painter, const QRect &clip, ThemeItem *item)
{
    if (!d->root)
        return;
    if (d->needLayout)
        layout();
    if (!item)
        item = d->root;
    paintItem(painter, item, clip);
}

/*!
  \reimp
  */
void ThemedView::paintEvent(QPaintEvent *pe)
{
    if (!d->root)
        return;
    if (d->needLayout)
        layout();
    const QRect &clip = pe->rect();

    QPainter bp(this);
    paintItem(&bp, d->root, clip);
}

/*!
  \internal
  Print, using qWarning(), all the items in the theme and their current states.
 */
void ThemedView::dumpState() const
{
    dumpState(d->root, 0);
}

void ThemedView::dumpState(ThemeItem *item, int _indent) const
{
    QByteArray indentba(_indent * 4, ' ');
    const char *indent = indentba.constData();

    qWarning() << indent << rttiToString(item->rtti()) << (item->itemName().isEmpty()?"NoName":item->itemName()) << item->rect() << item->geometry() << item->geometryHint();

    QList<ThemeItem*> children = item->children();
    foreach(ThemeItem *child, children)
        dumpState(child, _indent + 1);
}

QString ThemedView::rttiToString(int rtti) const
{
    switch(rtti) {
        case Item:
            return "Item";
        case Page:
            return "Page";
        case Animation:
            return "Animation";
        case Level:
            return "Level";
        case Status:
            return "Status";
        case Image:
            return "Image";
        case Text:
            return "Text";
        case Rect:
            return "Rect";
        case Line:
            return "Line";
        case Plugin:
            return "Plugin";
        case Exclusive:
            return "Exclusive";
        case Layout:
            return "Layout";
        case Group:
            return "Group";
        case Widget:
            return "Widget";
        case List:
            return "List";
        case Template:
            return "Template";
        case TemplateInstance:
            return "TemplateInstance";
        default:
            return "Unknown";
    };
}

/*!
  \internal
  Paint \a item and all sub-items on the painter \a painter.  \a clip dictates the
  clip rectangle to use relative to \a item.  That is a clip rect of
  QRect(0, 0, item->width(), item->height()) does not clip item at all.
 */
void ThemedView::paintItem(QPainter *painter, ThemeItem *item, const QRect &clip)
{
    if (!clip.isEmpty() &&
       (!item->transient() || item->active()) &&
       item->isVisible()) {
        QRect geom = item->geometry();
        QRect myClip = clip.intersected(geom).translated(-geom.topLeft());
        if (!myClip.isEmpty()) {
            painter->translate(geom.topLeft());
            painter->setClipRect(myClip);
            item->paint(painter, myClip);

            QList<ThemeItem*> children = item->children();
            foreach( ThemeItem *child, children ) {
                paintItem(painter, child, myClip);
            }

            painter->translate(-geom.topLeft());
        }
    }
}

/*!
  \reimp
*/
QSize ThemedView::sizeHint() const
{
    if (d->root) {
        Q_ASSERT(d->root->rtti() == ThemedView::Page);
        return static_cast<ThemePageItem*>(d->root)->sizeHint();
    }
    return QWidget::sizeHint();
}

ThemePageItem* ThemedView::pageItem() const
{
    ThemeItem* item = d->root;
    if ( !item ) { // get from factory if in the process of parsing
        Q_ASSERT(d->factory != 0);
        item = d->factory->root();
    }
    Q_ASSERT(item != 0);
    Q_ASSERT(item->rtti() == ThemedView::Page);
    return (static_cast<ThemePageItem*>(item));
}

/*!
  Returns the name of the theme this ThemedView belongs to as set by setThemeName().
*/
QString ThemedView::themeName() const
{
    return d->themeName;
}

/*!
  Sets the name of the theme this ThemedView belongs to be \a themeName.
  The name of the theme will be used as the key for text translations.
  themeName() simply returns this value.
*/
void ThemedView::setThemeName(const QString& themeName)
{
    d->themeName = themeName;
}

/*!
  Returns the \c name attribute of the \c page element from the themed view XML.
*/
const QString ThemedView::pageName() const
{
    ThemePageItem* item = pageItem();
    QString n;
    if (item)
        n = item->itemName();
    return n;
}

/*!
  Returns the base directory for this ThemedView.
  This function simply accesses the root theme item and returns ThemePageItem::base().
  As such, you cannot call this function before the root item has been constructed (ie.
  before a themed view XML has been successfully loaded).
*/
const QString ThemedView::base() const
{
    ThemePageItem* item = pageItem();
    QString b;
    if (item)
        b = item->base();
    return b;
}

/*!
   Returns the default directory for theme images.
   The default is $QPEDIR/themes/default.
*/
const QString ThemedView::defaultPics() const
{
    return QString(QLatin1String("themes/default/"));
}

/*!
  \deprecated
*/
QList<ThemeItem*> ThemedView::findItems(const QString &name, int type, int state) const
{
    return findItems(name, (Type)type, (ThemeItem::State)state);
}

/*!
  \deprecated
*/
ThemeItem *ThemedView::findItem(const QString &name, int type, int state) const
{
    return findItem(name, (Type)type, (ThemeItem::State)state);
}

/*!
  \deprecated
*/
ThemeItem *ThemedView::findItem(ThemeItem *item, const QString &name, int type, int state) const
{
    return findItem(item, name, (Type)type, (ThemeItem::State)state);
}

/*!
  \deprecated
*/
void ThemedView::findItems(ThemeItem *item, const QString &name, int type,
                           int pressed, QList<ThemeItem*> &list) const
{
    return findItems(item, name, (Type)type, (ThemeItem::State)pressed, list);
}

/*!
  Finds the item in the theme item tree that has the name \a name, the type \a type and the state \a state.
  If the value of \a type is ThemedView::Item, items with any type are searched for.
  The default value of \a state is ThemeItem::All which matches the state of all items.
  Returns a pointer to the item if found, otherwise returns 0.
*/
ThemeItem *ThemedView::findItem(const QString &name, ThemedView::Type type, ThemeItem::State state) const
{
    if (!d->root)
        return 0;
    ThemeItem *item = findItem(d->root, name, type, state);
    if (item && d->needLayout) {
        ThemedView *v = const_cast<ThemedView*>(this);
        v->layout();
    }

    return item;
}

ThemeItem *ThemedView::findItem(ThemeItem *item, const QString &name,
                                ThemedView::Type type, ThemeItem::State state) const
{
    if (!item)
        item = d->root;
    bool statematch = (state == ThemeItem::All || state == item->state());
    if ( (name.isNull() || item->itemName() == name) && (!type || type == item->rtti()) && statematch )
        return item;

    QList<ThemeItem*> c = item->children();
    foreach ( ThemeItem *itm, c ) {
        ThemeItem *fi = findItem(itm, name, type, state);
        if (fi)
            return fi;
    }

    return 0;
}

/*!
  Finds all of the items in the theme item tree that have the name \a name, the type \a type and the state \a state.
  If the value of \a type is ThemedView::Item, items with any type are searched for.
  By default, \a state if set to ThemeItem::All, and so items with any state are search for.
  Returns any items found.
*/
QList<ThemeItem*> ThemedView::findItems(const QString &name, ThemedView::Type type, ThemeItem::State state) const
{
    QList<ThemeItem*> list;
    if (d->root)
        findItems(d->root, name, type, state, list);
    return list;
}



void ThemedView::findItems(ThemeItem *item, const QString &name, ThemedView::Type type,
                           ThemeItem::State state, QList<ThemeItem*> &list) const
{
    if ( !item )
        return;
    bool statematch = (state == ThemeItem::All || state == item->state());
    if ( (name.isNull() || item->itemName() == name) && (!type || type == item->rtti()) && statematch )
        list.append(item);

    QList<ThemeItem*> c = item->children();
    foreach ( ThemeItem *itm, c ) {
        findItems(itm, name, type, state, list);
    }
}

/*!
  \fn void ThemedView::themeLoaded(const QString &file)
  Called whenever the theme, \a file, is loaded.  This allows derived classes to
  configure themselves appropriately for the new theme.

  The default implementation does nothing.
 */
void ThemedView::themeLoaded(const QString &)
{
}


/*!
  Lays out \a item an of its children.
*/
void ThemedView::layout(ThemeItem *item)
{
    if (!item) {
        item = d->root;
        d->needLayout = false;
    }
    if ( !item )
        return;
    if ( !item->parentItem() || item->parentItem()->rtti() != ThemedView::Layout ) {
        /* Don't call layout() on items that are direct children of a ThemeLayoutItem. ThemeLayoutItem
           calls layout() on its direct children before it lays out itself in order to get their 'real' geometry (stored in br).
           It needs to do this to work out how to place items within itself.
       */
        item->layout();
    }
    QList<ThemeItem*> c = item->children();
    foreach ( ThemeItem *itm, c ) {
            layout(itm);
    }
}

void ThemedView::paletteChange(const QPalette &p)
{
    if (d->root)
        paletteChange(d->root, p);
}

void ThemedView::paletteChange(ThemeItem *item, const QPalette &p)
{
    item->paletteChange(p);
    QList<ThemeItem*> c = item->children();
    foreach( ThemeItem *itm, c ) {
        paletteChange(itm, p);
    }
}

/*!
  \reimp
  */
void ThemedView::resizeEvent(QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    if (d->root) {
        if (isVisible())
            layout();
        else
            d->needLayout = true;
    }
}

/*!
  \reimp
  */
void ThemedView::showEvent(QShowEvent *)
{
    if (d->needLayout && d->root)
        layout();
}

void ThemedView::visChanged(ThemeItem *item, bool vis)
{
    emit visibilityChanged(item, vis);
}

bool ThemedView::isOn( ThemeItem *item ) const
{
    if ( !item )
        return false;
    if ( item->rtti() == ThemedView::Status )
    {
        ThemeStatusItem *statusItem = (ThemeStatusItem *)item;
        if ( !statusItem->isOn() )
            return false;
    }
    return true;
}


#include "themedview.moc"

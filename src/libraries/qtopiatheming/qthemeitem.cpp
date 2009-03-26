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

//#define DEBUG_SNAP
#include "qthemeitem_p.h"

#include <QThemedView>
#include <QThemedScene>
#include <QThemeItem>
#include <QThemeExclusiveItem>
#include <QThemeImageItem>
#include "qthemeimageitem_p.h"
#include <QThemeStatusItem>
#include <QThemeLayoutItem>
#include <QThemeTextItem>
#include <QThemeRectItem>
#include <QThemeGroupItem>
#include <QThemeLevelItem>
#include <QThemeWidgetItem>
#include <QThemeTemplateItem>
#include <QThemeListItem>
#include <QThemePluginItem>
#include <QThemeItemFactory>
#include <QXmlStreamReader>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QExpressionEvaluator>
#include <Qtopia>

#if !defined(THEME_EDITOR)
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#else //THEME_EDITOR
#define SNAP_DISTANCE (8)
#include "themeselectionbox.h"
#include "qthemeitemundo.h"
#include <QXmlStreamWriter>
#include <QUndoStack>
#endif

int QThemeItemPrivate::count = 0;
static const int ThemeItem = 0;

/*!
  \class QThemeItem
    \inpublicgroup QtBaseModule

  \since 4.4

  \brief The QThemeItem class is the base class of all items in a QThemedView.

  QThemeItem encapsulates functionality common to all theme items.
  It manages common attributes such as rect, whether the item is active, visible, layout attributes,
  user interactivity and so on.

  \sa QThemedView
  \ingroup qtopiatheming
*/

/*!
  Constructs a QThemeItem.
  The \a parent is passed to the base class constructor.
*/
QThemeItem::QThemeItem(QThemeItem *parent)
        : QGraphicsItem(parent), d(new QThemeItemPrivate)
{
    setData(ThemeItem, true);
#if defined(THEME_EDITOR)
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    setAcceptDrops(true);
    if (themedScene())
        d->id = themedScene()->nextId();
#endif
}

/*!
  Destroys the QThemeItem.
*/
QThemeItem::~QThemeItem()
{
    if (d->activeExpression) {
        delete d->activeExpression;
        d->activeExpression = 0;
    }
    delete d;
}

/*!
  Returns a pointer to QThemeItem if \a item is a QThemeItem or one of its subclass. Returns 0 otherwise.
*/
QThemeItem *QThemeItem::themeItem(QGraphicsItem *item)
{
    if (item && item->data(ThemeItem).toBool() == true)
        return static_cast<QThemeItem *>(item);
    return 0;
}

/*!
  Returns the item's name.
*/
QString QThemeItem::name() const
{
    return d->name;
}

/*!
  \reimp
*/
QRectF QThemeItem::boundingRect() const
{
    return d->boundingRect;
}

/*!
  Returns the themed view displaying this item.
*/
QThemedView *QThemeItem::themedView() const
{
    return themedScene()->themedView();
}

/*!
  \reimp
*/
void QThemeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setClipping(false);
#ifndef THEME_EDITOR
#else
    if(!themedScene()->visualAids())
        return;

    if (isSelected()) {
        painter->setPen(QPen(QBrush(Qt::white), 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(d->boundingRect.x() - 1, d->boundingRect.y() - 1,
                          d->boundingRect.width() + 2, d->boundingRect.height() + 2));
    } else if (themedScene()->highlighted(this)) {
        painter->setPen(QPen(QBrush(Qt::blue), 0, Qt::DashLine));
        painter->setBrush(QColor(255, 0, 0, 127));
        painter->drawRect(d->boundingRect);
    } else if (themedScene()->showInvisible() && invisibleThemeItem(this)) {
        painter->setPen(QPen(QBrush(Qt::red), 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(d->boundingRect.x() - 1, d->boundingRect.y() - 1,
                          d->boundingRect.width() + 2, d->boundingRect.height() + 2));

    }
#ifdef DEBUG_SNAP
    painter->setPen(QPen(Qt::magenta));
    painter->setBrush(Qt::blue);
    foreach(QRectF rect, d->snapRects){
        painter->drawPolygon(mapFromScene(rect));
    }
    if(d->oldPos != QPointF(-1,-1))
        scene()->update(scene()->sceneRect());
#endif
#endif //!THEME_EDITOR
}

/*!
  Loads the item using the XML \a reader.
*/
void QThemeItem::load(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.isStartElement());

    if (!filter(reader.attributes())) {
        reader.readElementText();
        return;
    }

    loadAttributes(reader);
    loadChildren(reader);
    ++d->count;
    setZValue(d->count);
    constructionComplete();
    d->loaded = true;
}

/*!
  Loads the item's children using the XML \a reader.

  The default implementation loads any kind of item.

  Reimplement this function if you want to load specific children.
*/
void QThemeItem::loadChildren(QXmlStreamReader &reader)
{
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isEndElement())
            break;

        if (reader.isStartElement()) {
            QThemeItem *item = QThemeItemFactory::create(reader.name().toString(), this);
            if (item)
                item->load(reader);
            else {
                qWarning() << "Unknown theme item:" << reader.name().toString();
                reader.readElementText();
            }
        }
    }
}

/*!
  Loads the item's attributes using the XML \a reader.

  Reimplement this function to read attributes specific to this item.
*/
void QThemeItem::loadAttributes(QXmlStreamReader &reader)
{
#ifdef THEME_EDITOR
    //Not Strictly THEME_EDITOR Specific, but this and related functions are
    //ifdefed out because below condition is always false outside the editor
    if (d->loaded)
        reset();
#endif
    d->name = reader.attributes().value("name").toString();
    parseRect(reader.attributes().value("rect"));
    d->transient = (reader.attributes().value("transient").toString() == "yes");
    QString actvdata = reader.attributes().value("active").toString();
    bool isliteral = !isExpression(actvdata);
    if (isliteral) {
        d->isActive = (actvdata != QLatin1String("no"));
        setActive(d->isActive);
    } else {
        d->activeAttribute = strippedExpression(actvdata);
    }
    setValueSpacePath(reader.attributes().value("vspath").toString());
    d->onClickAtts = parseSubAttributes(reader.attributes().value("onclick").toString());
    if (d->onClickAtts.contains("message")) {
        bool ok = false;
        ThemeMessageData data(parseMessage(d->onClickAtts["message"], &ok));
        if (ok)
            d->messages.append(data);
    }
}
#ifdef THEME_EDITOR
/*!
  Saves the items attributes using the XML \a writer.
  Reimplemented in subclasses to save attributes specific to other items.
*/
void QThemeItem::save(QXmlStreamWriter &writer)
{
    writer.writeStartElement(textOfType(this));
    saveAttributes(writer);
    saveChildren(writer);
    writer.writeEndElement();
}

void QThemeItem::saveAttributes(QXmlStreamWriter &writer)
{
    if (!d->name.isEmpty())
        writer.writeAttribute("name", d->name);
    QString rect;
    rect += QString::number(d->sr.x()) + (d->unit[0] == Pixel ? "" : textOfUnit(d->unit[0])) + ",";
    rect += QString::number(d->sr.y()) + (d->unit[1] == Pixel ? "" : textOfUnit(d->unit[1])) + ",";
    rect += QString::number(d->mode == Rect ? d->sr.width() : d->sr.right()) + (d->unit[2] == Pixel ? "" : textOfUnit(d->unit[2])) + (d->mode == Rect ? "x" : ",");
    rect += QString::number(d->mode == Rect ? d->sr.height() : d->sr.bottom()) + (d->unit[3] == Pixel ? "" : textOfUnit(d->unit[3]));
    if (rect != QString("0,0,0x0"))
        writer.writeAttribute("rect", rect);
    if (d->transient)
        writer.writeAttribute("transient", "yes");
    if (d->activeExpression) {
        writer.writeAttribute("active", "expr:" + d->activeExpression->expression());
    } else {
        if (!d->isActive)
            writer.writeAttribute("active", "no");
    }
    if (!d->vsPath.isEmpty())
        writer.writeAttribute("vspath", d->vsPath);
    QString onClick;
    foreach(ThemeMessageData data, d->messages) {
        onClick += "message=" + data.channel + "," + data.msg;
        if (!data.type.isEmpty())
            onClick += "(" + data.type + ")," + data.variant.toString();
        onClick += ';';
    }
    if (!onClick.isEmpty()) {
        onClick.chop(1);//no ';' for the last one
        writer.writeAttribute("onclick", onClick);
    }
}
void QThemeItem::saveChildren(QXmlStreamWriter &writer)
{
    foreach(QGraphicsItem *gItem, childItems()) {
        QThemeItem *item = QThemeItem::themeItem(gItem);
        if (item)
            item->save(writer);
    }
}
#endif

/*!
  \internal
*/
bool QThemeItem::filter(const QXmlStreamAttributes &atts)
{
    Q_UNUSED(atts);
#ifndef THEME_EDITOR
    bool keypad = atts.value(QLatin1String("keypad")) != QLatin1String("no");
    bool touchscreen = atts.value(QLatin1String("touchscreen")) != QLatin1String("no");
    if (!keypad && !Qtopia::mousePreferred())
        return false;
    if (!touchscreen && Qtopia::mousePreferred())
        return false;
#endif
    return true;
}

/*!
  \internal
*/
int QThemeItem::parseAlignment(const QString &attribute)
{
    int align = Qt::AlignLeft | Qt::AlignTop;

    QString val = attribute;
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
  \internal
*/
qreal QThemeItem::resolveUnit(qreal value, qreal bound, Unit unit) const
{
    switch (unit) {
    case Percent:
        return qRound(value * bound / 100.0);
    case Point:
        return qRound(value * themedScene()->themedView()->physicalDpiY() / 72.0);
    default:
        return qRound(value);
    }
}

/*!
  \internal
  Returns the scene.
*/
QThemedScene *QThemeItem::themedScene() const
{
    return static_cast<QThemedScene*>(scene());
}

/*!
  Lays out the item.

  The item calcuates its actual position and size based on the abstract geometry information
  specified for the rect attribute in the themed view XML.
*/
void QThemeItem::layout()
{
    qreal pw = parentItem()->boundingRect().width();
    qreal ph = parentItem()->boundingRect().height();

    qreal c1 = resolveUnit(d->sr.left(), pw, d->unit[0]);
    qreal c2 = resolveUnit(d->sr.top(), ph, d->unit[1]);
    setPos(d->sr.left() < 0 ? pw + c1 : c1, d->sr.top() < 0 ? ph + c2 : c2);

    if (d->mode == Coords) {
        c1 = resolveUnit(d->sr.right(), pw, d->unit[2]);
        c2 = resolveUnit(d->sr.bottom(), ph, d->unit[3]);
        resize((d->sr.right() <= 0 ? pw + c1 : c1) - pos().x(),
               ((d->sr.bottom() <= 0 ? ph + c2 : c2) - pos().y()));
    } else {
        resize((!d->sr.width() ? pw - pos().x() : resolveUnit(d->sr.width(), pw, d->unit[2])),
               (!d->sr.height() ? ph - pos().y() : resolveUnit(d->sr.height(), ph, d->unit[3])));
    }
#ifdef THEME_EDITOR //Only useful in theme editor with it's dynamic changes
    foreach(QGraphicsItem* item, childItems()){
        QThemeItem *child = qthemeitem_cast<QThemeItem*>(item);
        if(child){
            child->layout();
        }
    }
#endif
    bool rtl = (themedView()->layoutDirection() == Qt::RightToLeft);
    if ( rtl && boundingRect().width() < parentItem()->boundingRect().width() ) {
        QPointF p = pos();
        p.setX(parentItem()->boundingRect().width() - boundingRect().width() - p.x());
        setPos(p);
    }
}

/*!
  Sets the size of the item to \a width and \a height.
*/
void QThemeItem::resize(qreal width, qreal height)
{
    // Do not resize if not necessary.
    if (QSizeF(width, height) == boundingRect().size())
        return;

    prepareGeometryChange();
    d->boundingRect.setWidth(width);
    d->boundingRect.setHeight(height);
}

/*!
  \internal
*/
void QThemeItem::parseRect(const QStringRef &string)
{
    if (!string.isEmpty()) {
        const QChar percent('%');
        const QString pt("pt");
        qreal coords[4];
        QStringList args = string.toString().split(',');

        if (args.count() == 4) {
            d->mode = QThemeItem::Coords;

            for (int i = 0; i < 4; i++) {
                if (args[i].contains(percent)) {
                    d->unit[i] = QThemeItem::Percent;
                    coords[i] = args[i].remove(percent).toFloat();
                } else if (args[i].contains(pt)) {
                    d->unit[i] = QThemeItem::Point;
                    coords[i] = args[i].remove(pt).toFloat();
                } else {
                    d->unit[i] = QThemeItem::Pixel;
                    coords[i] = args[i].toFloat();
                }
            }
            d->sr.setCoords(coords[0], coords[1], coords[2], coords[3]);

        } else if (args.count() == 3 && args[2].contains('x')) {
            d->mode = QThemeItem::Rect;
            QString size = args[2];
            args.removeLast();
            args += size.split('x');

            for (int i = 0; i < 4; i++) {
                if (args[i].contains(percent)) {
                    d->unit[i] = QThemeItem::Percent;
                    coords[i] = args[i].remove(percent).toFloat();
                } else if (args[i].contains(pt)) {
                    d->unit[i] = QThemeItem::Point;
                    coords[i] = args[i].remove(pt).toFloat();
                } else {
                    d->unit[i] = QThemeItem::Pixel;
                    coords[i] = args[i].toFloat();
                }
            }
            d->sr.setRect(coords[0], coords[1], coords[2], coords[3]);
        }

    } else {
        d->sr.setRect(0, 0, 0, 0);
        d->mode = QThemeItem::Rect;
        for (int i = 0; i < 4; i++)
            d->unit[i] = QThemeItem::Pixel;
    }
}

/*!
  \reimp
  Returns the type of an item as an int. All standard graphicsitem classes
  are associated with a unique value; see QThemeItem::Type.

  Reimplementing this function and declaring a Type enum value equal to your
  custom item's type will enable use of qgraphicsitem_cast() with the item.

  Custom items must return a value larger than or equal to UserType (65536).
*/
int QThemeItem::type() const
{
    return Type;
}

/*!
  \internal
*/
quint64 QThemeItem::types() const
{
    return d->types;
}

/*!
  When subclassing QThemeItem, call this method in the constructor.
  It is required by qthemeitem_cast() to return the correct \a type.
*/
void QThemeItem::registerType(int type)
{
    if(type > ThemeItemType && type <= ThemeItemMaxUserType)
        d->types |= (1 << (type - ThemeItemType - 1));
}

/*!
  Returns true if \a string is an expression, false otherwise.
*/
bool QThemeItem::isExpression(const QString &string)
{
    QString str = string.trimmed();
    if (str.startsWith("expr:"))
        return true;
    return false;
}

/*!
  Strips the expression indicators from the expression \a s.

  isExpression(s) must return true, or this function will abort.
*/
QString QThemeItem::strippedExpression(const QString &s)
{
    Q_ASSERT(isExpression(s));
    QString str = s.trimmed();
    str = str.remove(0, QString("expr:").length());
    return str;
}

/*!
   Called by ThemedView for a QExpressionEvaluator instance when it emits the Expression::termsChanged() signal.
   \a expression is an expression registered through a call to QThemedView::registerExpression().

   The Expression \a expression will be an expression that belongs to this item.

   Subclasses should make sure they call the base class version of this function should they not know about \a expression.
*/
void QThemeItem::expressionChanged(QExpressionEvaluator *expression)
{
#ifndef THEME_EDITOR //Constantly resetting in theme editor makes it not worth it
    Q_ASSERT(d->activeExpression == expression);
#else
    //But it usually means something is wrong.
    if(d->activeExpression != expression){
        qWarning() << "ThemeItem::expressionChange() - expression is not active expression.";
        return;
    }
#endif
    QVariant result = getExpressionResult(expression, QVariant::Bool);

    if (!result.canConvert(QVariant::Bool)) {
        qWarning() << "ThemeItem::expressionChanged() - Cannot convert value to Bool";
    } else {
        setActive(result.toBool());
    }
}

/*!
  Called when the item has finished being constructed from the themed view XML.

  Reimplement to perform any required calculations on the item's data with the knowledge that the
  data is complete.

  Subclasses should always call the base class version of this function so that proper construction occurs at all levels.
*/
void QThemeItem::constructionComplete()
{
    Q_ASSERT(d->activeExpression == 0);
    QString expr = d->activeAttribute;
    if (!expr.isEmpty()) {
        d->activeExpression = createExpression(expr);
        if (d->activeExpression != 0)
            QThemeItem::expressionChanged(d->activeExpression);
        d->activeAttribute = QString();
    }
}

/*!
  \internal
*/
QExpressionEvaluator *QThemeItem::createExpression(const QString &data)
{
    QExpressionEvaluator* expr = 0;
    if (!data.isEmpty()) {
        // prepend this item's fullValueSpacePath to any vs terms in expression
        QString expandedData = data;
        expandedData.replace(QRegExp("'"), "\"");
        QString fp = fullValueSpacePath();

        if (!fp.isEmpty() && fp != "/") {
            expandedData = QString(data).replace("@", "@" + fp);
            //qWarning() << "ThemeItem::createExpression: expanded:" << expandedData << " orig:" << data;
        }

        expr = new QExpressionEvaluator(expandedData.toUtf8());

        if (expr->isValid())
            themedScene()->registerExpression(this, expr);
        else {
            qWarning("ThemeItem(name='%s')::createExpression(): Invalid expression given", d->name.toAscii().data());
            delete expr;
            expr = 0;
        }
    }
    return expr;
}

/*!
  \internal
*/
QVariant QThemeItem::getExpressionResult(QExpressionEvaluator *expression, const QVariant::Type &type)
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
  Returns true if the item is active. Returns false otherwise.
*/
bool QThemeItem::isActive() const
{
    return d->isActive;
}

/*!
  \internal

  Parses sub-attributes into key => value pairs from the attribute value \a subatts specified in the themed view XML.
  Sub attributes are attributes specified within themed view XML attributes.  For example if \a subatts contained
  "active=no;transient=yes", then this function would return a map containing "active" => "no", "transient" => "yes".
  */
QMap<QString, QString> QThemeItem::parseSubAttributes(const QString &subatts) const
{
    QMap<QString, QString> subAtts;
    if (subatts.trimmed().isEmpty())
        return subAtts;
    QStringList assignments = subatts.split(";");
    for (int i = 0 ; i < assignments.count() ; ++i) {
        QStringList assignment = assignments[i].split("=");
        if (assignment.count() == 2)
            subAtts.insert(assignment[0], assignment[1]);
    }
    return subAtts;
}

/*!
  \internal
*/
ThemeMessageData QThemeItem::parseMessage(const QString &message, bool *ok)
{
    //channel,msg([param])[,param1]
    if (ok)
        *ok = true;
    ThemeMessageData data;
    QStringList tokens = message.split(QRegExp("[,()]"), QString::SkipEmptyParts);
    if (tokens.count() != 2 && tokens.count() != 4) {
        *ok = false;
        qWarning("Error parsing message for theme element with name '%s'", name().toAscii().constData());
        return ThemeMessageData();
    }

    data.channel = tokens[0]; // could be a service or channel, resolved at send time

    data.msg = tokens[1];

    if (tokens.count() == 4) {
        // verify that the passed parameter type is the same type as
        data.type = tokens[2];
        if (tokens[3].length() == 1) {   // special case, passing a single character, store its unicode value so conversion to char is valid
            data.variant.setValue(tokens[3].at(0));
        } else {
            data.variant.setValue(tokens[3]);
        }
        QVariant::Type t = QVariant::nameToType(data.type.toAscii().constData());
        if (!data.variant.canConvert(t)) {
            qWarning("Error parsing message for theme element with name '%s' - Cannot convert parameter '%s' with data type '%s' to data type '%s'", name().toAscii().constData(), tokens[3].toAscii().constData(), data.variant.typeName(), QVariant::typeToName(t));
            *ok = false;
            return ThemeMessageData();
        }
        bool ok = data.variant.convert(t);
        Q_ASSERT(ok == true);
        Q_UNUSED(ok);
    }
    return data;
}

/*!
  Returns the current state of the item.
*/
QString QThemeItem::state() const
{
    return d->state;
}

/*!
  Sets the current \a state of the item.
  By default, an item is in the state 'default'.
*/
void QThemeItem::setState(const QString &state)
{
    d->state = state;
    update();
}

/*!
  \internal
*/
void QThemeItem::pressed(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    setState("onclick");
    themedScene()->themedView()->itemMousePressed(this);
#if defined (THEME_EDITOR)
    if (event->button() != Qt::LeftButton)
        return;

    themedScene()->itemClicked(this);
    themedScene()->makeHandles(this);
    d->mouseDelta = event->scenePos() - scenePos();//position vector of the mouse to the 0,0 of the object
    //Due to problems with synchronizing with the graphics view coordinates, all coordinates are based off of
    //the mouse event's position
    d->oldPos = event->scenePos() - d->mouseDelta;
    d->snapDelta = QPointF(0, 0);
    d->handleMove = false;
#endif
}

/*!
  \reimp
*/
void QThemeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef THEME_EDITOR
    if (invisibleThemeItem(this) && !themedScene()->showInvisible()){
        event->ignore();
        return;
    }
#endif
    if (QThemeGroupItem *group = parentGroup()) {
        group->mousePressEvent(event);
    } else {
        pressed(event);
    }
}

/*!
  \reimp
*/
void QThemeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
#if defined(THEME_EDITOR)
    aboutToChange(8);//Note that after the first move they will be ignored
    setPos(mapToParent(mapFromScene(event->scenePos() - d->mouseDelta)));
    if (d->oldPos != QPointF(-1, -1) && d->oldPos != event->scenePos() - d->mouseDelta) {
        d->snapDelta = QPointF(0, 0);
        if (!(Qt::ControlModifier & event->modifiers())) {//Snap move
            d->snapRects.clear();
            d->snapRects << QRectF(sceneBoundingRect().x() - SNAP_DISTANCE,
                    sceneBoundingRect().y() - SNAP_DISTANCE,
                    2*SNAP_DISTANCE+sceneBoundingRect().width(), 2*SNAP_DISTANCE);
            d->snapRects << QRectF(sceneBoundingRect().x() - SNAP_DISTANCE + sceneBoundingRect().width(),
                    sceneBoundingRect().y() - SNAP_DISTANCE,
                    2*SNAP_DISTANCE, 2*SNAP_DISTANCE + sceneBoundingRect().height());
            d->snapRects << QRectF(sceneBoundingRect().x() - SNAP_DISTANCE,
                    sceneBoundingRect().y() - SNAP_DISTANCE,
                    2*SNAP_DISTANCE, 2*SNAP_DISTANCE+sceneBoundingRect().height());
            d->snapRects << QRectF(sceneBoundingRect().x() - SNAP_DISTANCE,
                    sceneBoundingRect().y() - SNAP_DISTANCE + sceneBoundingRect().height(),
                    2*SNAP_DISTANCE + sceneBoundingRect().width(), 2*SNAP_DISTANCE);
            QGraphicsItem *snapItem = 0; //Only snap to one item

            foreach(QRectF snapRect, d->snapRects) {
                QList<QGraphicsItem*> items;
                if(snapItem){
                    items << snapItem;
                }else{
                    foreach(QGraphicsItem* item, themedScene()->items(snapRect,
                                Qt::IntersectsItemBoundingRect)){
                        QThemeItem *tItem = qthemeitem_cast<QThemeItem*>(item);
                        if(tItem && !invisibleThemeItem(tItem)){
                            if(themedScene()->highlighted(tItem)){
                                items.prepend(item);//Give priority to highlighted item
                            } else {
                                items << item;
                            }
                        }
                    }
                }

                foreach(QGraphicsItem* item, items) {
                    if (item == this || item->sceneBoundingRect().contains(snapRect)
                            || qgraphicsitem_cast<ThemeSelectionBox*>(item))
                        continue;//Not an Edge
                    qreal snapLeft = snapRect.x();
                    qreal snapRight = snapRect.x() + snapRect.width();
                    qreal snapTop = snapRect.y();
                    qreal snapBottom = snapRect.y() + snapRect.height();
                    qreal itemLeft = item->sceneBoundingRect().x();
                    qreal itemRight = item->sceneBoundingRect().x()
                                      + item->sceneBoundingRect().width();
                    qreal itemTop = item->sceneBoundingRect().y();
                    qreal itemBottom = item->sceneBoundingRect().y()
                                       + item->sceneBoundingRect().height();

                    //snap rect for left/right
                    if(snapRect.width() == 2*SNAP_DISTANCE && !d->snapDelta.x()){
                        if (snapLeft < itemLeft && snapRight > itemLeft) {
                            d->snapDelta.rx() = -((snapLeft + SNAP_DISTANCE) - itemLeft);
                        } else if (snapLeft < itemRight && snapRight > itemRight) {
                            d->snapDelta.rx() = -((snapLeft + SNAP_DISTANCE) - itemRight);
                        }
                        Q_ASSERT(qAbs(d->snapDelta.x()) <= SNAP_DISTANCE);
                    }

                    if (snapRect.height() == 2*SNAP_DISTANCE && !d->snapDelta.x()){
                        if (snapTop < itemTop && snapBottom > itemTop) {
                            d->snapDelta.ry() = -((snapTop + SNAP_DISTANCE) - itemTop);
                        } else if (snapTop < itemBottom && snapBottom > itemBottom) {
                            d->snapDelta.ry() = -((snapTop + SNAP_DISTANCE) - itemBottom);
                        }
                        Q_ASSERT(qAbs(d->snapDelta.y()) <= SNAP_DISTANCE);
                    }
                    break;
                }
            }
        }
        setPos(mapToParent(mapFromScene(event->scenePos() - d->mouseDelta + d->snapDelta)));
        bool flag = false;
        foreach(QGraphicsItem* item, themedScene()->items(scenePos())) {
            if (item != this && QThemeItem::themeItem(item) && !isAncestorOf(item)) {
                themedScene()->highlight(item);
                flag = true;
                break;
            }
        }
        if (!flag)
            themedScene()->highlight(0);

        if (!d->handleMove)
            themedScene()->clearHandles(this);
        if (d->editor)
            d->editor->refresh();

    }
#endif
}

/*!
  \reimp
*/
void QThemeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (QThemeGroupItem *group = parentGroup()) {
        group->mouseReleaseEvent(event);
    } else {
        QThemeItem::released(event);
    }
}

/*!
  \internal
*/
void QThemeItem::released(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
#if !defined(THEME_EDITOR)
    for (int i = 0 ; i < d->messages.count() ; ++i) {
        QString msg;
        msg = d->messages[i].msg + "(";
        if (!d->messages[i].type.isEmpty())
            msg += d->messages[i].type;
        msg += ")";

        // look for a service with this name, and get the channel from that if it exists
        QtopiaService service;
        bool isService = service.channel(d->messages[i].channel).length() != 0;
        if (isService) {
            QString channel = d->messages[i].channel;
            QtopiaServiceRequest req(d->messages[i].channel, msg);
            if (!d->messages[i].type.isEmpty())
                req.addVariantArg(d->messages[i].variant); // QtopiaServiceRequest will serialize the variant's value
            req.send();
        } else {
            QtopiaIpcEnvelope e(d->messages[i].channel, msg);
            if (!d->messages[i].type.isEmpty())
                QMetaType::save(e, d->messages[i].variant.type(), d->messages[i].variant.data());
        }
    }
    themedScene()->themedView()->itemMouseReleased(this);
    setState("default");
#else //ifdef THEME_EDITOR ; In the theme editor it needs to possibly reparent after a move is complete
    themedScene()->highlight(0);
    QPointF newScenePos = event->scenePos() - d->mouseDelta + d->snapDelta;
    if (d->oldPos != QPointF(-1, -1) && d->oldPos != newScenePos && themedScene()->items(scenePos()).size() > 1) {
        //Move and Reparent to the Item directly below the item
        themedScene()->changePending(this);
        foreach(QGraphicsItem* item, themedScene()->items(newScenePos)) {
            if (item != this && !isAncestorOf(item)) {
                if (item != parentItem())
                    setParentItem(item);
                break;
            }
        }
        QPointF newPos = mapToParent(mapFromScene(newScenePos));
        d->sr.moveLeft(convertUnit(newPos.x(), parentItem() ?
                                   parentItem()->boundingRect().width() : 0, Pixel, d->unit[0]));
        d->sr.moveTop(convertUnit(newPos.y(), parentItem() ?
                                  parentItem()->boundingRect().height() : 0, Pixel, d->unit[1]));
        if (d->editor)
            d->editor->refresh();
        themedScene()->changed(this);
        layout();
        update();
        themedScene()->invalidate(themedScene()->sceneRect());
        themedScene()->makeHandles(this);
        themedScene()->treeChanged();
        finishedChange(8);
    }
    d->oldPos = QPointF(-1, -1);
#endif
}

/*!
  Sets this item to be active if \a active is true, otherwise sets the item to be inactive.
*/
void QThemeItem::setActive(bool active)
{
    if (active != d->isActive) {
        d->isActive = active;
        setVisible(active);
        themedScene()->layout();
        // update me or my parent, position controlling layout?
        QThemeItem* item = this;
        QThemeItem *plhelp = 0;
        while ((plhelp = item->parentLayout()))
            item = plhelp;
        item->update();
    }
}

/*!
  \internal
*/
QThemeLayoutItem* QThemeItem::parentLayout() const
{
    QGraphicsItem *parent = parentItem();
    while (parent != 0) {
        if (QThemeLayoutItem *l = qgraphicsitem_cast<QThemeLayoutItem*>(parent))
            return l;
        parent = parent->parentItem();
    }
    return 0;
}

/*!
  \internal
*/
QThemeGroupItem* QThemeItem::parentGroup() const
{
    QGraphicsItem *parent = parentItem();
    while (parent != 0) {
        if (QThemeGroupItem *g = qgraphicsitem_cast<QThemeGroupItem*>(parent))
            return g;
        parent = parent->parentItem();
    }
    return 0;
}

/*!
  \internal
*/
QColor QThemeItem::colorFromString(const QString s) const
{
    if (s.startsWith(QChar('#'))) {
        return QColor(s);
    }

    int i = 0;
    while (colorTable[i].name) {
        if (QString(colorTable[i].name).toLower() == s.toLower()) {
            return themedScene()->palette().color(colorTable[i].role);
        }
        i++;
    }
    return QColor();
}

/*!
  Sets the valuespace \a path of this item.
*/
void QThemeItem::setValueSpacePath(const QString &path)
{
    d->vsPath = path.trimmed();
}

/*!
  Returns the valuespace path of the item.
*/
QString QThemeItem::valueSpacePath() const
{
    return d->vsPath;
}

/*!
  Returns the full vspath for this item.
  The full vspath is this item's vspath appended to the parent item's full vspath.
  A trailing slash is always included.
 */
QString QThemeItem::fullValueSpacePath() const
{
    QString parentPath;
    QString path = valueSpacePath();
    QThemeItem *parent = QThemeItem::themeItem(parentItem());
    if (parent != 0) {
        parentPath = parent->fullValueSpacePath();
    }
    if (!parentPath.endsWith("/"))
        parentPath += "/"; // parent path always ends in a /, even if it is empty

    if (path.startsWith("/"))
        path = path.remove(0, 1); // so remove a leading / in our path

    if (!path.isEmpty() && !path.endsWith("/"))
        path += "/"; // and add one incase we are the deepest caller

    return parentPath + path;
}

/*!
  \internal
*/
QThemeItem *QThemeItem::itemFromType(const QString & type)
{
    QThemeItem *item = 0;
    if (type == "text") {
        item = new QThemeTextItem(this);
    } else if (type == "exclusive") {
        item = new QThemeExclusiveItem(this);
    } else if (type == "image") {
        item = new QThemeImageItem(this);
    } else if (type == "status") {
        item = new QThemeStatusItem(this);
    } else if (type == "level") {
        item = new QThemeLevelItem(this);
    } else if (type == "layout") {
        item = new QThemeLayoutItem(this);
    } else if (type == "rect") {
        item = new QThemeRectItem(this);
    } else if (type == "group") {
        item = new QThemeGroupItem(this);
    } else if (type == "widget") {
        item = new QThemeWidgetItem(this);
    } else if (type == "template") {
        item = new QThemeTemplateItem(this);
    } else if (type == "list") {
        item = new QThemeListItem(this);
    } else if (type == "plugin") {
        item = new QThemePluginItem(this);
    } else {
        item = new QThemeItem(this);
    }
    return item;
}

#ifdef THEME_EDITOR
bool QThemeItem::invisibleThemeItem(QThemeItem *item)
{
    QThemeImageItem *img = qthemeitem_cast<QThemeImageItem*>(item);
    if(img){
        if(img->d->source.value(img->state()).toString().isEmpty())
            return true;
        return false;
    }
    QString typeStr = QString(textOfType(item));
    return (typeStr == "layout" || typeStr == "group" || typeStr=="exclusive");
}

QWidget *QThemeItem::editWidget()
{
    d->editor = new QThemeItemEditor(this);
    return d->editor;
}

qreal QThemeItem::convertUnit(qreal value, qreal pixelBound, Unit from, Unit to) const
{
    if (from == to)
        return value;
    qreal ret;
    value = resolveUnit(value, pixelBound, from);
    switch (to) {
    case Percent:
        ret = value / (pixelBound ? pixelBound : 320) * 100;
        break;
    case Point:
        ret =  value / themedScene()->themedView()->physicalDpiY() * 72.0;
        break;
    default:
        ret = value;
        break;
    }
    return ret;
}

void QThemeItem::normalize()
{
    if (d->mode == Coords) {
        d->mode = Rect;
        qreal pw = parentItem() ? parentItem()->boundingRect().width() : 240;
        qreal ph = parentItem() ? parentItem()->boundingRect().height() : 320;
        qreal c1 = resolveUnit(d->sr.right(), pw, d->unit[2]);
        qreal c2 = resolveUnit(d->sr.bottom(), ph, d->unit[3]);
        d->sr.setWidth(convertUnit((d->sr.right() <= 0 ? pw + c1 : c1) - pos().x(), pw, Pixel, d->unit[2]));
        d->sr.setHeight(convertUnit((d->sr.bottom() <= 0 ? ph + c2 : c2) - pos().y(), ph, Pixel, d->unit[3]));
    }
}

QString QThemeItem::textOfUnit(QThemeItem::Unit unit)
{
    switch (unit) {
    case(QThemeItem::Percent): return "%";
    case(QThemeItem::Point): return "pt";
    case(QThemeItem::Pixel): return "px";
    default:
        return QString();
    }
    return QString();
}

QString QThemeItem::textOfType(const QThemeItem *item)
{
    if(!item)
        return QString("NULL");
    //Switch to qthemeitem_cast?
    switch (item->type() - ThemeItemType) {
    case 1:
        return QString("item");
        break;
    case 2:
        return QString("page");
        break;
    case 3:
        return QString("text");
        break;
    case 4:
        return QString("exclusive");
        break;
    case 5:
        return QString("layout");
        break;
    case 6:
        return QString("image");
        break;
    case 7:
        return QString("status");
        break;
    case 8:
        return QString("rect");
        break;
    case 9:
        return QString("template");
        break;
    case 10:
        return QString("widget");
        break;
    case 11:
        return QString("list");
        break;
    case 13:
        return QString("level");
        break;
    case 14:
        return QString("group");
        break;
    case 15:
        return QString("plugin");
        break;
    default:
        return QString("ERROR");
        break;//for instance TemplateInstanceItem(12), shouldn't be saved;
    }
}



/*!
    \internal
    aboutToChange locks in the id, and all other change
    requests are ignored until that id is used to finish
    the change. Note that -1 is an id shared by many changes
    and is also the 'unset' value.

    This is used to get around the problem that some things
    can change from the user or from something else, for example
    moving changes the X Spinner, and with this locking a drag
    move will be one command, as opposed to one plus the hundred
    or so spinner changes.
*/
void QThemeItem::aboutToChange(int id)
{
    if (d->changeId != -1)
        return;

    d->beforeChange = QString();
    d->beforeParent = parentItem();
    QXmlStreamWriter write(&(d->beforeChange));
    write.writeStartElement(textOfType(this));
    saveAttributes(write);
    write.writeEndElement();
    d->changeId = id;
}

bool QThemeItem::finishedChange(int id)
{

    if (d->beforeChange.isNull()) {
        return false;
    }

    if (d->changeId != id)
        return false;

    QString afterChange;
    QXmlStreamWriter write(&afterChange);
    write.writeStartElement(textOfType(this));
    saveAttributes(write);
    write.writeEndElement();

    if (d->beforeChange != afterChange || d->beforeParent != parentItem()) {
        QThemeItemUndo *undo = new QThemeItemUndo(this, d->beforeChange, afterChange,
                ((id == -1) ? -1 : d->id + id), d->beforeParent, parentItem());
        //Note: Must reset here in case the redoing causes more 'commands' to be generated
        //If they are the same thing in a different place (AND it is reset) they'll be dropped as duplicates
        d->beforeChange = QString();
        d->changeId = -1;
        themedScene()->themedView()->undoStack()->push(undo);
    }
    d->beforeChange = QString();
    d->changeId = -1;
    return true;
}

void QThemeItem::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!event->mimeData()->hasFormat("application/x-theme-editor-internal"))
        return;
    themedScene()->highlight(0);
}

void QThemeItem::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!event->mimeData()->hasFormat("application/x-theme-editor-internal")) {
        event->ignore();
        return;
    }
    themedScene()->highlight(this);
    event->accept();
}

void QThemeItem::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!event->mimeData()->hasFormat("application/x-theme-editor-internal"))
        return;
    QString xmlString = event->mimeData()->data("application/x-theme-editor-internal");
    QXmlStreamReader reader(xmlString);
    reader.readNext();
    reader.readNext();//To get to element
    QThemeItem *droppedItem = itemFromType(reader.name().toString());
    droppedItem->setParentItem(this);
    droppedItem->load(reader);
    themedScene()->highlight(0);
    themedScene()->undeleteItem(droppedItem);
    //droppedItem->layout();
    //droppedItem->update();
    //themedScene()->treeChanged();
    //themedScene()->itemClicked(droppedItem);
    //themedScene()->makeHandles(droppedItem);
    //Above things should happen as a side effect of undeleting (due to the redo)
}

/*
    To be called before attempting to alter the item by calling
    load attributes on an already loaded item.
*/
void QThemeItem::reset()
{
    if (d->activeExpression) {
        delete d->activeExpression;
        d->activeExpression = 0;
    }
    d->isActive = true;
    setVisible(true);
    d->activeAttribute=QString();
    d->transient = false;
}

/*
    Recursively lays out this item and its QThemeItem children
*/
void QThemeItem::relayout()
{
    layout();
    if (textOfType(this) == QString("layout"));
    return;//Do Layouts inside layouts need re-laying-out?
    foreach(QGraphicsItem* child, children()) {
        QThemeItem *item = QThemeItem::themeItem(child);
        if (item)
            item->relayout();
    }
    update(boundingRect());
}
#endif

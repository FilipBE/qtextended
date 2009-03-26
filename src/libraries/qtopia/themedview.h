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

#ifndef THEMEDVIEW_H
#define THEMEDVIEW_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qmap.h>
#include <QAbstractListModel>
#include <QItemDelegate>
#include <QListView>

#include <qtopiaglobal.h>
#include <qexpressionevaluator.h>
#include <qvaluespace.h>

class ThemedView;
class ThemedViewPrivate;
class ThemedItemPlugin;
class ThemeAnimationItemPrivate;
class QPluginManager;
class QPainter;
class QXmlAttributes;
class QAbstractTextDocumentLayout;
class ThemeAttributes;
class ThemeMessageData;

/*
   THEME ITEM BASE CLASS
 */
struct ThemeItemPrivate;
class QTOPIA_EXPORT ThemeItem
{
    friend class ThemeTemplateInstanceItem;
    friend class ThemeFactory;
    friend struct ThemeItemPrivate;
public:
    enum State
    {
        Default = 1,
        Focus = 2,
        Pressed = 6, // Pressed | Focus
        All = 7 // Default | Pressed | Focus
    };

    ThemeItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeItem();

    virtual void setActive(bool active);
    void setVisible(bool visible);
    bool active() const;
    bool transient() const;

    QRect rect() const;
    QRect geometry() const;
    QRectF geometryHint()const;
    virtual void setGeometry(const QRect &rect);
    bool isVisible() const;

    virtual int rtti() const;

    QString itemName() const;

    bool isInteractive() const;
    bool pressed() const;
    bool hasFocus() const;

    virtual void setPressed(bool pressed);
    virtual void setFocus(bool focus);

    ThemeItem::State state() const;
    ThemeItem *parentItem() const;
    ThemedView* view() const;

    QString vsPath();
    QString fullVSPath();

    QList<ThemeItem*> children() const;

protected:
    virtual void stateChanged(const ThemeItem::State &state);
    void removeChild(ThemeItem *item);
    void addChild(ThemeItem *item);

    virtual void clickedEvent();

    bool isStringExpression(const QString &str);
    QString stripStringExpression(const QString &str);

    virtual void expressionChanged(QExpressionEvaluator *);
    virtual void constructionComplete();

    QExpressionEvaluator *createExpression(const QString &data);
    QVariant getExpressionResult(QExpressionEvaluator *expression, const QVariant::Type &type);

    void setAttribute(const QString &key, const int &val, ThemeItem::State st = ThemeItem::Default);
    void setAttribute(const QLatin1String &key, const int &val, ThemeItem::State st = ThemeItem::Default);
    int attribute(const QString &key, ThemeItem::State st = ThemeItem::Default) const;
    int attribute(const QLatin1String &key, ThemeItem::State st = ThemeItem::Default) const;
    void setAttribute(const QString &key, const QString &val, ThemeItem::State st = ThemeItem::Default);
    void setAttribute(const QLatin1String &key, const QString &val, ThemeItem::State st = ThemeItem::Default);
    QString strAttribute(const QString &key, ThemeItem::State st = ThemeItem::Default);
    QString strAttribute(const QLatin1String &key, ThemeItem::State st = ThemeItem::Default);

    virtual void paint(QPainter *painter, const QRect &rect);
    virtual void layout();

    /*!
      \internal
    */
    enum RMode { Rect, Coords };

    /*!
      \internal
    */
    enum Unit { Pixel, Percent, Point };

    void update();
    void update(int x, int y, int w, int h);
    QMap<QString,QString> parseSubAtts(const QString &subatts) const;
    int resolveUnit(qreal, int, Unit) const;
    int parseAlignment(const ThemeAttributes &atts, const QString &name = QString());

    virtual void addCharacters(const QString &characters);


protected:
    void setDataExpression(bool b);
    bool isDataExpression() const;

private:
    QRectF parseRect(const ThemeAttributes &atts, RMode &rmode, const QString &name = QString());
    ThemeItem* parentLayout() const;
    ThemeMessageData parseMessage(const QString &message, bool *ok);
    void paletteChange(const QPalette &palette);
    void setVSPath(const QString &path);
    ThemeItemPrivate *d;
    friend class ThemedView;
    friend class ThemedViewPrivate;
    friend class ThemeExclusiveItem;
    friend class ThemeLayoutItem;
    friend class ThemePageItem;
    friend class ThemeWidgetItem;
    friend class ThemeGroupItem;
};

/*
   NON-VISUAL THEME ITEMS
*/

struct ThemePageItemPrivate;
class QTOPIA_EXPORT ThemePageItem : public ThemeItem
{
public:
    ThemePageItem(ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemePageItem();

    const QString base() const;

    int rtti() const;

    QSize sizeHint() const;

protected:
    virtual void paint(QPainter *painter, const QRect &rect);
    virtual void layout();

private:
    QRectF calcPageGeometry(const QSize &defSize) const;
    void applyMask();

private:
    ThemePageItemPrivate *d;
};

class QTOPIA_EXPORT ThemeGroupItem : public ThemeItem
{
public:
    ThemeGroupItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);

    int rtti() const;

protected:
    virtual void setPressed(bool pressed);
};


struct ThemePluginItemPrivate;
class QTOPIA_EXPORT ThemePluginItem : public ThemeItem
{
public:
    ThemePluginItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    ~ThemePluginItem();

    void setPlugin(const QString &plugin);
    void setBuiltin(ThemedItemPlugin *);

    int rtti() const;
    virtual void paint(QPainter *painter, const QRect &rect);

protected:
    virtual void layout();

private:
    void releasePlugin();

private:
    ThemePluginItemPrivate *d;
};


struct ThemeLayoutItemPrivate;
class QTOPIA_EXPORT ThemeLayoutItem : public ThemeItem
{
public:
    ThemeLayoutItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeLayoutItem();

    int rtti() const;

protected:
    virtual void layout();
    virtual void paint(QPainter *painter, const QRect &rect);

private:
    ThemeLayoutItemPrivate* d;
};

struct ThemeTemplateItemPrivate;
class ThemeTemplateInstanceItem;
class QTOPIA_EXPORT ThemeTemplateItem : public ThemeItem
{
    friend class ThemeFactory;

public:
    ThemeTemplateItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeTemplateItem();

    virtual int rtti() const;

    ThemeTemplateInstanceItem* createInstance(const QString &uid);

private:
    void setData(const QString &data);
    ThemeTemplateItemPrivate* d;
};

class QTOPIA_EXPORT ThemeTemplateInstanceItem : public ThemeTemplateItem
{
public:
    ThemeTemplateInstanceItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeTemplateInstanceItem();
    virtual int rtti() const;
};


class QTOPIA_EXPORT ThemeExclusiveItem : public ThemeItem
{
public:
    ThemeExclusiveItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);

    int rtti() const;

protected:
    virtual void layout();
};

/* GRAPHICAL THEME ITEMS */
struct ThemeGraphicItemPrivate;
class QTOPIA_EXPORT ThemeGraphicItem : public ThemeItem
{
public:
    ThemeGraphicItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeGraphicItem();

protected:
    void setColor( const QString &key, const QColor &val, ThemeItem::State st = ThemeItem::Default );
    void setColor( const QLatin1String &key, const QColor &val, ThemeItem::State st = ThemeItem::Default );
    QColor color( const QString &key, ThemeItem::State st = ThemeItem::Default ) const;
    QColor color( const QLatin1String &key, ThemeItem::State st = ThemeItem::Default ) const;
    QFont font( const QString &key, ThemeItem::State st = ThemeItem::Default ) const;
    QFont font( const QLatin1String &key, ThemeItem::State st = ThemeItem::Default ) const;
    void setFont( const QString &key, const QFont &val, ThemeItem::State st = ThemeItem::Default );
    void setFont( const QLatin1String &key, const QFont &val, ThemeItem::State st = ThemeItem::Default );
    void setupColor( const QString &key, const QString &col, ThemeItem::State st = ThemeItem::Default );
    void setupColor( const QLatin1String &key, const QLatin1String& roleKey, const QString &col, ThemeItem::State st = ThemeItem::Default );
    void setupAlpha( const QString &key, const QString &al, ThemeItem::State st = ThemeItem::Default );
    void setupAlpha( const QLatin1String &key, const QString &al, ThemeItem::State st = ThemeItem::Default );

    QFont parseFont(const QFont &defFont, const QString &size, const QString &bold );
    int parseColor(const QString &val, QColor &col);
    QColor getColor(const QColor &col, int role) const;
private:
    ThemeGraphicItemPrivate* d;
};

struct ThemeTextItemPrivate;
class QTOPIA_EXPORT ThemeTextItem : public ThemeGraphicItem
{
    friend class ThemeFactory;

public:
    ThemeTextItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    ~ThemeTextItem();

    void setText(const QString &text);
    QString text() const;
    bool shortLabel() const;
    void setTextFormat(Qt::TextFormat format);
    Qt::TextFormat textFormat() const;
    int rtti() const;

protected:
    void constructionComplete();
    void expressionChanged(QExpressionEvaluator *expression);
    void setupFont(const QFont &deffont, const QString &size, const QString &bold, const QString &color,
                   const QString &outline, ThemeItem::State state = ThemeItem::Default);
    virtual void paint(QPainter *painter, const QRect &rect);
    void addCharacters(const QString &characters);

private:
    void setupThemeText();
    void drawOutline(QPainter *painter, const QRect &rect, int flags, const QString &text);
    void drawOutline(QPainter *painter, const QRect &rect,
                     const QPalette &palette, QAbstractTextDocumentLayout *rt);
    ThemeTextItemPrivate *d;
};

class QTOPIA_EXPORT ThemeRectItem : public ThemeGraphicItem
{
public:
    ThemeRectItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);

    int rtti() const;

    QColor brushColor( ThemeItem::State st = ThemeItem::Default ) const;

protected:
    virtual void paint(QPainter *painter, const QRect &rect);
};

class QTOPIA_EXPORT ThemeLineItem : public ThemeGraphicItem
{
public:
    ThemeLineItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);

    int rtti() const;

protected:
    virtual void paint(QPainter *painter, const QRect &rect);
};

/* PIXMAP THEME ITEMS */
struct ThemePixmapItemPrivate;
struct Image;
class QTOPIA_EXPORT ThemePixmapItem : public ThemeGraphicItem
{
public:
    ThemePixmapItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemePixmapItem();

    static void colorizeImage( QImage &img, const QColor& col, int alpha, bool colorroles = true);

protected:
    QPixmap loadImage(const QString &imgName, int colorRole, const QColor &color, int alpha, int width = 0, int height = 0);
    void setPixmap( const QString &key, const QPixmap &val, ThemeItem::State st = ThemeItem::Default, const QString &filename = QString::null );
    void setPixmap( const QLatin1String &key, const QPixmap &val, ThemeItem::State st = ThemeItem::Default, const QString &filename = QString::null );
    QPixmap pixmap( const QString &key, ThemeItem::State st = ThemeItem::Default ) const;
    QPixmap pixmap( const QLatin1String &key, ThemeItem::State st = ThemeItem::Default ) const;
    void scaleImage( const QString &key, int width, int height );
    void scaleImages( int count );
    void setHorizontalScale( bool enable );
    void setVerticalScale( bool enable );
    bool horizontalScale() const;
    bool verticalScale() const;
    bool replaceColor(QImage &image, const QColor &before, const QColor &after);

private:
    QPixmap scalePixmap( const QPixmap &pix, int width, int height );

    ThemePixmapItemPrivate* d;
};

class ThemeAnimationFrameInfo;
class QTOPIA_EXPORT ThemeAnimationItem : public ThemePixmapItem
{
public:
    ThemeAnimationItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    ~ThemeAnimationItem();

    virtual void setFrame(int);
    int frame() const;
    int frameCount() const;
    virtual void start();
    virtual void stop();

    int rtti() const;

protected:
    virtual void paint(QPainter *painter, const QRect &rect);
    virtual void layout();
    void advance();
    void constructionComplete();
    void expressionChanged( QExpressionEvaluator* expr );

protected:
    void addCharacters( const QString& ch );

private:
    void stateChanged( const ThemeItem::State& st );
    void setupAnimation( const QString &base, const QString &src, const QString &color, const QString &alpha,
            const QString &count, const QString &width, const QString &loop, const QString &looprev,
            const QString &delay, const QString& play, ThemeItem::State st = ThemeItem::Default );

    ThemeAnimationItemPrivate* d;

    friend class ThemeAnimationFrameInfo;
};

struct ThemeLevelItemPrivate;
class QTOPIA_EXPORT ThemeLevelItem : public ThemeAnimationItem
{
public:
    ThemeLevelItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    ~ThemeLevelItem();

    void setValue(int v);
    void setRange(int min, int max);

    int value() const;
    int minValue() const;
    int maxValue() const;

    int rtti() const;

    virtual void setFrame(int);
    virtual void layout();
    void stop();

private:
    void updateValue(int v);
    ThemeLevelItemPrivate* d;
};

struct ThemeStatusItemPrivate;
class QTOPIA_EXPORT ThemeStatusItem : public ThemePixmapItem
{
public:
    ThemeStatusItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    ~ThemeStatusItem();

    void setOn(bool e);
    bool isOn() const;

    int rtti() const;

protected:
    virtual void layout();
    void constructionComplete();
    virtual void paint(QPainter *painter, const QRect &rect);

    void expressionChanged( QExpressionEvaluator* expr );

private:
    void createImage( const QString &key, const QString &filename, const QString &col,
                const QString &alpha, ThemeItem::State st = ThemeItem::Default );
    void updateImage(const QString &key, ThemeItem::State st);

    ThemeStatusItemPrivate* d;
};

struct ThemeImageItemPrivate;
class QTOPIA_EXPORT ThemeImageItem : public ThemePixmapItem
{
public:
    ThemeImageItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeImageItem();

    void setImage(const QPixmap &pixmap, ThemeItem::State state = ThemeItem::Default);
    QPixmap image(ThemeItem::State state = ThemeItem::Default) const;

    int rtti() const;

protected:
    virtual void layout();
    virtual void paint(QPainter *painter, const QRect &rect);
    void paletteChange(const QPalette &);
    void constructionComplete();
    void expressionChanged( QExpressionEvaluator* expr );

private:
    void updateImage( ThemeItem::State st );
    ThemeImageItemPrivate* d;
};

class ThemeWidgetItemPrivate;
class QTOPIA_EXPORT ThemeWidgetItem : public ThemeGraphicItem
{
    friend class ThemeListItem;
    friend class ThemeWidgetItemPrivate;
public:
    ThemeWidgetItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeWidgetItem();
    virtual void setWidget( QWidget* );
    QWidget* widget() const;
    virtual int rtti() const;
    void setAutoDelete( bool ad );
    void setGeometry( const QRect& );

    virtual void setActive( bool f );
protected:
    void constructionComplete();
    void parseColorGroup( const QMap<QString,QString> &cgatts );
    void paint(QPainter *p, const QRect &clip);
    void layout();

private:
    void updateWidget();
    void paletteChange(const QPalette &);
    ThemeWidgetItemPrivate* d;
};

struct ThemeListItemPrivate;
class ThemeListModel;
class QTOPIA_EXPORT ThemeListItem : public ThemeWidgetItem
{
public:
    ThemeListItem(ThemeItem *parent, ThemedView *view, const ThemeAttributes &atts);
    virtual ~ThemeListItem();
    int rtti() const;
    QListView* listView() const;
    virtual void setWidget(QWidget *widget);
    void setModel(ThemeListModel *model);
    ThemeListModel *model() const;
private:
    ThemeListItemPrivate *d;
};

class QTOPIA_EXPORT ThemedView : public QWidget
{
    Q_OBJECT

public:
    enum Type { Item, Page, Animation, Level, Status, Image, Text, Rect,
                Line, Plugin, Exclusive, Layout, Group, Widget, List,
                Template, TemplateInstance };

    explicit ThemedView(QWidget *parent = 0, Qt::WFlags f = 0);
    virtual ~ThemedView();

    /* return whether a source has been loaded. */
    bool sourceLoaded() const;

    bool loadSource(const QString &file = QString());
    void setSourceFile(const QString &file);
    QSize sizeHint() const;

    QList<ThemeItem*> findItems(const QString &name, ThemedView::Type type = ThemedView::Item,
                                ThemeItem::State state = ThemeItem::All) const;

    ThemeItem *findItem(const QString &name, ThemedView::Type type = ThemedView::Item,
                        ThemeItem::State state = ThemeItem::All) const;

    QList<ThemeItem*> findItems(const QString &name, int type, int state = ThemeItem::All) const;
    ThemeItem *findItem(const QString &name, int type, int state = ThemeItem::All) const;

    QString themeName() const;
    void setThemeName(const QString& themeName);

    const QString pageName() const;
    const QString base() const;
    const QString defaultPics() const;

    ThemeItem *itemAt(const QPoint &pos) const;
    virtual QWidget *newWidget(ThemeWidgetItem *, const QString &);
    void setGeometryAndLayout(const QRect &r);
    void setGeometryAndLayout(int x, int y, int w, int h);

    void paint(QPainter *painter, const QRect &clip, ThemeItem *item = 0);

    void dumpState() const;
signals:
    void itemPressed(ThemeItem *);
    void itemReleased(ThemeItem *);
    void itemClicked(ThemeItem *);
    void visibilityChanged(ThemeItem *, bool);
    void loaded();

protected:

    virtual void themeLoaded(const QString &);
    void layout(ThemeItem *item=0);


    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);

    void paintEvent(QPaintEvent *);

    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);

    void registerExpression(ThemeItem *item, QExpressionEvaluator *expr);

private slots:
    void notifyExpressionChanged();
    void expressionDestroyed(QObject *obj);

private:
    QString rttiToString(int) const;
    void dumpState(ThemeItem *, int) const;
    ThemePageItem* pageItem() const;
    void paintItem(QPainter *p, ThemeItem *item, const QRect &clip);
    void paletteChange(const QPalette &);
    void paletteChange(ThemeItem *item, const QPalette &p);
    bool isOn(ThemeItem *item) const;
    ThemeItem *itemAt(const QPoint &pos, ThemeItem *item) const;
    ThemeItem *findItem(ThemeItem *item, const QString &name,
                        ThemedView::Type type = ThemedView::Item, ThemeItem::State state = ThemeItem::All) const;
    void findItems(ThemeItem *item, const QString &name, ThemedView::Type type,
                   ThemeItem::State state, QList<ThemeItem*> &list) const;
    ThemeItem *findItem(ThemeItem *item, const QString &name, int type, int state = ThemeItem::All) const;
    void findItems(ThemeItem *item, const QString &name, int type, int pressed, QList<ThemeItem*> &list) const;
    void visChanged(ThemeItem *item, bool);

    ThemedViewPrivate *d;
    friend class ThemeItem;
    friend class ThemeTemplateItem;
    friend class ThemeListDelegate;
};

//----------------------------------------------------------
/* declare ThemeListModelEntry */
class ThemeListModel;
struct ThemeListModelEntryPrivate;
class QTOPIA_EXPORT ThemeListModelEntry
{
public:
    explicit ThemeListModelEntry(ThemeListModel *model);
    virtual ~ThemeListModelEntry();

    /* unique id for this item, used in valuespace. */
    QString uid();

    /* the type (name attribute) that this entry corresponds to */
    virtual QString type() const = 0;

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key);

    ThemeListModel *model() const;

    ThemeTemplateInstanceItem *templateInstance();

private:
    void getTemplateInstance();
    QString valuespacePath();
    ThemeListModelEntryPrivate *d;
};
Q_DECLARE_METATYPE(ThemeListModelEntry*)

//-----------------------------------------------------------
/* declare ThemeListModel */
struct ThemeListModelPrivate;
class QTOPIA_EXPORT ThemeListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    // operates on a constructed list in the valuespace
    ThemeListModel(QObject *parent, ThemeListItem *li, ThemedView *view);
    virtual ~ThemeListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    ThemeListItem *listItem() const;
    ThemedView *themedView() const;

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex entryIndex(const ThemeListModelEntry *entry) const;

    ThemeListModelEntry *themeListModelEntry(const QModelIndex &index) const;

    void addEntry(ThemeListModelEntry *item);
    void removeEntry(const QModelIndex &index);
    void clear();

    void triggerUpdate();

protected:
    QList<ThemeListModelEntry*> items() const;

private:
    ThemeListModelPrivate *d;
};

#endif

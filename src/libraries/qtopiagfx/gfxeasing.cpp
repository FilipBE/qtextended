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

#include "gfxeasing.h"
#include <math.h>
#include <QHash>
#include <QPainter>
#include <QVariant>
#include <QDebug>
#include <QStringList>
#include <QMouseEvent>
#include "gfxtimeline.h"

typedef QHash<QString, qreal> GfxEasingProperties;
class GfxEasingFunction
{
public:
    virtual ~GfxEasingFunction() {}
    virtual float value(float t, float b, float c, float d) = 0;
    virtual GfxEasingFunction *copy() const = 0;
};

#include "easing.cpp"

struct ElasticEase : public GfxEasingFunction
{
    enum Type { In, Out };
    ElasticEase(Type t) : _t(t), _p(0.0f), _a(0.0f) {}

    Type _t;
    qreal _p;
    qreal _a;

    GfxEasingFunction *copy() const
    {
        ElasticEase *rv = new ElasticEase(_t);
        rv->_p = _p;
        rv->_a = _a;
        return rv;
    }

    float value(float t, float b, float c, float d)
    {
        if (t==0) return b;
        float t_adj = (float)t / (float)d;
        if (t_adj==1) return b+c;

        qreal p = _p?_p:(d * 0.3f);

        qreal a;
        qreal s;

        if(!_a || _a < ::fabs(c)) {
            a = c;
            s = p / 4.0f;
        } else {
            a = _a;
            s = p / (2 * M_PI) * ::asin(c / a);
        }

        if(_t == In) 
            t_adj -= 1.0f;

        return (a*::pow(2,-10*t_adj) * ::sin( (t_adj*d-s)*(2*M_PI)/p ) + c + b);
    }
};

struct BounceEase : public GfxEasingFunction
{
    enum Type { In, Out };
    BounceEase(Type t) : _t(t), _a(-1.0) {}

    Type _t;
    qreal _a;

    GfxEasingFunction *copy() const
    {
        BounceEase *rv = new BounceEase(_t);
        rv->_t = _t;
        rv->_a = _a;
        return rv;
    }

    float value(float t, float b, float c, float d)
    {
        if(In == _t)
            return c - bounce(d - t, 0, c, d) + b;
        else
            return bounce(t, b, c, d);
    }

    float bounce(float t, float b, float c, float d)
    {
        float t_adj = (float)t / (float)d;
        float amp = (_a == -1.0)?c:_a;
        if ((t_adj) < (1/2.75)) {
            if(c == 0. && _a != -1.0) {
                t_adj -= (0.5/2.75);
                return -amp * (1. - (30.25*t_adj*t_adj)) + b;
            } else {
                return c*(7.5625*t_adj*t_adj) + b;
            }
        } else if (t_adj < (2/2.75)) {
            t_adj -= (1.5/2.75);
            return -amp * (1. - (7.5625*t_adj*t_adj + .75)) + (b + c);
        } else if (t_adj < (2.5/2.75)) {
            t_adj -= (2.25/2.75);
            return -amp * (1. - (7.5625*t_adj*t_adj + .9375)) + (b + c);
        } else {
            t_adj -= (2.65/2.75);
            return -amp * (1. - (7.5625*t_adj*t_adj + .984375)) + (b + c);
        }
    }
};

static GfxEasingFunction *easeInElasticC(const GfxEasingProperties &p)
{
    ElasticEase *rv = new ElasticEase(ElasticEase::In);
    rv->_p = p["period"];
    rv->_a = p["amplitude"];
    return rv;
}


static GfxEasingFunction *easeOutElasticC(const GfxEasingProperties &p)
{
    ElasticEase *rv = new ElasticEase(ElasticEase::Out);
    rv->_p = p["period"];
    rv->_a = p["amplitude"];
    return rv;
}

static GfxEasingFunction *easeOutBounceC(const GfxEasingProperties &p)
{
    BounceEase *rv = new BounceEase(BounceEase::Out);
    rv->_a = p["amplitude"];
    return rv;
}

static GfxEasingFunction *easeInBounceC(const GfxEasingProperties &p)
{
    BounceEase *rv = new BounceEase(BounceEase::Out);
    rv->_a = p["amplitude"];
    return rv;
}


struct SimpleConfig : public GfxEasingFunction
{
    GfxEasing::Function func;

    float value(float t, float b, float c, float d)
    {
        return func(t, b, c, d);
    }

    GfxEasingFunction *copy() const
    {
        SimpleConfig *rv = new SimpleConfig;
        rv->func = func;
        return rv;
    }
};

GfxEasing::Function curveToFunc(GfxEasing::Curve curve)
{
    switch(curve)
    {
    case GfxEasing::None:
        return &easeNone;
    case GfxEasing::InQuad:
        return &easeInQuad;
    case GfxEasing::OutQuad:
        return &easeOutQuad;
    case GfxEasing::InOutQuad:
        return &easeInOutQuad;
    case GfxEasing::OutInQuad:
        return &easeOutInQuad;
    case GfxEasing::InCubic:
        return &easeInCubic;
    case GfxEasing::OutCubic:
        return &easeOutCubic;
    case GfxEasing::InOutCubic:
        return &easeInOutCubic;
    case GfxEasing::OutInCubic:
        return &easeOutInCubic;
    case GfxEasing::InQuart:
        return &easeInQuart;
    case GfxEasing::OutQuart:
        return &easeOutQuart;
    case GfxEasing::InOutQuart:
        return &easeInOutQuart;
    case GfxEasing::OutInQuart:
        return &easeOutInQuart;
    case GfxEasing::InQuint:
        return &easeInQuint;
    case GfxEasing::OutQuint:
        return &easeOutQuint;
    case GfxEasing::InOutQuint:
        return &easeInOutQuint;
    case GfxEasing::OutInQuint:
        return &easeOutInQuint;
    case GfxEasing::InSine:
        return &easeInSine;
    case GfxEasing::OutSine:
        return &easeOutSine;
    case GfxEasing::InOutSine:
        return &easeInOutSine;
    case GfxEasing::OutInSine:
        return &easeOutInSine;
    case GfxEasing::InExpo:
        return &easeInExpo;
    case GfxEasing::OutExpo:
        return &easeOutExpo;
    case GfxEasing::InOutExpo:
        return &easeInOutExpo;
    case GfxEasing::OutInExpo:
        return &easeOutInExpo;
    case GfxEasing::InCirc:
        return &easeInCirc;
    case GfxEasing::OutCirc:
        return &easeOutCirc;
    case GfxEasing::InOutCirc:
        return &easeInOutCirc;
    case GfxEasing::OutInCirc:
        return &easeOutInCirc;
    case GfxEasing::InElastic:
        return &easeInElastic;
    case GfxEasing::OutElastic:
        return &easeOutElastic;
    case GfxEasing::InOutElastic:
        return &easeInOutElastic;
    case GfxEasing::OutInElastic:
        return &easeOutInElastic;
    case GfxEasing::InBack:
        return &easeInBack;
    case GfxEasing::OutBack:
        return &easeOutBack;
    case GfxEasing::InOutBack:
        return &easeInOutBack;
    case GfxEasing::OutInBack:
        return &easeOutInBack;
    case GfxEasing::InBounce:
        return &easeInBounce;
    case GfxEasing::OutBounce:
        return &easeOutBounce;
    case GfxEasing::InOutBounce:
        return &easeInOutBounce;
    case GfxEasing::OutInBounce:
        return &easeOutInBounce;
    default:
        return 0;
    };
}

struct NameFunctionMap : public QHash<QString, GfxEasing::Function>
{
    NameFunctionMap()
    {
        insert("easeNone", easeNone);
        insert("easeInQuad", easeInQuad);
        insert("easeOutQuad", easeOutQuad);
        insert("easeInOutQuad", easeInOutQuad);
        insert("easeOutInQuad", easeOutInQuad);
        insert("easeInCubic", easeInCubic);
        insert("easeOutCubic", easeOutCubic);
        insert("easeInOutCubic", easeInOutCubic);
        insert("easeOutInCubic", easeOutInCubic);
        insert("easeInQuart", easeInQuart);
        insert("easeOutQuart", easeOutQuart);
        insert("easeInOutQuart", easeInOutQuart);
        insert("easeOutInQuart", easeOutInQuart);
        insert("easeInQuint", easeInQuint);
        insert("easeOutQuint", easeOutQuint);
        insert("easeInOutQuint", easeInOutQuint);
        insert("easeOutInQuint", easeOutInQuint);
        insert("easeInSine", easeInSine);
        insert("easeOutSine", easeOutSine);
        insert("easeInOutSine", easeInOutSine);
        insert("easeOutInSine", easeOutInSine);
        insert("easeInExpo", easeInExpo);
        insert("easeOutExpo", easeOutExpo);
        insert("easeInOutExpo", easeInOutExpo);
        insert("easeOutInExpo", easeOutInExpo);
        insert("easeInCirc", easeInCirc);
        insert("easeOutCirc", easeOutCirc);
        insert("easeInOutCirc", easeInOutCirc);
        insert("easeOutInCirc", easeOutInCirc);
        insert("easeInElastic", easeInElastic);
        insert("easeOutElastic", easeOutElastic);
        insert("easeInOutElastic", easeInOutElastic);
        insert("easeOutInElastic", easeOutInElastic);
        insert("easeInBack", easeInBack);
        insert("easeOutBack", easeOutBack);
        insert("easeInOutBack", easeInOutBack);
        insert("easeOutInBack", easeOutInBack);
        insert("easeInBounce", easeInBounce);
        insert("easeOutBounce", easeOutBounce);
        insert("easeInOutBounce", easeInOutBounce);
        insert("easeOutInBounce", easeOutInBounce);
    }
};
Q_GLOBAL_STATIC(NameFunctionMap, nameFunctionMap);

typedef GfxEasingFunction *(*ConfigurableFunction)(const GfxEasingProperties &);
struct ConfigFunctionMap : public QHash<QString, ConfigurableFunction>
{
    ConfigFunctionMap()
    {
        insert("easeInElastic", easeInElasticC);
        insert("easeOutElastic", easeOutElasticC);
        insert("easeInBounce", easeInBounceC);
        insert("easeOutBounce", easeOutBounceC);
    }
};
Q_GLOBAL_STATIC(ConfigFunctionMap, configFunctionMap);

/*!
    \class GfxEasing
    \inpublicgroup QtBaseModule
    \brief The GfxEasing class provides easing curves for controlling animation.

    Easing curves describe a function that controls how a value changes over
    time.  Easing curves allow transitions from one value to another to appear
    more natural than a simple linear motion would allow.  The GfxEasing class
    is usually used in conjunction with the GfxTimeLine class, but can be used
    on its own.

    To calculate the value at a given time, the easing curve function requires
    the starting value, the final value and the total time to change from the
    starting to the final value.  When using the GfxEasing class with
    GfxTimeLine, these values are supplied by the GfxTimeLine.  When using
    the GfxEasing class on its own, the programmer must specify them using
    the GfxEasing::setFrom(), GfxEasing::setTo() and GfxEasing::setLength()
    methods, or by passing them explicitly to the GfxEasing::valueAt() method.

    For example,
    \code
    GfxEasing easing(GfxEasing::InOutQuad);
    easing.setFrom(0);
    easing.setTo(1000);
    easing.setLength(1000);

    for(int milliseconds = 0; milliseconds < 1000; ++milliseconds)
        qWarning() << "Value at" << milliseconds << "milliseconds is
                   << easing.valueAt(milliseconds);
    \endcode
    will print the value at each millisecond for an InOutQuad transition from
    0 to 1000 over 1 second.

    When using a GfxTimeLine, the values are communicated implicitly.
    \code
    GfxTimeLine timeline;
    GfxValue value(0);

    timeline.move(value, 1000, GfxEasing(GfxEasing::InOutQuad), 1000);
    \endcode
    In this case, any values set using the previous setter methods would be
    ignored.
 */

/*!
    \enum GfxEasing::Curve

    The type of easing curve.

    \value None Easing equation function for a simple linear tweening, with no easing.
    \value InQuad Easing equation function for a quadratic (t^2) easing in: accelerating from zero velocity.
    \value OutQuad Easing equation function for a quadratic (t^2) easing out: decelerating to zero velocity.
    \value InOutQuad Easing equation function for a quadratic (t^2) easing in/out: acceleration until halfway, then deceleration.
    \value OutInQuad Easing equation function for a quadratic (t^2) easing out/in: deceleration until halfway, then acceleration.
    \value InCubic Easing equation function for a cubic (t^3) easing in: accelerating from zero velocity.
    \value OutCubic Easing equation function for a cubic (t^3) easing out: decelerating from zero velocity.
    \value InOutCubic Easing equation function for a cubic (t^3) easing in/out: acceleration until halfway, then deceleration.
    \value OutInCubic Easing equation function for a cubic (t^3) easing out/in: deceleration until halfway, then acceleration.
    \value InQuart Easing equation function for a quartic (t^4) easing in: accelerating from zero velocity.
    \value OutQuart Easing equation function for a quartic (t^4) easing out: decelerating from zero velocity.
    \value InOutQuart Easing equation function for a quartic (t^4) easing in/out: acceleration until halfway, then deceleration.
    \value OutInQuart Easing equation function for a quartic (t^4) easing out/in: deceleration until halfway, then acceleration.
    \value InQuint Easing equation function for a quintic (t^5) easing in: accelerating from zero velocity.
    \value OutQuint Easing equation function for a quintic (t^5) easing out: decelerating from zero velocity.
    \value InOutQuint Easing equation function for a quintic (t^5) easing in/out: acceleration until halfway, then deceleration.
    \value OutInQuint Easing equation function for a quintic (t^5) easing out/in: deceleration until halfway, then acceleration.
    \value InSine Easing equation function for a sinusoidal (sin(t)) easing in: accelerating from zero velocity.
    \value OutSine Easing equation function for a sinusoidal (sin(t)) easing out: decelerating from zero velocity.
    \value InOutSine Easing equation function for a sinusoidal (sin(t)) easing in/out: acceleration until halfway, then deceleration.
    \value OutInSine Easing equation function for a sinusoidal (sin(t)) easing out/in: deceleration until halfway, then acceleration.
    \value InExpo Easing equation function for an exponential (2^t) easing in: accelerating from zero velocity.
    \value OutExpo Easing equation function for an exponential (2^t) easing out: decelerating from zero velocity.
    \value InOutExpo Easing equation function for an exponential (2^t) easing in/out: acceleration until halfway, then deceleration.
    \value OutInExpo Easing equation function for an exponential (2^t) easing out/in: deceleration until halfway, then acceleration.
    \value InCirc Easing equation function for a circular (sqrt(1-t^2)) easing in: accelerating from zero velocity.
    \value OutCirc Easing equation function for a circular (sqrt(1-t^2)) easing out: decelerating from zero velocity.
    \value InOutCirc Easing equation function for a circular (sqrt(1-t^2)) easing in/out: acceleration until halfway, then deceleration.
    \value OutInCirc Easing equation function for a circular (sqrt(1-t^2)) easing out/in: deceleration until halfway, then acceleration.
    \value InElastic Easing equation function for an elastic (exponentially decaying sine wave) easing in: accelerating from zero velocity.  The peak amplitude can be set with the \i amplitude parameter, and the period of decay by the \i period parameter.
    \value OutElastic Easing equation function for an elastic (exponentially decaying sine wave) easing out: decelerating from zero velocity.  The peak amplitude can be set with the \i amplitude parameter, and the period of decay by the \i period parameter.
    \value InOutElastic Easing equation function for an elastic (exponentially decaying sine wave) easing in/out: acceleration until halfway, then deceleration.
    \value OutInElastic Easing equation function for an elastic (exponentially decaying sine wave) easing out/in: deceleration until halfway, then acceleration.
    \value InBack Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in: accelerating from zero velocity.
    \value OutBack Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out: decelerating from zero velocity.
    \value InOutBack Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in/out: acceleration until halfway, then deceleration.
    \value OutInBack Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out/in: deceleration until halfway, then acceleration.
    \value OutBounce Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out: decelerating from zero velocity.
    \value InBounce Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in: accelerating from zero velocity.
    \value InOutBounce Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in/out: acceleration until halfway, then deceleration.
    \value OutInBounce Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out/in: deceleration until halfway, then acceleration.
    */

/*!
    Construct a linear easing object.  This is equivalent to \c {GfxEasing(GfxEasing::None)}.
 */
GfxEasing::GfxEasing()
: _config(0), _func(&easeNone), _b(0.), _c(1.), _d(1.)
{
}

/*!
    Construct an easing object with the given \a curve.
 */
GfxEasing::GfxEasing(Curve curve)
: _config(0), _func(0), _b(0.), _c(1.), _d(1.)
{
    _func = curveToFunc(curve);
    if(!_func)
        qWarning("GfxEasing: Invalid curve type %d", curve);
}

/*!
    Construct an easing object with the given \a curve.  If \a curve does not
    describe a legal curve, a linear easing object is constructed.

    Curve names have the form
    \c {ease<CurveName>[(<arg>: <arg value>[, <arg2>: <arg value>])]}.  The
    \i CurveName is equivalent to the GfxEasing::Curve enum name.  Some more
    advanced curves can take arguments to further refine their behaviour.  Where
    applicable, these parameters are described in the corresponding
    GfxEasing::Curve value documentation.

    For example,
    \code
    GfxEasing easing("easeInOutQuad");
    GfxEasing easing2("easeInElastic(period: 5, amplitude: 100)");
    \endcode
 */
GfxEasing::GfxEasing(const QString &curve)
: _config(0), _func(&easeNone), _b(0.), _c(1.), _d(1.)
{
    if(curve.contains("(")) {
        QString easeName = curve.trimmed();
        GfxEasingProperties prop;
        if(!easeName.endsWith(")")) {
            qWarning("GfxEasing: Unmatched perenthesis in easing function '%s'",
                     curve.toLatin1().constData());
            return;
        }

        int idx = easeName.indexOf('(');
        QString prop_str =
            easeName.mid(idx + 1, easeName.length() - 1 - idx - 1);
        easeName = easeName.left(idx);

        QStringList props = prop_str.split(",");
        foreach(QString str, props) {
            int sep = str.indexOf(':');

            if(sep == -1) {
                qWarning("GfxEasing: Improperly specified property in easing function '%s'",
                        curve.toLatin1().constData());
                return;
            }

            QString propName = str.left(sep).trimmed();
            bool isOk;
            qreal propValue = str.mid(sep + 1).trimmed().toDouble(&isOk);

            if(propName.isEmpty() || !isOk) {
                qWarning("GfxEasing: Improperly specified property in easing function '%s'",
                        curve.toLatin1().constData());
                return;
            }

            prop.insert(propName, propValue);
        }

        QHash<QString, ConfigurableFunction>::Iterator iter =
            configFunctionMap()->find(easeName);

        if(iter != configFunctionMap()->end())
            _config = (*iter)(prop);
    } else {
        _func = *(nameFunctionMap()->find(curve));
    }

    if(!_func && !_config)
        qWarning("GfxEasing: Unknown easing curve '%s'",
                 curve.toLatin1().constData());
}

/*!
    Construct a copy of \a other.
 */
GfxEasing::GfxEasing(const GfxEasing &other)
: _config(0), _func(other._func), _b(other._b), _c(other._c), _d(other._d)
{
    if(other._config) _config = other._config->copy();
}

/*!
    Copy \a other.
 */
GfxEasing &GfxEasing::operator=(const GfxEasing &other)
{
    if(_config) { delete _config; _config = 0; }
    if(other._config) _config = other._config->copy();

    _func = other._func;
    _b = other._b;
    _c = other._c;
    _d = other._d;

    return *this;
}

/*!
    Returns true if this is a linear easing object.
 */
bool GfxEasing::isLinear() const
{
    return !_config && _func == &easeNone;
}

/*!
    Return the starting value for the easing curve.  By default this is 0.
 */
qreal GfxEasing::from() const
{
    return _b;
}

/*!
    Set the starting value for the easing curve to \a from.
 */
void GfxEasing::setFrom(qreal from)
{
    _b = from;
}

/*!
    Return the final value for the easing curve.  By default this is 0.
 */
qreal GfxEasing::to() const
{
    return _c;
}

/*!
    Set the final value for the easing curve to \a to.
 */
void GfxEasing::setTo(qreal to)
{
    _c = to;
}

/*!
    Return the length of the easing curve, in milliseconds.  By default this
    is 1.
 */
qreal GfxEasing::length() const
{
    return _d;
}

/*!
    Set the \a length of the easing curve, in milliseconds.
 */
void GfxEasing::setLength(qreal length)
{
    Q_ASSERT(length > 0);
    _d = length;
}

/*!
    Return the value for the easing curve at time \a t milliseconds, based on
    the parameters returned by GfxEasing::from(), GfxEasing::to() and
    GfxEasing::length().

    \a t is clamped to (0, GfxEasing::length()).
 */
qreal GfxEasing::valueAt(qreal t) const
{
    if(t < 0) t = 0;
    else if(t > length()) t = length();

    if(_config)
        return _config->value(t, _b, _c - _b, _d);
    else
        return _func(t, _b, _c - _b, _d);
}

/*
    Return the value for the easing curve at time \a t milliseconds, based on
    the provided parameters \a from, \a to and \a length.  The values returned
    from the object's GfxEasing::from(), GfxEasing::to() and GfxEasing::length()
    methods are ignored.

    \a t is clamped to (0, \a length).
 */
qreal GfxEasing::valueAt(qreal t, qreal from, qreal to, qreal length) const
{
    if(t < 0) t = 0;
    else if(t > length) t = length;

    if(_config)
        return _config->value(t, from, to - from, length);
    else
        return _func(t, from, to - from, length);
}

/*!
    \internal
 */
QStringList GfxEasing::curves()
{
    return nameFunctionMap()->keys();
}

struct GfxEasingDemoWidgetPrivate
{
    QList<QGfxValue *> values;
    QList<GfxEasing> easing;
    QStringList names;

    GfxTimeLine timeline;

    QRect rectFor(int idx, QWidget *);
};

QRect GfxEasingDemoWidgetPrivate::rectFor(int idx, QWidget *w)
{
    static const int cols = 6;
    static const int spacing = 10;
    static const int minSize = 30;

    const int rows = (names.count() + (cols - 1)) / cols;

    if(!cols || !rows)
        return QRect();

    int boxWidth = qMax(minSize, (w->width() - (cols - 1) * spacing) / cols );
    int boxHeight = qMax(minSize, (w->height() - (rows - 1) * spacing) / rows);

    int row = idx / cols;
    int col = idx - row * cols;

    QRect rv(col * (boxWidth + spacing), row * (boxHeight + spacing),
             boxWidth, boxHeight);
    rv.setHeight(rv.height());
    return rv;
}


GfxEasingDemoWidget::GfxEasingDemoWidget(QWidget *parent)
: QWidget(parent), d(new GfxEasingDemoWidgetPrivate)
{
    d->names = GfxEasing::curves();

    for(int ii = 0; ii < d->names.count(); ++ii)  {
        d->values << new QGfxValue(0., this);
        d->easing << GfxEasing(d->names.at(ii));
    }

    QObject::connect(&d->timeline, SIGNAL(updated()), this, SLOT(update()));
}

GfxEasingDemoWidget::~GfxEasingDemoWidget()
{
    delete d; d = 0;
}

void GfxEasingDemoWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPen pen(QColor(100, 100, 255), 3);
    QPen pen2(Qt::blue);
    QPen pen3(Qt::black);

    for(int idx = 0; idx < d->names.count(); ++idx) {
        QRect rect = d->rectFor(idx, this);
        int boxWidth = rect.width();
        int boxHeight = rect.height();

        p.setPen(pen);
        p.drawRect(rect);

        p.setPen(pen3);
        p.drawText(rect.topLeft() + QPoint(3, 13), d->names.at(idx));

        p.setPen(pen2);
        int prog = int(qreal(boxWidth) * d->values.at(idx)->value());
        QPoint lastPoint(0, boxHeight);
        for(int xx = 0; xx < prog; ++xx) {
            qreal val = d->easing.at(idx).valueAt(xx, 0, 1, boxWidth);
            QPoint point(xx, int((1 - val) * boxHeight));

            p.drawLine(lastPoint + rect.topLeft(), point + rect.topLeft());
            lastPoint = point;
        }
    }
}

void GfxEasingDemoWidget::mousePressEvent(QMouseEvent *e)
{
    QPoint p = e->pos();

    for(int idx = 0; idx < d->values.count(); ++idx) {
        if(d->rectFor(idx, this).contains(p)) {
            d->timeline.reset(*d->values.at(idx));
            d->values.at(idx)->setValue(0.);
            d->timeline.move(*d->values.at(idx), 1., 2000);
            break;
        }
    }

    QWidget::mousePressEvent(e);
}

void GfxEasingDemoWidget::showEvent(QShowEvent *e)
{
    d->timeline.clear();
    for(int ii = 0; ii < d->values.count(); ++ii) {
        d->values.at(ii)->setValue(0.);
        d->timeline.move(*d->values.at(ii), 1., 2000);
    }
    QWidget::showEvent(e);
}

void GfxEasingDemoWidget::hideEvent(QHideEvent *e)
{
    d->timeline.clear();
    QWidget::hideEvent(e);
}



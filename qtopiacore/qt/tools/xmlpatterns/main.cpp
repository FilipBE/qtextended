/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Patternist project on Trolltech Labs.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <QtCore/QDir>
#include <QtCore/QtDebug>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#include <QtXmlPatterns/QXmlFormatter>
#include <QtXmlPatterns/QXmlItem>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlSerializer>

#include "qapplicationargument_p.h"
#include "qapplicationargumentparser_p.h"
#include "qcoloringmessagehandler_p.h"

#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
/* Needed for opening stdout with _fdopen & friends. io.h seems to not be
 * needed on MinGW though. */
#include <io.h>
#include <fcntl.h>
#endif

#include "main.h"

QT_USE_NAMESPACE

/* The two Q_DECLARE_METATYPE macros must appear before the code
 * on a couple of HPUX platforms. */

/*!
 \internal
 \since 4.4
 Represents the name and value found in "-param name=value".
 */
typedef QPair<QString, QString> Parameter;
Q_DECLARE_METATYPE(Parameter);

/*!
 \internal
 \since 4.4
 For the -output switch.
 */
Q_DECLARE_METATYPE(QIODevice *);

/*!
 \class PatternistApplicationParser
 \brief Subclass to handle -param name=value
 \internal
 \since 4.4
 \reentrant
 */
class PatternistApplicationParser : public QApplicationArgumentParser
{
public:
    inline PatternistApplicationParser(int argc, char **argv) : QApplicationArgumentParser(argc, argv)
#ifdef Q_OS_WIN
                                                              , m_stdout(0)
#endif
    {
    }

#ifdef Q_OS_WIN
    virtual ~PatternistApplicationParser()
    {
        /* QFile::~QFile() nor QFile::close() frees the handle when
         * we use QFile::open() so we have to do it manually.
         *
         * "If stream is NULL, the invalid parameter handler is invoked," so
         * lets try to avoid that. */
        if(m_stdout)
            fclose(m_stdout);
    }
#endif

protected:
    virtual QVariant convertToValue(const QApplicationArgument &arg,
                                    const QString &input) const
    {
        if(arg.name() == QLatin1String("param"))
        {
            const int assign = input.indexOf(QLatin1Char('='));

            if(assign == -1)
            {
                message(QXmlPatternistCLI::tr("Each binding must contain an equal sign."));
                return QVariant();
            }

            const QString name(input.left(assign));
            const QString value(input.mid(assign + 1));

            if(!QXmlName::isNCName(name))
            {
                message(QXmlPatternistCLI::tr("The variable name must be a valid NCName, which %1 isn't.").arg(name));
                return QVariant();
            }

            /* The value.isNull() check ensures we can bind variables whose value is an empty string. */
            return qVariantFromValue(Parameter(name, value.isNull() ? QString(QLatin1String("")) : value ));
        }
        else if(arg.name() == QLatin1String("output"))
        {
            QFile *const f = new QFile(input);

            if(f->open(QIODevice::WriteOnly))
                return qVariantFromValue(static_cast<QIODevice *>(f));
            else
            {
                message(QXmlPatternistCLI::tr("Failed to open file %1 for writing: %2").arg(f->fileName(), f->errorString()));
                return QVariant();
            }
        }
        else
            return QApplicationArgumentParser::convertToValue(arg, input);
    }

    virtual QString typeToName(const QApplicationArgument &argument) const
    {
        if(argument.name() == QLatin1String("param"))
            return QLatin1String("name=value");
        else if(argument.name() == QLatin1String("output"))
            return QLatin1String("local file");
        else
            return QApplicationArgumentParser::typeToName(argument);
    }

    virtual QVariant defaultValue(const QApplicationArgument &argument) const
    {
        if(argument.name() == QLatin1String("output"))
        {
            QFile *const out = new QFile();

#ifdef Q_OS_WIN
            /* If we don't open stdout in "binary" mode on Windows, it will translate
             * 0xA into 0xD 0xA. See Trolltech task 173619, for an example. */
            _setmode(_fileno(stdout), _O_BINARY);
            m_stdout = QT_WA_INLINE(_wfdopen(_fileno(stdout), L"wb"),_fdopen(_fileno(stdout), "wb"));
            out->open(m_stdout, QIODevice::WriteOnly);
#else
            out->open(stdout, QIODevice::WriteOnly);
#endif

            return qVariantFromValue(static_cast<QIODevice *>(out));
        }
        else
            return QApplicationArgumentParser::defaultValue(argument);
    }

#ifdef Q_OS_WIN
private:
    mutable FILE *m_stdout;
#endif
};

int main(int argc, char **argv)
{
    enum ExitCode
    {
        /**
         * We start from 2, because QApplicationArgumentParser
         * uses 1.
         */
        QueryFailure = 2,
        StdOutFailure
    };

    const QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QLatin1String("xmlpatterns"));

    PatternistApplicationParser parser(argc, argv);
    parser.setApplicationDescription(QLatin1String("A tool for running XQuery queries."));
    parser.setApplicationVersion(QLatin1String("0.1"));

    /* Is there a better way to do this? Probably not, but if the class becomes public, we probably
     * want a helper function that wraps this hack. */
    const int parameterType = qVariantFromValue(Parameter()).userType();
    const int outputType = qVariantFromValue(static_cast<QIODevice *>(0)).userType();

    QApplicationArgument param(QLatin1String("param"),
                               QXmlPatternistCLI::tr("Binds an external variable. The value is directly available using the variable reference: $name."),
                               parameterType);
    param.setMaximumOccurrence(-1);
    parser.addArgument(param);

    const QApplicationArgument noformat(QLatin1String("no-format"),
                                        QXmlPatternistCLI::tr("By default output is formatted for readability. When specified, strict serialization is performed."));
    parser.addArgument(noformat);

    const QApplicationArgument isURI(QLatin1String("is-uri"),
                                     QXmlPatternistCLI::tr("If specified, the filename is interpreted as a URI instead of a local filename."));
    parser.addArgument(isURI);

    /* The temporary object is required to compile with g++ 3.3. */
    QApplicationArgument queryURI = QApplicationArgument(QString(), /* Nameless. */
                                                         QXmlPatternistCLI::tr("A local filename pointing to the query to run. "
                                                                               "If the name ends with .xq it's assumed "
                                                                               "to be an XQuery query. (In other cases too, but "
                                                                               "that interpretation may change in a future release of Qt.)"),
                                  QVariant::String);
    queryURI.setMinimumOccurrence(1);
    parser.addArgument(queryURI);

    QApplicationArgument output(QLatin1String("output"),
                                QXmlPatternistCLI::tr("A local file to which the output should be written. The file is overwritten, or if not exist, created. If absent, stdout is used."),
                                outputType);
    parser.addArgument(output);

    if(!parser.parse())
        return parser.exitCode();

    QXmlQuery query;

    /* Bind external variables. */
    {
        const QVariantList parameters(parser.values(param));
        const int len = parameters.count();

        for(int i = 0; i < len; ++i)
        {
            const Parameter p(qVariantValue<Parameter>(parameters.at(i)));
            query.bindVariable(p.first, QXmlItem(p.second));
        }
    }

    /* The final preparations and execute the query. */
    QPatternist::ColoringMessageHandler messageHandler;
    query.setMessageHandler(&messageHandler);

    /* Get the query URI. */
    QUrl userURI;
    {
        const QString stringURI(parser.value(queryURI).toString());

        if(parser.has(isURI))
            userURI = QUrl::fromEncoded(stringURI.toLatin1());
        else
            userURI = QUrl::fromLocalFile(stringURI);
    }
    const QUrl effectiveURI(QUrl::fromLocalFile(QDir::current().absolutePath() + QLatin1Char('/')).resolved(userURI));

    Q_ASSERT_X(userURI.isValid(), Q_FUNC_INFO,
               "QApplicationArgumentParser should promise us this.");

    query.setQuery(effectiveURI);

    QIODevice *const outDevice = qVariantValue<QIODevice *>(parser.value(output));
    Q_ASSERT(outDevice);
    Q_ASSERT(outDevice->isWritable());

    if(query.isValid())
    {
        QAbstractXmlReceiver *receiver = 0;

        if(parser.has(noformat))
            receiver = new QXmlSerializer(query, outDevice);
        else
            receiver = new QXmlFormatter(query, outDevice);

        const bool success = query.evaluateTo(receiver);
        delete outDevice;
        delete receiver;

        if(success)
            return parser.exitCode();
        else
            return QueryFailure;
    }
    else
    {
        delete outDevice;
        return QueryFailure;
    }
}


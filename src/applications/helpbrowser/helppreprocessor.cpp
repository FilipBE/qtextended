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
#include "helppreprocessor.h"

#include <QFile>
#include <QSettings>
#include <QTextCodec>
#include <QContentSet>
#include <QTextStream>
#include <QStyle>

#include <qdesktopwidget.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>

#include <QValueSpaceItem>
#include <QExpressionEvaluator>

#include <QDebug>

/*
    Supports very basic SSI
     - line based - don't put set/include/conditions on the same line
     - supports limited recursion (parameterize) to avoid DoS
    When changing this function, please ensure that doc/src/tools/help-preprocessor.qdoc
    is updated to accurately reflect the changes.
*/

HelpPreProcessor::HelpPreProcessor( const QString &file, int maxrecurs )
    : mFile( file ), levels(maxrecurs)
{
    // setup the compile-time variables
#if defined(QTOPIA_VOIP)
    replace["VOIP"]="1";
#endif
#if defined(QTOPIA_CELL)
    replace["CELL"]="1";
#endif
#if defined(QTOPIA_TELEPHONY)
    replace["TELEPHONY"]="1";
#endif
#if defined(QTOPIA_INFRARED)
    replace["INFRARED"]="1";
#endif
#if defined(QTOPIA_BLUETOOTH)
    replace["BLUETOOTH"]="1";
#endif
#if !defined(QT_NO_CLIPBOARD)
    replace["CLIPBOARD"]="1";
#endif
#if defined(QTOPIA_VPN)
    replace["VPN"]="1";
#endif

    if ( Qtopia::mousePreferred() )
        replace["TOUCH"]="1";
    else
        replace["KEYPAD"]="1";
    if ( Qtopia::hasKey( Qt::Key_Flip ) )
        replace["FLIP"]="1";
    if (QApplication::desktop()->numScreens() > 1)
        replace["MULTISCREEN"]="1";
}

QString HelpPreProcessor::text()
{
    return parse(mFile);
}

QString HelpPreProcessor::parse(const QString& filename)
{
    QFile f( filename );
    if ( !f.exists() ) {
        QStringList helpPaths = Qtopia::helpPaths();
        QStringList::Iterator it;
        for ( it = helpPaths.begin(); it != helpPaths.end(); it++ ) {
            QString file = (*it) + "/" + filename;
            f.setFileName( file );
            if ( f.exists() )
                break;
        }
        if ( it == helpPaths.end() )
            return tr("Could not locate %1", "%1 - file").arg( filename );
    }
    qLog(Help) << "Loading help file: " << filename << "(" << f.fileName() << ")" ;
    f.open( QIODevice::ReadOnly );
    QByteArray data = f.readAll();
    QTextStream ts( data, QIODevice::ReadOnly );
    ts.setCodec( QTextCodec::codecForName("UTF-8") );
    QString text;

    QString line;

    QRegExp tagAny( "<!--#" );
    QRegExp tagIf( "<!--#if\\s+expr=\"\\$([^\"]*)\"\\s*-->" );
    QRegExp tagElif( "<!--#elif\\s+expr=\"\\$([^\"]*)\"\\s*-->" ); // not supported
    QRegExp tagElse( "<!--#else\\s*-->" );
    QRegExp tagEndif( "<!--#endif\\s*-->" );
    QRegExp tagSet( "<!--#set\\s+var=\"([^\"]*)\"\\s*value=\"([^\"]*)\"\\s*-->" );
    QRegExp tagSetVS( "<!--#set\\s+var=\"([^\"]*)\"\\s*valuespace=\"([^\"]*)\"\\s*value=\"([^\"]*)\"\\s*-->" );
    QRegExp tagEcho( "<!--#echo\\s+var=\"([^\"]*)\"\\s*-->" );
    QRegExp tagInclude( "<!--#include\\s+file=\"([^\"]*)\"\\s*-->" );
    QRegExp tagExec( "<!--#exec\\s+cmd=\"([^\"]*)\"\\s*-->" );

    bool skip = false;
    int lnum=0;

    do {
        line = ts.readLine();
        lnum++;
        if ( tagAny.indexIn(line) != -1 ) {
            int offset;
            int matchLen;

            offset = 0;
            matchLen = 0;

            while ( (offset = tagIf.indexIn( line, offset + matchLen )) != -1 ) {
                matchLen = tagIf.matchedLength();
                tests.push(tagIf.cap(1).split(QRegExp("\\s*\\|\\|\\s*\\$")));
                inverts.push(false);
                QStringList t = tagIf.cap(1).split(QRegExp("\\s*\\|\\|\\s*\\$"));
                //text+="TEST("+t.join(" or ")+")";
            }

            offset = 0;
            matchLen = 0;
            while ( (offset = tagElse.indexIn( line, offset + matchLen )) != -1 ) {
                matchLen = tagEndif.matchedLength();
                inverts.push(!inverts.pop());
            }

            offset = 0;
            matchLen = 0;
            bool err=false;
            while ( (offset = tagEndif.indexIn( line, offset + matchLen )) != -1 ) {
                matchLen = tagEndif.matchedLength();
                if (!tests.isEmpty()) tests.pop(); else err=true;
                if (!inverts.isEmpty()) inverts.pop(); else err=true;
            }
            if (err)
                qWarning("%s:%d:Unexpected #endif",filename.toLatin1().data(),lnum);

            QStack<QStringList>::ConstIterator it;
            QStack<bool>::ConstIterator bit;

            // recalculate skip
            skip = false;
            for ( it = tests.begin(),bit=inverts.begin(); it != tests.end() && !skip; ++it,++bit ) {
                skip = true;
                foreach (QString t, *it)
                    if ( !replace[t].isEmpty() )
                        skip = false;
                if (*bit)
                    skip = !skip;
            }

            if ( !skip ) {
                offset = 0;
                matchLen = 0;
                while ( (offset = tagSetVS.indexIn( line, offset + matchLen )) != -1 ) {
                    matchLen = tagSetVS.matchedLength();
                    QString key = tagSetVS.cap(1);
                    QString valuespace = tagSetVS.cap(2);
                    QString value = tagSetVS.cap(3);

                    QRegExp exists("exists\\(@([^\\)]*)\\)");
                    if (exists.indexIn(valuespace) != -1) {
                        QByteArray vsKey = exists.cap(1).toUtf8();
                        int lastSlash = vsKey.lastIndexOf('/');
                        if (lastSlash >= 1) {
                            const QByteArray base = vsKey.mid(1, lastSlash - 1);
                            const QByteArray item = vsKey.mid(lastSlash + 1);

                            QValueSpaceItem vs(base);
                            if (vs.subPaths().contains(item))
                                replace[key] = "1";
                            else
                                replace[key] = "";
                        }
                    } else {
                        QExpressionEvaluator expression;
                        expression.setExpression(valuespace.toUtf8());
                        if (expression.isValid()) {
                            if (expression.evaluate()) {
                                replace[key] = expression.result().toString();
                            } else {
                                qWarning("%s:%d:Run-time error when evaluating expression",
                                            filename.toLatin1().data(), lnum);
                            }
                        } else {
                            qWarning("%s:%d:Syntax or semantic error in expression",
                                        filename.toLatin1().data(), lnum);
                        }
                    }
                }

                offset = 0;
                matchLen = 0;
                while ( (offset = tagSet.indexIn( line, offset + matchLen )) != -1 ) {
                    matchLen = tagSet.matchedLength();
                    QString key = tagSet.cap(1);
                    QString value = tagSet.cap(2);
                    replace[key] = value;
                }

                while ( (offset = tagEcho.indexIn( line )) != -1 ) {
                    QString key = tagEcho.cap(1);
                    line.replace( offset, tagEcho.matchedLength(), replace[key] );
                }

                while ( (offset = tagExec.indexIn( line )) != -1 ) {
                    QString cmd = tagExec.cap(1);
                    line.replace( offset, tagExec.matchedLength(), exec(cmd) );
                }

                if ( levels ) {
                    while ( (offset = tagInclude.indexIn( line )) != -1 ) {
                        QString file = tagInclude.cap(1);
                        // Recurse.
                        levels--;
                        line.replace( offset, tagInclude.matchedLength(), parse(file) );
                        levels++;
                    }
                }
            }
        }
        if ( !skip )
            text += line + "\n";
    } while (!ts.atEnd());
    return text;
}

QString HelpPreProcessor::exec(const QString& cmd)
{
    // For security reasons, we do NOT execute arbitrary commands.
    QStringList arg = cmd.split(" ");
    if ( arg[0] == "qpe-list-content" ) {
        QString ctype = arg[1];
        ctype[0] = ctype[0].toUpper();
        return listContent(ctype);
    } else if ( arg[0] == "qpe-list-help-pages" ) {
        QString r;
        QString filter = arg[1];
        filter.remove(".."); filter.remove("/"); // security
        QRegExp title("<title>(.*)</title>");
        foreach (QString path, Qtopia::helpPaths()) {
            QDir dir(path,filter);
            for (int i=0; i<(int)dir.count(); ++i) {
                QString file = dir[i];
                QFile f(path+"/"+file);
                if (f.open(QIODevice::ReadOnly)) {
                    QString head;
                    while (!f.atEnd()) {
                        QString l = f.readLine();
                        head += l;
                        if ( l.indexOf("</title>") >= 0 )
                            break;
                        if ( l.indexOf("</head>") >= 0 )
                            break;
                    }
                    if (title.indexIn(head) >= 0)
                        r += "<li><a href=\""+file+"\">"+title.cap(1)+"</a>\n";
                }
            }
        }
        return r;
    }
    return "";
}

QString HelpPreProcessor::listContent( const QString& name )
{
    QString s;
    int size = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
    QContentSet lnkset( QContentFilter::Category, name );
    typedef QMap<QString,QContent> OrderingMap;
    OrderingMap ordered;
    QContentList linkList = lnkset.items();
    foreach (const QContent &lnk, linkList) {
        ordered[Qtopia::dehyphenate( lnk.name() )] = lnk;
    }
    for( OrderingMap::ConstIterator mit=ordered.begin(); mit!=ordered.end(); ++mit ) {
        QString name = mit.key();
        const QContent &lnk = *mit;
        QString icon = ":image/" + lnk.iconName();
        QString helpFile = lnk.property("File","Help");
        if (helpFile.isEmpty())
            helpFile = lnk.executableName() + ".html";
        QStringList helpPath = Qtopia::helpPaths();
        QStringList::ConstIterator it;
        const char* prefix[]={"","qpe-",0};
        int pref=0;
        for (; prefix[pref]; ++pref) {
            for (it = helpPath.begin(); it != helpPath.end() && !QFile::exists( *it + "/" + prefix[pref] + helpFile ); ++it)
                ;
            if (it != helpPath.end())
                break;
        }
        if (it != helpPath.end()) {
            // SVG/PIC images are forced to load at a particular size (see above)
            // Force all app icons to be this size (to prevent them from being
            // different sizes, not all app icons are SVG/PIC).
            s += QString("<br><a href=%1%2><img src=%3 width=%4 height=%5> %6</a>\n")
                .arg( prefix[pref] )
                .arg( helpFile )
                .arg( icon )
                .arg( size )
                .arg( size )
                .arg( name );
#ifdef DEBUG
        } else {
            s += QString("<br>No <tt>%1</tt> for %2\n")
                .arg( helpFile )
                .arg( name );
#endif
        }
    }
    return s;
}


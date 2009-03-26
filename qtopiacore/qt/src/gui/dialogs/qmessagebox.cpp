/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include <QtGui/qmessagebox.h>

#ifndef QT_NO_MESSAGEBOX

#include <QtGui/qdialogbuttonbox.h>
#include "private/qlabel_p.h"
#include <QtCore/qlist.h>
#include <QtCore/qdebug.h>
#include <QtGui/qstyle.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qgridlayout.h>
#include <QtGui/qdesktopwidget.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/qaccessible.h>
#include <QtGui/qicon.h>
#include <QtGui/qtextdocument.h>
#include <QtGui/qapplication.h>
#include <QtGui/qtextedit.h>
#include <QtGui/qmenu.h>
#include "qdialog_p.h"
#include <QtGui/qfont.h>
#include <QtGui/qfontmetrics.h>
#include <QtGui/qclipboard.h>

#ifdef Q_OS_WINCE
extern bool qt_wince_is_mobile();    //defined in qguifunctions_wince.cpp
extern bool qt_wince_is_smartphone();//defined in qguifunctions_wince.cpp
extern bool qt_wince_is_pocket_pc(); //defined in qguifunctions_wince.cpp

#include "qguifunctions_wince.h"
#endif

QT_BEGIN_NAMESPACE

extern QHash<QByteArray, QFont> *qt_app_fonts_hash();

enum Button { Old_Ok = 1, Old_Cancel = 2, Old_Yes = 3, Old_No = 4, Old_Abort = 5, Old_Retry = 6,
              Old_Ignore = 7, Old_YesAll = 8, Old_NoAll = 9, Old_ButtonMask = 0xFF,
              NewButtonFlag = 0xFFFFFC00 };

enum DetailButtonLabel { ShowLabel = 0, HideLabel = 1 };
#ifndef QT_NO_TEXTEDIT
class QMessageBoxDetailsText : public QWidget
{
public:
    class TextEdit : public QTextEdit
    {
    public:
        TextEdit(QWidget *parent=0) : QTextEdit(parent) { }
        void contextMenuEvent(QContextMenuEvent * e)
        {
#ifndef QT_NO_CONTEXTMENU
            QMenu *menu = createStandardContextMenu();
            menu->exec(e->globalPos());
            delete menu;
#else 
            Q_UNUSED(e);
#endif
        }
    };

    QMessageBoxDetailsText(QWidget *parent=0)
        : QWidget(parent)
    {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setMargin(0);
        QFrame *line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);
        textEdit = new TextEdit();
        textEdit->setFixedHeight(100);
        textEdit->setFocusPolicy(Qt::NoFocus);
        textEdit->setReadOnly(true);
        layout->addWidget(textEdit);
        setLayout(layout);
    }
    void setText(const QString &text) { textEdit->setPlainText(text); }
    QString text() const { return textEdit->toPlainText(); }
    QString label(DetailButtonLabel label)
        { return label == ShowLabel ? QMessageBox::tr("Show Details...")
                                    : QMessageBox::tr("Hide Details..."); }
private:
    TextEdit *textEdit;
};
#endif // QT_NO_TEXTEDIT

#ifndef QT_NO_IMAGEFORMAT_XPM
/* XPM */
static const char * const qtlogo_xpm[] = {
/* width height ncolors chars_per_pixel */
"64 64 337 2",
/* colors */
"  	c None",
". 	c #0C481E",
"+ 	c #0F4C1F",
"@ 	c #387B2A",
"# 	c #509730",
"$ 	c #549B31",
"% 	c #58A032",
"& 	c #59A133",
"* 	c #519730",
"= 	c #498F2E",
"- 	c #43872D",
"; 	c #2B6C26",
"> 	c #5BA333",
", 	c #66B036",
"' 	c #5CA433",
") 	c #529931",
"! 	c #4A8F2E",
"~ 	c #44882D",
"{ 	c #296926",
"] 	c #5DA634",
"^ 	c #539A31",
"/ 	c #4B902F",
"( 	c #44892D",
"_ 	c #175521",
": 	c #60A934",
"< 	c #5EA734",
"[ 	c #4B912F",
"} 	c #458A2D",
"| 	c #4C922F",
"1 	c #559C31",
"2 	c #0D491E",
"3 	c #62AB35",
"4 	c #61AA35",
"5 	c #569D32",
"6 	c #4D932F",
"7 	c #468B2D",
"8 	c #1A5822",
"9 	c #579E32",
"0 	c #4E9430",
"a 	c #478C2E",
"b 	c #347629",
"c 	c #58A033",
"d 	c #69AC43",
"e 	c #7BBA52",
"f 	c #72B646",
"g 	c #65AF35",
"h 	c #468B2E",
"i 	c #296927",
"j 	c #497F48",
"k 	c #9FBE9B",
"l 	c #F5F9F5",
"m 	c #FFFFFF",
"n 	c #F4F9F0",
"o 	c #D4E9C6",
"p 	c #ABD491",
"q 	c #6EB440",
"r 	c #1F5F23",
"s 	c #245F2A",
"t 	c #BCD3B8",
"u 	c #F5FAF2",
"v 	c #ADD594",
"w 	c #67B037",
"x 	c #59A032",
"y 	c #135020",
"z 	c #EDF2EB",
"A 	c #CFE6C0",
"B 	c #6DB43F",
"C 	c #66AF36",
"D 	c #124F20",
"E 	c #155221",
"F 	c #F7FBF5",
"G 	c #E7F3DF",
"H 	c #D7EACA",
"I 	c #9BCB7B",
"J 	c #175621",
"K 	c #145122",
"L 	c #DBE8D9",
"M 	c #D3E8C5",
"N 	c #39733B",
"O 	c #B8DAA1",
"P 	c #377A2A",
"Q 	c #749F70",
"R 	c #DAE8D5",
"S 	c #729D6F",
"T 	c #C7DAC3",
"U 	c #ACD491",
"V 	c #42862C",
"W 	c #6E9B6B",
"X 	c #5CA533",
"Y 	c #104B1F",
"Z 	c #1D5927",
"` 	c #F9FBF8",
" .	c #B1D799",
"..	c #4B902E",
"+.	c #104D1F",
"@.	c #3F773F",
"#.	c #FDFEFD",
"$.	c #F9FCF7",
"%.	c #2C6D26",
"&.	c #B5CDB2",
"*.	c #397C2A",
"=.	c #6F9C6C",
"-.	c #D7EBCB",
";.	c #67B138",
">.	c #3C7E2A",
",.	c #7CA477",
"'.	c #9ECD80",
").	c #175520",
"!.	c #114D1F",
"~.	c #FAFCFA",
"{.	c #1C5A23",
"].	c #D4E2D1",
"^.	c #98CA78",
"/.	c #63AD36",
"(.	c #155220",
"_.	c #135021",
":.	c #F1F6F0",
"<.	c #D5E9C8",
"[.	c #5BA433",
"}.	c #316A34",
"|.	c #5AA132",
"1.	c #25602B",
"2.	c #76B84A",
"3.	c #286925",
"4.	c #ACC7A7",
"5.	c #45823C",
"6.	c #0F4B1E",
"7.	c #9FBF9A",
"8.	c #D9E6D7",
"9.	c #D6EAC9",
"0.	c #94C872",
"a.	c #89C264",
"b.	c #73B747",
"c.	c #45892D",
"d.	c #51854F",
"e.	c #EDF6E7",
"f.	c #3B7E2A",
"g.	c #6D9B69",
"h.	c #91B688",
"i.	c #A6C2A2",
"j.	c #B3D89B",
"k.	c #699665",
"l.	c #CBE4BB",
"m.	c #4D942F",
"n.	c #3D753E",
"o.	c #D1E3C9",
"p.	c #337428",
"q.	c #82AA7F",
"r.	c #BCDDA8",
"s.	c #5DA533",
"t.	c #1F5B27",
"u.	c #E6F1E0",
"v.	c #2A6A26",
"w.	c #A0BF9B",
"x.	c #B6D99F",
"y.	c #63AB35",
"z.	c #185423",
"A.	c #377533",
"B.	c #2C6631",
"C.	c #D5E3D2",
"D.	c #D8E9CF",
"E.	c #B1D39F",
"F.	c #B7D6A6",
"G.	c #C1DDB2",
"H.	c #99CA7A",
"I.	c #216024",
"J.	c #BFD3BB",
"K.	c #AFD696",
"L.	c #0C491E",
"M.	c #69B23A",
"N.	c #286926",
"O.	c #C6D8C2",
"P.	c #185621",
"Q.	c #E3EBE0",
"R.	c #A9D28D",
"S.	c #104D20",
"T.	c #0F4B1F",
"U.	c #76B84B",
"V.	c #0F4D1F",
"W.	c #0E4A1E",
"X.	c #A2CF85",
"Y.	c #83BF5D",
"Z.	c #195524",
"`.	c #9BCC7C",
" +	c #EBF2EB",
".+	c #91C66E",
"++	c #1E5A27",
"@+	c #97C977",
"#+	c #DFEADC",
"$+	c #9CCC7D",
"%+	c #165222",
"&+	c #9DCD7F",
"*+	c #195822",
"=+	c #EAF1E9",
"-+	c #96C975",
";+	c #A4D087",
">+	c #165321",
",+	c #F7F9F7",
"'+	c #8BC367",
")+	c #AAD390",
"!+	c #114E1F",
"~+	c #0E4A1F",
"{+	c #80BE59",
"]+	c #1F5D23",
"^+	c #D0E0CF",
"/+	c #266625",
"(+	c #B7CFB4",
"_+	c #B7DAA0",
":+	c #64AE36",
"<+	c #155321",
"[+	c #6BB33D",
"}+	c #2D6D26",
"|+	c #BEDDA9",
"1+	c #195624",
"2+	c #FAFDF9",
"3+	c #347529",
"4+	c #88AE84",
"5+	c #D1E7C2",
"6+	c #E9F4E3",
"7+	c #468C2D",
"8+	c #EFF7EA",
"9+	c #488D2E",
"0+	c #487D47",
"a+	c #C3E0B0",
"b+	c #367929",
"c+	c #77A172",
"d+	c #9ACB7A",
"e+	c #B9DBA3",
"f+	c #256525",
"g+	c #B9D1B6",
"h+	c #91C66F",
"i+	c #B3CCAF",
"j+	c #FEFFFE",
"k+	c #286826",
"l+	c #C5D8C1",
"m+	c #CDE5BE",
"n+	c #40842C",
"o+	c #5E8E5B",
"p+	c #BFDEAB",
"q+	c #F4F8F3",
"r+	c #E2F0D8",
"s+	c #2D6F27",
"t+	c #AFCAAC",
"u+	c #E8F3E0",
"v+	c #559D32",
"w+	c #68AD40",
"x+	c #215D29",
"y+	c #FDFEFC",
"z+	c #80BD58",
"A+	c #357829",
"B+	c #8BAF86",
"C+	c #B2D79A",
"D+	c #8AAF85",
"E+	c #C0D7B6",
"F+	c #7AA475",
"G+	c #C7D9C3",
"H+	c #DDEED3",
"I+	c #246324",
"J+	c #DDEDD2",
"K+	c #7CBB52",
"L+	c #41852C",
"M+	c #114F20",
"N+	c #538651",
"O+	c #EAF4E3",
"P+	c #62915F",
"Q+	c #DEEED4",
"R+	c #145120",
"S+	c #1B5825",
"T+	c #E9EFE7",
"U+	c #F6F9F7",
"V+	c #9FBD9D",
"W+	c #E4ECE2",
"X+	c #8CC468",
"Y+	c #165322",
"Z+	c #4F9530",
"`+	c #0D491F",
" @	c #367039",
".@	c #C1DFAE",
"+@	c #4C9330",
"@@	c #497F49",
"#@	c #F7FBF4",
"$@	c #D0E7C1",
"%@	c #3C7F2B",
"&@	c #689664",
"*@	c #4F8C3F",
"=@	c #8EBB7C",
"-@	c #BCDCAB",
";@	c #89C265",
">@	c #68B139",
",@	c #EFF5EE",
"'@	c #E6F2DE",
")@	c #8AC365",
"!@	c #5FA834",
"~@	c #3F842C",
"{@	c #2D6B2D",
"]@	c #558853",
"^@	c #C3D7C0",
"/@	c #EEF6E8",
"(@	c #5BA233",
"_@	c #26622D",
":@	c #468A2D",
"<@	c #1D5B22",
"[@	c #729E6E",
"}@	c #FEFEFD",
"|@	c #D9EBCD",
"1@	c #ABD393",
"2@	c #8BC368",
"3@	c #589F33",
"4@	c #26612C",
"5@	c #FCFDFB",
"6@	c #C6E2B4",
"7@	c #63AC35",
"8@	c #3D802B",
"9@	c #669562",
"0@	c #276725",
"a@	c #A9C5A4",
"b@	c #65AE36",
"c@	c #2A6B26",
"d@	c #5A9052",
"e@	c #91B48D",
"f@	c #CADCC6",
"g@	c #F2F8EE",
"h@	c #A3CF86",
"i@	c #64AE35",
"j@	c #569E32",
"k@	c #62AC35",
"l@	c #579F32",
"m@	c #63AD35",
"n@	c #64AD35",
"o@	c #41862C",
"p@	c #59A132",
"q@	c #4F9630",
"r@	c #509630",
"s@	c #367829",
"t@	c #5AA233",
"u@	c #488E2E",
"v@	c #42872C",
/* pixels */
"                      . . + @ # $ % & * = -                                                                                     ",
"                  . . . ; > , , , , , , , , , , ' ) ! ~                                                                         ",
"              . . . . { , , , , , , , , , , , , , , , , , , ] ^ / (                                                             ",
"              . . . _ : , , , , , , , , , , , , , , , , , , , , , , , , < $ [ }                                                 ",
"            . . . . | , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , : 1 | }                                     ",
"            . . . 2 3 , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 4 5 6 7                         ",
"            . . . 8 , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 3 9 0 a             ",
"          . . . . ; , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , , , , , : c d e f , , , , , , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , g h i j k l m m m n o p q , , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , c r s t m m m m m m m m m u v w , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , x y s z m m m m m m m m m m m m A B , , , , , C D E F G H I , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , 3 J K L m m m m m m m m m m m m m m M w , , , , 9 . N m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , P . Q m m m m m m m R S T m m m m m m U , , , , V . W m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , , X Y Z ` m m m m m m  ., ..+.@.#.m m m m $.B , , , %.. &.m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , , *.. =.m m m m m m -.;., , >.. ,.m m m m m '., , , ).!.~.m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , , {.. ].m m m m m m ^., , , /.(._.:.m m m m <., , [.. }.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , |.. 1.m m m m m m m 2., , , , 3.. 4.m m m m #.5.6.7.].8.m m m m 9.'.0.a.b., , , , , 7           ",
"          . . . . b , , , , , , c.. d.m m m m m m e., , , , , f.. g.m m m m m h.. i.m m m m m m m m m m j., , , , , 7           ",
"          . . . . b , , , , , , >.. k.m m m m m m l., , , , , m.. n.m m m m m o.. i.m m m m m m m m m m j., , , , , 7           ",
"          . . . . b , , , , , , p.. q.m m m m m m r., , , , , s.. t.m m m m m u.. i.m m m m m m m m m m j., , , , , 7           ",
"          . . . . b , , , , , , v.. w.m m m m m m x., , , , , y.. z.m m m m m u * A.B.C.m m m m D.E.F.G.H., , , , , 7           ",
"          . . . . b , , , , , , I.. J.m m m m m m K., , , , , C L._.m m m m m m M.N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , P.. Q.m m m m m m R., , , , , , S.T.m m m m m m U.N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , V.W.#.m m m m m m X., , , , , , (.. ~.m m m m m Y.N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , 3 . Z.m m m m m m m `., , , , , , P..  +m m m m m .+N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , ] . ++m m m m m m m @+, , , , , , {.. #+m m m m m $+N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , /.. %+m m m m m m m &+, , , , , , *+. =+m m m m m -+N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , S.W.m m m m m m m ;+, , , , , , >+. ,+m m m m m '+N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , _ .  +m m m m m m )+, , , , , , !+~+m m m m m m {+N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , ]+. ^+m m m m m m  ., , , , , , W.!.m m m m m m U.N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , /+. (+m m m m m m _+, , , , , :+. <+m m m m m m [+N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , }+. k m m m m m m |+, , , , , 4 . 1+m m m m m 2+, N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , 3+. 4+m m m m m m 5+, , , , , |.. 1.m m m m m 6+, N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , 7+. d.m m m m m m 8+, , , , , 9+. 0+m m m m m a+, N.. O.m m m m O , , , , , , , , , 7           ",
"          . . . . b , , , , , , 4 W.1+m m m m m m m b., , , , b+. c+m m m m m d+, N.. O.m m m m e+, , , , , , , , , 7           ",
"          . . . . b , , , , , , , f+. g+m m m m m m h+, , , , f+. i+m m m m j+f , k+. l+m m m m m+, , , , , , , , , 7           ",
"          . . . . b , , , , , , , n+. o+m m m m m m p+, , , 3 !+<+q+m m m m r+, , s+. t+m m m m u+, 4 v+w+, , , , , 7           ",
"          . . . . b , , , , , , , [.. x+m m m m m m y+z+, , A+. B+m m m m m C+, , *.. D+m m m m m E+F+G+H+, , , , , 7           ",
"          . . . . b , , , , , , , , I+. &.m m m m m m J+K+L+M+N+m m m m m O+[+, , h . P+m m m m m m m m Q+, , , , , 7           ",
"          . . . . b , , , , , , , , s.R+S+T+m m m m m m U+V+W+m m m m m j+X+, , , :+).Y+:.m m m m m m m Q+, , , , , 7           ",
"          . . . . b , , , , , , , , , Z+`+ @#.m m m m m m m m m m m m m .@, , , , , +@!+@@l m m m m #@$@`., , , , , 7           ",
"          . . . . b , , , , , , , , , , %@. &@m m m m m m m m m m m j+a+B , , , , , , X n+*@=@-@C+;@>@, , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , - D d.,@m m m m m m m m '@)@, , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , , !@~@{@]@^@m m m m m /@M., , , , , , , , , , , , , , , , , , , , , , , , 7           ",
"          . . . . b , , , , , , , , , , , , , , (@. _@m m m m m m ;@, , , , , , , , , , , , , , , , , , , , , , , , :@          ",
"          . . . . b , , , , , , , , , , , , , , , <@L.C.m m m m m m+, , , , , , , , , , , , , , , , , , , , , , , , V           ",
"          . . . . b , , , , , , , , , , , , , , , *.. [@m m m m m }@|@1@2@, , , , , , , , , , , , , , , , , , , , ,             ",
"          . . . . b , , , , , , , , , , , , , , , 3@W.4@5@m m m m m m m 6@, , , , , , , , , , , , , , , , , , , , 7@            ",
"          . . . . b , , , , , , , , , , , , , , , , 8@. 9@m m m m m m m 6@, , , , , , , , , , , , , , , , , , , , [             ",
"          . . . . b , , , , , , , , , , , , , , , , C 0@~+a@m m m m m m 6@, , , , , , , , , , , , , , , , , , , b@              ",
"          . . . . b , , , , , , , , , , , , , , , , , , ..c@d@e@f@g@O+|@h@, , , , , , , , , , , , , , , , , , , (               ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , i@}                 ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 3 j@9+                    ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , , k@9 m.h                               ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , , , , , , , 7@l@0 a L+                                        ",
"          . . . . b , , , , , , , , , , , , , , , , , , , , , , , m@l@0 a L+                                                    ",
"            . . . b , , , , , , , , , , , , , , , , , n@% Z+a o@                                                                ",
"              . . b , , , , , , , , , , , i@p@q@9+V                                                                             ",
"                . b , , , , , :+& r@9+V                                                                                         ",
"                  s@t@r@u@v@                                                                                                    "};
#endif // QT_NO_IMAGEFORMAT_XPM

class QMessageBoxPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QMessageBox)

public:
    QMessageBoxPrivate() : escapeButton(0), defaultButton(0), clickedButton(0), detailsButton(0),
#ifndef QT_NO_TEXTEDIT
                           detailsText(0),
#endif
                           compatMode(false), autoAddOkButton(true),
                           detectedEscapeButton(0), informativeLabel(0) { }

    void init(const QString &title = QString(), const QString &text = QString());
    void _q_buttonClicked(QAbstractButton *);

    QAbstractButton *findButton(int button0, int button1, int button2, int flags);
    void addOldButtons(int button0, int button1, int button2);

    QAbstractButton *abstractButtonForId(int id) const;
    int execReturnCode(QAbstractButton *button);

    void detectEscapeButton();
    void updateSize();
    int layoutMinimumWidth();
    void retranslateStrings();

#ifdef Q_OS_WINCE
    void hideSpecial();
#endif

    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                 const QString &title, const QString &text,
                                 int button0, int button1, int button2);
    static int showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                 const QString &title, const QString &text,
                                 const QString &button0Text,
                                 const QString &button1Text,
                                 const QString &button2Text,
                                 int defaultButtonNumber,
                                 int escapeButtonNumber);

    static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
                QMessageBox::Icon icon, const QString& title, const QString& text,
                QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

    static QPixmap standardIcon(QMessageBox::Icon icon, QMessageBox *mb);

    QLabel *label;
    QMessageBox::Icon icon;
    QLabel *iconLabel;
    QDialogButtonBox *buttonBox;
    QList<QAbstractButton *> customButtonList;
    QAbstractButton *escapeButton;
    QPushButton *defaultButton;
    QAbstractButton *clickedButton;
    QPushButton *detailsButton;
#ifndef QT_NO_TEXTEDIT
    QMessageBoxDetailsText *detailsText;
#endif
    bool compatMode;
    bool autoAddOkButton;
    QAbstractButton *detectedEscapeButton;
    QLabel *informativeLabel;
};

static QString *translatedTextAboutQt = 0;

void QMessageBoxPrivate::init(const QString &title, const QString &text)
{
    Q_Q(QMessageBox);

    if (!translatedTextAboutQt) {
        translatedTextAboutQt = new QString;
    }

    label = new QLabel;
    label->setObjectName(QLatin1String("qt_msgbox_label"));
    label->setTextInteractionFlags(Qt::TextInteractionFlags(q->style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, q)));
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setOpenExternalLinks(true);
#if defined(Q_WS_MAC)
    label->setContentsMargins(16, 0, 0, 8);
#elif !defined(Q_WS_QWS)
    label->setContentsMargins(2, 0, 0, 0);
    label->setIndent(9);
#endif
    icon = QMessageBox::NoIcon;
    iconLabel = new QLabel;
    iconLabel->setObjectName(QLatin1String("qt_msgboxex_icon_label"));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    buttonBox = new QDialogButtonBox;
    buttonBox->setObjectName(QLatin1String("qt_msgbox_buttonbox"));
    buttonBox->setCenterButtons(q->style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, q));
    QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)),
                     q, SLOT(_q_buttonClicked(QAbstractButton*)));

    QGridLayout *grid = new QGridLayout;
#ifndef Q_WS_MAC
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    grid->addWidget(label, 0, 1, 1, 1);
    // -- leave space for information label --
    grid->addWidget(buttonBox, 2, 0, 1, 2);
#else
    grid->setMargin(0);
    grid->setSpacing(0);
    q->setContentsMargins(24, 15, 24, 20);
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignLeft);
    grid->addWidget(label, 0, 1, 1, 1);
    // -- leave space for information label --
    grid->setRowStretch(1, 100);
    grid->setRowMinimumHeight(2, 10);
    grid->addWidget(buttonBox, 3, 1, 1, 1);
#endif

    grid->setSizeConstraint(QLayout::SetNoConstraint);
    q->setLayout(grid);

    if (!title.isEmpty() || !text.isEmpty()) {
        q->setWindowTitle(title);
        q->setText(text);
    }
    q->setModal(true);

#ifdef Q_WS_MAC
    QFont f = q->font();
    f.setBold(true);
    label->setFont(f);
#endif
    retranslateStrings();
}

int QMessageBoxPrivate::layoutMinimumWidth()
{
    Q_Q(QMessageBox);

    q->layout()->activate();
    return q->layout()->totalMinimumSize().width();
}

void QMessageBoxPrivate::updateSize()
{
    Q_Q(QMessageBox);

    if (!q->isVisible())
        return;

    QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();
#ifdef Q_WS_QWS
    // the width of the screen, less the window border.
    int hardLimit = screenSize.width() - (q->frameGeometry().width() - q->geometry().width());
#elif defined(Q_OS_WINCE)
    // the width of the screen, less the window border.
    int hardLimit = screenSize.width() - (q->frameGeometry().width() - q->geometry().width());
#else
    int hardLimit = qMin(screenSize.width() - 480, 1000); // can never get bigger than this
#endif
#ifdef Q_WS_MAC
    int softLimit = qMin(screenSize.width()/2, 420);
#elif defined(Q_WS_QWS)
    int softLimit = qMin(hardLimit, 500);
#else
    // note: ideally on windows, hard and soft limits but it breaks compat
#ifndef Q_OS_WINCE
    int softLimit = qMin(screenSize.width()/2, 500);
#else
    int softLimit = qMin(screenSize.width() * 3 / 4, 500);
#endif //Q_OS_WINCE
#endif

    if (informativeLabel)
        informativeLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    label->setWordWrap(false); // makes the label return min size
    int width = layoutMinimumWidth();

    if (width > softLimit) {
        label->setWordWrap(true);
        width = qMax(softLimit, layoutMinimumWidth());

        if (width > hardLimit) {
            label->d_func()->ensureTextControl();
            if (QTextControl *control = label->d_func()->control) {
                QTextOption opt = control->document()->defaultTextOption();
                opt.setWrapMode(QTextOption::WrapAnywhere);
                control->document()->setDefaultTextOption(opt);
            }
            width = hardLimit;
        }
    }

    if (informativeLabel) {
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QSizePolicy policy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        policy.setHeightForWidth(true);
        informativeLabel->setSizePolicy(policy);
        width = qMax(width, layoutMinimumWidth());
        if (width > hardLimit) { // longest word is really big, so wrap anywhere
            informativeLabel->d_func()->ensureTextControl();
            if (QTextControl *control = informativeLabel->d_func()->control) {
                QTextOption opt = control->document()->defaultTextOption();
                opt.setWrapMode(QTextOption::WrapAnywhere);
                control->document()->setDefaultTextOption(opt);
            }
            width = hardLimit;
        }
        policy.setHeightForWidth(label->wordWrap());
        label->setSizePolicy(policy);
    }

    QFontMetrics fm(qApp->font("QWorkspaceTitleBar"));
    int windowTitleWidth = qMin(fm.width(q->windowTitle()) + 50, hardLimit);
    if (windowTitleWidth > width)
        width = windowTitleWidth;

    q->layout()->activate();
    int height = (q->layout()->hasHeightForWidth())
                     ? q->layout()->totalHeightForWidth(width)
                     : q->layout()->totalMinimumSize().height();
    q->setFixedSize(width, height);
    QCoreApplication::removePostedEvents(q, QEvent::LayoutRequest);
}


#ifdef Q_OS_WINCE
/*!
  \internal
  Hides special buttons which are rather shown in the title bar
  on WinCE, to conserve screen space.
*/

void QMessageBoxPrivate::hideSpecial()
{
    Q_Q(QMessageBox);
    QList<QPushButton*> list = qFindChildren<QPushButton*>(q);
        for (int i=0; i<list.size(); ++i) {
            QPushButton *pb = list.at(i);
            QString text = pb->text();
            text.remove(QChar::fromLatin1('&'));          
            if (text == qApp->translate("QMessageBox", "OK" ))
                pb->setFixedSize(0,0);
        }
}
#endif

static int oldButton(int button)
{
    switch (button & QMessageBox::ButtonMask) {
    case QMessageBox::Ok:
        return Old_Ok;
    case QMessageBox::Cancel:
        return Old_Cancel;
    case QMessageBox::Yes:
        return Old_Yes;
    case QMessageBox::No:
        return Old_No;
    case QMessageBox::Abort:
        return Old_Abort;
    case QMessageBox::Retry:
        return Old_Retry;
    case QMessageBox::Ignore:
        return Old_Ignore;
    case QMessageBox::YesToAll:
        return Old_YesAll;
    case QMessageBox::NoToAll:
        return Old_NoAll;
    default:
        return 0;
    }
}

int QMessageBoxPrivate::execReturnCode(QAbstractButton *button)
{
    int ret = buttonBox->standardButton(button);
    if (ret == QMessageBox::NoButton) {
        ret = customButtonList.indexOf(button); // if button == 0, correctly sets ret = -1
    } else if (compatMode) {
        ret = oldButton(ret);
    }
    return ret;
}

void QMessageBoxPrivate::_q_buttonClicked(QAbstractButton *button)
{
    Q_Q(QMessageBox);
#ifndef QT_NO_TEXTEDIT
    if (detailsButton && detailsText && button == detailsButton) {
        detailsButton->setText(detailsText->isHidden() ? detailsText->label(HideLabel) : detailsText->label(ShowLabel));
        detailsText->setHidden(!detailsText->isHidden());
        updateSize();
    } else
#endif
    {
        clickedButton = button;
        q->done(execReturnCode(button)); // does not trigger closeEvent
    }
}

/*!
    \class QMessageBox
    
    \brief The QMessageBox class provides a modal dialog for informing
    the user or for asking the user a question and receiving an answer.

    \ingroup dialogs
    \mainclass

    A message box displays a primary \l{QMessageBox::text}{text} to
    alert the user to a situation, an \l{QMessageBox::informativeText}
    {informative text} to further explain the alert or to ask the user
    a question, and an optional \l{QMessageBox::detailedText}
    {detailed text} to provide even more data if the user requests
    it. A message box can also display an \l{QMessageBox::icon} {icon}
    and \l{QMessageBox::standardButtons} {standard buttons} for
    accepting a user response.

    Two APIs for using QMessageBox are provided, the property-based
    API, and the static functions. Calling one of the static functions
    is the simpler approach, but it is less flexible than using the
    property-based API, and the result is less informative. Using the
    property-based API is recommended.

    \section1 The Property-based API

    To use the property-based API, construct an instance of
    QMessageBox, set the desired properties, and call exec() to show
    the message. The simplest configuration is to set only the
    \l{QMessageBox::text} {message text} property.

    \snippet doc/src/snippets/code/src_gui_dialogs_qmessagebox.cpp 5

    The user must click the \gui{OK} button to dismiss the message
    box. The rest of the GUI is blocked until the message box is
    dismissed.
    
    \image msgbox1.png

    A better approach than just alerting the user to an event is to
    also ask the user what to do about it. Store the question in the
    \l{QMessageBox::informativeText} {informative text} property, and
    set the \l{QMessageBox::standardButtons} {standard buttons}
    property to the set of buttons you want as the set of user
    responses. The buttons are specified by combining values from
    StandardButtons using the bitwise OR operator. The display order
    for the buttons is platform-dependent. For example, on Windows,
    \gui{Save} is displayed to the left of \gui{Cancel}, whereas on
    Mac OS, the order is reversed.

    Mark one of your standard buttons to be your
    \l{QMessageBox::defaultButton()} {default button}.

    \snippet doc/src/snippets/code/src_gui_dialogs_qmessagebox.cpp 6

    This is the approach recommended in the
    \l{http://developer.apple.com/documentation/UserExperience/Conceptual/AppleHIGuidelines/XHIGWindows/chapter_18_section_7.html}
    {Mac OS X Guidlines}. Similar guidlines apply for the other
    platforms, but note the different ways the
    \l{QMessageBox::informativeText} {informative text} is handled for
    different platforms.
    
    \image msgbox2.png

    The exec() slot returns the StandardButtons value of the button
    that was clicked. 
    
    \snippet doc/src/snippets/code/src_gui_dialogs_qmessagebox.cpp 7

    To give the user more information to help him answer the question,
    set the \l{QMessageBox::detailedText} {detailed text} property. If
    the \l{QMessageBox::detailedText} {detailed text} property is set,
    the \gui{Show Details...} button will be shown.

    \image msgbox3.png

    Clicking the \gui{Show Details...} button displays the detailed text.

    \image msgbox4.png

    \section2 Rich Text and the Text Format Property

    The \l{QMessageBox::detailedText} {detailed text} property is
    always interpreted as plain text. The \l{QMessageBox::text} {main
    text} and \l{QMessageBox::informativeText} {informative text}
    properties can be either plain text or rich text. These strings
    are interpreted according to the setting of the
    \l{QMessageBox::textFormat} {text format} property. The default
    setting is \l{Qt::AutoText} {auto-text}.

    Note that for some plain text strings containing XML
    meta-characters, the auto-text \l{Qt::mightBeRichText()} {rich
    text detection test} may fail causing your plain text string to be
    interpreted incorrectly as rich text. In these rare cases, use
    Qt::convertFromPlainText() to convert your plain text string to a
    visually equivalent rich text string, or set the
    \l{QMessageBox::textFormat} {text format} property explicitly with
    setTextFormat().

    \section2 Severity Levels and the Icon and Pixmap Properties

    QMessageBox supports four predefined message severity levels, or
    message types, which really only differ in the predefined icon
    they each show. Specify one of the four predefined message types
    by setting the \l{QMessageBox::icon} {icon} property to one of the
    \l{QMessageBox::Icon} {predefined Icons}. The following rules are
    guidelines:

    \table
    \row
    \o \img qmessagebox-quest.png
    \o \l Question
    \o For asking a question during normal operations.
    \row
    \o \img qmessagebox-info.png
    \o \l Information
    \o For reporting information about normal operations.
    \row
    \o \img qmessagebox-warn.png
    \o \l Warning
    \o For reporting non-critical errors.
    \row
    \o \img qmessagebox-crit.png
    \o \l Critical
    \o For reporting critical errors.
    \endtable

    The default value is \l{QMessageBox::NoIcon} {No Icon}. The
    message boxes are otherwise the same for all cases. When using a
    standard icon, use the one recommended in the table, or use the
    one recommended by the style guidelines for your platform. If none
    of the standard icons is right for your message box, you can use a
    custom icon by setting the \l{QMessageBox::iconPixmap} {icon
    pixmap} property instead of setting the \l{QMessageBox::icon}
    {icon} property.

    In summary, to set an icon, use \e{either} setIcon() for one of
    the standard icons, \e{or} setIconPixmap() for a custom icon.

    \section1 The Static Functions API

    Building message boxes with the static functions API, although
    convenient, is less flexible than using the property-based API,
    because the static function signatures lack parameters for setting
    the \l{QMessageBox::informativeText} {informative text} and
    \l{QMessageBox::detailedText} {detailed text} properties. One
    work-around for this has been to use the \c{title} parameter as
    the message box main text and the \c{text} parameter as the
    message box informative text. Because this has the obvious
    drawback of making a less readable message box, platform
    guidelines do not recommend it. The \e{Microsoft Windows User
    Interface Guidelines} recommend using the
    \l{QCoreApplication::applicationName} {application name} as the
    \l{QMessageBox::setWindowTitle()} {window's title}, which means
    that if you have an informative text in addition to your main
    text, you must concatenate it to the \c{text} parameter.

    Note that the static function signatures have changed with respect
    to their button parameters, which are now used to set the
    \l{QMessageBox::standardButtons} {standard buttons} and the
    \l{QMessageBox::defaultButton()} {default button}.

    Static functions are available for creating information(),
    question(), warning(), and critical() message boxes.

    \snippet doc/src/snippets/code/src_gui_dialogs_qmessagebox.cpp 0

    The \l{dialogs/standarddialogs}{Standard Dialogs} example shows
    how to use QMessageBox and the other built-in Qt dialogs.

    \section1 Advanced Usage

    If the \l{QMessageBox::StandardButtons} {standard buttons} are not
    flexible enough for your message box, you can use the addButton()
    overload that takes a text and a ButtonRoleto to add custom
    buttons. The ButtonRole is used by QMessageBox to determine the
    ordering of the buttons on screen (which varies according to the
    platform). You can test the value of clickedButton() after calling
    exec(). For example,

    \snippet doc/src/snippets/code/src_gui_dialogs_qmessagebox.cpp 2

    \section1 Default and Escape Keys

    The default button (i.e., the button activated when \key Enter is
    pressed) can be specified using setDefaultButton(). If a default
    button is not specified, QMessageBox tries to find one based on
    the \l{ButtonRole} {button roles} of the buttons used in the
    message box.

    The escape button (the button activated when \key Esc is pressed)
    can be specified using setEscapeButton().  If an escape button is
    not specified, QMessageBox tries to find one using these rules:

    \list 1

    \o If there is only one button, it is the button activated when
    \key Esc is pressed.

    \o If there is a \l Cancel button, it is the button activated when
    \key Esc is pressed.

    \o If there is exactly one button having either
       \l{QMessageBox::RejectRole} {the Reject role} or the
       \l{QMessageBox::NoRole} {the No role}, it is the button
       activated when \key Esc is pressed.

    \endlist

    When an escape button can't be determined using these rules,
    pressing \key Esc has no effect.

    \sa QDialogButtonBox, {fowler}{GUI Design Handbook: Message Box}, {Standard Dialogs Example}, {Application Example}
*/

/*!
    \enum QMessageBox::StandardButton
    \since 4.2

    These enums describe flags for standard buttons. Each button has a
    defined \l ButtonRole.

    \value Ok An "OK" button defined with the \l AcceptRole.
    \value Open A "Open" button defined with the \l AcceptRole.
    \value Save A "Save" button defined with the \l AcceptRole.
    \value Cancel A "Cancel" button defined with the \l RejectRole.
    \value Close A "Close" button defined with the \l RejectRole.
    \value Discard A "Discard" or "Don't Save" button, depending on the platform,
                    defined with the \l DestructiveRole.
    \value Apply An "Apply" button defined with the \l ApplyRole.
    \value Reset A "Reset" button defined with the \l ResetRole.
    \value RestoreDefaults A "Restore Defaults" button defined with the \l ResetRole.
    \value Help A "Help" button defined with the \l HelpRole.
    \value SaveAll A "Save All" button defined with the \l AcceptRole.
    \value Yes A "Yes" button defined with the \l YesRole.
    \value YesToAll A "Yes to All" button defined with the \l YesRole.
    \value No A "No" button defined with the \l NoRole.
    \value NoToAll A "No to All" button defined with the \l NoRole.
    \value Abort An "Abort" button defined with the \l RejectRole.
    \value Retry A "Retry" button defined with the \l AcceptRole.
    \value Ignore An "Ignore" button defined with the \l AcceptRole.

    \value NoButton An invalid button.

    \omitvalue FirstButton
    \omitvalue LastButton

    The following values are obsolete:

    \value YesAll Use YesToAll instead.
    \value NoAll Use NoToAll instead.
    \value Default Use the \c defaultButton argument of
           information(), warning(), etc. instead, or call
           setDefaultButton().
    \value Escape Call setEscapeButton() instead.
    \value FlagMask
    \value ButtonMask

    \sa ButtonRole, standardButtons
*/

/*!
    Constructs a message box with no text and no buttons. \a parent is
    passed to the QDialog constructor.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    On Mac OS X, if \a parent is not 0 and you want your message box
    to appear as a Qt::Sheet of that parent, set the message box's
    \l{setWindowModality()} {window modality} to Qt::WindowModal
    (default). Otherwise, the message box will be a standard dialog.

*/
QMessageBox::QMessageBox(QWidget *parent)
: QDialog(*new QMessageBoxPrivate, parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init();
}

/*!
    Constructs a message box with the given \a icon, \a title, \a
    text, and standard \a buttons. Standard or custom buttons can be
    added at any time using addButton(). The \a parent and \a f
    arguments are passed to the QDialog constructor.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    On Mac OS X, if \a parent is not 0 and you want your message box
    to appear as a Qt::Sheet of that parent, set the message box's
    \l{setWindowModality()} {window modality} to Qt::WindowModal
    (default). Otherwise, the message box will be a standard dialog.

    \sa setWindowTitle(), setText(), setIcon(), setStandardButtons()
*/
QMessageBox::QMessageBox(Icon icon, const QString &title, const QString &text,
                         StandardButtons buttons, QWidget *parent,
                         Qt::WindowFlags f)
: QDialog(*new QMessageBoxPrivate, parent, f | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init(title, text);
    setIcon(icon);
    if (buttons != NoButton)
        setStandardButtons(buttons);
}

/*!
    Destroys the message box.
*/
QMessageBox::~QMessageBox()
{
}

/*!
    \since 4.2

    Adds the given \a button to the message box with the specified \a
    role.

    \sa removeButton(), button(), setStandardButtons()
*/
void QMessageBox::addButton(QAbstractButton *button, ButtonRole role)
{
    Q_D(QMessageBox);
    if (!button)
        return;
    removeButton(button);
    d->buttonBox->addButton(button, (QDialogButtonBox::ButtonRole)role);
    d->customButtonList.append(button);
    d->autoAddOkButton = false;
}

/*!
    \since 4.2
    \overload

    Creates a button with the given \a text, adds it to the message box for the
    specified \a role, and returns it.
*/
QPushButton *QMessageBox::addButton(const QString& text, ButtonRole role)
{
    Q_D(QMessageBox);
    QPushButton *pushButton = new QPushButton(text);
    addButton(pushButton, role);
    d->updateSize();
    return pushButton;
}

/*!
    \since 4.2
    \overload

    Adds a standard \a button to the message box if it is valid to do so, and
    returns the push button.

    \sa setStandardButtons()
*/
QPushButton *QMessageBox::addButton(StandardButton button)
{
    Q_D(QMessageBox);
    QPushButton *pushButton = d->buttonBox->addButton((QDialogButtonBox::StandardButton)button);
    if (pushButton)
        d->autoAddOkButton = false;
    return pushButton;
}

/*!
    \since 4.2

    Removes \a button from the button box without deleting it.

    \sa addButton(), setStandardButtons()
*/
void QMessageBox::removeButton(QAbstractButton *button)
{
    Q_D(QMessageBox);
    d->customButtonList.removeAll(button);
    if (d->escapeButton == button)
        d->escapeButton = 0;
    if (d->defaultButton == button)
        d->defaultButton = 0;
    d->buttonBox->removeButton(button);
    d->updateSize();
}

/*!
    \property QMessageBox::standardButtons
    \brief collection of standard buttons in the message box
    \since 4.2

    This property controls which standard buttons are used by the message box.

    By default, this property contains no standard buttons.

    \sa addButton()
*/
void QMessageBox::setStandardButtons(StandardButtons buttons)
{
    Q_D(QMessageBox);
    d->buttonBox->setStandardButtons(QDialogButtonBox::StandardButtons(int(buttons)));

    QList<QAbstractButton *> buttonList = d->buttonBox->buttons();
    if (!buttonList.contains(d->escapeButton))
        d->escapeButton = 0;
    if (!buttonList.contains(d->defaultButton))
        d->defaultButton = 0;
    d->autoAddOkButton = false;
    d->updateSize();
}

QMessageBox::StandardButtons QMessageBox::standardButtons() const
{
    Q_D(const QMessageBox);
    return QMessageBox::StandardButtons(int(d->buttonBox->standardButtons()));
}

/*!
    \since 4.2

    Returns the standard button enum value corresponding to the given \a button,
    or NoButton if the given \a button isn't a standard button.

    \sa button(), standardButtons()
*/
QMessageBox::StandardButton QMessageBox::standardButton(QAbstractButton *button) const
{
    Q_D(const QMessageBox);
    return (QMessageBox::StandardButton)d->buttonBox->standardButton(button);
}

/*!
    \since 4.2

    Returns a pointer corresponding to the standard button \a which,
    or 0 if the standard button doesn't exist in this message box.

    \sa standardButtons, standardButton()
*/
QAbstractButton *QMessageBox::button(StandardButton which) const
{
    Q_D(const QMessageBox);
    return d->buttonBox->button(QDialogButtonBox::StandardButton(which));
}

/*!
    \since 4.2

    Returns the button that is activated when escape is pressed.

    By default, QMessageBox attempts to automatically detect an
    escape button as follows:

    \list 1
    \o If there is only one button, it is made the escape button.
    \o If there is a \l Cancel button, it is made the escape button.
    \o On Mac OS X only, if there is exactly one button with the role
       QMessageBox::RejectRole, it is made the escape button.
    \endlist

    When an escape button could not be automatically detected, pressing
    \key Esc has no effect.

    \sa addButton()
*/
QAbstractButton *QMessageBox::escapeButton() const
{
    Q_D(const QMessageBox);
    return d->escapeButton;
}

/*!
    \since 4.2

    Sets the button that gets activated when the \key Escape key is
    pressed to \a button.

    \sa addButton(), clickedButton()
*/
void QMessageBox::setEscapeButton(QAbstractButton *button)
{
    Q_D(QMessageBox);
    if (d->buttonBox->buttons().contains(button))
        d->escapeButton = button;
}

/*!
    \since 4.3

    Sets the buttons that gets activated when the \key Escape key is
    pressed to \a button.

    \sa addButton(), clickedButton()
*/
void QMessageBox::setEscapeButton(QMessageBox::StandardButton button)
{
    Q_D(QMessageBox);
    setEscapeButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

void QMessageBoxPrivate::detectEscapeButton()
{
    if (escapeButton) { // escape button explicitly set
        detectedEscapeButton = escapeButton;
        return;
    }

    // Cancel button automatically becomes escape button
    detectedEscapeButton = buttonBox->button(QDialogButtonBox::Cancel);
    if (detectedEscapeButton)
        return;

    // If there is only one button, make it the escape button
    const QList<QAbstractButton *> buttons = buttonBox->buttons();
    if (buttons.count() == 1) {
        detectedEscapeButton = buttons.first();
        return;
    }

    // if the message box has one RejectRole button, make it the escape button
    for (int i = 0; i < buttons.count(); i++) {
        if (buttonBox->buttonRole(buttons.at(i)) == QDialogButtonBox::RejectRole) {
            if (detectedEscapeButton) { // already detected!
                detectedEscapeButton = 0;
                break;
            }
            detectedEscapeButton = buttons.at(i);
        }
    }
    if (detectedEscapeButton)
        return;

    // if the message box has one NoRole button, make it the escape button
    for (int i = 0; i < buttons.count(); i++) {
        if (buttonBox->buttonRole(buttons.at(i)) == QDialogButtonBox::NoRole) {
            if (detectedEscapeButton) { // already detected!
                detectedEscapeButton = 0;
                break;
            }
            detectedEscapeButton = buttons.at(i);
        }
    }
}

/*!
    \since 4.2

    Returns the button that was clicked by the user,
    or 0 if the user hit the \key Esc key and
    no \l{setEscapeButton()}{escape button} was set.

    If exec() hasn't been called yet, returns 0.

    Example:

    \snippet doc/src/snippets/code/src_gui_dialogs_qmessagebox.cpp 3

    \sa standardButton(), button()
*/
QAbstractButton *QMessageBox::clickedButton() const
{
    Q_D(const QMessageBox);
    return d->clickedButton;
}

/*!
    \since 4.2

    Returns the button that should be the message box's
    \l{QPushButton::setDefault()}{default button}. Returns 0
    if no default button was set.

    \sa addButton(), QPushButton::setDefault()
*/
QPushButton *QMessageBox::defaultButton() const
{
    Q_D(const QMessageBox);
    return d->defaultButton;
}

/*!
    \since 4.2

    Sets the message box's \l{QPushButton::setDefault()}{default button}
    to \a button.

    \sa addButton(), QPushButton::setDefault()
*/
void QMessageBox::setDefaultButton(QPushButton *button)
{
    Q_D(QMessageBox);
    if (!d->buttonBox->buttons().contains(button))
        return;
    d->defaultButton = button;
    button->setDefault(true);
    button->setFocus();
}

/*!
    \since 4.3

    Sets the message box's \l{QPushButton::setDefault()}{default button}
    to \a button.

    \sa addButton(), QPushButton::setDefault()
*/
void QMessageBox::setDefaultButton(QMessageBox::StandardButton button)
{
    Q_D(QMessageBox);
    setDefaultButton(d->buttonBox->button(QDialogButtonBox::StandardButton(button)));
}

/*!
  \property QMessageBox::text
  \brief the message box text to be displayed.

  The text will be interpreted either as a plain text or as rich text,
  depending on the text format setting (\l QMessageBox::textFormat).
  The default setting is Qt::AutoText, i.e., the message box will try
  to auto-detect the format of the text.

  The default value of this property is an empty string.

  \sa textFormat, QMessageBox::informativeText, QMessageBox::detailedText
*/
QString QMessageBox::text() const
{
    Q_D(const QMessageBox);
    return d->label->text();
}

void QMessageBox::setText(const QString &text)
{
    Q_D(QMessageBox);
    d->label->setText(text);
    d->label->setWordWrap(d->label->textFormat() == Qt::RichText
        || (d->label->textFormat() == Qt::AutoText && Qt::mightBeRichText(text)));
    d->updateSize();
}

/*!
    \enum QMessageBox::Icon

    This enum has the following values:

    \value NoIcon the message box does not have any icon.

    \value Question an icon indicating that
    the message is asking a question.

    \value Information an icon indicating that
    the message is nothing out of the ordinary.

    \value Warning an icon indicating that the
    message is a warning, but can be dealt with.

    \value Critical an icon indicating that
    the message represents a critical problem.

*/

/*!
    \property QMessageBox::icon
    \brief the message box's icon

    The icon of the message box can be specified with one of the
    values:
    
    \list
    \o QMessageBox::NoIcon
    \o QMessageBox::Question
    \o QMessageBox::Information
    \o QMessageBox::Warning
    \o QMessageBox::Critical
    \endlist

    The default is QMessageBox::NoIcon.

    The pixmap used to display the actual icon depends on the current
    \l{QWidget::style()} {GUI style}. You can also set a custom pixmap
    for the icon by setting the \l{QMessageBox::iconPixmap} {icon
    pixmap} property.

    \sa iconPixmap
*/
QMessageBox::Icon QMessageBox::icon() const
{
    Q_D(const QMessageBox);
    return d->icon;
}

void QMessageBox::setIcon(Icon icon)
{
    Q_D(QMessageBox);
    setIconPixmap(QMessageBoxPrivate::standardIcon((QMessageBox::Icon)icon,
                                                   this));
    d->icon = icon;
}

/*!
    \property QMessageBox::iconPixmap
    \brief the current icon

    The icon currently used by the message box. Note that it's often
    hard to draw one pixmap that looks appropriate in all GUI styles;
    you may want to supply a different pixmap for each platform.

    By default, this property is undefined.

    \sa icon
*/
QPixmap QMessageBox::iconPixmap() const
{
    Q_D(const QMessageBox);
    if (d->iconLabel && d->iconLabel->pixmap())
        return *d->iconLabel->pixmap();
    return QPixmap();
}

void QMessageBox::setIconPixmap(const QPixmap &pixmap)
{
    Q_D(QMessageBox);
    d->iconLabel->setPixmap(pixmap);
    d->updateSize();
    d->icon = NoIcon;
}

/*!
    \property QMessageBox::textFormat
    \brief the format of the text displayed by the message box

    The current text format used by the message box. See the \l
    Qt::TextFormat enum for an explanation of the possible options.

    The default format is Qt::AutoText.

    \sa setText()
*/
Qt::TextFormat QMessageBox::textFormat() const
{
    Q_D(const QMessageBox);
    return d->label->textFormat();
}

void QMessageBox::setTextFormat(Qt::TextFormat format)
{
    Q_D(QMessageBox);
    d->label->setTextFormat(format);
    d->label->setWordWrap(format == Qt::RichText
                    || (format == Qt::AutoText && Qt::mightBeRichText(d->label->text())));
    d->updateSize();
}

/*!
    \reimp
*/
bool QMessageBox::event(QEvent *e)
{
    bool result =QDialog::event(e);
    switch (e->type()) {
        case QEvent::LayoutRequest:
            d_func()->updateSize();
            break;
        case QEvent::LanguageChange:
            d_func()->retranslateStrings();
            break;
#ifdef Q_OS_WINCE
        case QEvent::OkRequest:
        case QEvent::HelpRequest: {
          QString bName =
              (e->type() == QEvent::OkRequest)
              ? qApp->translate("QMessageBox", "OK")
              : qApp->translate("QMessageBox", "Help");
          QList<QPushButton*> list = qFindChildren<QPushButton*>(this);
          for (int i=0; i<list.size(); ++i) {
              QPushButton *pb = list.at(i);
              if (pb->text() == bName) {
                  if (pb->isEnabled())
                      pb->click();
                  return pb->isEnabled();
              }
          } 
        }
#endif
        default:
            break;
    }
    return result;
}

/*!
    \reimp
*/
void QMessageBox::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
}

/*!
    \reimp
*/
void QMessageBox::closeEvent(QCloseEvent *e)
{
    Q_D(QMessageBox);
    if (!d->detectedEscapeButton) {
        e->ignore();
        return;
    }
    QDialog::closeEvent(e);
    d->clickedButton = d->detectedEscapeButton;
    setResult(d->execReturnCode(d->detectedEscapeButton));
}

/*!
    \reimp
*/
void QMessageBox::changeEvent(QEvent *ev)
{
    Q_D(QMessageBox);
    switch (ev->type()) {
    case QEvent::StyleChange:
    {
        if (d->icon != NoIcon)
            setIcon(d->icon);
        Qt::TextInteractionFlags flags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this));
        d->label->setTextInteractionFlags(flags);
        d->buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));
        if (d->informativeLabel)
            d->informativeLabel->setTextInteractionFlags(flags);
        // intentional fall through
    }
    case QEvent::FontChange:
    case QEvent::ApplicationFontChange:
#ifdef Q_WS_MAC
    {
        QFont f = font();
        f.setBold(true);
        d->label->setFont(f);
    }
#endif
    default:
        break;
    }
    QDialog::changeEvent(ev);
}

/*!
    \reimp
*/
void QMessageBox::keyPressEvent(QKeyEvent *e)
{
    Q_D(QMessageBox);
    if (e->key() == Qt::Key_Escape
#ifdef Q_WS_MAC
        || (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_Period)
#endif
        ) {
            if (d->detectedEscapeButton) {
#ifdef Q_WS_MAC
                d->detectedEscapeButton->animateClick();
#else
                d->detectedEscapeButton->click();
#endif
            }
            return;
        }

#if defined (Q_OS_WIN) && !defined(QT_NO_CLIPBOARD) && !defined(QT_NO_SHORTCUT)
        if (e == QKeySequence::Copy) {
            QString separator = QString::fromLatin1("---------------------------\n");
            QString textToCopy = separator;
            separator.prepend(QLatin1String("\n"));
            textToCopy += windowTitle() + separator; // title
            textToCopy += d->label->text() + separator; // text

            if (d->informativeLabel)
                textToCopy += d->informativeLabel->text() + separator;

            QString buttonTexts;
            QList<QAbstractButton *> buttons = d->buttonBox->buttons();
            for (int i = 0; i < buttons.count(); i++) {
                buttonTexts += buttons[i]->text() + QLatin1String("   ");
            }
            textToCopy += buttonTexts + separator;

            qApp->clipboard()->setText(textToCopy);
            return;
        }
#endif //QT_NO_SHORTCUT QT_NO_CLIPBOARD Q_OS_WIN

#ifndef QT_NO_SHORTCUT
    if (!(e->modifiers() & Qt::AltModifier)) {
        int key = e->key() & ~((int)Qt::MODIFIER_MASK|(int)Qt::UNICODE_ACCEL);
        if (key) {
            const QList<QAbstractButton *> buttons = d->buttonBox->buttons();
            for (int i = 0; i < buttons.count(); ++i) {
                QAbstractButton *pb = buttons.at(i);
                int acc = pb->shortcut() & ~((int)Qt::MODIFIER_MASK|(int)Qt::UNICODE_ACCEL);
                if (acc == key) {
                    pb->animateClick();
                    return;
                }
            }
        }
    }
#endif
    QDialog::keyPressEvent(e);
}

#ifdef Q_OS_WINCE
/*!
    \reimp
*/
void QMessageBox::setVisible(bool visible)
{
    Q_D(QMessageBox);
    if (visible)
        d->hideSpecial();
    QDialog::setVisible(visible);
}
#endif

/*!
    \reimp
*/
void QMessageBox::showEvent(QShowEvent *e)
{
    Q_D(QMessageBox);
    if (d->autoAddOkButton) {
        addButton(Ok);
#if defined(Q_OS_WINCE)
        d->hideSpecial();
#endif
    }
    if (d->detailsButton)
        addButton(d->detailsButton, QMessageBox::ActionRole);
    d->detectEscapeButton();
    d->updateSize();

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::Alert);
#endif
#ifdef Q_WS_WIN
    HMENU systemMenu = GetSystemMenu((HWND)winId(), FALSE);
    if (!d->detectedEscapeButton) {
        EnableMenuItem(systemMenu, SC_CLOSE, MF_BYCOMMAND|MF_GRAYED);
    }
    else {
        EnableMenuItem(systemMenu, SC_CLOSE, MF_BYCOMMAND|MF_ENABLED);
    }
#endif
    QDialog::showEvent(e);
}


static QMessageBox::StandardButton showNewMessageBox(QWidget *parent,
    QMessageBox::Icon icon,
    const QString& title, const QString& text,
    QMessageBox::StandardButtons buttons,
    QMessageBox::StandardButton defaultButton)
{
    // necessary for source compatibility with Qt 4.0 and 4.1
    // handles (Yes, No) and (Yes|Default, No)
    if (defaultButton && !(buttons & defaultButton))
        return (QMessageBox::StandardButton)
                    QMessageBoxPrivate::showOldMessageBox(parent, icon, title,
                                                            text, int(buttons),
                                                            int(defaultButton), 0);

    QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);
    QDialogButtonBox *buttonBox = qFindChild<QDialogButtonBox*>(&msgBox);
    Q_ASSERT(buttonBox != 0);

    uint mask = QMessageBox::FirstButton;
    while (mask <= QMessageBox::LastButton) {
        uint sb = buttons & mask;
        mask <<= 1;
        if (!sb)
            continue;
        QPushButton *button = msgBox.addButton((QMessageBox::StandardButton)sb);
        // Choose the first accept role as the default
        if (msgBox.defaultButton())
            continue;
        if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
            || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
            msgBox.setDefaultButton(button);
    }
    if (msgBox.exec() == -1)
        return QMessageBox::Cancel;
    return msgBox.standardButton(msgBox.clickedButton());
}

/*!
    \since 4.2

    Opens an information message box with the specified \a title and
    \a text. The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \key Enter is
    pressed. If \a defaultButton is QMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \key Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa question(), warning(), critical()
*/
QMessageBox::StandardButton QMessageBox::information(QWidget *parent, const QString &title,
                               const QString& text, StandardButtons buttons,
                               StandardButton defaultButton)
{
    return showNewMessageBox(parent, Information, title, text, buttons,
                             defaultButton);
}


/*!
    \since 4.2

    Opens a question message box with the specified \a title and \a
    text. The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \key Enter is
    pressed. If \a defaultButton is QMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \key Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), warning(), critical()
*/
QMessageBox::StandardButton QMessageBox::question(QWidget *parent, const QString &title,
                            const QString& text, StandardButtons buttons,
                            StandardButton defaultButton)
{
    return showNewMessageBox(parent, Question, title, text, buttons, defaultButton);
}

/*!
    \since 4.2

    Opens a warning message box with the specified \a title and \a
    text. The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \key Enter is
    pressed. If \a defaultButton is QMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \key Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.
    
    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa question(), information(), critical()
*/
QMessageBox::StandardButton QMessageBox::warning(QWidget *parent, const QString &title,
                        const QString& text, StandardButtons buttons,
                        StandardButton defaultButton)
{
    return showNewMessageBox(parent, Warning, title, text, buttons, defaultButton);
}

/*!
    \since 4.2

    Opens a critical message box with the specified \a title and \a
    text. The standard \a buttons are added to the message box. \a
    defaultButton specifies the button used when \key Enter is
    pressed. If \a defaultButton is QMessageBox::NoButton, QMessageBox
    chooses a suitable default automatically.

    Returns the identity of the standard button that was clicked. If
    \key Esc was pressed instead, the \l{Default and Escape Keys}
    {escape button} is returned.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa question(), warning(), information()
*/
QMessageBox::StandardButton QMessageBox::critical(QWidget *parent, const QString &title,
                         const QString& text, StandardButtons buttons,
                         StandardButton defaultButton)
{
    return showNewMessageBox(parent, Critical, title, text, buttons, defaultButton);
}

/*!
    Displays a simple about box with title \a title and text \a
    text. The about box's parent is \a parent.

    about() looks for a suitable icon in four locations:

    \list 1
    \o It prefers \link QWidget::windowIcon() parent->icon() \endlink
    if that exists.
    \o If not, it tries the top-level widget containing \a parent.
    \o If that fails, it tries the \link
    QApplication::activeWindow() active window. \endlink
    \o As a last resort it uses the Information icon.
    \endlist

    The about box has a single button labelled "OK".

    \sa QWidget::windowIcon() QApplication::activeWindow()
*/
void QMessageBox::about(QWidget *parent, const QString &title,
                        const QString &text)
{
    QMessageBox mb(title, text, Information, Ok + Default, 0, 0, parent);
    QIcon icon = mb.windowIcon();
    QSize size = icon.actualSize(QSize(64, 64));
    mb.setIconPixmap(icon.pixmap(size));
    mb.exec();
}

/*!
    Displays a simple message box about Qt, with the given \a title
    and centered over \a parent (if \a parent is not 0). The message
    includes the version number of Qt being used by the application.

    This is useful for inclusion in the \gui Help menu of an application,
    as shown in the \l{mainwindows/menus}{Menus} example.

    QApplication provides this functionality as a slot.

    \sa QApplication::aboutQt()
*/
void QMessageBox::aboutQt(QWidget *parent, const QString &title)
{
    QMessageBox mb(parent);

    QString c = title;
    if (c.isEmpty())
        c = tr("About Qt");
    mb.setWindowTitle(c);
    mb.setText(*translatedTextAboutQt);
#ifndef QT_NO_IMAGEFORMAT_XPM
    QImage logo(qtlogo_xpm);
#else
    QImage logo;
#endif

    if (qGray(mb.palette().color(QPalette::Active, QPalette::Text).rgb()) >
        qGray(mb.palette().color(QPalette::Active, QPalette::Base).rgb()))
    {
        // light on dark, adjust some colors
        logo.setColor(0, 0xffffffff);
        logo.setColor(1, 0xff666666);
        logo.setColor(2, 0xffcccc66);
        logo.setColor(4, 0xffcccccc);
        logo.setColor(6, 0xffffff66);
        logo.setColor(7, 0xff999999);
        logo.setColor(8, 0xff3333ff);
        logo.setColor(9, 0xffffff33);
        logo.setColor(11, 0xffcccc99);
    }
    QPixmap pm = QPixmap::fromImage(logo);
    if (!pm.isNull())
        mb.setIconPixmap(pm);
#if defined(Q_OS_WINCE)
    mb.setDefaultButton(mb.addButton(QMessageBox::Ok));
#else
    mb.addButton(QMessageBox::Ok);
#endif
    mb.exec();
}

/*!
    \internal
*/
QSize QMessageBox::sizeHint() const
{
    // ### Qt 5: remove
    return QDialog::sizeHint();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Source and binary compatibility routines for 4.0 and 4.1

static QMessageBox::StandardButton newButton(int button)
{
    // this is needed for source compatibility with Qt 4.0 and 4.1
    if (button == QMessageBox::NoButton || (button & NewButtonFlag))
        return QMessageBox::StandardButton(button & QMessageBox::ButtonMask);

#if QT_VERSION < 0x050000
    // this is needed for binary compatibility with Qt 4.0 and 4.1
    switch (button & Old_ButtonMask) {
    case Old_Ok:
        return QMessageBox::Ok;
    case Old_Cancel:
        return QMessageBox::Cancel;
    case Old_Yes:
        return QMessageBox::Yes;
    case Old_No:
        return QMessageBox::No;
    case Old_Abort:
        return QMessageBox::Abort;
    case Old_Retry:
        return QMessageBox::Retry;
    case Old_Ignore:
        return QMessageBox::Ignore;
    case Old_YesAll:
        return QMessageBox::YesToAll;
    case Old_NoAll:
        return QMessageBox::NoToAll;
    default:
        return QMessageBox::NoButton;
    }
#endif
}

static bool detectedCompat(int button0, int button1, int button2)
{
    if (button0 != 0 && !(button0 & NewButtonFlag))
        return true;
    if (button1 != 0 && !(button1 & NewButtonFlag))
        return true;
    if (button2 != 0 && !(button2 & NewButtonFlag))
        return true;
    return false;
}

QAbstractButton *QMessageBoxPrivate::findButton(int button0, int button1, int button2, int flags)
{
    Q_Q(QMessageBox);
    int button = 0;

    if (button0 & flags) {
        button = button0;
    } else if (button1 & flags) {
        button = button1;
    } else if (button2 & flags) {
        button = button2;
    }
    return q->button(newButton(button));
}

void QMessageBoxPrivate::addOldButtons(int button0, int button1, int button2)
{
    Q_Q(QMessageBox);
    q->addButton(newButton(button0));
    q->addButton(newButton(button1));
    q->addButton(newButton(button2));
    q->setDefaultButton(
        static_cast<QPushButton *>(findButton(button0, button1, button2, QMessageBox::Default)));
    q->setEscapeButton(findButton(button0, button1, button2, QMessageBox::Escape));
    compatMode = detectedCompat(button0, button1, button2);
}

QAbstractButton *QMessageBoxPrivate::abstractButtonForId(int id) const
{
    Q_Q(const QMessageBox);
    QAbstractButton *result = customButtonList.value(id);
    if (result)
        return result;
    if (id & QMessageBox::FlagMask)    // for compatibility with Qt 4.0/4.1 (even if it is silly)
        return 0;
    return q->button(newButton(id));
}

int QMessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                          const QString &title, const QString &text,
                                          int button0, int button1, int button2)
{
    QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
    messageBox.d_func()->addOldButtons(button0, button1, button2);
    return messageBox.exec();
}

int QMessageBoxPrivate::showOldMessageBox(QWidget *parent, QMessageBox::Icon icon,
                                            const QString &title, const QString &text,
                                            const QString &button0Text,
                                            const QString &button1Text,
                                            const QString &button2Text,
                                            int defaultButtonNumber,
                                            int escapeButtonNumber)
{
    QMessageBox messageBox(icon, title, text, QMessageBox::NoButton, parent);
    QString myButton0Text = button0Text;
    if (myButton0Text.isEmpty())
        myButton0Text = QDialogButtonBox::tr("OK");
    messageBox.addButton(myButton0Text, QMessageBox::ActionRole);
    if (!button1Text.isEmpty())
        messageBox.addButton(button1Text, QMessageBox::ActionRole);
    if (!button2Text.isEmpty())
        messageBox.addButton(button2Text, QMessageBox::ActionRole);

    const QList<QAbstractButton *> &buttonList = messageBox.d_func()->customButtonList;
    messageBox.setDefaultButton(static_cast<QPushButton *>(buttonList.value(defaultButtonNumber)));
    messageBox.setEscapeButton(buttonList.value(escapeButtonNumber));

    return messageBox.exec();
}

void QMessageBoxPrivate::retranslateStrings()
{
#ifndef QT_NO_TEXTEDIT
    if (detailsButton)
        detailsButton->setText(detailsText->isHidden() ? detailsText->label(HideLabel) : detailsText->label(ShowLabel));
#endif

#if defined(QT_NON_COMMERCIAL)
    QT_NC_MSGBOX
#else
    *translatedTextAboutQt = QMessageBox::tr(
        "<h3>About Qt</h3>"
        "%1<p>Qt is a C++ toolkit for cross-platform "
        "application development.</p>"
        "<p>Qt provides single-source "
        "portability across MS&nbsp;Windows, Mac&nbsp;OS&nbsp;X, "
        "Linux, and all major commercial Unix variants. Qt is also"
        " available for embedded devices as Qt Embedded.</p>"
        "<p>Qt is a Trolltech product. See "
        "<a href=\"http://www.trolltech.com/qt/\">www.trolltech.com/qt/</a> for more information.</p>"
       )
#if QT_EDITION != QT_EDITION_OPENSOURCE
       .arg(QMessageBox::tr("<p>This program uses Qt version %1.</p>"))
#else
       .arg(QMessageBox::tr("<p>This program uses Qt Open Source Edition version %1.</p>"
               "<p>Qt Open Source Edition is intended for the development "
               "of Open Source applications. You need a commercial Qt "
               "license for development of proprietary (closed source) "
               "applications.</p>"
               "<p>Please see <a href=\"http://www.trolltech.com/company/model/\">www.trolltech.com/company/model/</a> "
               "for an overview of Qt licensing.</p>"))
#endif
       .arg(QLatin1String(QT_VERSION_STR));
#endif

}

/*!
    \obsolete

    Constructs a message box with a \a title, a \a text, an \a icon,
    and up to three buttons.

    The \a icon must be one of the following:
    \list
    \o QMessageBox::NoIcon
    \o QMessageBox::Question
    \o QMessageBox::Information
    \o QMessageBox::Warning
    \o QMessageBox::Critical
    \endlist

    Each button, \a button0, \a button1 and \a button2, can have one
    of the following values:
    \list
    \o QMessageBox::NoButton
    \o QMessageBox::Ok
    \o QMessageBox::Cancel
    \o QMessageBox::Yes
    \o QMessageBox::No
    \o QMessageBox::Abort
    \o QMessageBox::Retry
    \o QMessageBox::Ignore
    \o QMessageBox::YesAll
    \o QMessageBox::NoAll
    \endlist

    Use QMessageBox::NoButton for the later parameters to have fewer
    than three buttons in your message box. If you don't specify any
    buttons at all, QMessageBox will provide an Ok button.

    One of the buttons can be OR-ed with the QMessageBox::Default
    flag to make it the default button (clicked when Enter is
    pressed).

    One of the buttons can be OR-ed with the QMessageBox::Escape flag
    to make it the cancel or close button (clicked when \key Esc is
    pressed).

    \snippet doc/src/snippets/dialogs/dialogs.cpp 2

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    The \a parent and \a f arguments are passed to
    the QDialog constructor.

    \sa setWindowTitle(), setText(), setIcon()
*/
QMessageBox::QMessageBox(const QString &title, const QString &text, Icon icon,
                             int button0, int button1, int button2, QWidget *parent,
                             Qt::WindowFlags f)
    : QDialog(*new QMessageBoxPrivate, parent,
              f /*| Qt::MSWindowsFixedSizeDialogHint #### */| Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    Q_D(QMessageBox);
    d->init(title, text);
    setIcon(icon);
    d->addOldButtons(button0, button1, button2);
}

/*!
    \obsolete

    Opens an information message box with the given \a title and the
    \a text. The dialog may have up to three buttons. Each of the
    buttons, \a button0, \a button1 and \a button2 may be set to one
    of the following values:

    \list
    \o QMessageBox::NoButton
    \o QMessageBox::Ok
    \o QMessageBox::Cancel
    \o QMessageBox::Yes
    \o QMessageBox::No
    \o QMessageBox::Abort
    \o QMessageBox::Retry
    \o QMessageBox::Ignore
    \o QMessageBox::YesAll
    \o QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa question(), warning(), critical()
*/
int QMessageBox::information(QWidget *parent, const QString &title, const QString& text,
                               int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays an information message box with the given \a title and
    \a text, as well as one, two or three buttons. Returns the index
    of the button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be
    used. \a button1Text is the text of the second button, and is
    optional. \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the escape button; pressing
    \key Esc is the same as clicking this button. It defaults to -1;
    supply 0, 1 or 2 to make pressing \key Esc equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa question(), warning(), critical()
*/

int QMessageBox::information(QWidget *parent, const QString &title, const QString& text,
                               const QString& button0Text, const QString& button1Text,
                               const QString& button2Text, int defaultButtonNumber,
                               int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Information, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}

/*!
    \obsolete

    Opens a question message box with the given \a title and \a text.
    The dialog may have up to three buttons. Each of the buttons, \a
    button0, \a button1 and \a button2 may be set to one of the
    following values:

    \list
    \o QMessageBox::NoButton
    \o QMessageBox::Ok
    \o QMessageBox::Cancel
    \o QMessageBox::Yes
    \o QMessageBox::No
    \o QMessageBox::Abort
    \o QMessageBox::Retry
    \o QMessageBox::Ignore
    \o QMessageBox::YesAll
    \o QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Yes, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), warning(), critical()
*/
int QMessageBox::question(QWidget *parent, const QString &title, const QString& text,
                            int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays a question message box with the given \a title and \a
    text, as well as one, two or three buttons. Returns the index of
    the button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional.
    \a button2Text is the text of the third button, and is optional.
    \a defaultButtonNumber (0, 1 or 2) is the index of the default
    button; pressing Return or Enter is the same as clicking the
    default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1 or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), warning(), critical()
*/
int QMessageBox::question(QWidget *parent, const QString &title, const QString& text,
                            const QString& button0Text, const QString& button1Text,
                            const QString& button2Text, int defaultButtonNumber,
                            int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Question, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}


/*!
    \obsolete

    Opens a warning message box with the given \a title and \a text.
    The dialog may have up to three buttons. Each of the button
    parameters, \a button0, \a button1 and \a button2 may be set to
    one of the following values:

    \list
    \o QMessageBox::NoButton
    \o QMessageBox::Ok
    \o QMessageBox::Cancel
    \o QMessageBox::Yes
    \o QMessageBox::No
    \o QMessageBox::Abort
    \o QMessageBox::Retry
    \o QMessageBox::Ignore
    \o QMessageBox::YesAll
    \o QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok or QMessageBox::No or ...)
    of the button that was clicked.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), question(), critical()
*/
int QMessageBox::warning(QWidget *parent, const QString &title, const QString& text,
                           int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
                                                   button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays a warning message box with the given \a title and \a
    text, as well as one, two, or three buttons. Returns the number
    of the button that was clicked (0, 1, or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional,
    and \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1, or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), question(), critical()
*/
int QMessageBox::warning(QWidget *parent, const QString &title, const QString& text,
                           const QString& button0Text, const QString& button1Text,
                           const QString& button2Text, int defaultButtonNumber,
                           int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Warning, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}

/*!
    \obsolete

    Opens a critical message box with the given \a title and \a text.
    The dialog may have up to three buttons. Each of the button
    parameters, \a button0, \a button1 and \a button2 may be set to
    one of the following values:

    \list
    \o QMessageBox::NoButton
    \o QMessageBox::Ok
    \o QMessageBox::Cancel
    \o QMessageBox::Yes
    \o QMessageBox::No
    \o QMessageBox::Abort
    \o QMessageBox::Retry
    \o QMessageBox::Ignore
    \o QMessageBox::YesAll
    \o QMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to QMessageBox::NoButton.

    One button can be OR-ed with QMessageBox::Default, and one
    button can be OR-ed with QMessageBox::Escape.

    Returns the identity (QMessageBox::Ok, or QMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), question(), warning()
*/

int QMessageBox::critical(QWidget *parent, const QString &title, const QString& text,
                          int button0, int button1, int button2)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text,
                                                 button0, button1, button2);
}

/*!
    \obsolete
    \overload

    Displays a critical error message box with the given \a title and
    \a text, as well as one, two, or three buttons. Returns the
    number of the button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional,
    and \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1, or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box is an \l{Qt::ApplicationModal}
    {application modal} dialog box. If \a parent is a widget, the
    message box is \l{Qt::WindowModal} {window modal} relative to \a
    parent.

    \sa information(), question(), warning()
*/
int QMessageBox::critical(QWidget *parent, const QString &title, const QString& text,
                            const QString& button0Text, const QString& button1Text,
                            const QString& button2Text, int defaultButtonNumber,
                            int escapeButtonNumber)
{
    return QMessageBoxPrivate::showOldMessageBox(parent, Critical, title, text,
                                                   button0Text, button1Text, button2Text,
                                                   defaultButtonNumber, escapeButtonNumber);
}


/*!
    \obsolete

    Returns the text of the message box button \a button, or
    an empty string if the message box does not contain the button.

    Use button() and QPushButton::text() instead.
*/
QString QMessageBox::buttonText(int button) const
{
    Q_D(const QMessageBox);

    if (QAbstractButton *abstractButton = d->abstractButtonForId(button)) {
        return abstractButton->text();
    } else if (d->buttonBox->buttons().isEmpty() && (button == Ok || button == Old_Ok)) {
        // for compatibility with Qt 4.0/4.1
        return QDialogButtonBox::tr("OK");
    }
    return QString();
}

/*!
    \obsolete

    Sets the text of the message box button \a button to \a text.
    Setting the text of a button that is not in the message box is
    silently ignored.

    Use addButton() instead.
*/
void QMessageBox::setButtonText(int button, const QString &text)
{
    Q_D(QMessageBox);
    if (QAbstractButton *abstractButton = d->abstractButtonForId(button)) {
        abstractButton->setText(text);
    } else if (d->buttonBox->buttons().isEmpty() && (button == Ok || button == Old_Ok)) {
        // for compatibility with Qt 4.0/4.1
        addButton(QMessageBox::Ok)->setText(text);
    }
}

#ifndef QT_NO_TEXTEDIT
/*!
  \property QMessageBox::detailedText
  \brief the text to be displayed in the details area.
  \since 4.2

  The text will be interpreted as a plain text.

  By default, this property contains an empty string.

  \sa QMessageBox::text, QMessageBox::informativeText
*/
QString QMessageBox::detailedText() const
{
    Q_D(const QMessageBox);
    return d->detailsText ? d->detailsText->text() : QString();
}

void QMessageBox::setDetailedText(const QString &text)
{
    Q_D(QMessageBox);
    if (text.isEmpty()) {
        delete d->detailsText;
        d->detailsText = 0;
        removeButton(d->detailsButton);
        delete d->detailsButton;
        d->detailsButton = 0;
        return;
    }

    if (!d->detailsText) {
        d->detailsText = new QMessageBoxDetailsText(this);
        QGridLayout* grid = qobject_cast<QGridLayout*>(layout());
        if (grid)
            grid->addWidget(d->detailsText, grid->rowCount(), 0, 1, grid->columnCount());
        d->detailsText->hide();
    }
    if (!d->detailsButton) {
        d->detailsButton = new QPushButton(d->detailsText->label(ShowLabel), this);
        QPushButton hideDetails(d->detailsText->label(HideLabel));
        d->detailsButton->setFixedSize(d->detailsButton->sizeHint().expandedTo(hideDetails.sizeHint()));
    }
    d->detailsText->setText(text);
}
#endif // QT_NO_TEXTEDIT

/*!
  \property QMessageBox::informativeText

  \brief the informative text that provides a fuller description for
  the message

  \since 4.2

  Infromative text can be used to expand upon the text() to give more
  information to the user. On the Mac, this text appears in small
  system font below the text().  On other platforms, it is simply
  appended to the existing text.

  By default, this property contains an empty string.

  \sa QMessageBox::text, QMessageBox::detailedText
*/
QString QMessageBox::informativeText() const
{
    Q_D(const QMessageBox);
    return d->informativeLabel ? d->informativeLabel->text() : QString();
}

void QMessageBox::setInformativeText(const QString &text)
{
    Q_D(QMessageBox);
    if (text.isEmpty()) {
        layout()->removeWidget(d->informativeLabel);
        delete d->informativeLabel;
        d->informativeLabel = 0;
#ifndef Q_WS_MAC
        d->label->setContentsMargins(2, 0, 0, 0);
#endif
        d->updateSize();
        return;
    }

    if (!d->informativeLabel) {
        QLabel *label = new QLabel;
        label->setObjectName(QLatin1String("qt_msgbox_informativelabel"));
        label->setTextInteractionFlags(Qt::TextInteractionFlags(style()->styleHint(QStyle::SH_MessageBox_TextInteractionFlags, 0, this)));
        label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        label->setOpenExternalLinks(true);
        label->setWordWrap(true);
#ifndef Q_WS_MAC
        d->label->setContentsMargins(2, 0, 0, 0);
        label->setContentsMargins(2, 0, 0, 6);
        label->setIndent(9);
#else
        label->setContentsMargins(16, 0, 0, 0);
        // apply a smaller font the information label on the mac
        label->setFont(qt_app_fonts_hash()->value("QTipLabel"));
#endif
        label->setWordWrap(true);
        QGridLayout *grid = static_cast<QGridLayout *>(layout());
        grid->addWidget(label, 1, 1, 1, 1);
        d->informativeLabel = label;
    }
    d->informativeLabel->setText(text);
    d->updateSize();
}

/*!
    \since 4.2

    This function shadows QWidget::setWindowTitle().

    Sets the title of the message box to \a title. On Mac OS X,
    the window title is ignored (as required by the Mac OS X
    Guidelines).
*/
void QMessageBox::setWindowTitle(const QString &title)
{
    // Message boxes on the mac do not have a title
#ifndef Q_WS_MAC
    QDialog::setWindowTitle(title);
#else
    Q_UNUSED(title);
#endif
}


/*!
    \since 4.2

    This function shadows QWidget::setWindowModality().

    Sets the modality of the message box to \a windowModality.

    On Mac OS X, if the modality is set to Qt::WindowModal and the message box
    has a parent, then the message box will be a Qt::Sheet, otherwise the
    message box will be a standard dialog.
*/
void QMessageBox::setWindowModality(Qt::WindowModality windowModality)
{
    QDialog::setWindowModality(windowModality);

    if (parentWidget() && windowModality == Qt::WindowModal)
        setParent(parentWidget(), Qt::Sheet);
    else
        setParent(parentWidget(), Qt::Dialog);
    setDefaultButton(d_func()->defaultButton);
}

#ifdef QT3_SUPPORT
/*!
    \compat

    Constructs a message box with the given \a parent, \a name, and
    window flags, \a f.
    The window title is specified by \a title, and the message box
    displays message text and an icon specified by \a text and \a icon.

    The buttons that the user can access to respond to the message are
    defined by \a button0, \a button1, and \a button2.
*/
QMessageBox::QMessageBox(const QString& title,
                         const QString &text, Icon icon,
                         int button0, int button1, int button2,
                         QWidget *parent, const char *name,
                         bool modal, Qt::WindowFlags f)
    : QDialog(*new QMessageBoxPrivate, parent,
              f | Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
    Q_D(QMessageBox);
    setObjectName(QString::fromAscii(name));
    d->init(title, text);
    d->addOldButtons(button0, button1, button2);
    setModal(modal);
    setIcon(icon);
}

/*!
    \compat
    Constructs a message box with the given \a parent and \a name.
*/
QMessageBox::QMessageBox(QWidget *parent, const char *name)
    : QDialog(*new QMessageBoxPrivate, parent,
              Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WStyle_SysMenu)
{
    Q_D(QMessageBox);
    setObjectName(QString::fromAscii(name));
    d->init();
}

/*!
  Returns the pixmap used for a standard icon. This
  allows the pixmaps to be used in more complex message boxes.
  \a icon specifies the required icon, e.g. QMessageBox::Information,
  QMessageBox::Warning or QMessageBox::Critical.

  \a style is unused.
*/

QPixmap QMessageBox::standardIcon(Icon icon, Qt::GUIStyle style)
{
    Q_UNUSED(style);
    return QMessageBox::standardIcon(icon);
}

/*!
    \fn int QMessageBox::message(const QString &title, const QString &text,
                                 const QString &buttonText, QWidget *parent = 0,
                                 const char *name = 0)

    Opens a modal message box with the given \a title and showing the
    given \a text. The message box has a single button which has the
    given \a buttonText (or tr("OK")). The message box is centred over
    its \a parent and is called \a name.

    Use information(), warning(), question(), or critical() instead.

    \oldcode
        QMessageBox::message(tr("My App"), tr("All occurrences replaced."),
                             tr("Close"), this);
    \newcode
        QMessageBox::information(this, tr("My App"),
                                 tr("All occurrences replaced."),
                                 QMessageBox::Close);
    \endcode
*/

/*!
    \fn bool QMessageBox::query(const QString &caption,
                                const QString& text,
                                const QString& yesButtonText,
                                const QString& noButtonText,
                                QWidget *parent, const char *name)

    \obsolete

    Queries the user using a modal message box with up to two buttons.
    The message box has the given \a caption (although some window
    managers don't show it), and shows the given \a text. The left
    button has the \a yesButtonText (or tr("OK")), and the right button
    has the \a noButtonText (or isn't shown). The message box is centred
    over its \a parent and is called \a name.

    Use information(), question(), warning(), or critical() instead.
*/

#endif

QPixmap QMessageBoxPrivate::standardIcon(QMessageBox::Icon icon, QMessageBox *mb)
{
    QStyle *style = mb ? mb->style() : QApplication::style();
    int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, mb);
    QIcon tmpIcon;
    switch (icon) {
    case QMessageBox::Information:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, mb);
        break;
    case QMessageBox::Warning:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, mb);
        break;
    case QMessageBox::Critical:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, mb);
        break;
    case QMessageBox::Question:
        tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mb);
    default:
        break;
    }
    if (!tmpIcon.isNull())
        return tmpIcon.pixmap(iconSize, iconSize);
    return QPixmap();
}

/*!
    \obsolete

    Returns the pixmap used for a standard icon. This allows the
    pixmaps to be used in more complex message boxes. \a icon
    specifies the required icon, e.g. QMessageBox::Question,
    QMessageBox::Information, QMessageBox::Warning or
    QMessageBox::Critical.

    Call QStyle::pixelMetric() with QStyle::SP_MessageBoxInformation etc.
    instead.
*/

QPixmap QMessageBox::standardIcon(Icon icon)
{
    return QMessageBoxPrivate::standardIcon(icon, 0);
}

/*!
    \typedef QMessageBox::Button
    \obsolete

    Use QMessageBox::StandardButton instead.
*/

/*!
    \fn int QMessageBox::information(QWidget *parent, const QString &title,
                                     const QString& text, StandardButton button0,
                                     StandardButton button1)
    \fn int QMessageBox::warning(QWidget *parent, const QString &title,
                                 const QString& text, StandardButton button0,
                                 StandardButton button1)
    \fn int QMessageBox::critical(QWidget *parent, const QString &title,
                                  const QString& text, StandardButton button0,
                                  StandardButton button1)
    \fn int QMessageBox::question(QWidget *parent, const QString &title,
                                  const QString& text, StandardButton button0,
                                  StandardButton button1)
    \internal

    ### Needed for Qt 4 source compatibility
*/

/*!
  \fn int QMessageBox::exec()

  Shows the message box as a \l{QDialog#Modal Dialogs}{modal dialog},
  blocking until the user closes it.

  When using a QMessageBox with standard buttons, this functions returns a
  \l StandardButton value indicating the standard button that was clicked.
  When using QMessageBox with custom buttons, this function returns an
  opaque value; use clickedButton() to determine which button was clicked.

  Users cannot interact with any other window in the same
  application until they close the dialog, either by clicking a
  button or by using a mechanism provided by the window system.

  \sa show(), result()
*/

QT_END_NAMESPACE

#include "moc_qmessagebox.cpp"

#endif // QT_NO_MESSAGEBOX

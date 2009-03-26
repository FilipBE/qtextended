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

/* Qt includes */
#include <QVariant>
#include <QBuffer>
#include <QByteArray>
#include <QHash>
#include <QVector>

/* System includes */
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Qt Extended includes */
#include "qexpressionevaluator.h"
#include "qfixedpointnumber_p.h"

#include <qvaluespace.h>

/*
    The expression framework is basically a simple compiler and software CPU
    system.  The compiler operates in 3 passes. First it tokenizes the input
    and reports any lexical errors (unexpected characters, unexpected end of
    input etc).  Second, it parses the input into a tree structure, enforcing
    expression syntax and precedence, and also performs semantic checking at
    this stage.  The third pass does code generation. Tokenization is
    represented by the class ExpressionTokenizer, parsing by ExpressionParser
    and semantic checking by ExpressionSemanticAnalyser, and the last stage of
    code generation is represented by ExpressionCodeGenerator. The intermeddiate
    tree representation is represented by the class ExpressionParserNode, and is
    passed from the ExpressionParser to the ExpressionCodeGenerator.
    ExpressionCodeGenerator, generates a list of instructions, executed from
    left to the right, and a list of data operands, which the instructions
    operate on, which is referenced left to right.

    This execution is performed by ExpressionMachine class. When the
    ExpressionMachine encounters a Load instruction, it takes data from the
    left of the data list and pushes it on the stack. Different instructions
    expect different types of parameters to be on the stack. Left to right
    associativity is represented in the stack as bottom to top - that is, data
    that is to the left of some other data in the expression is deeper in the
    stack. This means for a binary operation, the top value is the rvalue.
    Static coercion occurs on constants in the expression, eg for 3.0 * 3, 3
    will be be coerced to double during the semantic checking phase.

    In the case of variable data('Terms'), return types of expressions are
    checkecd to ensure runtime coercion can take place correctly. For example
    in the expression 2.0 * ("20" . ".5") the semantic checker sees that the
    lhs of the operation * is a double and the rhs is a string, the result of
    the . string operation.  The string is not known until runtime, and
    because static coercion is not possible,  but one double and one string
    are still valid operands to the * operator, the string value is coerced to
    a double at runtime. Failure to convert at runtime results in a runtime
    semantic error.
*/

/*  TODO: The assignment operator, =, has been disabled in ExpressionParser::parseAssignment()
    because it is not fully implemented. Use of the assignment operator will result in a
    syntax error.
*/

/* Supported Operators' Precedence
   ( )
   * / % ! ++expr --expr
   + -
   < > <= >=
   == !=
   &&
   ||
   =
*/

/* Expression Framework File Structure

qfixedpointnumber_p.h
    QFixedPointNumber Declaration

qfixedpointnumber.cpp
    QFixedPointNumber Definition

qexpressionevaluator.h
    Expression Declaration
    QAbstractExpressionEvaluatorTerm Declaration

qexpressionevaluator.cpp
    DECLARATIONS
    ExpressionToken
    ExpressionTokenizer
    ExpressionParserNode
    ExpressionParser
    ExpressionSemanticAnalyser
    ExpressionMachineOperandData
    ExpressionMachineOperand
    ExpressionMachineInstruction
    ExpressionCodeGenerator
    ExpressionMachine
    QExpressionEvaluatorPrivate

    DEFINITIONS
    ExpressionToken
    ExpressionTokenizer
    ExpressionParserNode
    ExpressionParser
    ExpressionSemanticAnalyser
    ExpressionMachineOperand
    ExpressionMachineInstruction
    ExpressionCodeGenerator
    ExpressionMachine
    QExpressionEvaluatorPrivate
    Expression
   */

//========================================
//= ExpressionTester Declaration/Definition
//=======================================

//+========================================================================================================================+
//                                                  CLASS DECLARATIONS
//+========================================================================================================================+

//========================================
//= ExpressionToken Declaration
//=======================================
struct ExpressionToken {
public:
    enum Type {
        // TODO - more operators - ^, ? :, pre/post inc/dec
        Multiply = '*',
        Divide = '/',
        Plus = '+',
        Minus = '-',
        Mod = '%',
        LParen =  '(',
        RParen = ')',
        Gt = '>',
        Lt = '<',
        Assign = '=',
        Not = '!',
        Cat = '.',
        GtEq = 200, // ensure unambiguous
        LtEq,
        Equal,
        NotEqual,
        String,
        Bool,
        LogicalOr,
        LogicalAnd,
        Integer,
        Double,
        Unknown,
        Term,
        Null
    };

    /* ExpressionToken Ctors */
    ExpressionToken();
    ExpressionToken( ExpressionToken::Type t );
    ExpressionToken( ExpressionToken::Type t, const QByteArray& d );

    ExpressionToken& operator=( const ExpressionToken& other );

    static const char* typeToName( ExpressionToken::Type t );
    static const char* tokenData( const ExpressionToken & t );

    /* Public data */
    Type type;
    QByteArray data;
};

//+========================================================================================================================+

// Replacement for QBuffer that does away with the
// overhead of QIODevice::getChar().
class ExpressionBuffer
{
public:
    ExpressionBuffer()
    {
	start = 0;
	end = 0;
    }

    void setData(const QByteArray& data)
    {
	start = data.constData();
	end = start + data.size();
    }

    inline bool atEnd() const
    {
	return (start >= end);
    }

    inline bool getChar(char *c)
    {
	if (start < end) {
	    *c = *start++;
	    return true;
	} else {
	    return false;
	}
    }

    inline void ungetChar(char)
    {
	--start;
    }

    inline QByteArray peek(int length) const
    {
	if (length <= (int)(end - start))
	    return QByteArray(start, length);
	else
	    return QByteArray();
    }

    inline int peek(char *c) const
    {
	if (start < end) {
	    *c = *start;
	    return 1;
	} else {
	    return -1;
	}
    }

private:
    const char *start;
    const char *end;
};

//========================================
//= ExpressionTokenizer Declaration
//=======================================
class ExpressionTokenizer
{
public:
    /* ExpressionTokenizer Ctors */
    ExpressionTokenizer();

    /* Public Methods */
    int hasTokens() const;

    ExpressionToken tokenAt(int i) const;
    bool tokenize();

    ExpressionToken next();

    bool test( ExpressionToken::Type t );

    ExpressionToken current();

    void setExpression( const QByteArray& data );

private:
    /* Private Methods */
    bool testChar( const char* tc, bool& ok );
    bool tokenizeNumber( char c );
    bool tokenizeString();
    bool tokenizeComparisonAndAssign( const ExpressionToken& curToken );

    /* Private Data */
    QVector<ExpressionToken> m_tokens;
    int m_curToken;

    QByteArray m_expressionData;
    ExpressionBuffer m_idevice;
};

//+========================================================================================================================+

//========================================
//= ExpressionParserNode Declaration
//=======================================
class ExpressionParser;
struct ExpressionParserNode
{
    /* ExpressionParserNode Ctors */
    ExpressionParserNode( ExpressionParser* parser, const ExpressionToken& t );

    /* Public Data */
    ExpressionToken token;
    ExpressionToken::Type returnType;
    ExpressionParserNode* leftChild, * rightChild;
    QString vskey;
};

//+========================================================================================================================+

//=======================================
//= ExpressionSemanticAnalyser Declaration
//=======================================
class ExpressionSemanticAnalyser
{
public:
    /* Public Methods */
    /* ExpressionParserNode Ctors */
    ExpressionSemanticAnalyser();
    bool checkSemantics( ExpressionParserNode* node );
    void setReturnType(ExpressionParserNode* node);
private:
    /* Private Methods */
    bool isNodeDataType( ExpressionParserNode* node ) const;
    bool allChildrenReturnType( ExpressionParserNode* node, ExpressionToken::Type t ) const;
    bool atleastOneChildReturnType( ExpressionParserNode* node, ExpressionToken::Type t ) const;
    bool isNodeDataConstantType( ExpressionParserNode* node ) const;
    bool isEquality( ExpressionToken::Type t ) const;
    bool isComparison( ExpressionToken::Type t ) const;
    bool isArithmetic( ExpressionToken::Type t ) const;
    ExpressionParserNode* getChildByReturnType(ExpressionParserNode* node,ExpressionToken::Type t);
    ExpressionParserNode* getChildByNotReturnType(ExpressionParserNode* node,ExpressionToken::Type t);
};

//+========================================================================================================================+

//=======================================
//= ExpressionParser Declaration
//=======================================
class ExpressionParser
{
public:
    /* Public Methods */
    /* ExpressionParser Ctors */
    ExpressionParser( const QByteArray& expr );

    /* ExpressionParser Dtor */
    ~ExpressionParser();

    void nodeAdded( ExpressionParserNode* node );

    ExpressionParserNode* parse();

private:
    /* Private Methods */
    ExpressionParserNode* parseAddSubCat();
    ExpressionParserNode* parseMultDivMod();
    ExpressionParserNode* parseAssignment();
    ExpressionParserNode* parseLogicalAnd();
    ExpressionParserNode* parseLogicalOr();
    ExpressionParserNode* parsePrimary();
    ExpressionParserNode* parseComparison();
    ExpressionParserNode* parseBinaryOp( ExpressionToken::Type t, ExpressionParserNode* left, ExpressionParserNode* (ExpressionParser::*func)() );
    ExpressionParserNode* parseEquality();


    bool test( ExpressionToken::Type t );

    /* Private Data */
    ExpressionTokenizer* m_tokenizer;
    QByteArray m_expressionData;
    QVector<ExpressionParserNode*> m_parserNodes;
    ExpressionSemanticAnalyser m_semanticAnalyser;
};

//+========================================================================================================================+

//========================================
//= ExpressionMachineOperandDataRef Declaration/Definition
//=======================================
struct ExpressionMachineOperandDataRef
{
    /* Public Methods */
    /* ExpressionMachineOperandDataRef Ctors */
    /* Public Data */
    union {
        QValueSpaceItem* t;
        QByteArray* s;
    };
    uint count;
};

//========================================
//= ExpressionMachineOperandData Declaration
//=======================================

union ExpressionMachineOperandData
{
    ExpressionMachineOperandData() {
        ref = 0;
    }
    bool b; // .1 bytes
    int i; // 4 bytes
    double d; // 8 bytes
    ExpressionMachineOperandDataRef *ref; // 8 bytes
    struct _f {
        QFixedPointNumberType value;
        unsigned char precision;
    } f; // 9 bytes
};

//+========================================================================================================================+

//========================================
//= ExpressionMachineOperand Declaration
//=======================================
struct ExpressionMachineOperand
{
    enum Type {
        Bool,
        Integer,
        Double, // exclusive with FixedPoint
        FixedPoint,
        String,
        Term
    };

    /* ExpressionMachineOperand Ctors */
    ExpressionMachineOperand();
    ExpressionMachineOperand( Type t );
    ExpressionMachineOperand( const ExpressionMachineOperand& other );

    /* ExpressionMachineOperand Dtor */
    virtual ~ExpressionMachineOperand();

    ExpressionMachineOperand& operator=( const ExpressionMachineOperand& other );
    void create( const ExpressionMachineOperand::Type& t );

    /* Public Data */
    Type type;
    mutable ExpressionMachineOperandData d;
private:
    /* Private Methods */
    void addref() const;
    void decref() const;
};

//+========================================================================================================================+

//========================================
//= ExpressionMachineInstruction Declaration
//=======================================
class ExpressionMachineInstruction
{
public:
    enum OpCode {
        NOOP,
        Load,
        Store,

        Sign, // d, integer
        Negate,
        Cmp,

        Add,
        Sub,
        Mul,
        Div,
        Mod,

        Cat,

        ProcessTerm, // retrieve the term's value from its interface and coerce it to the expected data type. if fails runtime error

        Assign,

        LogicalAnd,
        LogicalOr,

        LtEq,
        GtEq,
        Lt,
        Gt
    };

    /* ExpressionMachineInstruction Ctors */
    ExpressionMachineInstruction();
    ExpressionMachineInstruction( const ExpressionMachineInstruction& other );
    ExpressionMachineInstruction( const ExpressionMachineInstruction::OpCode& c, const ExpressionMachineOperand::Type& t );

    /* Public Data */
    OpCode code;
    ExpressionMachineOperand::Type type;
};

//+========================================================================================================================+

//=======================================
//= ExpressionCodeGenerator Declaration
//=======================================
class ExpressionCodeGenerator
{
public:
    /* ExpressionCodeGenerator Ctors */
    ExpressionCodeGenerator( ExpressionParserNode* node );

    /* Public Methods */
    bool generate();
    QVector<ExpressionMachineInstruction> instructions() const;
    QVector<ExpressionMachineOperand> data() const;
    void setFixedPoint( bool fp );

private:
    /* Private Methods */
    void generateOpCode( ExpressionParserNode* node );
    void generateDataCode( ExpressionParserNode* data );
    ExpressionMachineInstruction getCompareCode( ExpressionParserNode* node ) const;
    void generateHelper( ExpressionParserNode* node );
    ExpressionMachineOperand::Type tokenTypeToMachineType( ExpressionToken::Type t );
    bool isNodeDataType( ExpressionParserNode* node ) const;

    /* Private Data */
    ExpressionParserNode* m_root;
    QVector<ExpressionMachineInstruction> m_instructions;
    QVector<ExpressionMachineOperand> m_data;
    bool m_useFixedPoint;
};

//+========================================================================================================================+

//========================================
//= ExpressionMachine Declaration
//=======================================
class ExpressionMachine
{
public:
    /* ExpressionMachine Ctors */
    ExpressionMachine( const QVector<ExpressionMachineInstruction>& instructions, const QVector<ExpressionMachineOperand>& data);

    /* ExpressionMachine Dtor */
    ~ExpressionMachine();

    /* Public Methods */

    bool execute();
    QVariant result();
    bool error() const;

#ifdef EXPRESSION_TESTING
    QString dumpInfo() const;
#endif
private:
    /* Private Methods */
#ifdef EXPRESSION_TESTING
    QString dataTypeToText( ExpressionMachineOperand::Type type ) const;
    QString instructionToText( ExpressionMachineInstruction i ) const;
#endif

    QVariant processTerm( const QVariant::Type& t );
    int stackInteger();
    QByteArray stackString();
    double stackDouble();
    QFixedPointNumber stackFixedPoint();
    bool stackBool();

private:
    /* Private Data */
    QVector<ExpressionMachineOperand> m_s; //runtime stack
    int m_iptr, m_dptr;
    QVector<ExpressionMachineInstruction> m_i; //instructions
    QVector<ExpressionMachineOperand> m_d; //data operands
    QVariant m_result;
    bool m_errorflag;
};

//+========================================================================================================================+

//=======================================
//= QExpressionEvaluatorPrivate Declaration
//=======================================
struct QExpressionEvaluatorPrivate
{
    /* Public Functions */
    /* QExpressionEvaluatorPrivate Ctors */
    QExpressionEvaluatorPrivate();

    /* Public Data */
    QByteArray expressionData;
    QVariant result;
    ExpressionMachine* machine;
    QExpressionEvaluator::FloatingPointFormat floatingPointFormat;
};

//+========================================================================================================================+
//                                                  CLASS DEFINITIONS
//+========================================================================================================================+

//=======================================
//= ExpressionToken Definition
//=======================================
/* ExpressionToken Ctors */
ExpressionToken::ExpressionToken()
    : type(ExpressionToken::Null)
{}

ExpressionToken::ExpressionToken( ExpressionToken::Type t )
    : type(t)
{}

ExpressionToken::ExpressionToken( ExpressionToken::Type t, const QByteArray& d )
    : type(t), data(d)
{}

ExpressionToken& ExpressionToken::operator=( const ExpressionToken& other ) {
    type = other.type;
    data = other.data;
    return *this;
}

const char* ExpressionToken::typeToName( ExpressionToken::Type t )
{
    switch( t ) {
        case ExpressionToken::Bool:
        return "Bool";
        case ExpressionToken::Multiply:
        return "Multiply";
        case ExpressionToken::Divide:
        return "Divide";
        case ExpressionToken::Plus:
        return "Plus";
        case ExpressionToken::Minus:
        return "Minus";
        case ExpressionToken::LParen:
        return "LParen";
        case ExpressionToken::RParen:
        return "RParen";
        case ExpressionToken::Gt:
        return "Gt";
        case ExpressionToken::Lt:
        return "Lt";
        case ExpressionToken::Assign:
        return "Assign";
        case ExpressionToken::GtEq:
        return "GtEq";
        case ExpressionToken::LtEq:
        return "LtEq";
        case ExpressionToken::Equal:
        return "Equal";
        case ExpressionToken::NotEqual:
        return "NotEqual";
        case ExpressionToken::String:
        return "String";
        case ExpressionToken::Integer:
        return "Integer";
        case ExpressionToken::Double:
        return "Double";
        case ExpressionToken::Unknown:
        return "Unknown";
        case ExpressionToken::Term:
        return "Term";
        case ExpressionToken::Null:
        return "Null";
        case ExpressionToken::Not:
        return "Not";
        case ExpressionToken::LogicalOr:
        return "LogicalOr";
        case ExpressionToken::LogicalAnd:
        return "LogicalAnd";
        case ExpressionToken::Mod:
        return "Mod";
        case ExpressionToken::Cat:
        return "Cat";
    }
    return "Unknown";
}

const char* ExpressionToken::tokenData( const ExpressionToken & t ) {
    switch( t.type ) {
        case ExpressionToken::Multiply:
        case ExpressionToken::Divide:
        case ExpressionToken::Plus:
        case ExpressionToken::Minus:
        case ExpressionToken::Mod:
        case ExpressionToken::LParen:
        case ExpressionToken::RParen:
        case ExpressionToken::Gt:
        case ExpressionToken::Lt:
        case ExpressionToken::Assign:
        case ExpressionToken::Not:
        case ExpressionToken::Cat:
            return (const char *)&t.type;
        default:
            return t.data.constData();

    }
}

//+========================================================================================================================+

//=======================================
//= ExpressionTokenizer Definition
//=======================================
/* Public Methods */
/* ExpressionTokenizer Ctors */
ExpressionTokenizer::ExpressionTokenizer()
{
    m_curToken = -1;
}

int ExpressionTokenizer::hasTokens() const
{
    return m_tokens.count();
}

ExpressionToken ExpressionTokenizer::tokenAt(int i) const
{
    if( i < 0 || (i >= m_tokens.count() ) ) {
        return ExpressionToken(ExpressionToken::Null);
    }
    return m_tokens[i];
}

bool ExpressionTokenizer::tokenize()
{
    Q_ASSERT( m_expressionData.isEmpty() == false );
    m_tokens.clear();
    while( !m_idevice.atEnd() ) {
        char c = 0;
        if( !m_idevice.getChar( &c ) ) {
            // error: input error
            qWarning("Expression Input Error (2)");
            return false;
        }

        switch( c ) {
            case ExpressionToken::Multiply:
                m_tokens.append(ExpressionToken(ExpressionToken::Multiply));
                break;
            case ExpressionToken::Divide:
                m_tokens.append(ExpressionToken(ExpressionToken::Divide));
                break;
            case ExpressionToken::Plus:
                m_tokens.append(ExpressionToken(ExpressionToken::Plus));
                break;
            case ExpressionToken::Minus:
                m_tokens.append(ExpressionToken(ExpressionToken::Minus));
                break;
            case ExpressionToken::LParen:
                m_tokens.append(ExpressionToken(ExpressionToken::LParen));
                break;
            case ExpressionToken::RParen:
                m_tokens.append(ExpressionToken(ExpressionToken::RParen));
                break;
            case ExpressionToken::Mod:
                m_tokens.append(ExpressionToken(ExpressionToken::Mod));
                break;
            case '|':
                {
                if( !m_idevice.getChar(&c) ) {
                    qWarning("expression, unexpected end of input");
                    return false;
                }
                if( c != '|' ) {
                    qWarning("expression, syntax error at '|'");
                    return false;
                }
                m_tokens.append(ExpressionToken(ExpressionToken::LogicalOr, "||"));
                break;
                }
            case '&':
            {
                if( !m_idevice.getChar(&c) ) {
                    qWarning("expression, unexpected end of input");
                    return false;
                }
                if( c != '&' ) {
                    qWarning("expression, syntax error at '&'");
                    return false;
                }
                m_tokens.append(ExpressionToken(ExpressionToken::LogicalAnd, "&&"));
                break;
            }
            case ExpressionToken::Gt:
            case ExpressionToken::Lt:
            case ExpressionToken::Assign:
            case ExpressionToken::Not:
                if( !tokenizeComparisonAndAssign((ExpressionToken::Type)c) )
                    return false;
                break;
            case '\"':
                if( !tokenizeString() )
                    return false;
                break;
            default:
            {
                if( isspace( c ) ) {
                    while( m_idevice.getChar( &c ) && isspace( c ) );
                    if( !isspace( c ) ) {
                        m_idevice.ungetChar( c );
                    }
                } else if( (c == 't' && m_idevice.peek(3) == "rue") || (c == 'f' && m_idevice.peek(4) == "alse") ) {
                    int i = 0;
                    if( c == 't' ) {
                        m_tokens.append(ExpressionToken(ExpressionToken::Bool, "1"));
                        while(i++ < 3) m_idevice.getChar(&c);
                    } else {
                        m_tokens.append(ExpressionToken(ExpressionToken::Bool, ("0")));
                        while(i++ < 4) m_idevice.getChar(&c);
                    }
                } else if( isdigit( c ) || c == '.' ) {
                    char lookahead = 0;
                    bool ok = m_idevice.getChar( &lookahead );
                    if(ok) m_idevice.ungetChar( lookahead );
                    if( ok && c == '.' && !isdigit(lookahead))
                        m_tokens.append( ExpressionToken(ExpressionToken::Cat) );
                    else
                        if( !tokenizeNumber( c ) )
                            return false;
                } else {

                    // unknown tokens, assume pluggable term
                    QByteArray term;
                    term.reserve(64);   // reserve space to make term.append more efficient
                    term.append( c );
                    // FIXME : should switch to IL term interface to do simple parsing, instead of relying on space as delimiter
                    do {
                        if( !m_idevice.getChar( &c ) ) {
                            // error: input error
                            qWarning("Expression Input Error (3)");
                            return false;
                        }
                        if( isspace( c ) )
                            break;
                        term.append( c );
                    } while( !m_idevice.atEnd() );
                    m_tokens.append(ExpressionToken(ExpressionToken::Term, term));
                    // don't bother putting the space back on the input
                }
            }
        }
    }
    return true;
}

ExpressionToken ExpressionTokenizer::next()
{
    if( (m_curToken+1) >= m_tokens.count() ) {
        m_curToken=m_tokens.count();
        return ExpressionToken(ExpressionToken::Null);
    }
    return m_tokens[++m_curToken];
}

bool ExpressionTokenizer::test( ExpressionToken::Type t )
{
    if( (m_curToken+1) >= m_tokens.count() ) {
        if( t == ExpressionToken::Null )
            return true;
        else
            return false;
    }
    return m_tokens[m_curToken+1].type == t;
}

ExpressionToken ExpressionTokenizer::current()
{
    if( (m_curToken < 0 || m_curToken >= m_tokens.count() ) )
        return ExpressionToken(ExpressionToken::Null);
    return m_tokens[m_curToken];
}

void ExpressionTokenizer::setExpression( const QByteArray& data )
{
    m_tokens.clear();
#ifdef EXPRESSION_TESTING
    qWarning("ExpressionTokenizer::setExpression( %s )", data.constData());
#endif
    m_expressionData = data;
    m_idevice.setData(data);
}

/* Private Methods */
bool ExpressionTokenizer::testChar( const char* tc, bool& ok )
{
    char c = 0;
    ok = true;
    int r = m_idevice.peek( &c );
    if( r < 0 ) {
        // error: Input error
        qWarning("Expression Input Error (1)");
        ok = false;
        return false;
    }
    return *tc == c;
}

bool ExpressionTokenizer::tokenizeNumber( char c )
{
    bool seenDot = false;
    bool brokeAtEnd = false;
    QByteArray num;
    do {
        num.append( c );
        if( c == '.' )
            seenDot = true;
        if( m_idevice.atEnd() ) {
            brokeAtEnd = true;
            break;
        }
        if( !m_idevice.getChar( &c ) ) {
            // error: input error
            qWarning("Expression Input Error (4)");
            return false;
        }
    } while( isdigit( c ) || (!seenDot && c == '.'));
    if( num.endsWith('.') ) {
        // error: double must have decimal component
        qWarning("Expression Double must have decimal component");
        return false;
    }
    if( !brokeAtEnd )
        m_idevice.ungetChar( c );
    if( seenDot ) {
        // ExpressionToken takes ownership of char data
        m_tokens.append( ExpressionToken(ExpressionToken::Double, num) );
    } else {
        m_tokens.append( ExpressionToken(ExpressionToken::Integer, num) );
    }
    return true;
}

bool ExpressionTokenizer::tokenizeString()
{
    QByteArray string;
    char c = 0;
    while(true) {
        if( m_idevice.atEnd() ) {
            // error: unexpected end of input
            qWarning("Expression Unexpected end of input");
            return false;
        }
        if( !m_idevice.getChar( &c ) ) {
            // error: input error
            qWarning("Expression Input Error (6)");
            return false;
        }
        /* This will work for utf8 strings too, as the ascii values of \\ and " are < 128 and so the high order bit is always 0,
           the same as 0-128 utf8 ascii characters. All other utf8 bytes have the high order bit as 1.
           Therefore, we know utf8 or otherwise, when we find a character with the ascii value of '"'
           we have found the terminating character.
           */

        if( c == '\\' ) {
            string.append( c );
            if( m_idevice.atEnd() ) {
                // error: unexpected end of input
qWarning("Expression Unexpected end of input, expected 2nd part of escape sequence");
                return false;
            }
            if( !m_idevice.getChar( &c ) ) {
                // error: input error
qWarning("Expression Input Error7");
                return false;
            }
            // TODO : validate escape sequence?
            string.append( c );
        } else if( c == '\"' ) {
            break;
        } else {
            string.append( c );
        }
    }
    m_tokens.append( ExpressionToken(ExpressionToken::String, string) );
    return true;
}

bool ExpressionTokenizer::tokenizeComparisonAndAssign( const ExpressionToken& curToken )
{
    // gt gteq
    bool ok = false;
    bool testf = false;
    Q_ASSERT( curToken.type != ExpressionToken::Null );
    testf = testChar( "=", ok );
    if( !ok ) return false;
    if( testf ) {
        char c = 0;
        m_idevice.getChar( &c );
        switch( curToken.type ) {
            case ExpressionToken::Gt:
                m_tokens.append(ExpressionToken(ExpressionToken::GtEq, ">="));
                break;
            case ExpressionToken::Lt:
                m_tokens.append(ExpressionToken(ExpressionToken::LtEq, ">="));
                break;
            case ExpressionToken::Assign:
                m_tokens.append(ExpressionToken(ExpressionToken::Equal, "=="));
                break;
            case ExpressionToken::Not:
                m_tokens.append(ExpressionToken(ExpressionToken::NotEqual, "!="));
                break;
            default:
                qFatal("ExpressionTokenizer::processComparisonAndAssign() - Argument curToken does not have a valid type");
        }
    } else {
        m_tokens.append(curToken);
    }
    return true;
}

//+========================================================================================================================+

//=======================================
//= ExpressionParserNode Definition
//=======================================
/* Public Methods */
/* ExpressionParserNode Ctors */
ExpressionParserNode::ExpressionParserNode( ExpressionParser* parser, const ExpressionToken& t )
    : token(t)
    , returnType(ExpressionToken::Null)
    , leftChild(0)
    , rightChild(0)
{
    parser->nodeAdded( this ); // for easy tracking
}

//+========================================================================================================================+

//=======================================
//= ExpressionSemanticAnalyser Definition
//=======================================
/* Public Methods */
/* ExpressionSemanticAnalyser Ctors */
ExpressionSemanticAnalyser::ExpressionSemanticAnalyser()
{}

bool ExpressionSemanticAnalyser::checkSemantics( ExpressionParserNode* node )
{
    Q_ASSERT(node != 0);
    Q_ASSERT(node->leftChild != 0);
    ExpressionParserNode* leftChild = node->leftChild, * rightChild = node->rightChild;

    if( leftChild != 0 && rightChild == 0 ) {
       switch( node->token.type ) {
           case ExpressionToken::Not:
               return true; // valid on all data
           case ExpressionToken::Minus:
               if( atleastOneChildReturnType(node, ExpressionToken::String ) ) {
                   qWarning("Expression Semantic Error: Operand of type '%s' cannot be used with operator 'unary minus'", ExpressionToken::typeToName(leftChild->returnType));
                   return false;
               }
               return true;
           default:
               qFatal("checkSemantics node with one child does not have any matching unary operator i know about");
       }
    } else if( leftChild != 0 && rightChild != 0 ) {
        switch( node->token.type ) {
            case ExpressionToken::Cat:
                /*
                if( !atleastOneChildReturnType(node,ExpressionToken::String ) ) {
                    qWarning("Expression Semantic Error: Operands of type '%s' and '%s' cannot be used with string operator '%s'", ExpressionToken::typeToName(leftChild->returnType), ExpressionToken::typeToName(rightChild->returnType), ExpressionToken::tokenData(node->token.type));
                    return false;
                }
                */
                // all operands can be converted to strings
                return true;
            case ExpressionToken::Minus:
            case ExpressionToken::Multiply:
            case ExpressionToken::Divide:
            case ExpressionToken::Plus:
            case ExpressionToken::Mod:
            {
                if( allChildrenReturnType(node, ExpressionToken::String) ) {
                    qWarning("Expression Semantic Error: Two operands of type 'String' cannot be used with arithmetic operator '%s'", ExpressionToken::typeToName(node->token.type));
                    return false;
                }
                if( atleastOneChildReturnType(node,ExpressionToken::String) && atleastOneChildReturnType(node,ExpressionToken::Bool) ) {
                   qWarning("Expression Semantic Error: Operands of type 'Bool' and 'String' cannot be used with arithmetic operator '%s'", ExpressionToken::typeToName(node->token.type));
                    return false;
                }
                if( allChildrenReturnType(node, ExpressionToken::Bool) ) {
                    qWarning("Expression Semantic Error: Operands of type 'Bool' cannot be used with operator '%s'", ExpressionToken::typeToName(node->token.type));
                    return false;
                }
                if( node->token.type == ExpressionToken::Mod && atleastOneChildReturnType(node,ExpressionToken::Double) ) {
                    qWarning("Expression Semantic Error: Operand of type 'Double' cannot be used with operator 'Modulus'");
                    return false;
                }
                return true;
            }
            case ExpressionToken::Gt:
            case ExpressionToken::Lt:
            case ExpressionToken::GtEq:
            case ExpressionToken::LtEq:
            {
                if( allChildrenReturnType(node, ExpressionToken::String) ) {
                    qWarning("Expression Semantic Error: Operands of type 'String' cannot be used with > >= < <=");
                    return false;
                }
                return true;
            }
            case ExpressionToken::Equal:
            case ExpressionToken::NotEqual:
            {
                // all operand types can be compared, possibly with coercion
                return true;
            }
            case ExpressionToken::LogicalOr:
            case ExpressionToken::LogicalAnd:
            {
                // can always coerce in this case
                return true;
            }
            case ExpressionToken::Assign:
            {
                return true;
            }
        default:
            qFatal("ExpressionSemanticAnalyser: Unhandled operator for binary operation");
        }
    }
    qFatal("checkSemantics called with children count > 2");
    return false;
}

void ExpressionSemanticAnalyser::setReturnType(ExpressionParserNode* node)
{
    Q_ASSERT(node != 0);
    ExpressionParserNode* leftChild = 0, * rightChild = 0;
    leftChild = node->leftChild;
    rightChild = node->rightChild;

    if( !leftChild && !rightChild  ) {
        // Set return type manually
        switch( node->token.type ) {
            case ExpressionToken::String:
            case ExpressionToken::Integer:
            case ExpressionToken::Double:
            case ExpressionToken::Bool:
                node->returnType = node->token.type; // return type is just this data type
                break;
            case ExpressionToken::Term:
            {
                // just assume string, means @/valuespace/key * "3.2" won't work.
                node->returnType = ExpressionToken::String;
                break;
            }
            default:
                qFatal("ExpressionSemanticAnalyser: no children for node that is not a string, integer, double or term.");
        }
    } else if( leftChild != 0 && !rightChild ) {
        switch( node->token.type ) {
            case ExpressionToken::Not:
            {
                //if( isNodeDataConstantType(node->children[0]) )
                    leftChild->returnType = ExpressionToken::Bool; // static coercion
                node->returnType = ExpressionToken::Bool;
                break;
            }
            case ExpressionToken::Minus:
                // unary minus
                if( getChildByReturnType(node, ExpressionToken::Bool) ) {
                   leftChild->returnType = ExpressionToken::Integer; // will be statically coerced during generateDataCode
               }
                node->returnType = leftChild->returnType;
               break;
            default:
                qFatal("expression unhandled unary operator case when setting return type");
        }
    } else {
        Q_ASSERT(leftChild != 0 && rightChild != 0);
        /* In assignment r-value is always coerced to l-value type */
        if( node->token.type == ExpressionToken::Assign ) {
            node->returnType = leftChild->returnType;
            rightChild->returnType = node->returnType;
            // FIXME shouldn't coerce strings to arithmetic types?.
            return;
        }
        /* Which operators are we dealing with? */
        switch( node->token.type ) {
            case ExpressionToken::Cat:
                node->returnType = ExpressionToken::String;
                leftChild->returnType = ExpressionToken::String;
                rightChild->returnType = ExpressionToken::String;
                break;
            case ExpressionToken::Divide:
                node->returnType = ExpressionToken::Double;
                leftChild->returnType = ExpressionToken::Double;
                rightChild->returnType = ExpressionToken::Double;
                break;
            case ExpressionToken::Mod:
                node->returnType = ExpressionToken::Integer;
                leftChild->returnType = ExpressionToken::Integer;
                rightChild->returnType = ExpressionToken::Integer;
                break;
            case ExpressionToken::Minus:
            case ExpressionToken::Multiply:
            case ExpressionToken::Plus:
            case ExpressionToken::Gt:
            case ExpressionToken::Lt:
            case ExpressionToken::GtEq:
            case ExpressionToken::LtEq:
            case ExpressionToken::Equal:
            case ExpressionToken::NotEqual:
            {
                bool ia = isArithmetic( node->token.type );
                if( !ia )
                    node->returnType = ExpressionToken::Bool;
                if( leftChild->returnType == rightChild->returnType ) {
                    if( ia ) {
                        node->returnType = leftChild->returnType;
                        Q_ASSERT(node->returnType != ExpressionToken::String);
                    }
                    return;
                }
                ExpressionParserNode* n = 0;
                if( (isEquality(node->token.type) || isComparison(node->token.type)) && getChildByReturnType(node,ExpressionToken::Bool) != 0 && ((n = getChildByReturnType(node,ExpressionToken::String)) != 0)) {
                    //should always be set, as code generation and runtime conversion is based on return type of expressions
                    n->returnType = ExpressionToken::Bool; //static coercion
                    node->returnType = ExpressionToken::Bool;
                } else if( (getChildByReturnType(node,ExpressionToken::Double) != 0) && ((n = getChildByNotReturnType(node,ExpressionToken::Double)) != 0) ) // integer, string, bool in double expression
                {
                    if( !(isEquality(node->token.type) || isComparison(node->token.type)) )
                        node->returnType = ExpressionToken::Double;
                        n->returnType = ExpressionToken::Double;
                    // else runtime coercion
                } else if( getChildByReturnType(node,ExpressionToken::Integer) != 0 && (n=getChildByNotReturnType(node,ExpressionToken::Integer)) != 0 ) { // string, bool in integer expression
                    if ( !(isEquality(node->token.type) || isComparison(node->token.type)) )
                        node->returnType = ExpressionToken::Integer;
                    Q_ASSERT(n->token.type != ExpressionToken::Double);
                    n->returnType = ExpressionToken::Integer;
                    // else runtime coercion
                } else {
                    qFatal(QString("Unhandled combination of operands '%1' and '%2' for equality operator '%3'").arg(ExpressionToken::typeToName(leftChild->returnType)).arg(ExpressionToken::typeToName(rightChild->returnType)).arg(ExpressionToken::typeToName(node->token.type)).toAscii().data());
                }
                break;
            }
            case ExpressionToken::LogicalOr:
            case ExpressionToken::LogicalAnd:
            {
                node->returnType = ExpressionToken::Bool;
                leftChild->returnType = ExpressionToken::Bool;
                rightChild->returnType = ExpressionToken::Bool;
                break;
            }
            case ExpressionToken::Assign:
                // already handled before switch
                break;
            default:
                qFatal("ExpressionSemanticAnalyser Unhandled operator '%s' in setReturnType", ExpressionToken::typeToName(node->token.type));
        }
    }
}


/* Private Methods */
bool ExpressionSemanticAnalyser::isNodeDataConstantType( ExpressionParserNode* node ) const
{
    Q_ASSERT(node != 0);
    switch( node->token.type ) {
        case ExpressionToken::String:
        case ExpressionToken::Double:
        case ExpressionToken::Integer:
        case ExpressionToken::Bool:
            return true;
        default:
            return false;
    }
}

bool ExpressionSemanticAnalyser::isNodeDataType( ExpressionParserNode* node ) const
{
    Q_ASSERT(node != 0);
    switch( node->token.type ) {
        case ExpressionToken::String:
        case ExpressionToken::Double:
        case ExpressionToken::Integer:
        case ExpressionToken::Bool:
        case ExpressionToken::Term:
            return true;
        default:
            return false;
    }
}

bool ExpressionSemanticAnalyser::isEquality( ExpressionToken::Type t ) const
{
    switch( t ) {
        case ExpressionToken::Equal:
        case ExpressionToken::NotEqual:
            return true;
        default:
            return false;
    }
}

bool ExpressionSemanticAnalyser::isComparison( ExpressionToken::Type t ) const
{
    switch( t ) {
        case ExpressionToken::Gt:
        case ExpressionToken::Lt:
        case ExpressionToken::LtEq:
        case ExpressionToken::GtEq:
            return true;
        default:
            return false;
    }
}

bool ExpressionSemanticAnalyser::isArithmetic( ExpressionToken::Type t ) const
{
    switch( t ) {
        case ExpressionToken::Mod:
        case ExpressionToken::Multiply:
        case ExpressionToken::Plus:
        case ExpressionToken::Minus:
        case ExpressionToken::Divide:
            return true;
        default:
            return false;
    }
}

ExpressionParserNode* ExpressionSemanticAnalyser::getChildByReturnType(ExpressionParserNode* node,ExpressionToken::Type t)
{
    if( node->leftChild != 0 && node->leftChild->returnType == t )
        return node->leftChild;
    if( node->rightChild != 0 && node->rightChild->returnType == t )
        return node->rightChild;
    return 0;
}

ExpressionParserNode* ExpressionSemanticAnalyser::getChildByNotReturnType(ExpressionParserNode* node,ExpressionToken::Type t)
{
    if( node->leftChild != 0 && node->leftChild->returnType != t )
        return node->leftChild;
    if( node->rightChild != 0 && node->rightChild->returnType != t )
        return node->rightChild;
    return 0;
}

bool ExpressionSemanticAnalyser::allChildrenReturnType( ExpressionParserNode* node, ExpressionToken::Type t ) const
{
    bool f = true;
    Q_ASSERT(node != 0);
    if( node->leftChild != 0 )
        f &= node->leftChild->returnType == t;
    if( node->rightChild != 0 )
        f &= node->rightChild->returnType == t;
    return f;
}

bool ExpressionSemanticAnalyser::atleastOneChildReturnType( ExpressionParserNode* node, ExpressionToken::Type t ) const
{
    Q_ASSERT(node != 0);
    return (node->leftChild != 0 && node->leftChild->returnType == t) ||
        (node->rightChild != 0 && node->rightChild->returnType == t);
}


//+========================================================================================================================+

//=======================================
//= ExpressionMachineOperand Definition
//=======================================
/* Public Methods */
/* ExpressionMachineOperand Ctors */
ExpressionMachineOperand::ExpressionMachineOperand()
    : type(Bool)
{
}

ExpressionMachineOperand::ExpressionMachineOperand( const ExpressionMachineOperand& other ) // deep copy
    : type(Bool)
{
    (*this) = other;
}

ExpressionMachineOperand::ExpressionMachineOperand( ExpressionMachineOperand::Type t )
    : type(t)
{
    addref();
}

ExpressionMachineOperand::~ExpressionMachineOperand()
{
    decref();
}

void ExpressionMachineOperand::addref() const
{
    if( type == ExpressionMachineOperand::String
        || type == ExpressionMachineOperand::Term
      )
    {
        ExpressionMachineOperandData& dta = d;
        // if ref is 0, create
        if( !dta.ref ) {
            dta.ref = new ExpressionMachineOperandDataRef;
            dta.ref->count = 0;
            if( type == ExpressionMachineOperand::String )
                dta.ref->s = new QByteArray;
            else if ( type == ExpressionMachineOperand::Term )
                dta.ref->t = new QValueSpaceItem;
        }
        dta.ref->count++;
    }
}

void ExpressionMachineOperand::decref() const
{
    if( type == ExpressionMachineOperand::String
            || type == ExpressionMachineOperand::Term
    )
    {
        if( !d.ref )
            return;
        d.ref->count--;
        if( d.ref->count == 0 ) {
            if( type == ExpressionMachineOperand::String ) {
                delete d.ref->s;
            }
            else if ( type == ExpressionMachineOperand::Term ) {
                delete d.ref->t;
            }
            delete d.ref;
            d.ref = 0;
        }
    }
}

ExpressionMachineOperand& ExpressionMachineOperand::operator=( const ExpressionMachineOperand& other )
{
    // Inc other if ref counted, so we don't delete it if it's us
    other.addref();
    decref();

    // copy over
    type = other.type;
    d = other.d;

    return *this;
}

void ExpressionMachineOperand::create( const ExpressionMachineOperand::Type& t )
{
    decref();
    type = t;
    d.ref = 0;
    addref();
}

//+========================================================================================================================+

//=======================================
//= ExpressionMachineInstruction Definition
//=======================================
/* Public Methods */
/* ExpressionMachineInstruction Ctors */
ExpressionMachineInstruction::ExpressionMachineInstruction()
    : code(NOOP), type(ExpressionMachineOperand::Integer)
{}

ExpressionMachineInstruction::ExpressionMachineInstruction( const ExpressionMachineInstruction& other )
    : code(other.code), type(other.type)
{}


ExpressionMachineInstruction::ExpressionMachineInstruction( const ExpressionMachineInstruction::OpCode& c, const ExpressionMachineOperand::Type& t )
    : code(c), type(t)
{}

//+========================================================================================================================+

//=======================================
//= ExpressionCodeGenerator Definition
//=======================================
/* Public Methods */
/* ExpressionCodeGenerator Ctors */
ExpressionCodeGenerator::ExpressionCodeGenerator( ExpressionParserNode* node ) : m_root(node), m_useFixedPoint(false)
{
    Q_ASSERT(m_root != 0);
}

QVector<ExpressionMachineInstruction> ExpressionCodeGenerator::instructions() const
{
    return m_instructions;
}

QVector<ExpressionMachineOperand> ExpressionCodeGenerator::data() const
{
    return m_data;
}

bool ExpressionCodeGenerator::generate()
{
    Q_ASSERT(m_root != 0);
    generateHelper(m_root);
    Q_ASSERT(m_root->returnType != ExpressionToken::Null);
    if( m_instructions.count() ) { // FIXME : this should be an assertion?
        m_instructions.append( ExpressionMachineInstruction(ExpressionMachineInstruction::Store, tokenTypeToMachineType(m_root->returnType) ) );
        return true;
    }
    return false;
}

void ExpressionCodeGenerator::setFixedPoint( bool fp )
{
    m_useFixedPoint = fp;
}


/* Private Methods */
ExpressionMachineOperand::Type ExpressionCodeGenerator::tokenTypeToMachineType( ExpressionToken::Type t )
{
    switch( t ) {
        case ExpressionToken::Double:
            if( m_useFixedPoint )
                return ExpressionMachineOperand::FixedPoint;
            else
                return ExpressionMachineOperand::Double;
        case ExpressionToken::Integer:
            return ExpressionMachineOperand::Integer;
        case ExpressionToken::Bool:
            return ExpressionMachineOperand::Bool;
        case ExpressionToken::String:
            return ExpressionMachineOperand::String;
        case ExpressionToken::Term:
            return ExpressionMachineOperand::Term;
        default:
            qFatal("unhandled token tyep passed to tokenTypeToMachineType");
            return ExpressionMachineOperand::Integer; //shoosh compiler
    }
}

void ExpressionCodeGenerator::generateHelper( ExpressionParserNode* node )
{
    if( node->leftChild != 0 )
        generateHelper( node->leftChild );
    if( node->rightChild != 0 )
        generateHelper( node->rightChild );
    if( isNodeDataType(node) )
        generateDataCode(node);
    else
        generateOpCode(node);
}

void ExpressionCodeGenerator::generateDataCode( ExpressionParserNode* data )
{
    Q_ASSERT(data != 0);
    Q_ASSERT(isNodeDataType(data) == true);
    Q_ASSERT(data->leftChild == 0 && data->rightChild == 0);
    ExpressionMachineOperand::Type runtimeType = /* Silence compiler warning */ ExpressionMachineOperand::Bool;
    if( data->token.type == ExpressionToken::Term ) {
        Q_ASSERT( !data->vskey.isNull() );
        ExpressionMachineOperand operand( ExpressionMachineOperand::Term );
        *operand.d.ref->t = QValueSpaceItem( data->vskey );
        switch( data->returnType ) {
            case ExpressionToken::String:
                runtimeType = ExpressionMachineOperand::String;
                break;
            case ExpressionToken::Integer:
                runtimeType = ExpressionMachineOperand::Integer;
                break;
            case ExpressionToken::Double:
                if( m_useFixedPoint )
                    runtimeType = ExpressionMachineOperand::FixedPoint;
                else
                    runtimeType = ExpressionMachineOperand::Double;
                break;
            case ExpressionToken::Bool:
                runtimeType = ExpressionMachineOperand::Bool;
                break;
            default:
                qFatal("ExpressionCheckerAnalyser - No mapping for term type to machine data type");
        }
        // term identifier on stack
        m_data.append( operand );
        m_instructions.append( ExpressionMachineInstruction(ExpressionMachineInstruction::Load, ExpressionMachineOperand::Term ) );
        m_instructions.append( ExpressionMachineInstruction(ExpressionMachineInstruction::ProcessTerm, runtimeType) );
        return;
    } else if( data->returnType == ExpressionToken::Bool ) {
        runtimeType = ExpressionMachineOperand::Bool;
        ExpressionMachineOperand b(ExpressionMachineOperand::Bool);
        if( data->token.type == ExpressionToken::String ) {
            b.d.b = !data->token.data.isEmpty();
        } else if( data->token.type == ExpressionToken::Integer || data->token.type == ExpressionToken::Bool )  {
            // FIXME : error checking
            int i = atoi(data->token.data.constData());
            b.d.b = i != 0;
        } else {
            // FIXME : error checking
            double d = atof(data->token.data.constData());
            b.d.b = d != 0.0;
        }
        m_data.append( b );
    } else if( data->returnType == ExpressionToken::Integer ) {
        runtimeType = ExpressionMachineOperand::Integer;
        ExpressionMachineOperand i( ExpressionMachineOperand::Integer );
        i.d.i = atoi(data->token.data.constData());
        m_data.append( i );
    } else if( data->returnType == ExpressionToken::Double ) {
        // Either double or fixedpoint depending on configuration
        if( m_useFixedPoint ) {
            runtimeType = ExpressionMachineOperand::FixedPoint;
            ExpressionMachineOperand fp( ExpressionMachineOperand::FixedPoint );
            QFixedPointNumber f( data->token.data.constData() );
            fp.d.f.value = f.value;
            fp.d.f.precision = f.precision;
            // FIXME error checking
            m_data.append( fp );
        } else {
            runtimeType = ExpressionMachineOperand::Double;
            ExpressionMachineOperand d( ExpressionMachineOperand::Double );
            d.d.d = atof(data->token.data.constData());
            // FIXME : error checking
            m_data.append( d );
        }
    } else if( data->returnType == ExpressionToken::String ) {
        runtimeType = ExpressionMachineOperand::String;
        ExpressionMachineOperand string( ExpressionMachineOperand::String );
        *string.d.ref->s = data->token.data;
        m_data.append( string  );
    } else
        qFatal("ExpressionCodeGenerator - Cannot generate data code for node that is not data type");

    m_instructions.append( ExpressionMachineInstruction(ExpressionMachineInstruction::Load, runtimeType) );
}

bool ExpressionCodeGenerator::isNodeDataType( ExpressionParserNode* node ) const
{
    Q_ASSERT(node != 0);
    switch( node->token.type ) {
        case ExpressionToken::String:
        case ExpressionToken::Double:
        case ExpressionToken::Integer:
        case ExpressionToken::Bool:
        case ExpressionToken::Term:
            return true;
        default:
            return false;
    }
}

void ExpressionCodeGenerator::generateOpCode( ExpressionParserNode* node )
{
    Q_ASSERT(isNodeDataType(node) == false);
    // If expression is not unary, both children must be non-null
    Q_ASSERT(node->token.type == ExpressionToken::Not || node->token.type == ExpressionToken::Minus ||
             (node->leftChild != 0 && node->rightChild != 0));
    // If expressiion is not binary, one child must be non-null
    Q_ASSERT((node->token.type != ExpressionToken::Not && node->token.type != ExpressionToken::Minus) ||
             ((node->leftChild != 0 && node->rightChild == 0) || (node->token.type == ExpressionToken::Minus && node->rightChild != 0)));
    // If the both children are non-null, their types must match
    Q_ASSERT(node->rightChild == 0 || node->leftChild->returnType == node->rightChild->returnType);

    ExpressionMachineOperand::Type dt = tokenTypeToMachineType(node->leftChild->returnType); // data type for operation is child return type
    ExpressionMachineInstruction instr( ExpressionMachineInstruction::NOOP, dt );
    switch( node->token.type )
    {
        case ExpressionToken::Not:
            Q_ASSERT(node->leftChild != 0);
            instr.code = ExpressionMachineInstruction::Negate;
            break;
        case ExpressionToken::Minus:
        {
            if( node->rightChild == 0 ) {
                instr.code = ExpressionMachineInstruction::Sign;
            } else {
                Q_ASSERT(node->rightChild != 0);
                instr.code = ExpressionMachineInstruction::Sub;
            }
            break;
        }
        case ExpressionToken::Multiply:
            instr.code = ExpressionMachineInstruction::Mul;
            break;
        case ExpressionToken::Plus:
            instr.code = ExpressionMachineInstruction::Add;
            break;
        case ExpressionToken::Mod:
        {
            Q_ASSERT(node->leftChild->returnType == ExpressionToken::Integer);
            instr.code = ExpressionMachineInstruction::Mod;
            break;
        }
        case ExpressionToken::Divide:
        {
            Q_ASSERT(node->leftChild->returnType == ExpressionToken::Double);
            instr.code = ExpressionMachineInstruction::Div;
            break;
        }
        case ExpressionToken::Gt:
            instr.code = ExpressionMachineInstruction::Gt;
            break;
        case ExpressionToken::Lt:
            instr.code = ExpressionMachineInstruction::Lt;
            break;
        case ExpressionToken::GtEq:
            instr.code = ExpressionMachineInstruction::GtEq;
            break;
        case ExpressionToken::LtEq:
            instr.code = ExpressionMachineInstruction::LtEq;
            break;
        case ExpressionToken::Equal:
            instr.code = ExpressionMachineInstruction::Cmp;
            break;
        case ExpressionToken::NotEqual:
            m_instructions.append(ExpressionMachineInstruction(ExpressionMachineInstruction::Cmp, dt));
            instr.code = ExpressionMachineInstruction::Negate;
            break;
        case ExpressionToken::LogicalOr:
            instr.code = ExpressionMachineInstruction::LogicalOr;
            break;
        case ExpressionToken::LogicalAnd:
            instr.code = ExpressionMachineInstruction::LogicalAnd;
            break;
        case ExpressionToken::Assign:
            instr.code = ExpressionMachineInstruction::Assign;
            break;
        case ExpressionToken::Cat:
            instr.code = ExpressionMachineInstruction::Cat;
            break;
        default:
            qFatal("ExpressionCodeGenerator unhandled operator in code generation");
    }
    if( instr.code != ExpressionMachineInstruction::NOOP )
        m_instructions.append( instr );
}

//+========================================================================================================================+

//=======================================
//= ExpressionParser Definition
//=======================================
/* Public Methods */
/* ExpressionParser Ctors */
ExpressionParser::ExpressionParser( const QByteArray& expr )
    : m_expressionData(expr)
{
}

/* ExpressionParser Dtor */
ExpressionParser::~ExpressionParser()
{
    for( int i = 0 ; i < m_parserNodes.count() ; ++i )
        delete m_parserNodes[i];
    m_parserNodes.clear();
}

void ExpressionParser::nodeAdded( ExpressionParserNode* node )
{
    m_parserNodes.append( node );
}

ExpressionParserNode* ExpressionParser::parse()
{
    m_tokenizer = new ExpressionTokenizer();
    m_tokenizer->setExpression( m_expressionData );
    if( !m_tokenizer->tokenize() )
        return 0;

    ExpressionParserNode* tree = parseAssignment();
    if( !tree ) {
        qWarning("Expression Error parsing expression");
    } else if( !m_tokenizer->test(ExpressionToken::Null) ) {
        // FIXME : not necessarily syntax error, now that semantic checking is done at parse time
        qWarning("Expression Error Syntax error at token '%s' %s", ExpressionToken::tokenData(m_tokenizer->current().type), ExpressionToken::typeToName(m_tokenizer->current().type));
        for( int i = 0 ; i < m_parserNodes.count() ; ++i )
            delete m_parserNodes[i];
        m_parserNodes.clear();
        tree = 0;
    }
    delete m_tokenizer;
    m_tokenizer = 0;
    return tree;
}

/* Private Methods */
ExpressionParserNode* ExpressionParser::parseAddSubCat()
{
    /* + - */
    ExpressionParserNode* left = parseMultDivMod(); // up the precedence hierarchy
    if( !left )
        return 0;

    while( true ) {
        if( test(ExpressionToken::Plus) ) {
            left = parseBinaryOp(ExpressionToken::Plus, left, &ExpressionParser::parseMultDivMod);
            if( !left )
                return 0;
        } else if(test(ExpressionToken::Minus)) {
            left = parseBinaryOp(ExpressionToken::Minus, left, &ExpressionParser::parseMultDivMod);
            if( !left )
                return 0;
        } else if(test(ExpressionToken::Cat)) {
            left = parseBinaryOp(ExpressionToken::Cat, left, &ExpressionParser::parseMultDivMod);
            if( !left )
                return 0;
        } else {
            return left;
        }
    }
    return left;
}

ExpressionParserNode* ExpressionParser::parseMultDivMod()
{
    /* / * */
    ExpressionParserNode* left = parsePrimary(); // up the precedence hierarchy
    if( !left )
        return 0;

    while( true ) {
        if( test(ExpressionToken::Multiply) ) {
            left = parseBinaryOp(ExpressionToken::Multiply, left, &ExpressionParser::parsePrimary);
            if( !left )
                return 0;
        } else if( test(ExpressionToken::Divide) ) {
            left = parseBinaryOp(ExpressionToken::Divide, left, &ExpressionParser::parsePrimary);
            if( !left )
                return 0;
        } else if( test(ExpressionToken::Mod) ) {
            left = parseBinaryOp(ExpressionToken::Mod, left, &ExpressionParser::parsePrimary);
            if( !left )
                return 0;
        } else {
            return left;
        }
    }
    return left;
}

ExpressionParserNode* ExpressionParser::parseAssignment()
{
    ExpressionParserNode* left = parseLogicalOr(); // up the precedence hierarchy
    if( !left )
        return 0;

    if( test(ExpressionToken::Assign) ) {
        //Assignment operator is disabled, use is a syntax error
        return 0;

        m_tokenizer->next();
        Q_ASSERT( m_tokenizer->current().type == ExpressionToken::Assign );
        ExpressionParserNode* node = new ExpressionParserNode( this,  m_tokenizer->current() );
        node->leftChild = left;
        node->rightChild = parseAssignment();
        if( !node->rightChild ) {
            qWarning("Expression Expected rvalue of assignment");
            return 0;
        }
        left = node;
        if( !m_semanticAnalyser.checkSemantics(node) )
            return 0;
        m_semanticAnalyser.setReturnType(node);
    }
    return left;
}

ExpressionParserNode* ExpressionParser::parseLogicalOr()
{
    ExpressionParserNode* left = parseLogicalAnd(); // up the precedence hierarchy
    if( !left )
        return 0;

    while( true ) {
        if( test(ExpressionToken::LogicalOr) ) {
            left = parseBinaryOp(ExpressionToken::LogicalOr, left, &ExpressionParser::parseLogicalAnd);
            if( !left )
                return 0;
        } else {
            return left;
        }
    }
    return left;
}

ExpressionParserNode* ExpressionParser::parseBinaryOp( ExpressionToken::Type t, ExpressionParserNode* left, ExpressionParserNode* (ExpressionParser::*func)() ) {
    m_tokenizer->next();
    Q_ASSERT( m_tokenizer->current().type == t );
    Q_UNUSED(t);
    ExpressionParserNode* node = new ExpressionParserNode( this,  m_tokenizer->current() );
    node->leftChild = left;
    node->rightChild = (this->*func)();
    if( !node->rightChild )
        return 0;
    if( !m_semanticAnalyser.checkSemantics(node) )
        return 0;
    m_semanticAnalyser.setReturnType(node);
    return node;
}

ExpressionParserNode* ExpressionParser::parsePrimary()
{
    m_tokenizer->next();
    // variables, numbers, sub expressions, unary minus, postfix inc and dec
    ExpressionParserNode* node = 0;
    switch( m_tokenizer->current().type ) {
        case ExpressionToken::Term:
        {
            node = new ExpressionParserNode( this, m_tokenizer->current() );

            QString vskey = QString(m_tokenizer->current().data.constData());
            if( !vskey.startsWith( "@" ) ) {
                qWarning("Expression - Error - Unknown term, only valuespace keys that start with '@' are handled.");
                return 0;
            }
            if( vskey.count() < 2 ) {
                qWarning("Expression - Error - Null valuespace term specified. Smallest possible term is '/'.");
                return 0;
            }
            vskey = vskey.right( vskey.count()-1 );
            Q_ASSERT(node->vskey.isEmpty());
            node->vskey = vskey;

            m_semanticAnalyser.setReturnType(node);
            return node;
        }
        case ExpressionToken::Bool:
        case ExpressionToken::Integer:
        case ExpressionToken::Double:
        case ExpressionToken::String:
        {
            node = new ExpressionParserNode( this,  m_tokenizer->current() );
            m_semanticAnalyser.setReturnType(node);
            return node;
        }
        case ExpressionToken::LParen:
        {
            node = parseAssignment();
            if( !m_tokenizer->test(ExpressionToken::RParen) ) {
                qWarning("Expression mismatching parenthesis");
                return 0;
            }
            m_tokenizer->next(); // RParen
            if( node == 0 ) {
                qWarning("Error, parenthesis must contain expression");
                return 0;
            }
            // sub expression, semantic checking already done
            return node;
        }
        case ExpressionToken::Minus:
        case ExpressionToken::Not:
        {
            // binary operators
            node = new ExpressionParserNode( this,  m_tokenizer->current() );
            node->leftChild = parsePrimary();
            if( !m_semanticAnalyser.checkSemantics(node) )
                return 0;
            m_semanticAnalyser.setReturnType(node);
            return node;
        }
        case ExpressionToken::Null:
        default:
        {
            qWarning("Exprsssion primary expected, i see '%s'", m_tokenizer->current().data.constData());

            // error: primary expected
            return 0;
        }
    }
}

ExpressionParserNode* ExpressionParser::parseComparison()
{
   /* < > <= >= */
    ExpressionParserNode* left = parseAddSubCat(); // up the precedence hierarchy
    if( !left )
        return 0;

    while( true ) {
        if( test(ExpressionToken::Gt) ) {
            left = parseBinaryOp(ExpressionToken::Gt, left, &ExpressionParser::parseAddSubCat);
            if( !left )
                return 0;
        } else if(test(ExpressionToken::Lt)) {
            left = parseBinaryOp(ExpressionToken::Lt, left, &ExpressionParser::parseAddSubCat);
            if( !left )
                return 0;
        } else if( test(ExpressionToken::GtEq) ) {
            left = parseBinaryOp(ExpressionToken::GtEq, left, &ExpressionParser::parseAddSubCat);
            if( !left )
                return 0;
        } else if( test(ExpressionToken::LtEq) ) {
            left = parseBinaryOp(ExpressionToken::LtEq, left, &ExpressionParser::parseAddSubCat);
            if( !left )
                return 0;
        } else {
            return left;
        }
    }
    return left;
}

ExpressionParserNode* ExpressionParser::parseEquality()
{
    /* == !=  */
    ExpressionParserNode* left = parseComparison(); // up the precedence hierarchy
    if( !left )
        return 0;

    while( true ) {
        if( test(ExpressionToken::Equal) ) {
            left = parseBinaryOp(ExpressionToken::Equal, left, &ExpressionParser::parseComparison);
            if( !left )
                return 0;
        } else if( test(ExpressionToken::NotEqual) ) {
            left = parseBinaryOp(ExpressionToken::NotEqual, left, &ExpressionParser::parseComparison);
            if( !left )
                return 0;
        } else {
            return left;
        }
    }
    return left;

}

ExpressionParserNode* ExpressionParser::parseLogicalAnd()
{
    /* && */
    ExpressionParserNode* left = parseEquality(); // up the precedence hierarchy
    if( !left )
        return 0;

    while( true ) {
        if( test(ExpressionToken::LogicalAnd) ) {
            left = parseBinaryOp(ExpressionToken::LogicalAnd, left, &ExpressionParser::parseEquality);
            if( !left )
                return 0;
        } else {
            return left;
        }
    }
    return left;
}

bool ExpressionParser::test( ExpressionToken::Type t )
{
    Q_ASSERT(m_tokenizer != 0);
    return m_tokenizer->test(t);
}

//+========================================================================================================================+

//=======================================
//= ExpressionMachine Definition
//=======================================

/* Public Methods */
/* ExpressionMachine Ctors */
ExpressionMachine::ExpressionMachine( const QVector<ExpressionMachineInstruction>& instructions, const QVector<ExpressionMachineOperand>& data)
    : m_i(instructions)
    , m_d(data)
    , m_errorflag(false)
{}


/* ExpressionMachine Dtor */
ExpressionMachine::~ExpressionMachine()
{
}

/* ExpressionMachine Helper Macros */
#define DO_DOP(a) op_result.create(ExpressionMachineOperand::Double); op_result.d.d = a; m_s.append( op_result );
#define DO_IOP(a) op_result.create(ExpressionMachineOperand::Integer); op_result.d.i = a; m_s.append( op_result );
#define DO_BOP(a) op_result.create(ExpressionMachineOperand::Bool); op_result.d.b = a; m_s.append( op_result );
#define DO_SOP(a) op_result.create(ExpressionMachineOperand::String); *op_result.d.ref->s = a; m_s.append( op_result );
#define DO_FPOP(a) op_result.create(ExpressionMachineOperand::FixedPoint); fpr = a; \
                   op_result.d.f.precision = fpr.precision; op_result.d.f.value = fpr.value; \
                       m_s.append( op_result ); fpr.value = 0; fpr.precision = 0;

bool ExpressionMachine::execute()
{
    QFixedPointNumber fpr;
    double dr = 0.0;
    int ir = 0;
    bool br = false;
    QByteArray sr;
    m_iptr = 0;
    m_dptr = 0;
    ExpressionMachineOperand op_result;
    // get rid of anything left on the stack too
    m_s.clear();

    while( m_iptr < m_i.count() ) {
        ExpressionMachineInstruction& curInstr = m_i[m_iptr];
        Q_ASSERT(curInstr.code != ExpressionMachineInstruction::NOOP);
        switch( curInstr.type ) {
            case ExpressionMachineOperand::Double:
            {
                switch( curInstr.code ) {
                    case ExpressionMachineInstruction::Load:
                        Q_ASSERT(m_d.count() != 0);
                        Q_ASSERT(m_dptr < m_d.count() );
                        m_s.append(m_d[m_dptr]);
                        Q_ASSERT(curInstr.type == ExpressionMachineOperand::Double);
                        ++m_dptr;
                        break;
                    case ExpressionMachineInstruction::Store:
                        m_result.setValue( stackDouble() );
                        break;
                    case ExpressionMachineInstruction::Negate:
                        DO_BOP( !stackBool() );
                        break;
                    case ExpressionMachineInstruction::Add:
                        dr = stackDouble();
                        DO_DOP( stackDouble() + dr );
                        break;
                    case ExpressionMachineInstruction::Sub:
                        dr = stackDouble();
                        DO_DOP( stackDouble() - dr );
                        break;
                    case ExpressionMachineInstruction::Mul:
                        dr = stackDouble();
                        DO_DOP( stackDouble() * dr );
                        break;
                    case ExpressionMachineInstruction::Div:
                        Q_ASSERT(m_s.count() >= 2);
                        dr = stackDouble();
                        if( dr == 0.0 ) {
                            qWarning("ExpressionMachine: Runtime error - divide by zero");
                            m_errorflag = true;
                            return false;
                        }
                        DO_DOP( stackDouble() / dr );
                        break;
                    case ExpressionMachineInstruction::Sign:
                        DO_DOP( -stackDouble() );
                        break;
                    case ExpressionMachineInstruction::Cmp:
                        dr = stackDouble();
                        DO_DOP( stackDouble() == dr );
                        break;
                    case ExpressionMachineInstruction::LtEq:
                        dr = stackDouble();
                        DO_DOP( stackDouble() <= dr );
                        break;
                    case ExpressionMachineInstruction::GtEq:
                        dr = stackDouble();
                        DO_DOP( stackDouble() >= dr );
                        break;
                    case ExpressionMachineInstruction::Lt:
                        dr = stackDouble();
                        DO_DOP( stackDouble() < dr );
                        break;
                    case ExpressionMachineInstruction::Gt:
                        dr = stackDouble();
                        DO_DOP( stackDouble() > dr );
                        break;
                    case ExpressionMachineInstruction::ProcessTerm:
                    {
                        QVariant cv = processTerm(QVariant::Double);
                        if( cv.isNull() )
                            return false;
                        DO_DOP( cv.toDouble() );
                        break;
                    }
                    default:
                        qFatal("Expression unhandled instruction/data type combination %d/%d", m_i[m_iptr].code, m_i[m_iptr].type);
                }
                break;
            }
            case ExpressionMachineOperand::FixedPoint:
            {
                switch( curInstr.code ) {
                    case ExpressionMachineInstruction::Load:
                        Q_ASSERT(m_d.count() != 0);
                        Q_ASSERT(m_dptr < m_d.count() );
                        m_s.append(m_d[m_dptr]);
                        Q_ASSERT(curInstr.type == ExpressionMachineOperand::FixedPoint);
                        ++m_dptr;
                        break;
                    case ExpressionMachineInstruction::Store:
                        m_result.setValue( QString(stackFixedPoint().toString()) );
                        break;
                    case ExpressionMachineInstruction::Negate:
                        DO_BOP( !stackBool() );
                        break;
                    case ExpressionMachineInstruction::Add:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() + fpr );
                        break;
                    case ExpressionMachineInstruction::Sub:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() - fpr );
                        break;
                    case ExpressionMachineInstruction::Mul:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() * fpr );
                        break;
                    case ExpressionMachineInstruction::Div:
                        Q_ASSERT(m_s.count() >= 2);
                        fpr = stackFixedPoint();
                        if( fpr == false ) {
                            qWarning("ExpressionMachine: Runtime error - divide by zero");
                            m_errorflag = true;
                            return false;
                        }
                        DO_FPOP( stackFixedPoint() / fpr );
                        break;
                    case ExpressionMachineInstruction::Sign:
                        DO_FPOP( -stackFixedPoint() );
                        break;
                    case ExpressionMachineInstruction::Cmp:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() == fpr );
                        break;
                    case ExpressionMachineInstruction::LtEq:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() <= fpr );
                        break;
                    case ExpressionMachineInstruction::GtEq:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() >= fpr );
                        break;
                    case ExpressionMachineInstruction::Lt:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() < fpr );
                        break;
                    case ExpressionMachineInstruction::Gt:
                        fpr = stackFixedPoint();
                        DO_FPOP( stackFixedPoint() > fpr );
                        break;
                    case ExpressionMachineInstruction::ProcessTerm:
                    {
                        QVariant cv = processTerm(QVariant::ByteArray);
                        if( cv.isNull() )
                            return false;
                        DO_FPOP( cv.toByteArray().constData() );
                        break;
                    }
                    default:
                        qFatal("Expression unhandled instruction/data type combination %d/%d", m_i[m_iptr].code, m_i[m_iptr].type);
                }
                break;
            }
            case ExpressionMachineOperand::Integer:
            {
                switch( curInstr.code ) {
                    case ExpressionMachineInstruction::Load:
                        Q_ASSERT(m_d.count() != 0);
                        Q_ASSERT(m_dptr < m_d.count());
                        m_s.append( m_d[m_dptr] );
                        Q_ASSERT(curInstr.type == ExpressionMachineOperand::Integer);
                        ++m_dptr;
                        break;
                    case ExpressionMachineInstruction::Store:
                        m_result.setValue( stackInteger() );
                        break;
                    case ExpressionMachineInstruction::Negate:
                        DO_BOP( !stackBool() );
                        break;
                    case ExpressionMachineInstruction::Add:
                        ir = stackInteger();
                        DO_IOP( stackInteger() + ir );
                        break;
                    case ExpressionMachineInstruction::Sub:
                        ir = stackInteger();
                        DO_IOP( stackInteger() - ir );
                        break;
                    case ExpressionMachineInstruction::Mul:
                        ir = stackInteger();
                        DO_IOP( stackInteger() * ir );
                        break;
                    case ExpressionMachineInstruction::Mod:
                        ir = stackInteger();
                        DO_IOP( stackInteger() % ir );
                        break;
                    case ExpressionMachineInstruction::Sign:
                        DO_IOP( -stackInteger() );
                        break;
                    case ExpressionMachineInstruction::Cmp:
                        ir = stackInteger();
                        DO_IOP( stackInteger() == ir );
                        break;
                    case ExpressionMachineInstruction::LtEq:
                        ir = stackInteger();
                        DO_IOP( stackInteger() <= ir );
                        break;
                    case ExpressionMachineInstruction::GtEq:
                        ir = stackInteger();
                        DO_IOP( stackInteger() >= ir );
                        break;
                    case ExpressionMachineInstruction::Lt:
                        ir = stackInteger();
                        DO_IOP( stackInteger() < ir );
                        break;
                    case ExpressionMachineInstruction::Gt:
                        ir = stackInteger();
                        DO_IOP( stackInteger() > ir );
                        break;
                    case ExpressionMachineInstruction::ProcessTerm:
                    {
                        QVariant cv = processTerm(QVariant::Int);
                        if( cv.isNull() )
                            return false;
                        DO_IOP( cv.toInt() );
                        break;
                    }
                    default:
                        qFatal("Expression unhandled instruction/data type combination %d/%d", m_i[m_iptr].code, m_i[m_iptr].type);
                }
                break;
            }
            case ExpressionMachineOperand::Bool:
            {
                switch( curInstr.code ) {
                    case ExpressionMachineInstruction::Load:
                        Q_ASSERT(m_d.count() != 0);
                        Q_ASSERT(m_dptr < m_d.count());
                        m_s.append( m_d[m_dptr] );
                        Q_ASSERT(curInstr.type == ExpressionMachineOperand::Bool);
                        ++m_dptr;
                        break;
                    case ExpressionMachineInstruction::Store:
                        m_result.setValue( stackBool() );
                        break;
                    case ExpressionMachineInstruction::LogicalOr:
                        {
                        br = stackBool();
                        DO_BOP( stackBool() || br );
                        break;
                        }
                    case ExpressionMachineInstruction::LogicalAnd:
                        br = stackBool();
                        DO_BOP( stackBool() && br );
                        break;
                    case ExpressionMachineInstruction::Cmp:
                    {
                        br = stackBool();
                        DO_BOP( stackBool() == br );
                        break;
                    }
                    case ExpressionMachineInstruction::Negate:
                        DO_BOP( !stackBool() );
                        break;
                    case ExpressionMachineInstruction::ProcessTerm:
                    {
                        QVariant cv = processTerm(QVariant::Bool);
                        if( cv.isNull() )
                            return false;
                        DO_BOP( cv.toBool() );
                        break;
                    }
                    case ExpressionMachineInstruction::LtEq:
                        br = stackBool();
                        DO_BOP( stackBool() <= br );
                        break;
                    case ExpressionMachineInstruction::GtEq:
                        br = stackBool();
                        DO_BOP( stackBool() >= br );
                        break;
                    case ExpressionMachineInstruction::Lt:
                        br = stackBool();
                        DO_BOP( stackBool() < br );
                        break;
                    case ExpressionMachineInstruction::Gt:
                        br = stackBool();
                        DO_BOP( stackBool() > br );
                        break;
                    default:
                        qFatal("Expression unhandled instruction/data type combination %d/%d", curInstr.code, curInstr.type);
                }
                break;
            }
            case ExpressionMachineOperand::String:
            {
                switch( curInstr.code ) {
                    case ExpressionMachineInstruction::Load:
                        Q_ASSERT(m_d.count() != 0);
                        Q_ASSERT(m_dptr < m_d.count());
                        m_s.append( m_d[m_dptr] );
                        Q_ASSERT(curInstr.type == ExpressionMachineOperand::String);
                        ++m_dptr;
                        break;
                    case ExpressionMachineInstruction::Store:
                        m_result.setValue( QString::fromUtf8(stackString()) ); // decode and store as QString, as storing QByteArray prevents easy conversion to numbers through QVariant
                        break;
                    case ExpressionMachineInstruction::Negate:
                    {
                        DO_BOP( !stackBool() );
                        break;
                    }
                    case ExpressionMachineInstruction::Cmp:
                    {
                        sr = stackString();
                        DO_BOP(stackString() == sr);
                        break;
                    }
                    case ExpressionMachineInstruction::Cat:
                    {
                        sr = stackString();
                        QByteArray sl = stackString();
                        DO_SOP( sl + sr );
                        break;
                    }
                    case ExpressionMachineInstruction::ProcessTerm:
                    {
                        QVariant cv = processTerm(QVariant::String);
                        if( cv.isNull() )
                            return false;
                        op_result.create(ExpressionMachineOperand::String);
                        *op_result.d.ref->s = cv.toString().toUtf8();
                        m_s.append( op_result );
                        break;
                    }
                    default:
                        qFatal("Expression unhandled instruction/data type combination %d/%d", curInstr.code, curInstr.type);
                }
                break;
            }
            case ExpressionMachineOperand::Term:
            {
                switch( curInstr.code ) {
                    case ExpressionMachineInstruction::Load:
                        Q_ASSERT(m_d.count() != 0);
                        Q_ASSERT(m_dptr < m_d.count());
                        m_s.append(m_d[m_dptr]);
                        Q_ASSERT(curInstr.type == ExpressionMachineOperand::Term);
                        ++m_dptr;
                        break;
                    default:
                        qFatal("Expression unhandled instruction/data type combination %d/%d", curInstr.code, curInstr.type);
                }
                break;
            }
            default:
                // shouldn't be Term
            qFatal("Expression instruction has invalid data type");
            break;
        }
        ++m_iptr;
    }
    Q_ASSERT(m_s.count() == 0);
    Q_ASSERT(m_result.isValid());
    return true;
}

QVariant ExpressionMachine::result()
{
    if( m_result.isValid() )
        return m_result;
    return QVariant();
}

bool ExpressionMachine::error() const
{
    return m_errorflag;
}

#ifdef EXPRESSION_TESTING
QString ExpressionMachine::dumpInfo() const
{
    //  dumps generated instructions and data, for debugging
    QString string;
    int j = 0;
    for( int i = 0 ; i < m_i.count() ; ++i ) {
        string += instructionToText( m_i[i] ) + "(" + dataTypeToText(m_i[i].type) + ")";
        if( m_i[i].code == ExpressionMachineInstruction::Load ) {
            Q_ASSERT(j < m_d.count());
        string += " " + dataTypeToText(m_d[j].type) + "(" ;
            switch( m_d[j].type ) {
                case ExpressionMachineOperand::Double:
                    {
                QString dstr;
                dstr.setNum( m_d[j].d.d );
                string +=  dstr;
                    }
                break;
                case ExpressionMachineOperand::FixedPoint:
                {
                    string += QFixedPointNumber(m_d[j].d.f.value, m_d[j].d.f.precision).toString();
                }
                break;
                case ExpressionMachineOperand::Integer:
                {
                string += QString::number(m_d[j].d.i);
                }
                break;
                case ExpressionMachineOperand::String:
                {
                string += QString("\"") + m_d[j].d.ref->s->constData() + "\"";
                }
                break;
                case ExpressionMachineOperand::Bool:
                {
                string += m_d[j].d.b  ? "1" : "0";
                }
                break;
                case ExpressionMachineOperand::Term:
                {
                    string += "Term";
                        break;
                }
            }
                string += ")";
                ++j;
        }
        string += " ";
    }
    return string;
}

/* Private Methods */
QString ExpressionMachine::dataTypeToText( ExpressionMachineOperand::Type type ) const
{
    switch( type ) {
        case ExpressionMachineOperand::Integer:
            return "Integer";
        case ExpressionMachineOperand::Double:
            return "Double";
        case ExpressionMachineOperand::Bool:
            return "Bool";
        case ExpressionMachineOperand::String:
            return "String";
        case ExpressionMachineOperand::Term:
            return "Term";
        default:
            return "<unknown>";
    }
}

QString ExpressionMachine::instructionToText( ExpressionMachineInstruction i ) const
{
    switch( i.code ) {
        case ExpressionMachineInstruction::NOOP:
            return "NOOP";
        case ExpressionMachineInstruction::Load:
            return "Load";
        case ExpressionMachineInstruction::Div:
            return "Div";
        case ExpressionMachineInstruction::Store:
            return "Store";
        case ExpressionMachineInstruction::Add:
            return "Add";
        case ExpressionMachineInstruction::Sub:
            return "Sub";
        case ExpressionMachineInstruction::Mul:
            return "Mul";
        case ExpressionMachineInstruction::Mod:
            return "Mod";
        case ExpressionMachineInstruction::Cat:
            return "Cat";
        case ExpressionMachineInstruction::ProcessTerm:
            return "ProcessTerm";
        case ExpressionMachineInstruction::Assign:
            return "Assign";
        case ExpressionMachineInstruction::LogicalAnd:
            return "LogicalAnd";
        case ExpressionMachineInstruction::LogicalOr:
            return "LogicalOr";
        case ExpressionMachineInstruction::Cmp:
            return "Cmp";
        case ExpressionMachineInstruction::Negate:
            return "Negate";
        case ExpressionMachineInstruction::LtEq:
            return "LtEq";
        case ExpressionMachineInstruction::GtEq:
            return "GtEq";
        case ExpressionMachineInstruction::Lt:
            return "Lt";
        case ExpressionMachineInstruction::Gt:
            return "Gt";
        default:
            return "<unknown>";
    }
}
#endif

QVariant ExpressionMachine::processTerm( const QVariant::Type& t )
{
    Q_ASSERT(m_s.count() != 0);
    Q_ASSERT(m_s.last().type == ExpressionMachineOperand::Term);
    QVariant cv;
    cv = m_s.last().d.ref->t->value();
    m_s.pop_back();
    if( cv.isNull() ) { // FIXME : decide on the semantics of handling runtime errors. a valuesapce key that currently has a null value shouldn't
                        // necessarily trigger a 'hard' error
        cv.setValue( QString("") );
    }
    if( !cv.canConvert(t) ) {
        /* XXX special case
           Number types can't be directly converted to a bytearray using QVariant::toByteArray().
            if t == ByteArray and QVariant::canConvert() fails, fall back to trying to convert to a string and then a byterray.
            if that fails, then the conversion fails.
        */
        if(t == QVariant::ByteArray && cv.canConvert(QVariant::String)) {
            cv.setValue(cv.toString());
            Q_ASSERT(cv.canConvert(t));
        } else {
            qWarning("ExpressionMachine - cannot coerce term to QVariant::Type %d", t); // FIXME : printing qvariant enum isn't great runtime error reporting
            m_errorflag = true;
        }
    }
    return cv;
}

int ExpressionMachine::stackInteger()
{
    Q_ASSERT(m_s.count() != 0);
    ExpressionMachineOperand& data = m_s.last(); // refer to the value on stack, don't copy locally
    Q_ASSERT(ExpressionMachineOperand::Term != data.type);
    int i = 0;
    switch( data.type ) { // runtime type conversion * -> int
        case ExpressionMachineOperand::Integer:
            i = data.d.i;
            break;
        case ExpressionMachineOperand::FixedPoint:
            {
            QFixedPointNumber f( data.d.f.value, data.d.f.precision );
            i = f.whole();
            break;
            }
        case ExpressionMachineOperand::Bool:
            i = (int)data.d.b;
            break;
        case ExpressionMachineOperand::String:
        {
            Q_ASSERT(data.d.ref != 0);
            Q_ASSERT(data.d.ref->s != 0);
            const char *c = data.d.ref->s->constData();
            if( c == 0 ) i = 0;
            else i = atoi(c);
            // FIXME : should check error code, runtime error if can't convert
            break;
        }
        case ExpressionMachineOperand::Double:
            i = (int)data.d.d; // don't do any rounding
            break;
        case ExpressionMachineOperand::Term:
            qFatal("ExpressionMachine::stackInteger() called with Term on stack");
    }
    m_s.pop_back(); // discard
    return i;
}

QByteArray ExpressionMachine::stackString()
{
    Q_ASSERT(m_s.count() != 0);
    ExpressionMachineOperand data = m_s.last(); // runtime type conversion * -> string
    Q_ASSERT(ExpressionMachineOperand::Term != data.type);
    char buf[64];
    QByteArray s;
    switch( data.type ) {
        case ExpressionMachineOperand::String:
            Q_ASSERT(data.d.ref != 0);
            Q_ASSERT(data.d.ref->s != 0);
            s = *data.d.ref->s;
            break;
        case ExpressionMachineOperand::Double:
            {
            int n = snprintf(buf, 64, "%f", data.d.d);
            if( n > 64 ) {
                qWarning("ExpressionMachine::stackString() - Conversion to double truncated to 64 chars from %d", n);
            }
            s = QByteArray(buf); // force a deep copy and creation
            break;
            }
        case ExpressionMachineOperand::FixedPoint:
            {
            QFixedPointNumber f( data.d.f.value, data.d.f.precision );
            QString str = f.toString();
            s = QByteArray((const char*)str.toAscii().data()); // FIXME - two copies of data here
            break;
            }
        case ExpressionMachineOperand::Bool:
        {
            s = QByteArray(data.d.b ? "1" : "0");
            break;
        }
        case ExpressionMachineOperand::Integer:
        {
            int n = snprintf(buf, 64, "%d", data.d.i );
            if( n > 64 ) {
                qWarning("ExpressionMachine::stackString() - Conversion to integer truncated to 64 from %d", n);
            }
            s = QByteArray((const char *)buf); // force a deep copy and creation
            break;
        }
        case ExpressionMachineOperand::Term:
            qFatal("ExpressionMachine::stackString() called with Term on stack");
    }
    m_s.pop_back();
    return s;
}

double ExpressionMachine::stackDouble()
{
    Q_ASSERT(m_s.count() != 0);
    ExpressionMachineOperand& data = m_s.last(); // runtime type conversion * -> double
    Q_ASSERT(ExpressionMachineOperand::Term != data.type);
    Q_ASSERT(ExpressionMachineOperand::FixedPoint != data.type);
    double d = 0;
    switch( data.type ) {
        case ExpressionMachineOperand::Double:
            d = data.d.d;
            break;
        case ExpressionMachineOperand::String:
        {
            Q_ASSERT(data.d.ref != 0);
            Q_ASSERT(data.d.ref->s != 0);
            d = atof((data.d.ref->s->constData()));
            // FIXME : should check error code, runtime error if can't convert
            break;
        }
        case ExpressionMachineOperand::Bool:
            d = data.d.b ? 1.0 : 0.0;
            break;
        case ExpressionMachineOperand::Integer:
            d = (double)data.d.i;
            break;
        case ExpressionMachineOperand::FixedPoint:
            Q_ASSERT(0); // can't happen
            break;
        case ExpressionMachineOperand::Term:
            qFatal("ExpressionMachine::stackDouble() called with Term on stack");
    }
    m_s.pop_back();
    return d;
}

QFixedPointNumber ExpressionMachine::stackFixedPoint()
{
    Q_ASSERT(m_s.count() != 0);
    ExpressionMachineOperand& data = m_s.last(); // runtime type conversion * -> double
    Q_ASSERT(ExpressionMachineOperand::Term != data.type);
    Q_ASSERT(ExpressionMachineOperand::Double != data.type);
    QFixedPointNumber f;
    switch( data.type ) {
        case ExpressionMachineOperand::Double:
            Q_ASSERT(0);
            break; // can't happen
        case ExpressionMachineOperand::FixedPoint:
            f.value = data.d.f.value;
            f.precision = data.d.f.precision;
            break;
        case ExpressionMachineOperand::String:
        {
            Q_ASSERT(data.d.ref != 0);
            Q_ASSERT(data.d.ref->s != 0);
            f = (const char*)data.d.ref->s->constData();
            // FIXME : error checking?
            //Q_ASSERT(ok == true); // failing is a bug, as semantic checking was not properly done
            break;
        }
        case ExpressionMachineOperand::Bool:
            f.precision = 0;
            f.value = data.d.b ? 1 : 0;
            break;
        case ExpressionMachineOperand::Integer:
            f.precision = 0;
            f.value = data.d.i;
            break;
        case ExpressionMachineOperand::Term:
            qFatal("ExpressionMachine::stackDouble() called with Term on stack");
    }
    m_s.pop_back();
    return f;
}

bool ExpressionMachine::stackBool()
{
    Q_ASSERT(m_s.count() != 0);
    ExpressionMachineOperand& data = m_s.last(); // runtime type conversion * -> bool
    Q_ASSERT(ExpressionMachineOperand::Term != data.type);
    bool b = false;
    switch( data.type ) {
        case ExpressionMachineOperand::Bool:
            b = data.d.b;
            break;
        case ExpressionMachineOperand::FixedPoint:
            b = data.d.f.value != 0;
            break;
        case ExpressionMachineOperand::Double:
            b = data.d.d != 0.0;
            break;
        case ExpressionMachineOperand::String:
            Q_ASSERT(data.d.ref != 0);
            Q_ASSERT(data.d.ref->s != 0);
            b = !data.d.ref->s->isEmpty();
            break;
        case ExpressionMachineOperand::Integer:
            b = data.d.i != 0;
            break;
        case ExpressionMachineOperand::Term:
            qFatal("ExpressionMachine::stackDouble() called with Term on stack");
    }
    m_s.pop_back();
    return b;
}

//+========================================================================================================================+

//=======================================
//= QExpressionEvaluatorPrivate Definition
//=======================================

/*!
    \class QExpressionEvaluator
    \inpublicgroup QtBaseModule

    \brief The QExpressionEvaluator class computes the results of arithmetic, logical and string based expressions.

    Most interestingly, the expression evaluator has the ability to use values
    from the Qt Extended valuespace in its expressions and signal when the resulting
    expression changes through termsChanged().

    \section1 Overview
    It takes as input through setExpression() an expression such as "2 + 2" and calculates the result.

    Here is an example of basic usage of the expression evaluator.
    \code
    QExpressionEvaluator ee;
    ee.setExpression("2 + 2");
    if(!ee.isValid()) { // check if syntax/semantics correct.
        qWarning("Syntax or semantic error in expression.");
    } else {
        if(!ee.evaluate()) { // evaluate the expression
            qWarning("Run-time error when evaluating expression"); // run-time error - type coercion or divide by zero.
        } else {
            QVariant result = ee.result();  // retrieve the result
            int x = result.toInt(); // x == 4
        }
    }
    \endcode

    \section1 Details

    The expression evaluator functionally operates in 2 phases.
    The first phase is the setup phase, where the specified expression is tokenized, parsed, semantically checked and byte code generated.
    The isValid() function returns whether this entire setup phase is successful ie. the specified expression is syntactually and semantically valid.

    The second phase is the evaluation phase, where the generated bytecode from the first phase is executed to compute the result of the expression.
    The evaluate() function uses a virtual machine to execute the bytecode and store the resulting value.
    Run-time errors occur when an expression attempts to divide-by-zero or if an expression evaluator term cannot be coerced to a required type.
    The return value of evaluate() indicates whether there were any run-time errors.
    If evaluate() returns true, result() can then be called to retreive the calcuated result of the expression as a QVariant. You should
    cast the variant to the data type you expect.

    \section1 Supported Data Types

    The expression evaluator supports 5 data types: int, real, string and bool.
    real numbers are represented either using the C++ double data type, or
    by using fixed point calculations. C++ double is the default, but you probably
    want to use the fixed point format for user visible applications of the expression evaluator, such as
    a desktop calculator. This can be set using the setFloatingPointFormat() function.

    \section1 Supported Operators

    Table is in highest to lowest precedence.
    \table
    \header
       \o Operator
       \o Description
    \row
        \o ( ) - ! \o Brackets to control evaluation order of expression, unary minus, logical negation. eg. "2 * (3 + 4)" "-20" "!false"
    \row
        \o  * / % \o Arithmetic multiplication, division and modulus. eg. "2 * 3 / 3 % 3"
    \row
        \o + - . \o Arithmetic addition and subtraction. String concatenation. eg. "2 + 3 - 4" "\"hello\" . \"world\""
    \row
        \o < > <= >= \o Logical comparison. eg "10.9 > = 10.8"
    \row
        \o == != \o Logical equality. eg. "true != false"
    \row
        \o && \o Logical AND eg. "true && true"
    \row
        \o || \o Logical OR eg. "false || true"
    \endtable

    \section1 Type Conversion

    Operators in the expression evaluator work with operands of the same type ie. the '+' operator can only add two integer or two real numbers.
    However, a user may still mix data types and the operands will be coerced to the correct type for the operator.

    Consider the expression: 2 + "2"

    The '+' operator adds two number operands, but the RHS operand is a string. As the LHS is an integer, the expression evaluator tries to coerce the RHS string to an integer to meet the requirements of the '+' operator. In this case, the coercion would succeed and "2" would simply become an integer 2. The result of the expression would then be the integer 4.

    If operands cannot be coerced to the required types for an operator, then for static coercion the setup phase fails and isValid() returns false and for run-time coercion evaluate() returns false.

    Run-time coercion failures can occur for 2 reasons:
    \list
        \i a term, eg. a key in the valuespace, does not return a value that can be coerced to the required type, or
        \i a return value of one side of the expression cannot be coerced to the required type. eg. 2 + ("2" . "@/value/in/valuespace") would fail at run-time because the RHS of '+' is a string, calculated at runtime, that cannot be coerced to an integer.
    \endlist

    Below are the data type requirements of the expression evaluator's operators.
    \table
    \header
        \o Operator
        \o Required operand types
        \o Return type
    \row
        \o Basic arithmetic (* / + -)
        \o Alteast 1 double or integer.
        \o / always returns a real, others return real or integer depending on operands.
    \row
        \o Modulus (%)
        \o Two integers.
        \o integer
    \row
        \o String Concatentation (.)
        \o Atleast 1 string.
        \o string
    \row
        \o Logical Comparison (< > <= >=)
        \o Atleast 1 double, integer or bool.
        \o bool
    \row
        \o Other Logical (== != && || !)
        \o All types.
        \o bool
    \endtable

    \section1 Valuespace integration
    The expression evaluator has the ability to use values from the Qt Extended valuespace in its expressions.

    \code
    2 + @/Value/Space/Key
    \endcode

    The above code would add 2 and the value for /Value/Space/Key in the valuespace.

    The '@' character uniquely identifies the subsequent characters as a valuespace key to the expression evaluator.

    In the above example, /Value/Space/Key must return data that can be
    converted to an integer. If conversion fails, a run-time error will
    occur and evaluate() will return false.

    If a value in the valuespace changes, expressions which use that value will
    emit the termsChanged() signal.

    \ingroup misc
*/

/*
    \o Multiplication(*)
    \o Division(/)
    \o Addition(+)
    \o Subtraction(-)
    \o Less Than(<)
    \o Greater Than(>)
    \o Less Than Or Equal To(<=)
    \o Greater Than Or Equal To(>=)
    \o Equality(==)
    \o Inequality(!=)
    \o Logical AND(&&)
    \o Logical OR(||)
    \o Negation(!)
*/

/*!
    \fn void QExpressionEvaluator::termsChanged()

    Emitted when a pluggable term in the expression changes.
*/

/*!
    \enum QExpressionEvaluator::FloatingPointFormat

    Controls the floating point format used by the expression evaluator.

    \value Double C++ double data type.
    \value FixedPoint Fixed point arithmetic using an 8byte integer.
*/


/* Public Methods */
/* QExpressionEvaluatorPrivate Ctors */
QExpressionEvaluatorPrivate::QExpressionEvaluatorPrivate()
    : machine(0), floatingPointFormat(QExpressionEvaluator::Double)
{}

//+========================================================================================================================+

//=======================================
//= QExpressionEvaluator Definition
//=======================================

/* Public Methods */

/* QExpressionEvaluator Ctors */
/*!
    Constructs a QExpressionEvaluator.
    \a parent is passed to the QObject constructor.
*/
QExpressionEvaluator::QExpressionEvaluator( QObject* parent )
    : QObject(parent)
{
    d = new QExpressionEvaluatorPrivate;
}

/*!
    Constructs a QExpressionEvaluator.

    \a expr is passed to setExpression().
    \a parent is passed to the QObject constructor.
*/
QExpressionEvaluator::QExpressionEvaluator( const QByteArray& expr, QObject* parent )
    : QObject(parent)
{
    d = new QExpressionEvaluatorPrivate;
    setExpression( expr );
}

/*!
    Destroys the QExpressionEvaluator.
*/
QExpressionEvaluator::~QExpressionEvaluator()
{
    if( d->machine )
        delete d->machine;

    delete d;
}

/*!
    Returns true if this expression is valid, otherwise false.

    An expression is valid if it is not empty, it is syntactually
    and semantically correct, and a previous call to evaluate() did not result in a run-time error.

    You must not call evaluate() unless this function returns true.
*/
bool QExpressionEvaluator::isValid() const
{
    return !d->expressionData.isEmpty() && d->machine != 0 && !d->machine->error();
}

/*!
    Evaluates the expression specified through setExpression().

    isValid() must return true before you call this function, otherwise it will abort().

    Returns true if the expression was successfully evaluated, or false if a run-time error occured.

    A run-time error can occur due to a coercion error or divide by zero.

    If this function returns true, the result of the evaluation can be retrieved using result().
*/
bool QExpressionEvaluator::evaluate()
{
    Q_ASSERT(d->machine != 0);
    if( !d->machine->execute() ) {
        // error: machine runtime error
        qWarning("error: machine runtime error");
        return false;
    }
    d->result = d->machine->result();
    return true;
}

/*!
    Returns the result of the last call to evaluate() as a QVariant.

    Returns an empty QVariant if evaluate() has not yet been called.
*/
QVariant QExpressionEvaluator::result()
{
    if( d->machine )
        return d->machine->result();
    return QVariant();
}

/*!
    Clears the set expression and any associated data structures.

    The expression becomes invalid ie. isValid() returns false.
*/
void QExpressionEvaluator::clear()
{
    if( d->machine != 0 ) {
        delete d->machine;
        d->machine = 0;
    }
}

/*!
    Sets the floating point format to be used by the expression evaluator to \a fmt.

    The default is QExpressionEvaluator::Double, but setting QExpressionEvaluator::FixedPoint
    may give results appropriate for user level applications.
*/
void QExpressionEvaluator::setFloatingPointFormat( const FloatingPointFormat& fmt ) {
    if( d->floatingPointFormat != fmt ) {
        d->floatingPointFormat = fmt;
        if( d->machine != 0 )
            qWarning("QExpressionEvaluator::setFloatingPointFormat - Called with existing expression, no effect until next call to QExpressionEvaluator::setExpression()");
    }
}

/*!
    Returns the current floating point format.
*/
QExpressionEvaluator::FloatingPointFormat QExpressionEvaluator::floatingPointFormat() const {
    return d->floatingPointFormat;
}

/*!
    Sets the expression for the evaluator to be \a expr

    \a expr is parsed and semantically checked immediat

    Returns true on success; otherwise returns false.

    You should call isValid() after this function to determine whether
    the setup phase was completed successfully.
*/
bool QExpressionEvaluator::setExpression( const QByteArray& expr )
{
    bool ok = true;
    if( d->expressionData != expr ) {
        d->expressionData = expr;
        clear();
        if( d->expressionData.isEmpty() )
            return true;
        /* Parse the expression now */

        /* Create a parser instance */
        ExpressionParser* pp = new ExpressionParser( expr );
        ExpressionParserNode* node = pp->parse();
        if( node == 0 ) {
            // error: Unable to parse the expression
            qWarning("error: unable to parse the expression");
            ok = false;

        } else {
            /* Do semantic checking and code generator on the parse tree */
            ExpressionCodeGenerator* ss = new ExpressionCodeGenerator( node );
            ss->setFixedPoint( d->floatingPointFormat == QExpressionEvaluator::FixedPoint );
            if( ss->generate() ) {
                /* Checked out OK, should have generated code */
//  dumps generated instructions and data, for debugging

                QVector<ExpressionMachineOperand> data = ss->data();
                for( int i = 0 ; i < data.count() ; ++i )
                    if( data[i].type == ExpressionMachineOperand::Term )
                        connect(data[i].d.ref->t, SIGNAL(contentsChanged()), this, SIGNAL(termsChanged()));
                d->machine = new ExpressionMachine( ss->instructions(), data);

#ifdef EXPRESSION_TESTING
                qWarning("machine data is:\n%s", d->machine->dumpInfo().toAscii().data());
#endif
            } else {
                // error: semantic error in the expression
                qWarning("error: semantic error in the expression");
                ok = false;
            }
            delete ss;
        }
        delete pp; // only thing left in memory is machine
    }
    return ok;
}

/*!
    Returns the current expression data the evaluator is operating on.
*/
QByteArray QExpressionEvaluator::expression() const
{
    return d->expressionData;
}

//+========================================================================================================================+

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

//TESTED_COMPONENT=PIM: Productivity: Calculator (18572)

testcase = {

    initTestCase: function()
    {
        waitForQtopiaStart();

        // All tests can be run consecutively, so only run it once
        startApplication( "Calculator" );
    },

    cleanupTestCase: function()
    {
        deletePath("$HOME/Documents/text/plain") ;
    },

    init: function()
    {
        calculator = focusWidget();
    },

    cleanup: function()
    {
        if (currentApplication() == "calculator")
            select( "Clear All", optionsMenu() );
        else
            startApplication( "Calculator" );
    },

/*
    \req QTOPIA-231
    \groups Acceptance
*/
    binary_operations_data: {
        "operation1": ["0", "+", "0", "0"],
        "operation2": ["0", "-", "0", "0"],
        "operation3": ["0", "*", "0", "0"],
        "operation4": ["0", "/", "0", "Divide by zero error"],
        "operation5": ["0", "+", "1", "1"],
        "operation6": ["0", "-", "1", "-1"],
        "operation7": ["0", "*", "1", "0"],
        "operation8": ["0", "/", "1", "0"],
        "operation9": ["0.56789", "+", "7453.0145", "7453.58239"],
        "operation10": ["0.56789", "-", "7453.0145", "-7452.44661"],
        "operation11": ["0.56789", "*", "7453.0145", "4232.4924044"],
        "operation12": ["0.56789", "/", "7453.0145", "0.000076196"]
    },

    binary_operations: function(FirstOperand, Operator, SecondOperand, ExpectedResult)
    {
        enter( FirstOperand );
        select(Operator, calculator);
        enter( SecondOperand );
        select( "=", calculator);

        // FIXME: Wait for clock icon to fade out
        wait(500);
        // FIXME: This really should be a simple:  compare( getText(), "something" );
        verifyImage( "binary_operations_" + currentDataTag(), "", "Calculate " + FirstOperand + Operator + SecondOperand + " = " + ExpectedResult );
    },

/*
    \req QTOPIA-235
*/
    bounds_checking_data: {
        "operation1": ["4294967295", "+", "1", "", "", "4294967296"], //Unsigned integer overflow
        "operation2": ["2147483647", "+", "1", "", "", "2147483648"], //Positive Integer Overflow
        "operation3": ["0", "-", "2147483648", "-", "1", "-2147483649"], //Negative integer overflow
        "operation4": ["0", "-", "999999999999", "-", "1", "Calc surpasses limit"], //Largest negative number overflow
        "operation5": ["999999999999", "*", "10", "+", "10", "Calc surpasses limit"], //Largest positive number overflow
        "operation6": ["123456789123", "/", "2", "", "", "61728394561.5"] //See Hooligan task 199553
    },

    bounds_checking: function(FirstOperand,Operator,SecondOperand,SecondOperator,ThirdOperand,ExpectedResult)
    {
        enter( FirstOperand );
        select( Operator, calculator );
        enter( SecondOperand );

        if (SecondOperator != "") {
            select( SecondOperator, calculator );
            enter( ThirdOperand );
        }

        select( "=", calculator);

        verifyImage( "bounds_checking_" + currentDataTag(), "",
            "Calculate " + FirstOperand + Operator + SecondOperand + SecondOperator + ThirdOperand + " = " + ExpectedResult );
    },

/*
    \req QTOPIA-232
    \groups Acceptance
*/
    continuous_binary_operations: function()
    {
        enter( "2.12" );
        select( "-", calculator );
        enter( "44" );
        select( "=", calculator );
        verifyImage( "continuous_binary_operations_step1", "", "Verify that the number '-41.88' is displayed." );

        select( "*", calculator );
        enter( "2.4354" );
        select( "=", calculator );
        verifyImage( "continuous_binary_operations_step2", "", "Verify that the result displayed is '-101.994552'." );

        select( "+", calculator );
        enter( "3210.5" );
        select( "=", calculator );
        verifyImage( "continuous_binary_operations_step3", "", "Verify that the result is '3108.505448'." );

        select( "/", calculator );
        enter( "59" );
        select( "=", calculator );
        verifyImage( "continuous_binary_operations_step4", "",
            "Verify that the results for the running calculation is '52.686533017'." );
    },

/*
    \req QTOPIA-234
*/
    clearing_values: function()
    {
        enter( "123" );
        select( "Clear All", optionsMenu() ) ;
        verifyImage( "clearing_values_step1", "", "Verify that the number is cleared and the displayed value is set to '0'." );

        enter( "456" );
        select("Delete", softMenu());
        verifyImage( "clearing_values_step2", "",
            "Verify that the '6' appended to the entered number '456' is removed resulting in the number '45'." );

        select("Delete", softMenu());
        verifyImage( "clearing_values_step3", "",
            "Verify that the '5' is removed from the number leaving '4'." );

        enter( "789" );
        verifyImage( "clearing_values_step4", "",
            "Verify that the entered number is appended to the displayed number, resulting it '4789'." );

        select( "+", calculator );
        enter( "654" );
        select("Delete", softMenu());
        verifyImage( "clearing_values_step5", "",
            "Verify that the displayed number is now '65'." );

        select("Delete", softMenu());
        select("Delete", softMenu());
        select("Delete", softMenu());
        verifyImage( "clearing_values_step6", "",
            "Verify that the numbers '5', '6' (including the operator) and '9' from the previous number '4789' are removed." );

        select( "+", calculator );
        enter( "11" );
        select( "=", calculator );
        verifyImage( "clearing_values_step7", "",
            "Verify that the result is calculated in '489'." );

        select("Delete", softMenu());
        verifyImage( "clearing_values_step8", "",
            "Verify that the whole result is cleared leaving '478+11'." );

        select("Delete", softMenu());
        verifyImage( "clearing_values_step9", "",
            "Verify that the whole number and operator '+ 11' is removed." );

        select( "-", calculator );
        enter( "22" );
        select( "=", calculator );
        verifyImage( "clearing_values_step10", "",
            "Verify that the number '456' is displayed." );

        select( "Clear All", optionsMenu() ) ;
        verifyImage( "clearing_values_step11", "",
            "Verify that the entire calculation is cleared." );
    },

/*
    \req QTOPIA-231
*/
    copying_results: function()
    {
        //expectFail( "BUG-212377" );    // Copy/paste not working in Qtopia 4.4

        enter( "3892.392" );
        select( "Copy", optionsMenu() ) ;
        waitFor() { return ( getClipboardText() == "3892.392" ); }
        select( "Clear All", optionsMenu() );

        enter( "112" );
        select( "/", calculator );
        enter( "3.3" );
        select( "Copy", optionsMenu() ) ;
        waitFor() { return ( getClipboardText() == "3.3" ); }
        select( "Clear All", optionsMenu() );

        enter( "293.49" );
        select( "+", calculator );
        enter( "1002" );
        select( "=", calculator );
        select( "Copy", optionsMenu() ) ;
        waitFor() { return ( getClipboardText() == "1295.49" ); }
        select( "Clear All", optionsMenu() );

        enter( "25.4" );
        select( "-", calculator );
        enter( "250.2" );
        select( "=", calculator );
        select( "Copy", optionsMenu() ) ;
        waitFor() { return ( getClipboardText() == "-224.8" ); }
    },

/*
    \req QTOPIA-231
*/
    pasting_results: function()
    {
        //expectFail( "BUG-212377" );    // Copy/paste not working in Qtopia 4.4
        //expectFail( "BUG-199904" );    // Cannot select equals after pasting into Calculator

        setClipboardText( "12345" );
        select( "Paste", optionsMenu() );
        select( "+", calculator );
        setClipboardText( "11111" );
        select( "Paste", optionsMenu() );
        select( "=", calculator );
        select( "Copy", optionsMenu() ) ;
        compare( getClipboardText(), "23456" );
        select( "Clear All", optionsMenu() );

        //expectFail( "BUG-199542" ); //Cannot paste negative numbers into Calculator
        setClipboardText( "-357" );
        select( "Paste", optionsMenu() );
        select( "+", calculator );
        enter( "1000" );
        select( "=", calculator );
        select( "Copy", optionsMenu() ) ;
        compare( getClipboardText(), "643" );
    },

/*
    \req QTOPIA-231
*/
    changing_operators_data:
    {
        "test01": ["123","999","+","+"],
        "test02": ["223","899","+","-"],
        "test03": ["323","799","+","/"],
        "test04": ["423","699","+","*"],
        "test05": ["523","599","+","+"],
        "test06": ["623","499","+","-"],
        "test07": ["723","399","+","/"],
        "test08": ["823","299","+","*"],
        "test09": ["923","199","+","+"],
        "test10": ["023","356","+","-"],
        "test11": ["123","456","+","/"],
        "test12": ["223","684","+","*"],
        "test13": ["323","698","+","+"],
        "test14": ["423","258","+","-"],
        "test15": ["523","457","+","/"],
        "test16": ["623","479","+","*"]
    },

    changing_operators: function(Number1,Number2,Operator1,Operator2)
    {
        enter( Number1 );
        select( Operator1, calculator );
        select( Operator2, calculator );
        enter( Number2 );
        select( "=", calculator );
        verifyImage(  "changing_operators_" + currentDataTag(), "",
            "Verify that the correct result is calculated for " + Number1 + " " + Operator2 + " " + Number2 + " = " + eval(Number(Number1) + Operator2 + Number(Number2)) );
    },

/*
    \req
*/
    perf_startup: function()
    {
        Performance.testStartupTime("Applications/Calculator");
    }

} // end of test

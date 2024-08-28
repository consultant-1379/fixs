//*****************************************************************************
// 
// .NAME
//     GetOptImpl - Command line parser
// .LIBRARY 3C++
// .INCLUDE Get_opt.h

// .COPYRIGHT
//  COPYRIGHT Ericsson Utvecklings AB, Sweden 2002.
//  All rights reserved.
//
//  The Copyright to the computer program(s) herein 
//  is the property of Ericsson Utvecklings AB, Sweden.
//  The program(s) may be used and/or copied only with 
//  the written permission from Ericsson Utvecklings AB or in 
//  accordance with the terms and conditions stipulated in the 
//  agreement/contract under which the program(s) have been 
//  supplied.

// .DESCRIPTION 
// Iterator for parsing command-line arguments.
// This is a C++ wrapper for getopt(3c).
//

// .ERROR HANDLING
//
//  .fill_off
//  General rule:
//  The error handling is specified for each method.
//
//  No methods initiate or send error reports unless specified.
//  .fill_on

// AUTHOR 
//      2010-04-09 by QSALCAM

// CHANGES
//
//      RELEASE REVISION HISTORY
//
//      REV NO          DATE            NAME    DESCRIPTION
//      A               xx-xx-xx        MNNN    First release
//
//*****************************************************************************

#ifndef GET_OPT_H
#define GET_OPT_H

//Iterator for parsing command-line arguments.
//This is a C++ wrapper for getopt(3c).
class Get_opt
{
public:
    // Initialize the internal data when the first call is made.  Start
    // processing options with <argv>-element 0 + <skip_argv0>; the
    // sequence of previously skipped non-option <argv>-elements is
    // empty.
    //
    // <optstring> is a string containing the legitimate option
    // characters.  A colon in <optstring> means that the previous
    // character is an option that wants an argument.  The argument is
    // taken from the rest of the current <argv>-element, or from the
    // following <argv>-element, and returned in <optarg>.
    //
    // If an option character is seen that is not listed in <optstring>,
    // return '?' after printing an error message.  If you set <opterr>
    // to zero, the error message is suppressed but we still return '?'.
    //
    // If a char in <optstring> is followed by a colon, that means it
    // wants an arg, so the following text in the same <argv>-element,
    // or the text of the following <argv>-element, is returned in
    // <optarg>.
    Get_opt(int argc, char **argv, const char *optstring, 
        int skip_argv0 = 1, int report_errors = 0);
    
    // Default dtor.
    ~Get_opt();
    
    // Scan elements of <argv> (whose length is <argc>) for option
    // characters given in <optstring>.
    //
    // If an element of <argv> starts with '-', and is not exactly "-"
    // or "--", then it is an option element.  The characters of this
    // element (aside from the initial '-') are option characters.  If
    // <operator()> is called repeatedly, it returns successively each
    // of the option characters from each of the option elements.
    //
    // If <operator()> finds another option character, it returns that
    // character, updating <optind> and <nextchar> so that the next call
    // to <operator()> can resume the scan with the following option
    // character or <argv>-element.
    //
    // If there are no more option characters, <operator()> returns
    // <EOF>.  Then <optind> is the index in <argv> of the first
    // <argv>-element that is not an option.  (The <argv>-elements have
    // been permuted so that those that are not options now come last.)
    
    // = Public data members (should be hidden...).
    int operator()();
    
    int operator()(bool dummy);
    // For communication from <operator()> to the caller.  When
    // <operator()> finds an option that takes an argument, the argument
    // value is returned here.
    char *optarg;
    
    // Index in <argv> of the next element to be scanned.  This is used
    // for communication to and from the caller and for communication
    // between successive calls to <operator()>.  On entry to
    // <operator()>, zero means this is the first call; initialize.
    //
    // When <get_opt> returns <EOF>, this is the index of the first of
    // the non-option elements that the caller should itself scan.
    //
    // Otherwise, <optind> communicates from one call to the next how
    // much of <argv> has been scanned so far.
    int optind;
    
    // Callers store zero here to inhibit the error message for
    // unrecognized options.
    int opterr;
    
    // Holds the <argc> count.
    int argc_;
    
    // Holds the <argv> pointer.
    char **argv_;
    
private:
    // The next char to be scanned in the option-element in which the
    // last option character we returned was found.  This allows us to
    // pick up the scan where we left off.
    //
    // If this is zero, or a null string, it means resume the scan
    // by advancing to the next <argv>-element.
    char *nextchar_;
    
    // Holds the option string.
    const char *optstring_;
};

#endif

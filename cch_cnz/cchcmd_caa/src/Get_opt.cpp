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

#include <string.h>
#include <cstdio>
#include <iostream>
#include "Get_opt.h"

using namespace std;

//*****************************************************************************
//
//##ModelId=3CCCE4D3033B
Get_opt::Get_opt(int argc, char **argv, const char *optstring,
                                       int skip_argv0, int report_errors):
optarg (0), optind (skip_argv0), opterr (report_errors),
argc_ (argc), argv_ (argv), nextchar_ (0), optstring_ (optstring)
{
}

//*****************************************************************************
//
//##ModelId=3CCCE4D30363
Get_opt::~Get_opt()
{
}

//*****************************************************************************
//
//##ModelId=3CCCE4D30364
int Get_opt::operator()()
{
    if (argv_ == 0)
    {
        optind = 0;
        return EOF;
    }
    
    int opt; // Character checked for validity.
    const char *oli; // Option letter index.
    
    if (nextchar_ == 0 || *nextchar_ == '\0')
    { 
        // Update scanning pointer.
        
        if (optind >= argc_ || (*(nextchar_ = argv_[optind])) != '-')
        {
            nextchar_ = "";
            optarg = argv_[optind];
            ++optind;
            return EOF;
        }
        
        if (nextchar_[1] != 0 && *(++nextchar_) == '-') 
        {	
            // Found "--".
      		++optind;
            nextchar_ = "";
//      	strcpy(nextchar_, "");
            return -2;
        }
    }			
    
    // Option letter okay? 
    opt = (int) *(nextchar_++);
    if (opt == (int) ':' || ((oli = strchr(optstring_, opt)) == 0))
    {
        // If the user didn't specify '-' as an option, assume it means
        // EOF.
        if (opt == (int) '-')
        	return EOF;
        
        if (*nextchar_ == 0)
            ++optind;
        
        if (opterr && *optstring_ != ':')
            cerr << argv_[0] 
            << ": illegal option -- "
            << opt
            << "\n";
        return '?';
    }
    
    if (*++oli != ':') 
    { // Don't need argument.
        optarg = 0;
        
        if (!*nextchar_)
            ++optind;
    }
    else 
    { // Need an argument.
        if (*nextchar_) // No white space.
		{
			optarg = nextchar_;
			if (opterr)
				cerr << argv_[0]
				<< ": option requires a white space -- "
				<< opt
				<< "\n";
			return '?';
		}
        else if (argc_ <= ++optind) 
        { 
            // No arg.
            nextchar_ = "";
//        	strcpy(nextchar_, "");
            
            if (*optstring_ == ':')
                return ':';
            
            if (opterr)
                cerr << argv_[0]
                << ": option requires an argument -- "
                << opt
                << "\n";
            return '?';
        }
        else // White space.
            optarg = argv_[optind];
        
        nextchar_ = "";
//      strcpy(nextchar_, "");
        ++optind;
    }
    return opt; // Dump back option letter.
}

int Get_opt::operator()(bool dummy)
{
    if (argv_ == 0)
    {
        optind = 0;
        return EOF;
    }

    int opt; // Character checked for validity.
    const char *oli; // Option letter index.

    if (nextchar_ == 0 || *nextchar_ == '\0')
    {
        // Update scanning pointer.
        bool flag =0;
		if (optind >= argc_)
        {
            nextchar_ = "";
            return EOF;
        }

		if(*(nextchar_ = argv_[optind]) != '-')
			flag = 1;


        if (nextchar_[1] != 0 && *(++nextchar_) == '-')
        {
            // Found "--".
      		++optind;
            nextchar_ = "";
            return -2;
        }

		if(flag == 1)
		{
			optarg = argv_[optind];
			opt = '-';
			++optind;
			nextchar_ = "";
			return opt;
		}
    }

    // Option letter okay?
    opt = (int) *(nextchar_++);
    if (opt == (int) ':' || ((oli = strchr(optstring_, opt)) == 0))
    {
        // If the user didn't specify '-' as an option, assume it means
        // EOF.
        if (opt == (int) '-')
            return EOF;

        if (*nextchar_ == 0)
            ++optind;

        if (opterr && *optstring_ != ':')
            cerr << argv_[0]
            << ": illegal option -- "
            << opt
            << "\n";
        return '?';
    }

    if (*++oli != ':')
    { // Don't need argument.
        optarg = 0;

        if (!*nextchar_)
            ++optind;
    }
    else
    { // Need an argument.
        if (*nextchar_) // No white space.
		{
			optarg = nextchar_;
			if (opterr)
				cerr << argv_[0]
				<< ": option requires a white space -- "
				<< opt
				<< "\n";
			return '?';
		}
        else if (argc_ <= ++optind)
        {
            // No arg.
            nextchar_ = "";

            if (*optstring_ == ':')
                return ':';

            if (opterr)
                cerr << argv_[0]
                << ": option requires an argument -- "
                << opt
                << "\n";
            return '?';
        }
        else // White space.
            optarg = argv_[optind];

        nextchar_ = "";
        ++optind;
    }
    return opt; // Dump back option letter.
}

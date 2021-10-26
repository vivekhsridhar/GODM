/*=============================================================================================================
                                                 utils.cpp
===============================================================================================================
 
 Miscellaneous utility routines
 
 C++-code accompanying:
 
        (ms. in prep).
 
 Written by:
        G. Sander van Doorn
        Centre for Ecological and Evolutionary Studies - Theoretical Biology Group
        University of Groningen
        the Netherlands
 
 Program version
        xx/xx/xxxx	:
 
=============================================================================================================*/

#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include "utils.h"

/*=============================================================================================================
                                    frequently used mathematical constants
=============================================================================================================*/

const double	PI2		= 6.28318530717958;
const double	PI		= 3.14159265358979;
const double    SQRT2   = 1.41421356237309;

/*=============================================================================================================
                                                (error) logging
=============================================================================================================*/

bool echo_stdclog = true;

void echo(const std::string &message, const bool &write_to_clog)
{
	//writes messages to the log file
	static std::ofstream fp_out;
	static bool init = true;
	if(init)
	{
		fp_out.open("messages.txt");
		verify(fp_out.is_open());
		init = false;
	}
    std::time_t the_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string log_entry(std::ctime(&the_time));
	log_entry.insert(log_entry.length() - 1," : " + message);
	fp_out << log_entry;
	fp_out.flush();
    if(write_to_clog) std::clog << log_entry;
}

void error(const std::string &function, const std::string &message)
{
    std::ostringstream oss;
	oss << "error! in function " << function << (message.empty() ? "().\n" :  "() -> " + message + ".\n");
	echo(oss.str(), true);
	exit(1);
}

void warning(const std::string &function, const std::string &message)
{
    std::ostringstream oss;
	oss << "warning! in function " << function << (message.empty() ? "().\n" :  "() -> " + message + ".\n");
	echo(oss.str());
}

/*=============================================================================================================
                                         miscellaneous utility functions
=============================================================================================================*/

void dot() {std::cout << '.';}

void clip_range(double &val, const double &min, const double &max)
{
	if(val < min) val = min;
	else if(val > max) val = max;
}

void clip_high(double &val, const double &max)
{
	if(val > max) val = max;
}

void clip_low(double &val, const double &min)
{
	if(val < min) val = min;
}

void mark_time(const bool &set)
{
    static bool init = true;
    static std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    
    if(set || init) init = false;
    else {
        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
        std::cout << "Elapsed time between calls to mark_time(): " << time_span.count() << " seconds.\n";
        t0 = t1;
    }
}

void wait_for_return()
{
	std::cout << "Hit <Enter> to continue\n";
    getchar();
}
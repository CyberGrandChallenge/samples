/*

Author: Steve Wood <swood@cromulence.co>

Copyright (c) 2014 Cromulence LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "dates.h"
#include "libc.h"

int strtodate(char *buff, date_type *date) {
unsigned month, day, year;

char *tmp;
int slash_count=0;
int digit_count=0;

//parse string in the from of MM/DD/YYYY into the structure elements

    while (isspace(*buff))
        ++buff;

    tmp=buff;
    
    // first validate that the buffer is only digits and slashes
    while (*tmp!=0) {
    
        if ( !(isdigit(*tmp) || *tmp=='/'))  {
            
            return (-1);
        }
        else
            if (*tmp=='/')  {
                ++slash_count;
            }
                
        ++tmp;
    }
    
    // if it doesn't have two slashes, error
    if (slash_count != 2)
        return (-1);
    
    tmp=buff;
    
    month=atoi(tmp);
    
    if (month < 1 || month > 12)
        return(-2);

    
    while (*tmp!='/' && *tmp!=0)
        ++tmp;
        
    ++tmp;
    
    day=atoi(tmp);
    
    if (day < 1)
        return (-2);

    if ((month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) && day > 31)
        return (-2);
    
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30)
        return (-2);
    
    while (*tmp!='/' && *tmp!=0)
        ++tmp;
        
    ++tmp;
    
    year=atoi(tmp);
    
    // constrain the year so that other calculations are valid
    if (year < 1582 || year > 2050)
        return (-2);
    
    date->leap_year=leap_year(year);
    
    // now that we have the year and if its a leap year, validate the Feb dates
    if (month == 2 && day > 28+date->leap_year)
        return (-2);
    
    
    // put everyting into the custom structure
    date->month=month;
    date->day=day;
    date->year=year;
    date->doy=day_of_year(year, month, day);
    date->remaining_days_in_year=remaining_days(year, month, day);

return 0;

}


void print_date (date_type date) {
    
    print_int(date.month,0);
    print_str("/");
    print_int(date.day, 0);
    print_str("/");
    print_int(date.year, 0);
    
}

int diff_between_dates(date_type *start, date_type *end) {

// unimplemented right now
return 0;

}

// classic leap year calculation
unsigned int leap_year(unsigned int year) {
    
    if ((year%400==0 || year%100!=0) && (year%4==0))
        return 1;
    else
        return 0;

}

// for accounting reasons it might be handy to have the day of the year number
unsigned int day_of_year(unsigned year, unsigned month, unsigned day) {

int cumulative_days[]= { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
unsigned int doy;

    doy=cumulative_days[month-1] + day;
    
    if (month > 2)
        doy+=leap_year(year);
        
    return (doy);

}

unsigned int remaining_days(unsigned year, unsigned month, unsigned day) {
unsigned doy;

    doy=day_of_year(year, month, day);
    
    if (leap_year(year))
        return (366-doy);
    else
        return (365-doy);

}

// Useful for validating proposal dates, etc
int compare_dates(date_type *sdate, date_type *edate)  {
    
    
    // if the first operand is later, return 1
    // if the first operand is earlier, return -1
    // if they are the same date, return 0

    if (edate->year > sdate->year)
	
        return 1;
    

    else if (edate->year < sdate->year)  {
        
        return -1;
    }
    else if (edate->month > sdate->month)
    
        return 1;
    
    else if (edate->month < sdate->month) {
        
        return -1;	    
    }
    else if (edate->day > sdate->day)  {
        
        return 1;
    }
    else if (edate->day < sdate->day ) {
        
        return -1;
    }
    else
        return 0;
    
}
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

#ifndef DATES_H
#define DATES_H

typedef  struct {

unsigned int year;  //year as 4 digit int
unsigned int month;  // month 1-12
unsigned int day;  //day of month
unsigned int doy;  //day of year
unsigned int leap_year;  // 1 = leap year
unsigned int remaining_days_in_year;  //number of days in year - DOY
unsigned long epoch_days;  // number of days since the epoch

} date_type;


int strtodate(char *buff, date_type *date);
void print_date (date_type date);
int diff_between_dates(date_type *start, date_type *end);
unsigned int leap_year(unsigned int year);
unsigned int day_of_year(unsigned year, unsigned month, unsigned day);
unsigned int remaining_days(unsigned year, unsigned month, unsigned day);
int compare_dates(date_type *sdate, date_type *edate);

#endif

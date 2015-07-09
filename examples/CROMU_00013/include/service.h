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

#ifndef SERVICE_H
#define SERVICE_H

#include "fp_numbers.h"
#include "dates.h"

#define PROPOSAL_YEARS  5
#define HOURS_PER_YEAR  2000
#define MAX_HOURS PROPOSAL_YEARS * HOURS_PER_YEAR
#define ENG_GRADES 5
#define FIRST_YEAR 2015
#define NUM_RESOURCE_INPUTS 600

typedef struct {

int year;
long fringe;
long oh;
long gna;

} indirectrates_type;

typedef struct {

int year;
fp_number grade[ENG_GRADES];

} laborrates_type;

typedef struct {

unsigned labor_grade;
date_type start_date;
date_type end_date;
unsigned long num_hours;
unsigned curve_type;

}  resource_inputs_type;

typedef struct {

resource_inputs_type *resources;
int count;

}  proposal_inputs_type;

typedef struct {
    int is_proposal_setup;
    int start_month;
    int start_dom;
    int start_year;
    int end_month;
    int end_dom;
    int end_year;
    date_type start_date;
    date_type end_date;
    fp_number fee;
    char title[1024];
} proposal_info_type;

int proposal_setup(proposal_info_type *);
void set_proposal_name(proposal_info_type *info);
void set_proposal_pop(proposal_info_type *info);
void display_proposal_info(proposal_info_type *info);
void set_proposal_fee(proposal_info_type *info);
int validate_proposal_setup(proposal_info_type *info);
int get_proposal_inputs(proposal_info_type *prop_info, proposal_inputs_type *prop_inputs  );
void display_indirectrates(indirectrates_type *rates);
void display_prop_inputs(proposal_inputs_type *prop_inputs);
void run_pricing_algorithm(proposal_info_type *prop_info, indirectrates_type *rates,
                            laborrates_type *laborrates, proposal_inputs_type *prop_inputs);

#endif

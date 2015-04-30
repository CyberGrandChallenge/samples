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

#include <libcgc.h>
#include "libc.h"
#include "service.h"

void display_indirectrates(indirectrates_type *rates)  {
int i;
    
    print_nl();
    print_nl();
    
    print_str("                   Indirect Rates\n");
    print_str("**********************************************************\n");
    
    print_str("Fiscal Year\tFringe\t\tOH\t\tG&A\n");
    
    for (i=0;i<PROPOSAL_YEARS;++i)  {
	
	print_int(rates[i].year,0);
	print_str("\t\t");

	print_fp(rates[i].fringe);
	print_str("%\t\t");

	print_fp(rates[i].oh);
	print_str("%\t\t");

	print_fp(rates[i].gna);
	print_str("%\t\t");

	print_nl();
	
    }

    print_str("**********************************************************\n");
    
}

void display_laborrates(laborrates_type *rates)  {
int i;
int x;

    print_nl();
    print_nl();
    
    print_str("            Direct Engineering Labor Rates\n");
    print_str("**********************************************************\n");
    
    print_str("Fiscal Year\tE01\tE02\tE03\tE04\tE05\n");

    for (i=0;i<PROPOSAL_YEARS;++i)  {
	
		print_int(rates[i].year,0);
		print_str("\t\t");

		
		for (x=0;x<5;++x)  {  

			print_str("$");
			print_fp(rates[i].grade[x]);
			print_str("\t");
	
		}
	
		print_nl();
	
    }

    print_str("**********************************************************\n");
    
}


void init_indirectrates(indirectrates_type *rates)
{
int i;

    rates[0].year=FIRST_YEAR;
    rates[0].fringe=3125;
    rates[0].oh=6512;
    rates[0].gna=811;

    for (i=1;i<PROPOSAL_YEARS;++i)  {
    
		rates[i].year=FIRST_YEAR+i;
		rates[i].fringe=rates[i-1].fringe*1.02;
		rates[i].oh=rates[i-1].oh*1.02;
		rates[i].gna=rates[i-1].gna*1.02;

    }

}

void init_laborrates(laborrates_type *rates)
{
 int i;
 
 // initialize first year rates
    rates[0].year=FIRST_YEAR;
    rates[0].grade[0]=6550;
    rates[0].grade[1]=10010;
    rates[0].grade[2]=10522;
    rates[0].grade[3]=11069;
    rates[0].grade[4]=13599;

    // apply a 2% escalation for the next 4 years
    for (i=1;i<5;++i)  {
    
	rates[i].year=FIRST_YEAR+i;
	rates[i].grade[0]=rates[i-1].grade[0]*1.02;
	rates[i].grade[1]=rates[i-1].grade[1]*1.02;
	rates[i].grade[2]=rates[i-1].grade[2]*1.02;
	rates[i].grade[3]=rates[i-1].grade[3]*1.02;
	rates[i].grade[4]=rates[i-1].grade[4]*1.02;

    }

}

void init_prop_info(proposal_info_type *info) {
    
    info->is_proposal_setup=0;
    info->title[0]=0;
    info->start_month=0;
    info->start_dom=0;
    info->start_year=0;
    info->end_month=0;
    info->end_dom=0;
    info->end_year=0;
    info->start_date.doy=0;
    info->end_date.doy=0;
    info->fee=0;
}

int main(void) {
    
    char buf[1024];
    size_t rcv_cnt;    
    indirectrates_type rates[PROPOSAL_YEARS];
    laborrates_type laborrates[PROPOSAL_YEARS];
    proposal_info_type prop_info;

    proposal_inputs_type prop_inputs;
    
    int i;
    char *msg;
    char dest[100];
    int ret;
    size_t size;
    unsigned int sum=0;

    
    char *topmenu[] = {
    "Proposal Pricing System\n\n",
    "A - Setup Proposal\n",
    "B - Display Indirect Rates\n",
    "C - Display Labor Rates\n",
    "D - Enter Labor Estimates\n",
    "E - Display Labor Estimates\n",
    "F - Price Proposal\n",
    "G - Exit System\n",
    ":"
    };
    

// populate all the initial data elements 
    init_indirectrates(rates);
    init_laborrates(laborrates);
    init_prop_info(&prop_info);
    
    prop_inputs.count=0;
    
    
// this could have been statically allocated    
    ret=allocate(sizeof(resource_inputs_type)*NUM_RESOURCE_INPUTS,0, (void **)&prop_inputs.resources);
    
    if (ret!=0)
    {
		print_str("error allocating memory\n");
		_terminate(1);
    }

    // start the main menu loop
    
    while (1){
    
		print_nl();
		
		for (i=0;i< sizeof(topmenu)/sizeof(char *);++i)
		    print_str(topmenu[i]);
		
	        rcv_cnt=getnstr(buf, sizeof(buf));
		
		if (rcv_cnt==0)
		    continue;
		
		
		switch (buf[0]){
		    
		    case 'a':
		    case 'A':
				proposal_setup(&prop_info);
				break;
		    case 'b':
		    case 'B':
				display_indirectrates(rates);
				break;
		    case 'c':
		    case 'C':
				display_laborrates(laborrates);
				break;
		    case 'd':
		    case 'D':
			
				if (prop_info.is_proposal_setup) {

				//	get_proposal_inputs( &prop_info, &(prop_inputs.resources[prop_inputs.count++])   );
					get_proposal_inputs( &prop_info, &prop_inputs);
			    //	display_prop_inputs(&prop_inputs);

				}
				else
			    	print_str("\nProposal must be setup before resource estimates can be entered\n\n");
				break;
			case 'e':
			case 'E':

				display_prop_inputs(&prop_inputs);
				break;

		    case 'f':
		    case 'F':

		    	if (prop_inputs.count > 0)
					run_pricing_algorithm(&prop_info, rates, laborrates, &prop_inputs);
				else
					print_str("\nNo proposal resources have been entered\n\n");

				break;	

			case 'g':
			case 'G':
			_terminate(0);
		} // switch


    }  // while
 
}  // main  


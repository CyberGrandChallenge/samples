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
#include "dates.h"

 
int get_proposal_inputs(proposal_info_type *prop_info, proposal_inputs_type *prop_inputs  )  {

char buf[1024];
size_t rcv_cnt;
date_type sdate;
date_type edate;
int ret_code;


 	// loop getting proposal resource inputs until a blank line is entered for the start date   
	while (1)  {

		// finite memory has been preallocated
		if (prop_inputs->count >=  NUM_RESOURCE_INPUTS)  {

			print_str("\nProposal resource limits reached\n\n");
			return 0;
		}


		// loop until a valid date is entered (or a blank line terminates input)
	    while (1)  {
		
			print_nl();
			print_str("Enter the estimate's start date (MM/DD/YYYY) or a blank line to exit: ");
		    
			rcv_cnt=getnstr(buf, sizeof(buf));

			if (rcv_cnt == 0)
				return 0;
			
			ret_code=strtodate(buf, &sdate);
			
			if (ret_code==-1)  {
			    print_str("Invalid date format\n");
			    continue;
			}
			else if (ret_code==-2)  {
			    
			    print_str("Invalid date entered\n");
			    continue;
			}
			else if (compare_dates(&prop_info->start_date, &sdate) >=0 &&
						compare_dates(&prop_info->end_date, &sdate) == -1)  {
			    
			    break;
			}
			else  {
			    print_str("Date must be during the proposal period of performance\n");
			    continue;
			}
	    }

	    prop_inputs->resources[prop_inputs->count].start_date = sdate;
	    

	    // loop until a valid end date is entered
	    while (1)  {
		
			print_str("\nEnter the estimate's end date (MM/DD/YYYY): ");
		    
			rcv_cnt=getnstr(buf, sizeof(buf));
			
			ret_code=strtodate(buf, &edate);
			
			if (ret_code==-1)  {
			    print_str("Invalid date format\n");
			    continue;
			}
			else if (ret_code==-2)  {
			    
			    print_str("Invalid date entered\n");
			    continue;
			}
			else if (compare_dates(&sdate, &edate) ==1 &&
						compare_dates( &prop_info->end_date, &edate) < 1)  {
			    
			    break;
			}
			else  {
			    print_str("Date must be during the proposal period of performance and after the estimate's start date\n");
			    continue;
			}

	    }    
	    
	    prop_inputs->resources[prop_inputs->count].end_date = edate;
	    
	    // loop until a valid engineering grade level is entered
	    while (1)  {
		
			print_str("\nEnter the engineering grade (1-5): ");
		    
			rcv_cnt=getnstr(buf, sizeof(buf));
			
			ret_code=atoi(buf);
			
			if (ret_code<1 || ret_code  > 5)  {
			    print_str("Invalid labor grade\n");
			    continue;
			}
			else
			    break;
	    }
	    
	    prop_inputs->resources[prop_inputs->count].labor_grade = ret_code;
	    
	    // loop until total hours are entered.  Limited to be between 1-10,000
	    while (1)  {
		
			print_str("\nEnter the number of engineering hours for this resource: ");
		    
			rcv_cnt=getnstr(buf, sizeof(buf));
			
			if (rcv_cnt == 0)  {
			    	    
			    continue;
			}
			else {
			    
			    prop_inputs->resources[prop_inputs->count].num_hours=atoi(buf);
			    
			    if (prop_inputs->resources[prop_inputs->count].num_hours > MAX_HOURS)  {
				
					print_str("Maximum hours is limited to ");
					print_int(MAX_HOURS, 1);
					print_nl();
					continue;
			    }
			    else if (prop_inputs->resources[prop_inputs->count].num_hours < 1)  {
				
					print_str("Hours must be greater than 0\n");
					continue;
			    }
			    else
				break;
			}

	    }  // while

	    ++(prop_inputs->count);
	    
	}  // outer while

}


void display_prop_inputs(proposal_inputs_type *prop_inputs)  {
    int i;
    
    print_nl();    
    print_str("Total number of resources: ");
    print_int(prop_inputs->count, 0);
    print_nl();
 

    for (i=0; i< prop_inputs->count;++i)  {
	
		print_str("Entry: ");
		print_int(i,0);
		
		print_str("  Start: ");
		print_date(prop_inputs->resources[i].start_date);
		
		print_str("  End: ");
		print_date(prop_inputs->resources[i].end_date);
		
		print_str("  Grade: E0");
		print_int(prop_inputs->resources[i].labor_grade, 0);
		
		print_str("  Hours: ");
		print_int(prop_inputs->resources[i].num_hours, 1);
		
		print_nl();
    }
}
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


int proposal_setup(proposal_info_type *info){
    
    char *proposal_setup_menu[] = {
	"Proposal Setup\n\n",
	"A - Display Proposal Info\n",
	"B - Enter Proposal Name\n",
	"C - Enter Proposal Fee/Profit\n",
	"D - Enter Proposal POP\n",
	"E - Exit Proposal Setup\n:"
    };
	
    char buf[10];
    size_t rcv_cnt;
    int i;

    
    // loop over the proposal setup menu
    while (1)  {

		print_nl();
	
		for (i=0;i< sizeof(proposal_setup_menu)/sizeof(char *);++i)
	    	print_str(proposal_setup_menu[i]);
        
		rcv_cnt=getnstr(buf, sizeof(buf));
	
		if (rcv_cnt==0)
	    	continue;
	
	
			switch (buf[0]){
	    
		    case 'a':
		    case 'A':
				display_proposal_info(info);
				break;

		    case 'b':
		    case 'B':
				set_proposal_name(info);
				break;

		    case 'c':
		    case 'C':
				set_proposal_fee(info);
				break;

		    case 'd':
		    case 'D':
				set_proposal_pop(info);
				break;

		    case 'e':
		    case 'E':
		
			if (validate_proposal_setup(info)==-1)  {

			    display_proposal_info(info);			    
			    
			    while (1) {
					print_str("Proposal is not completely setup.  Are you sure you wish to exit setup? ");

				        rcv_cnt=getnstr(buf, sizeof(buf));
					
					if (rcv_cnt==0)
					    continue;
					
					if (buf[0]=='y' || buf[0]=='Y')
					    return 0;
					else
					    break;
			    } // while
			}

			// proposal has been setup so now other things are allowed to happen
			else {
			    info->is_proposal_setup=1;
			    return 0;
			}
			
			break;
	    
		}
	
    }

    return 0;
}


void set_proposal_name(proposal_info_type *info) {
    char buf[101];
    size_t rcv_cnt;
    
    print_str("Enter the proposal name (max 100 chars): ");
    
    rcv_cnt=getnstr(buf, 101);
    
    if (rcv_cnt > 0)
		strlcpy(buf, info->title, rcv_cnt);

    return;
    
}

void set_proposal_fee(proposal_info_type *info) {
    int ret_code;
    fp_number value;
    
    
    while (1) {
    
		print_str("Enter the % fee/profit: ");
	
		ret_code=get_fp(&value);
	
		if (ret_code < 0) {
	    
	    	print_str("\nInvalid number entered\n");
	    	continue;
	    
		}

		if (value < 0 || value > 1500)  {

			print_str("\nInvalid fee entered.  Valid range is 0-15\n");
			continue;
		}

		info->fee = value;
		return;
    }
}

void set_proposal_pop(proposal_info_type *info) {
    char buf[100];
    size_t rcv_cnt;
    date_type sdate;
    date_type edate;
    
    int ret_code;
    
    
    while (1)  {
	
		print_str("Enter the program start date (MM/DD/YYYY): ");
	    
		rcv_cnt=getnstr(buf, sizeof(buf));
		
		ret_code=strtodate(buf, &sdate);
		
		if (ret_code==-1)  {
		    print_str("Invalid date format\n");
		    continue;
		}
		else if (ret_code==-2)  {
		    
		    print_str("Invalid date entered\n");
		    continue;
		}
		else if (sdate.year < FIRST_YEAR || sdate.year > FIRST_YEAR + PROPOSAL_YEARS)  {
		    
		    print_str("Valid start dates must be in the years ");
		    print_int(FIRST_YEAR, 0);
		    print_str(" through ");
		    print_int(FIRST_YEAR+PROPOSAL_YEARS, 0);
		    print_str("\n\n");
		    continue;
		    
		}
		else
		    break;

	} // while

    info->start_date=sdate;
    
    while (1)  {
	
		print_str("Enter the program end date (MM/DD/YYYY): ");
    
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
		else if (edate.year < FIRST_YEAR || edate.year > FIRST_YEAR + PROPOSAL_YEARS)  {
		    
		    print_str("Valid end dates must be in the years ");
		    print_int(FIRST_YEAR, 0);
		    print_str(" through ");
		    print_int(FIRST_YEAR+PROPOSAL_YEARS, 0);
		    print_str("\n\n");
		    continue;
		    
		}
		else if (edate.year > sdate.year)
		
		    break;
		
		else if (edate.year < sdate.year)  {
		    
		    print_str("End date must be after start date\n\n");
		    continue;
		}
		else if (edate.month > sdate.month)
		
		    break;
		
		else if (edate.month < sdate.month) {
		    
		    print_str("End date must be after start date\n\n");
		    continue;	    
		}
		else if (edate.day <= sdate.day)  {
		    
		    print_str("End date must be after start date\n\n");
		    continue;
		}
		else
		    break;

	}  // while  
	    
	info->end_date=edate;
		
    return;
    
}

void display_proposal_info(proposal_info_type *info) {
    
    print_nl();
    print_nl();
    
    print_str("*************************************************\n");
    print_str("Title: ");
    print_str(info->title);
    print_nl();
    
    print_str("Start Date: ");
    
    if (info->start_date.month !=0)  {
		print_date(info->start_date);
    }

    print_nl();
    print_str("End Date: ");
    
    if (info->end_date.month !=0)  {
		print_date(info->end_date);
    }
    
    print_nl();
    
    print_str("Fee/Profit: ");
    print_fp(info->fee);
    print_nl();
    print_str("*************************************************\n");
    
}

int validate_proposal_setup(proposal_info_type *info) {
	int ret_code=0;
 
 	// make sure that the title and POP are set.  Fee doesn't matter and can be 0   
    if (info->title[0]==0)  {
		ret_code=-1;
    }
    
    if (info->start_date.doy==0) {
		ret_code=-1;
    }
    
    if (info->end_date.doy==0) {
		ret_code=-1;
    }
    
    
    return ret_code;
}
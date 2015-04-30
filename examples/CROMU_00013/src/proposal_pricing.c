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

#include "service.h"
#include "libc.h"


void run_pricing_algorithm(proposal_info_type *prop_info, indirectrates_type *rates,
                            laborrates_type *laborrates, proposal_inputs_type *prop_inputs)  {
                            
                            
    int i;
    int labor_grade;
     
    fp_number direct_cost=0;
    fp_number fringe=0;
    fp_number oh_cost=0;
    fp_number gna_cost=0;
    fp_number profit=0;
    fp_number price=0;
     
    fp_number total_direct_cost=0;
    fp_number total_fringe=0;
    fp_number total_oh_cost=0;
    fp_number total_gna_cost=0;
    fp_number total_burdened_labor=0;
    fp_number total_profit=0;
    fp_number total_price=0;
    
    long sum_direct_cost=0;
    long sum_fringe=0;
    long sum_oh_cost=0;
    long sum_gna_cost=0;
    long sum_burdened_labor=0;
    long sum_burdened_labor2=0;
    long sum_profit=0;
    long sum_price=0;

    
    for (i=0;i<prop_inputs->count; ++i)  {
        
        // copy the labor_grade to a local variable just to simplify further equations
        labor_grade=prop_inputs->resources[i].labor_grade;
        
        direct_cost=prop_inputs->resources[i].num_hours * laborrates[0].grade[labor_grade-1];
        total_direct_cost+=direct_cost;
        sum_direct_cost+=div_fp(direct_cost+50,100);
        
        fringe=div_fp(direct_cost * rates[0].fringe, 10000);
        total_fringe+=fringe;
        sum_fringe+=div_fp(fringe+50,100);
        
        oh_cost=div_fp((fringe + direct_cost) * rates[0].oh,10000);
        total_oh_cost+=oh_cost;
        sum_oh_cost+=div_fp(oh_cost+50,100);
        
        gna_cost=div_fp((direct_cost + fringe + oh_cost) * rates[0].gna,10000);
        total_gna_cost+=gna_cost;
        sum_gna_cost+=div_fp(gna_cost+50,100);
        
        total_burdened_labor+=direct_cost + fringe + oh_cost + gna_cost;
        sum_burdened_labor+=div_fp(direct_cost + fringe + oh_cost + gna_cost + 50,100);
        sum_burdened_labor2=sum_direct_cost+sum_fringe+sum_oh_cost+sum_gna_cost;
        
        profit = div_fp((direct_cost + fringe + oh_cost + gna_cost) * prop_info->fee,10000);
        total_profit+=profit;
        sum_profit+=div_fp(profit+50, 100);
        
        price = direct_cost + fringe + oh_cost + gna_cost + profit;
        total_price+=price;
        sum_price+=div_fp(direct_cost + fringe + oh_cost + gna_cost + profit + 50,100);
        
    }
 
    // now that all the resources entries have been added in, output the final price
    print_str("Total Direct Labor: \t$ ");
    print_fp(total_direct_cost);
    print_nl();

    print_str("Total Fringe: \t\t$ ");
    print_fp(total_fringe);
    print_nl();
    
    print_str("Total Overhead: \t$ ");
    print_fp(total_oh_cost);
    print_nl();
    
    print_str("Total G&A: \t\t$ ");
    print_fp(total_gna_cost);
    print_nl();
    
    print_str("\t\t\t-------------------\n");
   
    print_str("Burdened Labor: \t$ ");
    print_fp(total_direct_cost + total_fringe + total_oh_cost + total_gna_cost);
    print_str("\n\n");
        
    print_str("Total Profit: \t\t$ ");
    print_fp(total_profit);
    print_nl();
    
    print_str("\t\t\t-------------------\n");        
    print_str("Total Price: \t\t$ ");
    print_fp(total_price);

    print_nl();
    print_nl();                            
}
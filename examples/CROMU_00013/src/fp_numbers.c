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
#include "fp_numbers.h"

int get_fp(fp_number *number)  {
    
char buffer[1024];
int len;
int ret_code;
    
    len=getnstr(buffer, sizeof(buffer));
    
    if (len > 0)
        ret_code=strtofp(buffer, number);
    else
        ret_code=FP_ENVAL;

    return(ret_code);    

}

void print_fp(fp_number number) {
    
fp_number temp;
fp_number temp2;

    temp = div_fp(number,100);
    temp2 = mod_fp(number,100);
    
    print_int(temp, 1);
    print_str(".");
    
    if (number < 0)
        number=-number;
    
    if (temp2 == 0)
        print_str("00");
    else if (temp2< 10)  {
        print_str("0");
        print_int(temp2,0);
    }
    else  {
        print_int(temp2,0);
        
    }
    
    return;
}


int strtofp(char *buffer, fp_number *number)  {
    
int i=0;
int len;
int dec_point_count=0;
int negative=0;

    *number = 0;
    
    // trim leading whitespace
    while (isspace(buffer[i]))
        ++i;

    // look for a negative sign
    if (buffer[i]=='-'){
        negative=1;
        ++i;
    }
    // sure, I'll take a positive sign too
    else if (buffer[i]=='+')
        ++i;
    
    // now loop through the rest of the string
    for (; buffer[i]!=0; ++i) {
    
        if (isdigit(buffer[i])) {
        
            // don't let it overflow a signed 32 bit int
            if (*number > 21474836) {
                *number=2147483600;
                return(FP_ERANGE);
            }
            
            *number=*number*10 + (buffer[i]-'0');
            
            // if we've received a decimal point already, keep track of digits after
            if (dec_point_count > 0) {
                ++ dec_point_count;
                
                // stop after receiving two digital past the decimal point
                if (dec_point_count == 3)
                break;
            }
        }            
        else if (buffer[i] == '.')  {

            // if this is the first decimal point, good.
            if (dec_point_count == 0)
                ++dec_point_count;
            else
                break;
        }
        // if any character other than a digit or decimal point is entered, its an error
        else  {
            *number=0;
            return (FP_ENVAL);
        }
        
            
    }  // for
    
    // if no decimal point was entered, or no digits after it, add two 0s
    if (dec_point_count == 0 || dec_point_count ==1)
        *number*=100;
    
    // if only one digit past the decimal point, pretend there was a trailing 0    
    else if (dec_point_count == 2)
        *number*=10;
      
    if (negative)
         *number=-*number;

    return 0;    

}

// this routine is needed because the fp_number type is actually long long
// and there is no division operator in DECREE for long long operands
fp_number div_fp(fp_number a, fp_number b) {

fp_number quotient;
fp_number temp;
fp_number remainder;

    quotient=0;
    temp=b;
    remainder=a;

    while (temp <= a)
        temp <<= 1;
    while ((temp >>= 1) >= b) {
        if (temp <= remainder) {
            quotient = 1 + (quotient << 1);
            remainder -= temp;
        }
        else
            quotient <<= 1;
    }
    return quotient;
    
}

// like division, there is no modulous function for long long
fp_number mod_fp(fp_number a, fp_number b) {

fp_number quotient;
fp_number temp;
fp_number remainder;

    quotient=0;
    temp=b;
    remainder=a;

    while (temp <= a)
        temp <<= 1;
    while ((temp >>= 1) >= b) {
        if (temp <= remainder) {
            quotient = 1 + (quotient << 1);
            remainder -= temp;
        }
        else
            quotient <<= 1;
    }
    return remainder;
    
}

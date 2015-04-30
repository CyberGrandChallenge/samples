# CGC Challenge Binary 00013 - Proposal Pricing System

## Author Information

Steve Wood <swood@cromulence.co>

### DARPA performer group
Cromulence

## Description

This challenge binary implements a pricing system for government cost proposals.  

### Feature List

It has a menu driven interface and allows the user to view and set proposal parameters and to enter up to 600 labor estimates. Pricing is performed one resource entry at a time, with a final summary of the price and its buildup showing all indirect cost element calculations. 

Although the flow of the application is relatively open, with only a few paths constrained, the general workflow is to first go to the Proposal Setup Menu and enter values for the proposal's name/title, the overall period of performance, and the desired fee/profit used in the final price.  From there, a return back to the Main menu allows for the entry of resource estimates.  Resources are entered one at a time.  Inputs are halted with a blank line is entered for the start date of the estimate, or 600 total resources have been entered.

After any resources have been entered, the current proposal price can be calculated and displayed.  Additional resources can be added, or even the proposal setup, e.g. profit, can be updated at any time and the price recalculated. 

## Vulnerability
The vulnerability is a string buffer overflow in a low-level print integer function.  The buffer is statically sized one byte too short if a negative, 10 digit, comma-separated number is output.  

The vulnerability is in print_int() at libc.c:122.  


### Generic class of vulnerability
Stack Buffer Overflow

### CWE classification
Stack Buffer Overflow
CWE-121

## Challenges

In this application, reaching this vulnerability is only possible by overflowing an integer value used to calculate the proposal price.  All individual user inputs are constrained to reasonable values, but by entering almost 600 resource entries, at maximum labor grade, maximum hours, and with a max fee of 15%, an overall price is achieved that exceeds the size of a signed 32 bit integer, rolling over to a large negative number.  This results in a sign being added to the output string, thus overflowing the buffer.  A stack overwrite occurs that causes a crash.

A CRS will have to be able to reason that only one variable in the entire application is able to reach such a large value, and that the manipulation of over 1000 other inputs are required cause it to roll over.  Even maximum resource values, but without setting the fee to maximum, will not overflow the integer.




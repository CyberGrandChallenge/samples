# CGC Challenge Binary Template

## Author Information

"Ben Schmidt" <info@narfindustries.com>

### DARPA performer group

Narf Industries (NRFIN)

## Description

CableGrind is a full-featured packet parser, ideal for all your packet dissection needs! Choose CableGrind, and you'll be tearing packets apart in no time.

### Feature List

Thanks to our easy-to-use parsing API and strong security precautions, we have quickly become the de-facto standard for analyzing network traffic! Our core CableGrind developers are constantly adding new protocols and new datatypes to please both users and dissector contributors. Because of our committment to new development, we now support over 500 different protocols, and are always open for more. And you can feel safe using CableGrind, because our unique stack verification ensures vulnerabilities won't be exploitable! Upload your DUPE (Dissector Upload Packet Encapsulation) to our demo server today and give it a try!


## Vulnerability

A simple stack buffer overflow vulnerability exists inside customprotos.c in dissector function process\_trolololo due to incorrect calculation of maximum input buffer size. By providing a bsize value greater than 12 but less than 48, it is possible to overflow the local stack buffer "buf". To complicate exploitation, this function is protected by a custom stack canary function.
Competitor's must be able to defeat this in addition to overflowing the buffer to prove vulnerability. In practice, this vulnerability is very similar to CVE-2013-5720.

Additionally, there are 180 other autogenerated vulnerabilities that allow for access outside the bounds of an array leading to the use of an invalid function pointer.
These serve to test CRS ability to detect and patch the same vulnerability spread across a CB, as well as the ability to seperate casual analysis of the binary (which will find this) to more in-depth analysis.

### Generic class of vulnerability

Buffer overflow
Improper bounds checking

### CWE classification

CWE-131: Incorrect Calculation of Buffer Size
CWE-121: Stack-based buffer overflow

## Challenges

* Competitors must correctly follow a large number of calls through function pointers in their analysis.
* Competitors must deal with a large (~12MB) CB and discover a vulnerability in one of many, very similar, functions.  
* Competitors must utilize control over a local variable to avoid triggering stack cookie checks but still causing a crash.
* Competitors must find and mitigate a large number of potential vulnerabilities.

# KPRCA-00006: SNAIL Server

## Author Information
"Sagar Momin" <sagar.momin@kapricasecurity.com>

### DARPA performer group
Kaprica Security (KPRCA)

## Description:

This service implements a basic SNAIL server that allows a user to send up to 10 
messages. The server was modeled after RFC 2821 
http://www.ietf.org/rfc/rfc2821.txt with some hints from Google's SNAIL server. 
There is a single user/pass combo that the user needs to authenticate to before 
proceeding to send a message, and from then a user can send a message through 
the typical POST FROM->SEND TO->DATA cycle. 

### Feature List:

The SNAIL server supports the minimum requirements as described by RFC 2821 
(4.5.1 - Minimum Implementation). The SNAIL server requires a user to first 
authenticate to the server, and then allows him or her to send up to 10 
messages. Each message will always originate from the authenticated user and 
may include up to 16 recipients. The SNAIL server breaks the RFC's guidelines 
with VRFY, but this is in order to give a user the ability to verify what 
messages were "sent".

## Vulnerability

The vulnerability is triggered when the VRFY command is called to view all 
messages sent by the user during the session. A user can stimulate the 
vulnerability in the "DATA" command which will store up to 512 bytes of user 
input in the stack. When outputting the message's contents the unpatched binary 
naively prints the buffer without a format specifier:

    fdprintf(STDOUT, msgs[i].body);

### Generic class of vulnerability
Format String

### CWE classification
CWE-134: Uncontrolled Format String

## Challenges

This service is meant to be a robust SNAIL server that would enable a variety of 
messages to be sent. Because the message is entirely derived by user input the 
majority of the service is spent doing string processing and input validation. 
As a result the service will only ever exit upon a "QUIT" command or the format 
string vulnerability.

The vulnerability was meant to be simple in order to test the ability to 
saturate possible inputs and analyze what is a successful or unsuccessful 
command. The CB is a state machine that requires a system to transition 
linearly through 6 states [HELO->AUTH LOGIN->POST FROM->SEND TO->DATA->VRFY]. 
The bug must be inputted in state 5 (DATA) and can only be actuated after a 
successful transition to state 6 (VRFY) by ending the DATA state with a 
<LF>.<LF>. Calling VRFY before successfully inputting the DATA command returns 
a constant string.

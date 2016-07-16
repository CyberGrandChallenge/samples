# CGC Challenge Binary Template

## Author Information

"John Q Public" <john@example.com>

### DARPA performer group
Lunge Technology (LUNGE)

## Description

This is a template challenge binary for use within DARPA's Cyber Grand Challenge.  Typically, a concise, descriptive summary of the service appears here (i.e. A library service to help manage books.).

### Feature List

This section should describe a 'feature list' suitable for a 'play-by-play' announcer during CFE.  (i.e.  This service supports adding new books, checking out sections of a book, keyword searching, compression, and special features such as deciphering contract officer language into understandable English.

## Vulnerability
Upon specific input (input beginning with 0x4142), there is a opportunity for register set PoV or secret data leak.

Specifically, in service.c:108, the input buffer is checked to be starting with 0x4142, wich signifies if a POV should be generated.  

If the following byte is 0x43, then a Type 1 POV is executed by setting a function pointer and argument from the socket, then calling the function.

If the following byte is not 0x43, then a Type 2 POV is executed, where the entire contents of the flag page are returned.

### Generic class of vulnerability
N/A

### CWE classification
N/A

## Challenges

This CB is only provided to show only the organizational structure used to create challenge binaries.

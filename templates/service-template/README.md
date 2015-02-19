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
Upon specific input (input beginning with 0x4142), there is a opportunity for a Null pointer dereference.

Specifically, in service.c:26 the input buffer is checked for the condition starting with 0x4142.  Then pointer  p is set to 0.  Finally at service:28 p is accessed:

p[0] = 10

### Generic class of vulnerability
NULL Pointer Dereference

### CWE classification
NULL Pointer Dereference
CWE-476

## Challenges

This CB is only provided to show only the organizational structure used to create challenge binaries.

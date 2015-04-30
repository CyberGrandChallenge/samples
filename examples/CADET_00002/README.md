# Ats and Asterisks

## Author Information

CGC Summer Intern 2014
Cadet from West Point Military Academy

### DARPA performer group
CADET

## Description:

This is an unusually simple service written by a military cadet as his second ever C program and to study heap vulnerabilities. It is intended as a data point as an incredibly simple service, with incredibly simple vulnerabilities, written by an inexperienced programmer.

The underlying service itself very similar to the tic-tac-toe game.

## Vulnerability

CWE-122: Heap based Buffer Overflow
CWE-121: Stack based Buffer Overflow

There are two vulnerabilities in this service. The first is a vanilla heap based buffer overflow where the user enters their name. This vulnerability is actually reachable via two code paths which may pose a small challenge for patching systems.

The second vulnerability is a stack overflow where the user inputs their move. This vulnerability was not originally intended by the author but was left in the challenge to preserve the case study of inexperienced programmers and their vulnerabilities.

## Challenges

The only intended challenge posed by this service is that it was written by an inexperienced C programmer and consequently it may look different than other test cases. The steps the cadet chose to write it more securely reflect his research into how heap overflows can lead to register control.

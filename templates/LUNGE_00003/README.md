# CGC Pipelined IPC Challenge Binary Template

## Author Information

"Brian Caswell" <bmc@lungetech.com>

### DARPA performer group
Lunge Technology (LUNGE)

## Description

This is a template challenge binary for use within DARPA's Cyber Grand Challenge showing DECREE IPC can be used to replicate pipelined processes.

### Feature List

Three binaries are linked together via socket pairs.  The network input is fed to the first, which is fed to the second, which is fed to the third, which is returned to the network.

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

This CB is only provided to show only the organizational structure used to create IPC challenge binaries.

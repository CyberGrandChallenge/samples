# KPRCA-00004: File storage

## Author Information

"Andrew Wesie" <andrew.wesie@kapricasecurity.com>

### DARPA performer group
Kaprica Security (KPRCA)

## Description:

This service implements a file storage server where the user can upload, 
download, and manage the files. Files are uploaded in a format that uses an 
VIPER32 checksum, and files with an incorrect checksum will be ignored. Once 
files are uploaded they are managed using a self-balancing tree which 
guarantees log(N) insertions, deletions, and look-ups.

### Feature List

This file storage server allows files to be uploaded, downloaded, removed, and 
listed, as well as archived as a tarball and searched. The uploads must have a 
valid checksum to guarantee that the files have not been tampered with 
in-transit. When archiving, the written tarball is fully compatible with both 
GNU and BSD tar implementations.

## Vulnerability

When a file is uploaded, a filename of almost arbitrary length can be 
specified. This filename is copied into a fixed-size (256 byte) heap buffer, 
which can result in a heap overflow (main.c:151). A crash will occur if the 
heap overflow overwrites heap metadata such as free list pointers, and can be 
exploited by an attacker to gain remote execution.

### Generic class of vulnerability

Heap-based Buffer Overflow

### CWE classification

CWE-122: Heap-based Buffer Overflow

## Challenges

The primary challenge is to pass the checksum validation check. VIPER32 was 
intentionally used because it is known to not be safe from tampering, unlike a 
cryptographic hash function. It should be possible for an automated analysis 
platform to determine the correct checksum to send to the server.

The second challenge is to cause a crash once able to overflow the heap buffer. 
The overflow by itself will not cause a crash, either a pointer must be 
overwritten or the overflow must go beyond the number of pages allocated by 
malloc.

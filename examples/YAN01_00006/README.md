# Syscall Benchmarker 

Author: Lok Yan
Group: AFRL

## Description

This server implements a simple benchmarker utility for the CGC Syscalls. The basic idea is to run as many instances of a single system call as possible within the user defined limits.

This service uses a comma separated protocol where all entries are interpretted as hex.

	TestNum,IterCount,PrintGran,Option1,Option2,StopOnError

Where TestNum is the syscall number of the syscall to test. Each syscall has its own testing function except for deallocate since that is paird with allocate - i.e. TestNum 6 is not supported. Also, since there isn't a syscall number 0, TestNum of 0 is used for userspace testing.

IterCount is used to specify how many iterations of a test should be performed.

PrintGran is used to specify how often a message should be printed out (returned to) the client.

Option1 and Option2 are optional arguments that are passed to the test functions. These options are not used by the User and _terminate tests. Option1 is used by the transmit and receive tests to specify the number of bytes to transmit and receive per syscall. The fdwait test uses option1 to specify the number of usec to wait and option2 to specify the fd to wait on. The allocate test uses option1 as the number of bytes to allocate and then deallocate. Finally, the random test retrives option1 number of random bytes from the random system call and then prints 4 bytes of the random data at a period of option2 to STDERR.

StopOnError is a flag that tells the benchmark to stop (terminate) if an error is detected during the benchmark test.

## Vulnerabilities

The vulnerability is an uninitialized function pointer [CWE-824: Access of Uninitialized Pointer](http://cwe.mitre.org/data/definitions/824.html) that can be triggered, leading to a segmentation fault.

## Additional Notes

This challenge binary is designed to help CRSes test their ability to handle function pointers. 

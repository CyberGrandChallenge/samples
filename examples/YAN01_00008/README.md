# Diophantine Game

@Author: Lok Yan <Lok.Yan@us.af.mil>
@Group: AFRL

## Description

This server is a very simple contrived game where the user is tasked with providing a list of solutions to the diophantine equations ranging from powers of 3 to 10. It is more like a dio equation checker than a game. 

The main purpose of this CB is for CRS authors to be able to test their integration with theorem provers. That is, the vulnerability (a classic buffer overflow) is reachable only when a solution to an n.1.2 equation (i.e. a^n + b^n = c^n) with 3 <= n <= 10 is provided. According to Fermat, this is not possible for n >=3 as long as n, a, b and c are integers. However, this CB uses uint32_t, meaning multiplication and addition will result in numerical overflows, which makes this possible. 

## Vulnerabilities

This CB contains a very simple stack based buffer overflow [CWE-121: Stack-based Buffer Overflow](http://cwe.mitre.org/data/definitions/121.html) due to a call to the receive system call. 

## Notes

To help the CB focus on the task of testing the solver integration, the user inputs the values of a, b, c, ... in binary format so that they are written to the corresponding variables directly. This will help reduce the complexity of determining the reachability since no transformation functions are used.

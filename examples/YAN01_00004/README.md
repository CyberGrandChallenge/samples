# Online Dictionary

@Author: Lok Yan <Lok.Yan@us.af.mil>
@Group: AFRL

## Description:

This is a simple online dictionary where the client can request for definitions from the server. The dictionary contents are stored statically in a table that is readily visible to CRSes. This dictionary also allows the client to add and remove their own words and definitions to the dictinoary. 

The protocol is a simple:

	L word : Looks up a word
	A word defn : Adds the word with definition to the user dictionary
	R word : Remove the word from the user dictionary
	B : Bye

During processing, he server might return *WORD NOT FOUND*, *WORD EXISTS*, *DICTIONARY FULL*, *SUCCESS*, and *BAD CMD* to indicate an error during processing.
 
The vulnerability itself is a simple Use After Free which occurs when a client removes a word from their own dictionary and then looks up the word. Since all user definitions are stored in dynamically allocated pages, which are deallocated when the corresponding word is removed, removing a word from the user dictionary leaves a dangling pointer to unallocated memory.

## Vulnerability Class: 

The vulnerability falls under [CWE-416: Use After Free](http://cwe.mitre.org/data/definitions/416.html)
CWE-122: Heap-based Buffer Overflow

## Additional Notes:

A python script and dictionaries are  provided for generating the initial dictionary. The *dict/* folder contains the list of words and their definitions from WordNet as well as a list of workds from /usr/share/dict/words. 

As an example, we can use the following command to generate the .h file that is included in the *src/* directory as well as the .txt file that is used by the poll generator.

	python wordnetParse.py -d dict/data.noun -w dict/words -c src/definitions.h > definitions.txt

## Unintended Vulnerability

A vulnerability not intended by the original CB author was found internally by the CB team. In this vulnerability the printf() internal buffering code attempted to flush to the socket when the buffer filled. The underlying printf() code assumed that the transmit() would block and at least partially complete before returning into code which would keep accumulating data into the buffer. If the receiver did not accept any data then the printf() code would overflow its own heap buffer. This was fixed by wrapping the transmit call in a blocking fdwait() loop but could been equivalently been fixed with a call to _terminate(). This fix and a PoV addressing the unintended vulnerability have been included to preserve the additional vulnerability in the corpus.


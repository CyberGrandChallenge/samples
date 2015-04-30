#!/usr/bin/env python

import os
from generator.actions import Actions
import random
import string

class Dictinoary(Actions):

    def start(self):
        self.dictionary = {}
        self.mydictionary = {}
        self.prevremoved = []

        path = os.path.join(os.path.dirname(__file__), '..', '..', 'support', 'definitions.txt')
        for f in open(path):
          f = f.rstrip('\n')
          if f and f[0] != '#' :
            a = f.split(' : ')
            self.dictionary[a[0]] = a[1]
 
    def input(self):
        pass

    def badcmd(self):
        self.write(random.choice(string.lowercase) + "\n")
        self.read(delim='\n', expect="BAD CMD\n")

    def addword(self):
        if len(self.mydictionary) < 64 :
          word = "".join([random.choice(string.letters) for _ in xrange(random.randint(5,10))]) 
          if not word in self.prevremoved and not word in self.mydictionary :
            self.mydictionary[word] = "".join([random.choice(string.letters) for _ in xrange(random.randint(10,100))])
            self.write("A " + word + " " + self.mydictionary[word] + "\n")
            self.read(delim="\n", expect="SUCCESS\n")

    def lookup(self):
        d = self.dictionary
        if random.randint(0,10) < 5 and len(self.mydictionary) > 0 :
          d = self.mydictionary

        word = random.choice(d.keys()) 
        self.write("L " + word + "\n")
        self.read(delim='\n', expect=word + " : " + d[word] + "\n")

    def removeword(self):
        if len(self.mydictionary) > 0 :
          word = random.choice(self.mydictionary.keys())
          self.write("R " + word + "\n")
          self.read(delim='\n', expect="SUCCESS\n")
          del self.mydictionary[word]
          self.prevremoved.append(word)

    def end(self):
        self.write("B\n")
        self.read(delim='\n', expect="BYE\n")




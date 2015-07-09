#!/usr/bin/env python

from generator.actions import Actions
import random
import string

formatescapechars = '$%'
formatescapesubchars = '0xXdDnNsuU$%'
escapechars = '\\'
escapesubchars = '\\nt\"\''

escapedict = { key : formatescapesubchars for key in formatescapechars }
escapedict.update({ key : escapesubchars for key in escapechars })

class Echo(Actions):

    def cleanup(self, s):
        temp = ""
        #we need to remove any valid escape characters 
        i = 0
        while i < len(s) :
          c = s[i]
          temp += c
          i += 1
 
          #keep incrementing in case the current char is an escape char
          if c in escapedict :
            while (i < len(s)) and (s[i] in escapedict[c]) :
              i += 1

        return temp
        

    def start(self):
        pass

    def write_echo(self):
        done = False
        while not done :
          s = "".join([random.choice(string.printable) for _ in xrange(random.randint(1,127))])
          s = self.cleanup(s)
          done = len(s)
  
        self.write(s)
        self.read(length=len(s), expect=s)

    def end(self):
        pass




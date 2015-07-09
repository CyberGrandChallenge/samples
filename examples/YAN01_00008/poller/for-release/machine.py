#!/usr/bin/env python

from generator.actions import Actions
import copy
import random
import string
import numpy
import struct

known_solutions = { 3 : [ [6, 3, 4, 5], [9, 1, 6, 8], [19, 3, 10, 18], [20, 7, 14, 17], [25, 4, 17, 22], [28, 18, 19, 21], [29, 11, 15, 27], [41, 2, 17, 40], [41, 6, 32, 33], [44, 16, 23, 41], [46, 3, 36, 37], [46, 27, 30, 37], [53, 29, 34, 44], [54, 12, 19, 53], [58, 15, 42, 49], [67, 22, 51, 54], [69, 36, 38, 61], [70, 7, 54, 57], [71, 14, 23, 70], [72, 34, 39, 65], [75, 38, 43, 66], [76, 31, 33, 72] ], 4 : [ [353, 30, 120, 272, 315], [651, 240, 340, 430, 599], [2487, 435, 710, 1384, 2420] ], 5 : [ [72, 19, 43, 46, 47, 67], [94, 21, 23, 37, 79, 84] ], 6 : [ [251, 8, 12, 30, 78, 102, 138, 165, 246] ], 7 : [ [568, 127, 258, 266, 413, 430, 439, 525] ], 8 : [ [1409, 90, 223, 478, 524, 748, 1088, 1190, 1324] ], 9 : [ [917, 42, 99, 179, 475, 542, 574, 625, 668, 822, 851] ], 10 : [ [228, 6, 13, 49, 57, 59, 73, 85, 122, 128, 179, 187, 204, 210] ] }

def tobytes(i) :
  return struct.pack("<I", i)

class DioGame(Actions):

    def start(self):
        self.name = "".join([random.choice(string.ascii_uppercase) for _ in xrange(random.randint(1,63))])
        self.score = 0
        self.difficulty = random.randint(0,7) + 3;
        self.chosen_one = [0]
        self.note = ""

        self.read(delim='\n', expect="Welcome to the Diophantine Equation Finding Game\n")

    def player_name(self):
        self.read(delim='\n', expect="Please enter your name\n")
        self.write(self.name + "\n")

    def difficulty_level(self):
        self.read(delim='\n', expect="Please choose a difficulty level (Power) from 3-10\n")
        self.write(tobytes(self.difficulty))

    def solution(self):
        self.chosen_one = known_solutions[self.difficulty][random.randint(0,len(known_solutions[self.difficulty]) - 1)]

        self.read(delim='\n', expect="What is the solution?\n")
        self.write(tobytes(self.chosen_one[0]))

    def num_terms(self):
        self.read(delim='\n', expect="How many terms (integers) are in your solution?\n")
        self.write(tobytes(len(self.chosen_one) - 1))

    def num_terms_end(self):
        self.read(delim='\n', expect="How many terms (integers) are in your solution?\n")
        i = random.randint(0, 0xFFFFFFFF)
        if (i >= 2) and (i <= 10) :
          i = i % 2

        self.write(tobytes(i))
 
    def terms(self):
        self.read(delim='\n', expect="Enter your numbers\n")
        for i in xrange(1,len(self.chosen_one)) :
          self.write(tobytes(self.chosen_one[i]))

        self.score += self.difficulty * (len(self.chosen_one) - 1) 
        self.note = "%d.1.%d" % (self.difficulty, len(self.chosen_one) - 1)

    def end(self):
        self.read(delim='\n', expect="Thanks for playing. Here are the final scores\n")
        printed_before = 0
        i = 10
        while (self.score <= i) and (i > 0) :
          self.read(delim='\n', expect="Player %u\t%010u\t%u\n" % (printed_before, 10 - printed_before, 10 - printed_before))
          printed_before += 1
          i -= 1
        
        if printed_before < 10 :
          self.read(delim='\n', expect="%s\t%010u\t%s\n" % (self.name, self.score, self.note))

        #up to this point - printed before will be the total number of players printed + 1 to include
        # the current player as long as the current player actually placed
        for i in xrange(10 - printed_before - 1) :
          self.read(delim='\n', expect="Player %u\t%010u\t%u\n" % (i + printed_before, 10 - (i + printed_before), 10 - (i + printed_before)))




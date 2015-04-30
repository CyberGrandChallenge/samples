#!/usr/bin/env python

from generator.actions import Actions
import copy
import random
import string
import numpy

func_set = "0123457"
max_iterations = { "0":0x10000, "1":0x10000, "2":0x10000, "3":0x100, "4":0x10000, "5":0x10000, "6":0x10000, "7":0x1400 }

def genUser() :
    return [0,0] 

def genTerminate() :
    return [0,0] 

def genTransmit() :
    return [random.randint(1,4095), 0]

def genReceive() :
    return [random.randint(1,10), 0]

def genFDWait() :
    return [0,0] 

def genAllocate() :
    #1 to 10 pages of allocation at a time
    return [(random.randint(1,10) * 0x1000), 0]

def genDeallocate() :
    return genAllocate()

def genRandom() :
    #unfortunately we can't do the prng test because we can't control the seed, so option2 will be 0
    # we use 1,5000 but actually anything over 4096 gets reduced down to 4096
    return [random.randint(1, 5000), 0]

func_map = { "0":genUser, "1":genTerminate, "2":genTransmit, "3":genReceive, "4":genFDWait, "5":genAllocate, "6":genDeallocate, "7":genRandom }

class Benchmark(Actions):

    def start(self):
        self.funcNum = random.choice(func_set)
        self.iCount = random.randint(0,max_iterations[self.funcNum])
        self.count = "%x" % self.iCount
        if not self.iCount :
          self.iPrintGran = 0
        else :
          self.iPrintGran = (random.randint(0, self.iCount - 1) / 2) + 1
        self.printGran = "%x" % self.iPrintGran 
        self.iOption1, self.iOption2 = func_map[self.funcNum]()
        self.option1 = "%x" % self.iOption1
        self.option2 = "%x" % self.iOption2
        #it shouldn't matter if we stop on error or not since cb-test should end if an error occurs
        # meaning there won't be any more matching, but we will use 0 just to be safe
        #bStopOnError = "%x" % random.randint(0,1)
        self.bStopOnError = "%x" % 0
        output = ','.join([self.funcNum, self.count, self.printGran, self.option1, self.option2, self.bStopOnError])
        self.write(output + "."*(128 - len(output)))

    def running(self):
        if self.funcNum == "1" :
            pass
        elif self.funcNum == "0" or self.funcNum == "4" or self.funcNum == "5" or self.funcNum == "7" :
            i = 0
            while i < self.iCount :
                self.read(delim='\n', expect="0x%08X\n" % i)
                i = i + self.iPrintGran
        elif self.funcNum == "2" : #transmit
            i = 0
            while i < self.iCount :
                if i == 0 :
                    self.read(length=self.iOption1, expect_format="pcre", expect=".*")
                    self.read(delim='\n', expect="0x%08X\n" % i)
                else :
                    self.read(length=self.iOption1 * self.iPrintGran, expect_format="pcre", expect=".*")
                    self.read(delim='\n', expect="0x%08X\n" % i)
                i = i + self.iPrintGran
        elif self.funcNum == "3" : #receive
            i = 0
            total = 0
            while i < self.iCount :
                leftOver = ((self.iCount * self.iOption1) - total) 
                if leftOver < (self.iOption1 * self.iPrintGran) :
                    self.write("." * leftOver)
                    total = total + leftOver
                else : 
                    self.write("." * self.iOption1 * self.iPrintGran)
                    total = total + (self.iOption1 * self.iPrintGran)
                self.read(length=11, expect="0x%08X\n" % i)
                i = i + self.iPrintGran
                

    def end(self):
        pass



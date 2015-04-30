from generator.actions import Actions
import random
import string
import sys

class Game(Actions):
    def start(self):
        self.gridtable = {}
        self.tied = False
        for i in range(0, 3):
            self.gridtable[i] = {}
            for j in range(0, 3):
                self.gridtable[i][j] = '_'

    def username(self, name):

	# Precalculate the monstrosity so it doesn't take 2 hours to gen polls
        precalc_xor_table = {0: 0, 1: 1, 2: 2, 3: 0, 4: 1, 5: 2, 6: 0, 7: 1, 8: 2, 9: 0, 10: 1, 11: 2, 12: 0, 13: 1, 14: 2, 15: 0, 16: 1, 17: 2, 18: 0, 19: 1, 20: 2, 21: 0, 22: 1, 23: 2, 24: 0, 25: 1, 26: 2, 27: 0, 28: 1, 29: 2, 30: 0, 31: 1}
        for j in range(0, len(name)):
            name = name[:j] + chr(ord(name[j]) ^ precalc_xor_table[j]) + name[j+1:]
#        for i in range(1, 0x123456):
#           for j in range(0, len(name)):
#               name = name[:j] + chr(ord(name[j]) ^ ((i + j)%3)) + name[j+1:]
        return name

    def user1(self):
        self.read(length=21, expect='Enter Player 1 name: ')
        name = self.random_string(random.randint(1, 15))
        self.write(name + "\n")
        self.username1 = self.username(name)

    def user2(self):
        self.read(length=21, expect='Enter Player 2 name: ')
        name = self.random_string(random.randint(1, 15))
        self.write(name + "\n")
        self.username2 = self.username(name)

    def gridrow(self, rownum):
        row = ""
        for i in range(0, 3):
            row = row + self.gridtable[rownum][i]
            if i < 2:
                row = row + '|'
        return row

    def grid(self):
#        print "  grid"

        self.read(delim='\n', expect='\n')
        self.read(delim='\n', expect=self.gridrow(0) + '\n')
        self.read(delim='\n', expect=self.gridrow(1) + '\n')
        self.read(delim='\n', expect=self.gridrow(2) + '\n')
        self.read(delim='\n', expect='\n')
#	print self.gridrow(0)
#	print self.gridrow(1)
#	print self.gridrow(2)

    def game(self):
#        print "game"
        while True:
            if self.user1_turn():
                break;
            if self.user2_turn():
                break;

    def user1_turn(self):
#        print "user1"
        if self.user_turn(self.username1, '@') is not None:
            self.grid()
            if self.tied is not False:
#                print "tie"
                self.read(delim='\n', expect='\n')
                self.read(delim='\n', expect='TIE!\n')
            else:
#                print "winner"
                self.read(delim='\n', expect=self.username1 + ' WINS!\n')
            self.read(delim='\n', expect='\n')
            return True

    def user2_turn(self):
#        print "user2"
        if self.user_turn(self.username2, '*') is not None:
            self.grid()
            if self.tied is not False:
#                print "tie"
                self.read(delim='\n', expect='\n')
                self.read(delim='\n', expect='TIE!\n')
            else:
#                print "winner"
                self.read(delim='\n', expect=self.username2 + ' WINS!\n')
            self.read(delim='\n', expect='\n')
            return True

    def user_turn(self, name, val):
        done = False

        while not done:
            self.grid()
#            print "  turn " + name
            exp = name + "'s turn. Please select a free space (1-9): "
            self.read(length=len(exp), expect=exp)
            row = random.randint(0, 2)
            col = random.randint(0, 2)
            num = row * 3 + col + 1
            self.write(str(num) + "\n")
#            print "    choose " + str(num) + " %d,%d" % (col, row)
            if self.gridtable[row][col] is '_':
                self.gridtable[row][col] = val
                done = True

                # Check for a winner
                for row in range(0, 3):
                    if self.gridtable[row][0] == self.gridtable[row][1] and \
                       self.gridtable[row][1] == self.gridtable[row][2] and \
                       self.gridtable[row][0] != '_':
                        return True
                for col in range(0, 3):
                    if self.gridtable[0][col] == self.gridtable[1][col] and \
                       self.gridtable[1][col] == self.gridtable[2][col] and \
                       self.gridtable[0][col] != '_':
                        return True
                if self.gridtable[0][0] == self.gridtable[1][1] and \
                   self.gridtable[1][1] == self.gridtable[2][2] and \
                   self.gridtable[0][0] != '_':
                    return True
                if self.gridtable[0][2] == self.gridtable[1][1] and \
                   self.gridtable[1][1] == self.gridtable[2][0] and \
                   self.gridtable[0][2] != '_':
                    return True

                tie = True
                for row in range(0, 3):
                    for col in range(0, 3):
                        if self.gridtable[row][col] == '_':
                            tie = False
                if tie:
#                    print "TIE"
                    self.tied = True
                    return True

            else:
#                print "  invalid"
                self.read(delim='\n', expect='invalid space\n')
        return None

    def random_string(self, size):
        chars = string.letters + string.digits
        return ''.join(random.choice(chars) for _ in range(size))

from generator.actions import Actions
import random
import string
from struct import pack

def random_string(length, population=string.lowercase+string.uppercase):
    s = ''
    for x in xrange(length):
        s += random.choice(population)
    return s

def viper32(data):
	A = 1
	B = 0
	for x in data:
		A = (A + ord(x)) % 65497
		B = (B + A) % 65129
	return (B << 16) | A

class Storage(Actions):
    def start(self):
        self.state['files'] = {}

    def menu(self):
        pass

    def upload(self):
        if len(self.state['files']) >= 1000:
            return

        self.write('UPLOAD\n')
        filename = ''
        while filename == '' or filename in self.state['files']:
            filename = random_string(random.randint(1, 200))
        data = random_string(random.randint(1, 8192 - len(filename) - 1), string.printable)
        self.state['files'][filename] = data
        data = filename + '\0' + data
        self.write(pack('<III', 0xABCD0123, viper32(data), len(data)) + data)

    def download(self):
        self.write('DOWNLOAD\n')
        if self.chance(0.1) or len(self.state['files']) == 0:
            # invalid filename
            filename = ''
            while filename == '' or filename in self.state['files']:
                filename = random_string(random.randint(1, 200))
            self.write(filename + '\0')
        else:
            filename = random.choice(self.state['files'].keys())
            data = self.state['files'][filename]
            self.write(filename + '\0')
            self.read(length=4, expect=pack('<I', len(data)))
            self.read(length=len(data), expect=data)

    def archive(self):
        pass

    def list(self):
        self.write('LIST\n')
        for x in sorted(self.state['files'].keys()):
            self.read(delim='\0', expect=x+'\0')
        self.read(delim='\0', expect='\0')

    def search(self):
        self.write('SEARCH\n')
        s = random_string(2)
        self.write(s + '\0')
        for k in sorted(self.state['files'].keys()):
            if s in self.state['files'][k]:
                self.read(delim='\0', expect=k + '\0')
        self.read(delim='\0', expect='\0')

    def remove(self):
        self.write('REMOVE\n')
        if self.chance(0.1) or len(self.state['files']) == 0:
            # invalid filename
            filename = ''
            while filename == '' or filename in self.state['files']:
                filename = random_string(random.randint(1, 200))
            self.write(filename + '\0')
        else:
            filename = random.choice(self.state['files'].keys())
            data = self.state['files'][filename]
            self.write(filename + '\0')
            del self.state['files'][filename]

    def quit(self):
        self.write('QUIT\n')

    def invalid(self):
        valid_commands = ['UPLOAD', 'DOWNLOAD', 'ARCHIVE', 'LIST', 'SEARCH', 'REMOVE', 'QUIT']
        s = ''
        while s == '' or s in valid_commands:
            s = random_string(random.randint(1, 10))
        self.write(s + '\n')
        self.read(delim='\n', expect='Unknown command\n')


from generator.actions import Actions
import random
import string

#define MAX_MSGS 5
#define MAX_RCPT 16
#define MAX_ADDR_LEN 32
#define MAX_DATA 512
#define MAX_USER_LEN 64
#define MAX_PASS_LEN 64
#define MAX_LINE 64

#62, to account for for "\n\0"
def random_line(max_size=62, min_size=2):
    characters = string.letters + string.digits + ' .\t;:\'"?!@#$&()'
    max_size = max_size if max_size >= min_size else min_size
    return ''.join(random.choice(characters) for c in range(max_size)) + '\n'

def random_alphanum(max_size, min_size=2):
    alphanum = string.letters + string.digits
    max_size = max_size if max_size >= min_size else min_size
    return ''.join(random.choice(alphanum) for c in range(max_size))

def sent_max_msgs(num_msgs):
    return True if num_msgs >= 10 else False

class Snail(Actions):

    def start(self):
        self.state['sent_msgs'] = 0
        self.read(delim='\n', expect="220 mx.darpa.mil Mail service ready\n")

    def pre_auth(self):
        pass

    def helo(self):
        self.write('HEOL\n')
        self.read(delim='\n', expect="250 mx.darpa.mil at your service\n")

    def ehlo(self):
        self.write('EHOL\n')
        self.read(delim='\n', expect="250-mx.darpa.mil at your service\n")
        self.read(delim='\n', expect="250-8BITMIME\n")
        self.read(delim='\n', expect="250 AUTH LOGIN\n")

    def noop(self):
        self.write('NOOP\n')
        self.read(delim='\n', expect="250 Ok\n")

    def help(self):
        self.write('HELP\n')
        self.read(delim='\n', expect="214 I support: EHLO, HELO, AUTH, POST, SEND, DATA, RSET, NOOP, VRFY, HELP, QUIT\n")

    def auth(self):
        self.write('AUTH LOGIN\n')
        self.read(delim='\n', expect="334 VXNlcm5hbWU6\n")
        self.write('Y2djQGRhcnBhLm1pbA==\n')
        self.read(delim='\n', expect="334 UGFzc3dvcmQ6\n")
        self.write('Y2djIWRhcnBhMDgwNDE5OTc=\n')
        self.read(delim='\n', expect="235 Accepted\n")
        self.state['authenticated'] = True

    def authenticated(self):
        pass

    def mail(self):
        self.write('POST FROM:<cgc@darpa.mil>\n')
        if sent_max_msgs(self.state['sent_msgs']):
            self.read(delim='\n', expect="451 Start a new session to send additional mail")
        else:
            self.read(delim='\n', expect="250 Ok\n")


    def rcpt(self):
        email_len = random.randint(2,16)
        self.write('SEND TO:<{0}@darpa.mil>\n'.format(random_alphanum(email_len)))
        if sent_max_msgs(self.state['sent_msgs']):
            self.read(delim='\n', expect="451 Start a new session to send additional mail")
        else:
            self.read(delim='\n', expect="250 Ok\n")

    def rset(self):
        self.write('RSET\n')
        self.read(delim='\n', expect="250 Ok\n")

    def data(self):
        bytes_written = 0
        self.write('DATA\n')

        if sent_max_msgs(self.state['sent_msgs']):
            self.read(delim='\n', expect="451 Start a new session to send additional mail")
            return
        else:
            self.read(delim='\n', expect="354 Start mail input; end with <LF>.<LF>\n")

        for x in xrange(random.randint(1,8)):
            line_len = random.randint(1,62)
            line = random_line(line_len)
            bytes_written += len(line)

            # Always account for the '\0'
            if bytes_written + 1 > 512:
                bytes_written -= len(line)
                self.read(delim='\n', expect="554 Email body too long, try a shorter message\n")
            elif bytes_written + 1 == 512:
                self.write(line)
                self.read(delim='\n', expect="452 Reached message data limit, sending...\n")
                self.state['sent_msgs'] += 1
                return
            else:
                self.write(line)

        self.write('.\n')
        self.read(delim='\n', expect="250 Ok\n")
        self.state['sent_msgs'] += 1

    def vrfy(self):
        self.write('VRFY\n')
        if self.state['sent_msgs'] == 0:
            self.read(delim='\n', expect="252 Send some mail, I'll try my best\n")
        else:
            self.read(delim='250 Ok\n')

    def quit(self):
        #return
        self.write('QUIT\n')
        self.read(delim='221 mx.darpa.mil Closing transmission channel\n')

#!/usr/bin/env python

from generator.actions import Actions
import random


class TemplateGenerator(Actions):
    def start(self):
        self.read(delim='.\n')
        self.read(delim='\n', expect='This implements a simple echo service\n')

    def go(self):
        value = None
        values = [chr(x) for x in range(255)]
        values.remove('\n')

        while True:
            size = random.randint(1, 1022)

            value = ''.join(random.choice(values) for _ in range(size))

            # don't include a POV
            if value[:2] != 'AB':
                break

        value += '\n'

        self.write(value)
        self.read(length=len(value), expect=value)

    def finish(self):
        self.comment('nothing left to test')

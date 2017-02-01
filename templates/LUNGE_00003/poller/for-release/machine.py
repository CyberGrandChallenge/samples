#!/usr/bin/env python

from generator.actions import Actions
import random


class TemplateGenerator(Actions):
    def start(self):
        self.read(delim='\n', expect='This implements a simple echo service\n')

    def go(self):
        value = None

        while True:
            value = ''.join(chr(random.randint(0, 255)) for _ in
                            range(random.randint(1, 1023)))

            # don't include a POV
            if value[:2] != 'AB':
                break

        self.write(value)
        self.read(length=len(value), expect=value)

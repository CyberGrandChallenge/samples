#!/usr/bin/env python

from generator.actions import Actions
import random
import subprocess
import string

def random_string(size=20):
    return ''.join([random.choice(string.ascii_letters + string.digits) for x in xrange(random.randint(1,size))])

class RCL(Actions):

    def start(self):
        self.string = ''
        self.db_name = ''
        self.tbl_name = ''
        self.cols = list()
        self.sofar = 0
        self.deleted = False

        self.num_queries = random.randint(0, 100)

    def create_db(self):
        if self.db_name == '':
            self.db_name = random_string()
        db_name = ''
        if random.randint(0, 1) % 2 == 0:
            db_name = self.db_name
        else:
            db_name = random_string()
        query = 'CREATE DATABASE %s\n' % db_name
        self.string += query
        self.sofar += 1

    def create_tbl(self):
        try:
            types = ['INTEGER', 'FLOAT', 'VARCHAR']
            cols = ''
            self.num_cols = random.randint(0, 12)
            self.pk_idx = random.randint(0, self.num_cols)
            for i in xrange(self.num_cols):
                t = ''
                if random.randint(0, 99) == 50:
                    t = random_string(5)
                else:
                    t = random.choice(types)
                self.cols.append((random_string(5), t))
            for n,t in self.cols:
                cols += '%s %s,' % (n, t)
            if len(cols) > 0:
                cols = cols[:-1]
            if random.randint(0, 49) == 25 or len(self.cols) == 0:
                primary_col = random_string(5)
            else:
                primary_col = self.cols[self.pk_idx][0]
            if self.db_name != '' and self.tbl_name == '':
                self.tbl_name = random_string()
            tbl_name = ''
            if self.tbl_name != '':
                tbl_name = self.tbl_name
            else:
                tbl_name = random_string()
            query = 'CREATE TABLE %s %s (%s)\n' % (tbl_name, primary_col, cols)
            self.string += query
            self.sofar += 1
        except:
            pass

    def insert_into(self):
        values = ''
        tbl_name = ''
        if self.db_name != '' and (self.tbl_name == '' or random.randint(0, 49) == 25):
            tbl_name = random_string()
        else:
            tbl_name = self.tbl_name
        for n,t in self.cols:
            if t == 'INTEGER':
                neg = 1
                if random.randint(0, 1) % 2 == 0:
                    neg = -1
                values += '%d,' % (neg * random.randint(0, 2147483647))
            elif t == 'FLOAT':
                neg = 1
                if random.randint(0, 1) % 2 == 0:
                    neg = -1
                values += '%.02f,' % (neg * random.random() * random.randint(1, 1000))
            elif t == 'VARCHAR':
                values += '%s,' % random_string(25)
        if len(values) > 0:
            values = values[:-1]
        query = 'INSERT INTO %s VALUES (%s)\n' % (tbl_name, values)
        self.string += query
        self.sofar += 1

    def delete_from(self):
        try:
            ops = ['=', '<', '<=', '>', '>=']
            tbl_name = ''
            col_name = ''
            op = ''
            value = ''

            if self.tbl_name == '' or random.randint(0, 49) == 25:
                tbl_name = random_string(25)
            else:
                tbl_name = self.tbl_name
            if random.randint(0, 9) == 5:
                op = random_string(2)
            else:
                op = random.choice(ops)
            if random.randint(0, 49) == 25 or len(self.cols) == 0:
                col_name = random_string(5)
            else:
                col_name = self.cols[self.pk_idx][0]
            if random.randint(0, 19) == 10:
                value = random_string(3)
            else:
                if len(self.cols) > 0:
                    t = self.cols[self.pk_idx][1]
                    if t == 'INTEGER':
                        neg = 1
                        if random.randint(0, 1) % 2 == 0:
                            neg = -1
                        value = '%d' % (neg * random.randint(0, 2147483647))
                    elif t == 'FLOAT':
                        neg = 1
                        if random.randint(0, 1) % 2 == 0:
                            neg = -1
                        value = '%.02f' % (neg * random.random() * random.randint(1, 1000))
                    elif t == 'VARCHAR':
                        value = random_string(25)
                else:
                    value = random_string(25)

            query = 'DELETE FROM %s WHERE %s %s %s\n' % (tbl_name, col_name, op, value)
            self.string += query
            self.sofar += 1
            self.deleted = True
        except:
            pass

    def stats(self):
        if not self.deleted:
            query = 'STATS\n'
            self.string += query
            self.sofar += 1

    def select_from(self):
        try:
            ops = ['=', '<', '<=', '>', '>=']
            value = ''
            if random.randint(0, 9) == 5:
                op = random_string(2)
            else:
                op = random.choice(ops)
            if random.randint(0, 49) == 25 or len(self.cols) == 0:
                primary_col = random_string(5)
            else:
                primary_col = self.cols[self.pk_idx][0]
            if random.randint(0, 19) == 10:
                value = random_string(3)
            else:
                if len(self.cols) > 0:
                    t = self.cols[self.pk_idx][1]
                    if t == 'INTEGER':
                        neg = 1
                        if random.randint(0, 1) % 2 == 0:
                            neg = -1
                        value = '%d' % (neg * random.randint(0, 2147483647))
                    elif t == 'FLOAT':
                        neg = 1
                        if random.randint(0, 1) % 2 == 0:
                            neg = -1
                        value = '%.02f' % (neg * random.random() * random.randint(1, 1000))
                    elif t == 'VARCHAR':
                        value = random_string(25)
                else:
                    value = random_string(25)
            tbl_name = ''
            if self.tbl_name == '' or random.randint(0, 49) == 25:
                tbl_name = random_string(25)
            else:
                tbl_name = self.tbl_name
            query = 'SELECT FROM %s WHERE %s %s %s\n' % (tbl_name, primary_col, op, value)
            self.string += query
            self.sofar += 1
        except:
            pass

    def doit(self):
        funcs = [self.create_db, self.create_tbl, self.insert_into, self.select_from, self.delete_from, self.stats]
        self.create_db()
        self.create_tbl()
        while self.sofar < self.num_queries:
            n = random.randint(0, 99)
            if 0 <= n <= 4:
                self.create_db()
            elif 5 <= n <= 9:
                self.create_tbl()
            elif 10 <= n <= 19:
                self.stats()
            elif 20 <= n <= 59:
                self.insert_into()
            elif 60 <= n <= 79:
                self.select_from()
            elif 80 <= n <= 99:
                self.delete_from()
        process = subprocess.Popen(['./bin/KPRCA_00005'], stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        process.stdin.write(self.string)
        exp_out = process.communicate()[0]
        self.write(self.string)
        if len(exp_out) > 0:
            self.read(length=len(exp_out), expect=exp_out)

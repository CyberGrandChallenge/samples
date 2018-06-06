#!/usr/bin/env python

import os
import re
import json

CURDIR = os.path.dirname(os.path.abspath(__file__))
path = os.path.join(CURDIR, 'cqe-challenges')
os.chdir(path)

rlist = {'r1': r"(CWE-[\d]{2,3}).?\n{1,2}",
         'r2': r"\n(.*)\s\((CWE-[\d]{2,3})\)\n",
         'r3': r"\n{1,2}(.*)\n(CWE-[\d]{2,3})",
         'r4': r"\n{0,1}(CWE-[\d]{2,3}).?\s([^]\n]*).*?(?!\band\b)",
         'r5': r"([\d]{2,3}):\s(.*)"}

def dump(cwe_dict):
    with open(os.path.join(CURDIR,'data.txt'),'w') as f:
        f.write(json.dumps(cwe_dict, f, ensure_ascii=False, sort_keys=True,
                           indent=4))
        f.close()

def extract_cwe():
    cwe = dict()
    for p in os.listdir('.'):
        vlist = []
        cve = []
        with open(os.path.join(p, 'README.md')) as f:
            buf = f.read()
            for k, r in rlist.iteritems():
                vlist = re.findall(r,buf)
                if len(vlist):
                    if k == 'r3' and any('classification' in s[0] for s in vlist):
                        continue
                    elif type(vlist[0]) is not tuple:
                        continue
                    elif k == 'r3' and any(b in p for b in ['KPRCA', 'CROMU']) and vlist[0][0] == '':
                        continue
                    elif k == 'r2' or k == 'r3':
                        vlist[:] = [(c[1], c[0]) for c in vlist]
                    elif k == 'r5':
                        vlist[:] = [('CWE-'+c[0], c[1]) for c in vlist]
                    elif 'NRFIN' in p and type(vlist[0]) is not tuple:
                        continue
                    break
            f.close()
        for v in vlist:
            if type(v) is tuple:
                cve.append({'CWE':v[0],
                            'desc':v[1].replace(':','').strip()})
            else:
                cve.append({'CWE':v, 'desc':''})
        cwe[p] = cve
    return cwe

if __name__ == '__main__':
    cwe_dict = extract_cwe()
    dump(cwe_dict)

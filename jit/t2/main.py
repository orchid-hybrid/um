import os
import sys
from rpython.rlib.rarithmetic import r_uint

def debug(msg):
    print "debug", msg

# _______ ENtry Ppoint ________-


## portable endian handling thanks to http://commandcenter.blogspot.co.uk/2012/04/byte-order-fallacy.html
def from_endian_little(a,b,c,d):
    return (a<<0)|(b<<8)|(c<<16)|(d<<24)
def from_endian_big(a,b,c,d):
    return (a<<24)|(b<<16)|(c<<8)|(d<<0)

def program_words(b):
    s = int(len(b)/4)
    a = [r_uint(0)] * s
    for i in range(0, s): #xrange?
        a[i] = from_endian_big(r_uint(ord(b[4*i+0])),
                               r_uint(ord(b[4*i+1])),
                               r_uint(ord(b[4*i+2])),
                               r_uint(ord(b[4*i+3])))
    return a

def mainloop(program):
    pc = 0
    while pc < len(program):
        print "[", program[pc], "]"
        
        pc += 1

## following two functions taken from pypy-tutorial
def run(fp):
    program_code = ""
    while True:
        read = os.read(fp, 4096)
        if len(read) == 0:
            break
        program_code += read
    os.close(fp)
    program = program_words(program_code)
    mainloop(program)

def entry_point(argv):
    try:
        filename = argv[1]
    except IndexError:
        print "You must supply a filename"
        return 1
    run(os.open(filename, os.O_RDONLY, 0777))
    return 0

# ____ Defune and set up rtarget ___

def target(*args):
    return entry_point, None

if __name__ == "__main__":
    entry_point(sys.argv)

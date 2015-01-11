import os
import sys
from rpython.rlib.rarithmetic import r_uint32, intmask
from rpython.rlib.jit import JitDriver
jitdriver = JitDriver(greens=['pc'],
                      reds=['reg', 'arr', 'fresh', 'free'])

def jitpolicy(driver):
    from pypy.jit.codewriter.policy import JitPolicy
    return JitPolicy()

# _______ ENtry Ppoint ________-


## portable endian handling thanks to http://commandcenter.blogspot.co.uk/2012/04/byte-order-fallacy.html
def from_endian_little(a,b,c,d):
    return (a<<0)|(b<<8)|(c<<16)|(d<<24)
def from_endian_big(a,b,c,d):
    return (a<<24)|(b<<16)|(c<<8)|(d<<0)

def program_words(b):
    s = int(len(b)/4)
    a = [r_uint32(0)] * s
    for i in range(0, s): #xrange?
        a[i] = r_uint32(from_endian_big(ord(b[4*i+0]),
                                        ord(b[4*i+1]),
                                        ord(b[4*i+2]),
                                        ord(b[4*i+3])))
    return a

def mainloop(program):
    pc = 0
    reg = [r_uint32(0)] * 8 # registers
    arr = [program]
    fresh = 1
    free = []
    
    while True:
        jitdriver.jit_merge_point(pc=pc,
                                  reg=reg, arr=arr, fresh=fresh, free=free)
        p = intmask(arr[0][pc])
        
        op = p >> (32-4)
        a = (p & 0b111000000) >> 6
        b = (p & 0b000111000) >> 3
        c = (p & 0b000000111) >> 0
        
        #print "[", op, ",", a, ",", b, ",", c, "]"
        
        if op == 0: # Conditional Move
            if(intmask(reg[c]) != 0):
                reg[a] = reg[b]
            pc += 1
        elif op == 1: # Array Index
            reg[a] = arr[reg[b]][reg[c]]
            pc += 1
        elif op == 2: # Array Amendment
            arr[reg[a]][reg[b]] = reg[c]
            pc += 1
        elif op == 3: # Addition
            reg[a] = r_uint32(intmask(reg[b]) + intmask(reg[c]))
            pc += 1
        elif op == 4: # Multiplication
            reg[a] = r_uint32(intmask(reg[b]) * intmask(reg[c]))
            pc += 1
        elif op == 5: # Division
            reg[a] = r_uint32(intmask(reg[b]) / intmask(reg[c]))
            pc += 1
        elif op == 6: # Nand
            reg[a] = r_uint32(~(intmask(reg[b]) & intmask(reg[c])))
            pc += 1
        elif op == 7: # Halt
            break
        elif op == 8: # Allocation
            #if len(free) == 0:
            arr.append([r_uint32(0)] * reg[c])
            reg[b] = r_uint32(len(arr)-1)
            #else:
            #    reg[b] = free.pop()
            #    print "reusing", reg[b]
            #    arr[reg[b]] = [r_uint32(0)] * reg[c]
            pc += 1
        elif op == 9: # Abandonment
            arr[reg[c]] = None
            free.append(reg[c])
            pc += 1
        elif op == 10: # Output
            os.write(1, chr(reg[c]))
            pc += 1
        elif op == 11:
            #print "11"
            pc += 1
        elif op == 12: # Load Program
            if intmask(reg[b]) != 0:
                arr[0] = list(arr[reg[b]])
            pc = intmask(reg[c])
        elif op == 13: # Orthography
            a = p >> (32-4-3) & 0b111
            value = p & 0b00000001111111111111111111111111
            
            reg[a] = value
            
            pc += 1
        else:
            assert False

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

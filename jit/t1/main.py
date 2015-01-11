def debug(msg):
    print "debug", msg

# _______ ENtry Ppoint ________-

def entry_point(argv):
    debug("hello world")
    return 0

# ____ Defune and set up rtarget ___

def target(*args):
    return entry_point, None


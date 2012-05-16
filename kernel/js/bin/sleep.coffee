[bin, timeout] = OS.argv
if timeout
    OS.alarm Number(timeout), ->
        do OS.exit
else
    OS.write OS.stderr, "Usage: sleep <milliseconds>\n"
    do OS.exit
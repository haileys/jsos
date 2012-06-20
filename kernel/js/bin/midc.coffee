OS.open "/dev/sound", (err, fd) ->
    OS.ioctl fd, "play", 261.63
    OS.alarm 1000, ->
        OS.write OS.stdout, "HELLO WORLD"
        OS.ioctl fd, "stop"
        OS.close fd
        do OS.exit
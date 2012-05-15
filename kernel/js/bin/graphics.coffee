OS.open "/dev/vga", (err, vga) ->
    OS.read OS.stdin, 0, ->
        OS.close vga
        do OS.exit
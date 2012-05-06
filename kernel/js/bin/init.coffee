OS.write OS.stdout, "\n Listening contents of /:\n"
OS.readDirectory "/", (err, ents) ->
    for ent in ents
        OS.write OS.stdout, "  - #{ent.name}\n"

OS.open "/bin/sh.jmg", (err, fd) ->
    OS.write OS.stdout, "fd for /bin/sh.jmg: #{fd}\n"
    OS.read fd, 1024, (err, buff) ->
        OS.loadImage buff
        OS.write OS.stdout, "HI!\n"
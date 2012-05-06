OS.write OS.stdout, "\n Listing contents of /:\n"
OS.readDirectory "/", (err, ents) ->
    for ent in ents
        OS.write OS.stdout, "  - #{ent.name}\n"

OS.stat "/bin/sh.jmg", (err, stat) ->
    OS.open stat.path, (err, fd) ->
        OS.read fd, stat.size, (err, buff) ->
            OS.spawnChild ->
                OS.loadImage buff
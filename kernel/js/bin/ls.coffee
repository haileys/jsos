[bin, dir] = OS.argv

if dir
    OS.stat dir, (err, stat) ->
        if err
            OS.write OS.stderr, "ls: #{err}\n"
            return OS.exit()
        if stat.type isnt "directory"
            OS.write OS.stderr, "ls: #{dir} is not a directory\n"
            return OS.exit()
        OS.readDirectory dir, (err, ents) ->
            for ent in ents
                OS.write OS.stdout, "#{ent.name}\n"
            OS.exit()
else
    OS.write OS.stderr, "Usage: ls <directory>\n"
    OS.exit()
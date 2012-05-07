[bin, file] = OS.argv

if file
    OS.stat file, (err, stat) ->
        if err
            OS.write OS.stderr, "#{bin}: #{err}\n"
            return OS.exit()
        for k, v of stat
            OS.write OS.stdout, "#{k}: #{v}\n"
        OS.exit()
else
    OS.write OS.stderr, "Usage: #{bin} <file>\n"
    OS.exit()


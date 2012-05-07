[bin] = OS.argv

OS.stat "/etc/fortune.txt", (err, stat) ->
    if err
        OS.write OS.stderr, "#{bin}: #{err}\n"
        return OS.exit()
    OS.open "/etc/fortune.txt", (err, fd) ->
        OS.read fd, stat.size, (err, buff) ->
            fortunes = buff.split "%"
            idx = Math.floor(Math.random() * fortunes.length)
            OS.write OS.stdout, "#{fortunes[idx].trim()}\n"
            OS.exit()
print = (x...) -> OS.write OS.stdout, x.join("")

findInPath = (cmd, callback, paths) ->
    unless paths
        PATH = OS.env "PATH"
        paths = PATH.split ":"
    return callback null unless paths.length
    OS.stat "#{paths[0]}/#{cmd}.jmg", (err, stat) ->
        if err or stat.type isnt "file"
            return findInPath cmd, callback, paths[1..]
        else
            callback stat

chomp = (str) ->
    if str[str.length - 1] is "\n"
        str.substr 0, str.length - 1
    else
        str
        
print "I'm JSH on pid##{OS.pid()}\n\n"

do prompt = ->
    print "$ "
    OS.read OS.stdin, 0, (err, buff) ->
        [cmd, argv...] = buff.trim().split " "
        if cmd is "exit"
            OS.exit()
        else
            findInPath cmd, (stat) ->
                if stat
                    OS.open stat.path, (err, fd) ->
                        OS.read fd, stat.size, (err, buff) ->
                            OS.close fd
                            proc = OS.spawnChild buff, cmd, argv...
                            OS.wait proc.pid, ->
                                do prompt
                else
                    print "-jsh: #{cmd}: command not found\n"
                    do prompt
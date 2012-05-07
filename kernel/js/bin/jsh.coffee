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
            callback stat.path

chomp = (str) ->
    if str[str.length - 1] is "\n"
        str.substr 0, str.length - 1
    else
        str
        
print "I'm JSH on pid##{OS.pid()}\n"

do prompt = ->
    print "$ "
    OS.read OS.stdin, 0, (err, buff) ->
        buff = buff.trim()
        if buff is "exit"
            OS.exit()
        else
            findInPath buff, (path) ->
                if path
                    print "would execute: #{path}\n"
                else
                    print "-jsh: #{buff}: command not found\n"
                do prompt
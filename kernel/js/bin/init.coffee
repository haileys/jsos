OS.env "PATH", "/bin"

OS.stat "/bin/jsh.jmg", (err, stat) ->
    OS.open stat.path, (err, fd) ->
        OS.read fd, stat.size, (err, buff) ->
            proc = OS.spawnChild buff
            OS.wait proc.pid, (status) ->
                OS.write OS.stdout, "#{stat.path} (##{proc.pid}) exited with status '#{status}'"
print = (x...) -> OS.write OS.stdout, x.join("")

do prompt = ->
    print "$ "
    OS.read OS.stdin, 0, (err, buff) ->
        print "you entered: #{buff}\n"
        do prompt
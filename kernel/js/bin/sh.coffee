print = (x...) -> OS.write OS.stdout, x.join("")

for k of { a: 1, b: 2, c: 3 }
    print "#{k}\n"

do prompt = ->
    print "$ "
    OS.read OS.stdin, 0, (err, buff) ->
        print "you entered: #{buff}\n"
        do prompt
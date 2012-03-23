(function lol() {
    OS.read(OS.stdin, function(err, c) {
        if(!err) {
            OS.write(OS.stdout, c);
        }
        lol();
    });
})()
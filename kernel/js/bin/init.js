OS.read(OS.stdin, function(err, data) {
    OS.write(OS.stdout, "received '" + data + "'\n");
});
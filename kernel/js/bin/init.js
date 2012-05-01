OS.write(OS.stdout, "\nListing contents of /:\n");
var entries = OS.readDirectory("/");
for(var i = 0; i < entries.length; i++) {
    OS.write(OS.stdout, "  - " + entries[i].name + "\n");
}


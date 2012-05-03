OS.write OS.stdout, "\n Listening contents of /:\n"
for ent in OS.readDirectory "/"
    OS.write OS.stdout, "  - #{ent.name}\n"


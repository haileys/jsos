raw_msg = OS.argv[1..].join(" ")
msg = []
while raw_msg.length > 40
    msg.push raw_msg[0...40]
    raw_msg = raw_msg[40..]
msg.push raw_msg

line_len = msg[0].length

out = " "
out += "-" for _ in [1..(line_len+2)]
out += " \n"
if msg.length == 1
    out += "< #{msg[0]} >\n"
else
    [first, mid..., last] = msg
    out += "/ #{first} \\\n"
    out += "| #{line} |\n" for line in mid
    out += "\\ #{last}"
    out += " " for _ in [1..(40 - last.length)]
    out += " //\n"
out += " "
out += "-" for _ in [1..(line_len+2)]
out += " \n"

out += "        \\   ^__^               \n" +
       "         \\  (oo)\\_______      \n" +
       "            (__)\\       )\\/\\ \n" +
       "                ||----w |       \n" +
       "                ||     ||       \n"

OS.write OS.stdout, out

do OS.exit
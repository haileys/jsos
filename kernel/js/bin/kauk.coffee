rand = -> Math.floor(Math.random() * 16) * 16

OS.open "/dev/vga", (err, vga) ->
    OS.open "/dev/serial", (err, com) ->
        offset = 0
        do render = ->
            r = 80
            colors = ({ r: do rand, g: do rand, b: do rand } for i in [0..20])
            for i, color of colors
                th = 2 * Math.PI * i / 20 + offset
                x = 160 + r * Math.cos th
                y = 100 + r * Math.sin th
                OS.ioctl vga, "fillCircle", x, y, 6, color.r, color.g, color.b
            offset += Math.PI / 10
            OS.alarm 100, render
        OS.read OS.stdin, 0, ->
            do OS.exit
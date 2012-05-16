OS.open "/dev/vga", (err, vga) ->
    [x, y, r, dx, dy] = [160, 100, 10, 9, 9]
    drawBall = ->
        [old_x, old_y] = [x, y]
        x += dx
        y += dy
        #dy += 0.1
        if x < r
            dx *= -1
            x = r
        if x > 320 - r
            dx *= -1
            x = 320 - r
        if y < r
            dy *= -1
            y = r
        if y > 200 - r
            dy *= -1
            y = 200 - r
        OS.ioctl vga, "fillCircle", old_x, old_y, r, 0, 0, 0
        OS.ioctl vga, "fillCircle", x, y, r, 255, 0, 0
        OS.alarm 40, drawBall
    
    OS.read OS.stdin, 0, ->
        OS.close vga
        do OS.exit
        
    OS.ioctl vga, "clear"
    do drawBall
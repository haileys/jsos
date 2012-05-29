(function() {
    function Mode13h(baseAddr) {
        this.baseAddr = baseAddr || 0xa0000;
    };
    
    Mode13h.prototype.init = function() {
        // mov ax, 13h; int 10h; ret
        var code = "\xb8\x13\x00\xcd\x10\xc3";
        Kernel.realExec(code);
        this.setPalette();
        Kernel.memset(this.baseAddr, 0, 320*200);
    };
    
    Mode13h.prototype.close = function() {
        // mov ax, 03h; int 10h; ret
        var code = "\xb8\x03\x00\xcd\x10\xc3";
        Kernel.realExec(code);
    };
    
    Mode13h.prototype.setPalette = function() {
        // we'll map palette indexes to RGB values like so:
        // RRGGGBBB
        for(var r = 0; r <= 3; r++) {
            for(var g = 0; g <= 7; g++) {
                for(var b = 0; b <= 7; b++) {
                    var index = (r << 6) | (g << 3) | (b << 0);
                    Kernel.outb(0x3c8, index);
                    Kernel.outb(0x3c9, r * 21);
                    Kernel.outb(0x3c9, g * 9);
                    Kernel.outb(0x3c9, b * 9);
                }
            }
        }
    };
    
    Mode13h.prototype.copyBuffer = function(other) {
        Kernel.memcpy(this.baseAddr, other.baseAddr, 320*200);
    }
    
    Mode13h.prototype.rgbTo256 = function(r, g, b) {
        return ((r & 0xc0) >> 0) | ((g & 0xe0) >> 2) | ((b & 0xe0) >> 5);
    };
    
    Mode13h.prototype.drawRgb = function(x, y, width, height, data) {
        var maxy = y + height;
        var maxx = x + width;
        var base = this.baseAddr;
        var poke8 = Kernel.poke8;
        var rgbTo256 = this.rgbTo256;
        for(var iy = y; iy < maxy; iy++) {
            for(var ix = x; ix < maxx; ix++) {
                var addr = iy * width + ix;
                var r = BinaryUtils.readU8(data, addr * 3);
                var g = BinaryUtils.readU8(data, addr * 3 + 1);
                var b = BinaryUtils.readU8(data, addr * 3 + 2);
                var color = rgbTo256(r, g, b);
                poke8(base + addr, color);
            }
        }
    };
    
    Mode13h.prototype.draw256 = function(x, y, width, height, data) {
        var base = this.baseAddr;
        var writeMemory = Kernel.writeMemory;
        for(var iy = 0; iy < height; iy++) {
            writeMemory(base + (320 * (iy + y)) + x, data.substr(width * iy, width));
        }
    };
    
    Mode13h.prototype.drawPixel = function(x, y, color) {
        var addr = y * 320 + x;
        Kernel.poke8(this.baseAddr + addr, color);
    };
    
    Mode13h.prototype.drawRect = function(x1, y1, x2, y2, r, g, b) {
        var base = this.baseAddr;
        var poke8 = Kernel.poke8;
        var color = this.rgbTo256(r, g, b);
        for(var xi = x1; xi <= x2; xi++) {
            poke8(base + y1 * 320 + xi, color);
            poke8(base + y2 * 320 + xi, color);
        }
        for(var yi = y1; yi <= y2; yi++) {
            poke8(base + yi * 320 + x1, color);
            poke8(base + yi * 320 + x2, color);
        }
    };
    
    Mode13h.prototype.drawCircle = function(cx, cy, radius, r, g, b) {
        var base = this.baseAddr;
        var poke8 = Kernel.poke8;
        var color = this.rgbTo256(r, g, b);
        var cos = Math.cos;
        var sin = Math.sin;
        var floor = Math.floor;
        var pi = Math.PI;
        var pi2 = pi * 2;
        var circumference = pi2 * radius;
        var inc = pi2 / circumference;
        for(var theta = 0; theta < pi2; theta += inc) {
            poke8(base + floor(cy + sin(theta) * radius) * 320 + floor(cx + cos(theta) * radius), color);
        }
    };
    
    Mode13h.prototype.drawLine = function(x1, x2, y1, y2, r, g, b) {
        var base = this.baseAddr;
        var color = this.rgbTo256(r, g, b);
        var dx = Math.abs(x2 - x1);
        var dy = Math.abs(y2 - y1);
        var sx = 1;
        var sy = 1;
        if(x1 > x2) {
            sx = -1;
        }
        if(y1 > y2) {
            sy = -1;
        }
        var err = dx - dy;
        while(true) {
            this.drawPixel(x1, y1, color);
            if(x1 === x2 && y1 === y2) {
                break;
            }
            var e2 = 2 * err;
            if(e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if(e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }
    
    Mode13h.prototype.fillCircle = function(cx, cy, radius, r, g, b) {
        var base = this.baseAddr;
        var poke8 = Kernel.poke8;
        var color = this.rgbTo256(r, g, b);
        var sqrt = Math.sqrt;
        var memset = Kernel.memset;
        for(var y = -radius; y <= radius; y++) {
            var ycy = y + cy;
            if(ycy < 0) {
                continue;
            }
            if(ycy >= 320) {
                break;
            }
            var lineMiddle = base + 320 * (ycy) + cx;
            var lineWidth = sqrt(radius * radius - y * y);
            memset(lineMiddle - lineWidth, color, lineWidth * 2);
        }
    };
    
    Mode13h.prototype.fillRect = function(x1, y1, x2, y2, r, g, b) {
        var base = this.baseAddr;
        var poke8 = Kernel.poke8;
        var color = this.rgbTo256(r, g, b);
        for(var y = y1; y <= y2; y++) {
            Kernel.memset(base + y * 320 + x1, color, x2 - x1 + 1);
        }
    };
    
    Mode13h.prototype.clear = function() {
        this.fillRect(0, 0, 320, 200, 0, 0, 0);
    };
    
    Drivers.VGA = {
        Mode13h: Mode13h
    };
    
    // /dev/vga device:
    
    var openCount = 0;
    var mode13h = new Mode13h();
    Kernel.devfs.register("vga", {
        open: function() {
            openCount++;
            if(openCount === 1) {
                mode13h.init();
            }
            var file = {
                openCount: 0,
                read: function(size, callback) {
                    callback("can't read from /dev/vga");
                },
                close: function() {
                    openCount--;
                    if(openCount === 0) {
                        mode13h.close();
                    }
                },
                ioctl: {
                    drawPixel: function(fd, x, y, r, g, b) {
                        if( typeof x === "number" && typeof y === "number" &&
                            typeof r === "number" && typeof g === "number" &&
                            typeof b === "number") {
                            mode13h.drawPixel(x, y, mode13h.rgbTo256(r, g, b));
                        }
                    },
                    drawPixels: function(fd, data) {
                        var max = Math.max(Math.min(320 * 200, data.length), 0);
//                        Kernel.writeByteArrayToMemory(mode13h.baseAddr, data, max)
                        for(var i = 0; i < max; i++) {
                            if(typeof data[i] === "number") {
                                Kernel.poke8(mode13h.baseAddr + i, data[i]);
                            }
                        }
                    }
                }
            };
            var ioctls = {  drawRect: 7, drawCircle: 6, drawLine: 7,
                            fillCircle: 6, fillRect: 7, clear: 0 };
            for(var ioctl in ioctls) {
                (function(ioctl) {
                    file.ioctl[ioctl] = function() {
                        if(arguments.length < ioctls[ioctl] + 1) return false;
                        for(var i = 1; i < arguments.length; i++) {
                            if(typeof arguments[i] !== "number") return false;
                        }
                        mode13h[ioctl].apply(mode13h, arguments.slice(1));
                        return true;
                    };  
                })(ioctl);
            }
            return file;
        }
    });
})();
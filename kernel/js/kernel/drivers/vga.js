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
    
    Drivers.VGA = {
        Mode13h: Mode13h
    };
})();
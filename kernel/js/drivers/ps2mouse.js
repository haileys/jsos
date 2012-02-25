(function() {
    var MOUSE_IRQ       = 12;
    var MOUSE_PORT      = 0x60;
    var MOUSE_STATUS    = 0x64;
    var MOUSE_ABIT      = 0x02;
    var MOUSE_BBIT      = 0x01;
    var MOUSE_WRITE     = 0xD4;
    var MOUSE_FBIT      = 0x20;
    
    function PS2Mouse() {
        this._cycle = 0;
        this._bytes = [0, 0, 0];
    }
    
    PS2Mouse.prototype.wait = function(bit) {
        var timeout = 1000;
        while(--timeout) {
            if(Kernel.inb(MOUSE_STATUS) & bit) {
                return;
            }
        }
    };
    
    PS2Mouse.prototype.write = function(byte) {
        this.wait(MOUSE_ABIT);
        Kernel.outb(MOUSE_STATUS, MOUSE_WRITE);
        this.wait(MOUSE_ABIT);
        Kernel.outb(MOUSE_PORT, byte);
    };
    
    PS2Mouse.prototype.read = function() {
        this.wait(MOUSE_BBIT);
        return Kernel.inb(MOUSE_PORT);
    };
    
    PS2Mouse.prototype.enable = function() {
        this.wait(MOUSE_ABIT);
        Kernel.outb(MOUSE_STATUS, 0xA8);
        this.wait(MOUSE_ABIT);
        Kernel.outb(MOUSE_STATUS, 0x20);
        this.wait(MOUSE_BBIT);
        var status = Kernel.inb(MOUSE_PORT) | 2;
        this.wait(MOUSE_ABIT);
        Kernel.outb(MOUSE_STATUS, 0x60);
        this.wait(MOUSE_ABIT);
        Kernel.outb(MOUSE_PORT, status);
        this.write(0xF6);
        this.read();
        this.write(0xF4);
        this.read();
        
        var self = this;
        Kernel.isrs[44] = function() {
            self.onInterrupt();
        };
    };
    
    PS2Mouse.prototype.disable = function() {
        // @TODO: disable mouse properly
        Kernel.isrs[44] = null;
    };
    
    PS2Mouse.prototype.onInterrupt = function() {
        var status = Kernel.inb(MOUSE_STATUS);
        while(status & MOUSE_BBIT) {
            var mouse_in = Kernel.inb(MOUSE_PORT);
            if(status & MOUSE_FBIT) {
                this._bytes[this._cycle++] = mouse_in;
                if(this._cycle === 3) {
                    if(this._bytes[0] & 0xc0) {
                        // bad packet
                        console.log("bad mouse packet");
                        return;
                    }
                    var data = {
                        leftButton: (this._bytes[0] & 0x01) !== 0,
                        rightButton: (this._bytes[0] & 0x02) !== 0,
                        middleButton: (this._bytes[0] & 0x04) !== 0,
                        x: this._bytes[1],
                        y: this._bytes[2]
                    };
                    if(this._bytes[0] & 0x20 /* y sign bit */) {
                        data.y -= 256;
                    }
                    if(this._bytes[0] & 0x10 /* x sign bit */) {
                        data.x -= 256;
                    }
                    if(typeof this.onUpdate === "function") {
                        this.onUpdate(data);
                    }
                    this._cycle = 0;
                }
            }
            status = Kernel.inb(MOUSE_STATUS);
        }
    };
    
    Drivers.PS2Mouse = PS2Mouse;
})();
(function() {
    function PS2Keyboard() { }
    
    PS2Keyboard.prototype.init = function() {
        var self = this;
        Kernel.isrs[33] = function() {
            self.onInterrupt();
        };
        // disable and re-enable keyboard device
        var b = Kernel.inb(0x61);
        Kernel.outb(0x61, b | 0x80);
        Kernel.outb(0x61, b & 0x7f);
    };
    
    PS2Keyboard.prototype.onInterrupt = function() {
        var scanCode = Kernel.inb(0x60);
        if(typeof this.onScanCode === "function") {
            this.onScanCode(scanCode);
        }
    };
    
    Drivers.PS2Keyboard = PS2Keyboard;
})();
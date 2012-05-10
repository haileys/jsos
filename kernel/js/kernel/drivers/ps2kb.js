(function() {
    function PS2Keyboard() { }
    
    PS2Keyboard.prototype.init = function() {
        var self = this;
        Kernel.isrs[33] = function() {
            self.onInterrupt();
        };
        Kernel.outb(0x20, 0x20);
    };
    
    PS2Keyboard.prototype.onInterrupt = function() {
        while(Kernel.inb(0x64) & 2) ;
        var scanCode = Kernel.inb(0x60);
        if(typeof this.onScanCode === "function") {
            this.onScanCode(scanCode);
        }
    };
    
    Drivers.PS2Keyboard = PS2Keyboard;
})();
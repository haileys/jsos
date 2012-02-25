(function() {
    function PS2Keyboard() { }
    
    PS2Keyboard.prototype.enable = function() {
        var self = this;
        Kernel.isrs[33] = function() {
            self.onInterrupt();
        };
    };
    
    PS2Keyboard.prototype.disable = function() {
        Kernel.isrs[33] = null;
    };
    
    PS2Keyboard.prototype.onInterrupt = function() {
        var scanCode = Kernel.inb(0x60);
        if(typeof this.onScanCode === "function") {
            this.onScanCode(scanCode);
        }
    };
    
    Drivers.PS2Keyboard = PS2Keyboard;
})();
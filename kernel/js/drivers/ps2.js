(function() {
    function PS2() {
        this.irq = 33;
        this.port = 0x60;
    }
    
    PS2.prototype.enable = function() {
        var self = this;
        Kernel.isrs[this.irq] = function() {
            self.onInterrupt();
        };
    };
    
    PS2.prototype.disable = function() {
        Kernel.isrs[this.irq] = null;
    };
    
    PS2.prototype.onInterrupt = function() {
        var scanCode = Kernel.inb(this.port);
        if(typeof this.onScanCode === "function") {
            this.onScanCode(scanCode);
        }
    };
    
    Drivers.PS2 = PS2;
})();


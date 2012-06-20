(function() {
    var PIT_CLOCK_FREQ = 1193180;
    
    var listeners = [];
    Drivers.PIT = {
        init: function(frequency) {
            var divisor = PIT_CLOCK_FREQ / frequency;
            if(divisor & ~0xffff) {
                throw new RangeError("PIT frequency too low");
            }
            
            Kernel.cli();
            Kernel.outb(0x43, 0x36);
            Kernel.outb(0x40, divisor & 0xff);
            Kernel.outb(0x40, (divisor >> 8) & 0xff);
            Kernel.sti();
            
            Kernel.isrs[32] = function() {
                for(var i = 0; i < listeners.length; i++) {
                    listeners[i]();
                }
            };
        },
        subscribe: function(callback) {
            listeners.push(callback);
        }
    };
})();
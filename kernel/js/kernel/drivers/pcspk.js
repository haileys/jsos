(function() {
    Drivers.PCSpeaker = {
        play: function(freq) {
            var divisor = Math.round(1193180 / freq);
            Kernel.cli();
            Kernel.outb(0x43, 0xb6);
            Kernel.outb(0x42, divisor & 0xff);
            Kernel.outb(0x42, (divisor >> 8) & 0xff);
            Kernel.sti();
            
            Kernel.outb(0x61, Kernel.inb(0x61) | 3);
        },
        stop: function() {
            Kernel.outb(0x61, Kernel.inb(0x61) & 0xfc);
        }
    }
    
    Kernel.devfs.register("sound", {
        open: function() {
            return {
                openCount: 0,
                ioctl: {
                    play: function(fd, freq) {
                        if(typeof freq !== "number") {
                            return false;
                        }
                        Drivers.PCSpeaker.play(freq);
                        return true;
                    },
                    stop: function(fd) {
                        Drivers.PCSpeaker.stop();
                        return true;
                    }
                }
            };
        }
    });
})();
(function() {
    Drivers = {};
    
    Kernel.loadImage(Kernel.modules["/kernel/drivers/bios_hdd.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/fs.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/mbr.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/fat16.jmg"]);
    
    (function() {
        Kernel.filesystem = new Filesystem();
        var hdd = new Drivers.BiosHDD(Kernel.bootDevice);
        var mbr = new Drivers.MBR(hdd);
        Kernel.filesystem.mount("/", new Drivers.FAT16(mbr.partitions[0]));  
    })();
    
    (function() {
        Console.write("Detecting IDE... ");
        var ideDriver = Kernel.filesystem.read("/kernel/drivers/ide.jmg");
        if(ideDriver === null) {
            Console.write("could not load driver\n");
            return;
        }
        Kernel.loadImage(ideDriver);
        var hdd = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
        if(!hdd.detect()) {
            Console.write("no IDE drive detected\n");
            return;
        }
        var mbr = new Drivers.MBR(hdd);
        var fat16 = new Drivers.FAT16(mbr.partitions[0]);
        fat16.init();
        if(fat16.find("/kernel/drivers/fat16.jmg") === null) {
            Console.write("did not boot from master on primary channel\n");
            return;
        }
        Kernel.filesystem.unmount("/");
        Kernel.filesystem.mount("/", fat16);
        Console.write("ok\n");
    })();
    
    /*
    var hdd = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
    if(!hdd.detect()) {
        Kernel.panic("did not detect IDE drive on 0x1f0:MASTER")
    } else {
        Console.write("Detected IDE drive of size " + Math.round(hdd.getDriveSize() / 1024 / 1024) + " MiB\n");
    }
    */
    
    var images = [  "utils", "keyboard", "drivers/ps2kb",
                    "drivers/serial", "drivers/pit", "vm", "process",
                    "drivers/rtc", "drivers/vga" ];
    for(var i = 0; i < images.length; i++) {
        var path = "/kernel/" + images[i] + ".jmg";
        Console.write("Loading " + path + "... ");
        var data = Kernel.filesystem.read(path);
        if(!data) {
            throw new Error("could not load " + path);
        }
        Kernel.loadImage(data);
        Console.write("ok.\n");
    }
    
    Kernel.keyboard = new Keyboard(new Drivers.PS2Keyboard(), "US");
    
    Kernel.serial = new Drivers.Serial(Drivers.Serial.COM1);
    Kernel.serial.init();
    
    Drivers.PIT.init(100);
    
    Kernel.reboot = function() {
        while(Kernel.inb(0x64) & 0x02);
        Kernel.outb(0x64, 0xFE);
        while(true) { }
    };
    
    (function() {
        var init = new Process();
        init.enqueueCallback(function() { init.load("/bin/init.jmg"); });
        // stdin:
        init.fds[0] = new Pipe();
        Kernel.keyboard.onKeyDown = function(c, scancode) {
            if(c) {
                init.fds[0].write(c);
            }
        };
        // stdout/stderr:
        init.fds[1] = new Pipe.Sink(function(data) {
            Console.write(data);
        });
        init.fds[2] = init.fds[1];
    })();
    
    while(true) {
        Kernel.dispatchInterrupts();
        if(!Process.scheduleNext()) {
            Kernel.hlt();
        }
    }
})();
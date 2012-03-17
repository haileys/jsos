(function() {
    Drivers = {};
    
    Kernel.loadImage(Kernel.modules["/kernel/drivers/bios_hdd.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/ide.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/mbr.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/fat16.jmg"]);
    
    Console.write("Booting from drive number " + Kernel.bootDevice + "\n");
    
    var hdd = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
    if(!hdd.detect()) {
        Kernel.panic("did not detect IDE drive on 0x1f0:MASTER")
    } else {
        Console.write("Detected IDE drive of size " + Math.round(hdd.getDriveSize() / 1024 / 1024) + " MiB\n");
    }
    
    var mbr = new Drivers.MBR(hdd);
    var fs = new Drivers.FAT16(mbr.partitions[0]);
    fs.init();
    Kernel.filesystem = fs;
    
    var images = [  "utils", "keyboard", "keymaps", "drivers/ps2kb",
                    "drivers/serial", "drivers/pit", "vm", "process",
                    "drivers/rtc", "drivers/vga" ];
    for(var i = 0; i < images.length; i++) {
        var path = "/kernel/" + images[i] + ".jmg";
        Console.write("Loading " + path + "... ");
        var file = fs.find(path);
        if(!(file instanceof Drivers.FAT16.File)) {
            throw new Error("could not load " + path);
        }
        Kernel.loadImage(file.readAllBytes());
        Console.write("ok.\n");
    }
    
    Kernel.keyboard = new Keyboard(new Drivers.PS2Keyboard(), "US");
    
    Kernel.serial = new Drivers.Serial(Drivers.Serial.COM1);
    Kernel.serial.init();
    
    Kernel.log = function(str) {
        Kernel.serial.writeString(str + "\n");
    }
    
    Drivers.PIT.init(50);
    
    Kernel.reboot = function() {
        while(Kernel.inb(0x64) & 0x02);
        Kernel.outb(0x64, 0xFE);
        while(true) { }
    };
    
    Console.write("\nReading time...\n");
    var time = Drivers.RTC.readTime();
    Console.write("The time is: " + time.hours + ":" + time.minutes + ":" + time.seconds + "  " + time.day + "/" + time.month + "/" + time.year + "\n\n");
    
    Kernel.keyboard.onKeyDown = function(char, scancode) { Console.write(char); };
    
    var a = new Process();
    a._vm.globals.processName = "A";
    a.enqueueCallback(function() { a.load("/userland.jmg"); });
    
    var b = new Process();
    b._vm.globals.processName = "B";
    b.enqueueCallback(function() { b.load("/userland.jmg"); });
    
    while(true) {
        if(!Process.scheduleNext()) {
            Kernel.panic("no processes scheduled");
        }
    }
})();
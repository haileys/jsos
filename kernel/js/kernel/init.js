(function() {
    Drivers = {};
    
    Kernel.loadImage(Kernel.modules["/kernel/drivers/bios_hdd.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/fs.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/mbr.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/fat16.jmg"]);
    
    Kernel.filesystem = new Filesystem();
    var hdd = new Drivers.BiosHDD(Kernel.bootDevice);
    var mbr = new Drivers.MBR(hdd);
    Kernel.filesystem.mount("/", new Drivers.FAT16(mbr.partitions[0]));
    
    /*
    var hdd = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
    if(!hdd.detect()) {
        Kernel.panic("did not detect IDE drive on 0x1f0:MASTER")
    } else {
        Console.write("Detected IDE drive of size " + Math.round(hdd.getDriveSize() / 1024 / 1024) + " MiB\n");
    }
    */
    
    var images = [  "utils", "keyboard", "keymaps", "drivers/ps2kb",
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
    a.enqueueCallback(function() { a.load("/userland.jmg"); });
    
    Console.write("Have some random numbers!\n");
    for(var i = 0; i < 10; i++) {
        Console.write(Math.random() + "\n");
    }
    
    /*
    while(true) {
        if(!Process.scheduleNext()) {
            Kernel.panic("no processes scheduled");
        }
    }
    */
})();
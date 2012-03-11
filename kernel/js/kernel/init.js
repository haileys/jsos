(function() {    
    Drivers = {};

    Kernel.loadImage(Kernel.modules["/kernel/drivers/ide.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/mbr.jmg"]);
    Kernel.loadImage(Kernel.modules["/kernel/drivers/fat16.jmg"]);
    
    var hdd = new Drivers.MBR(new Drivers.IDE(0x1f0, Drivers.IDE.MASTER));
    var fs = new Drivers.FAT16(hdd.partitions[0]);
    fs.init();
    Kernel.filesystem = fs;
    
    var images = [  "utils", "keyboard", "keymaps", "drivers/ps2kb",
                    "drivers/serial", "drivers/pit", "process",
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
    Drivers.PIT.subscribe(function() {
        Process.tick();
    });
    
    Kernel.reboot = function() {
        
        while(Kernel.inb(0x64) & 0x02);
        Kernel.outb(0x64, 0xFE);
        while(true) { }
    };
    
    Console.write("\nReading time...\n");
    var time = Drivers.RTC.readTime();
    Console.write("The time is: " + time.hours + ":" + time.minutes + ":" + time.seconds + "  " + time.day + "/" + time.month + "/" + time.year + "\n\n");
    
    Console.write("jit compiling a function... ");
    var fn = Kernel.jit(function() { });
    Console.write("it returns: ");
    Console.write(fn() + "\n");
    
    var a = new Process();
    a._vm.globals.log = a._vm.exposeFunction(Console.write);
    a._vm.globals.yield = a._vm.exposeFunction(Process.yield);
    a._vm.globals.processName = "A";
    
    var b = new Process();
    b._vm.globals.log = b._vm.exposeFunction(Console.write);
    b._vm.globals.yield = b._vm.exposeFunction(Process.yield);
    b._vm.globals.processName = "B";
    
    Process.yield(function() { a.load("/userland.jmg"); });
    Process.yield(function() { b.load("/userland.jmg"); });
})();
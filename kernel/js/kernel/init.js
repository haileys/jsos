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
                    "drivers/serial", "drivers/pit", "process" ];
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
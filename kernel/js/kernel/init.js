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
                    "drivers/serial", "process" ];
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
    
    Console.write("HELLO!\n");
    
    var init = new Process();
    init.load("/userland.jmg");
})();
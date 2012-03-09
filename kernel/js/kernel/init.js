Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

// hard drive
Drivers.loadDriver("ide");
Drivers.loadDriver("mbr");
Drivers.loadDriver("fat16");

var hdd = new Drivers.MBR(new Drivers.IDE(0x1f0, Drivers.IDE.MASTER));
var fs = new Drivers.FAT16(hdd.partitions[0]);
fs.init();

Drivers.setLoader(function(name) {
    return fs.find(name).readAllBytes();
});

Kernel.loadImage(fs.find("/kernel/keyboard.jmg").readAllBytes());
Kernel.loadImage(fs.find("/kernel/keymaps.jmg").readAllBytes());

// keyboard
Drivers.loadDriver("ps2kb");
var ps2kb = new Drivers.PS2Keyboard();
var keyboard = new Keyboard(ps2kb, "US");

// serial port
Drivers.loadDriver("serial");
var serial = new Drivers.Serial(Drivers.Serial.COM1);
serial.init();
function log(str) {
    if(Drivers.Serial) {
        serial.writeString(str + "\n");
    }
}

var userland = new VM();
Console.write(userland + "\n");
Console.write(new VM().id + "\n");
userland.globals.log = function(str) {
    Console.write("[user process]  " + str + "\n");
};
Console.write("Jumping into userland...\n");
userland.execute(fs.find("/userland.jmg").readAllBytes());
Console.write("back from userland!\n");
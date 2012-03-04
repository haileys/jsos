Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/keymaps.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

Kernel.isrs[32] = function() { Console.write("."); };

// keyboard
Drivers.loadDriver("ps2kb");
var ps2kb = new Drivers.PS2Keyboard();
var keyboard = new Keyboard(ps2kb, "US");

// serial port
Drivers.loadDriver("serial");
var serial = new Drivers.Serial(Drivers.Serial.COM1);
serial.init();
function log(str) {
    serial.writeString(str + "\n");
}

// hard drive
Drivers.loadDriver("ide");
Drivers.loadDriver("mbr");
Drivers.loadDriver("fat16");

var hdd = new Drivers.MBR(new Drivers.IDE(0x1f0, Drivers.IDE.MASTER));
var fs = new Drivers.FAT16(hdd.partitions[0]);
fs.init();

Console.write("before read\n");
var imageData = fs.find("/jsos.rgb").readAllBytes();

// vga
Drivers.loadDriver("vga");
var ctx = new Drivers.VGA.Mode13h();
ctx.init();
ctx.drawRgb(0, 0, 320, 200, imageData);
Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/keymaps.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

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

Console.write("Partitions:\n");
for(var i = 0; i < 4; i++) {
    var part = hdd.partitions[i];
    Console.write("  #" + i + "  " + Drivers.MBR.systemIDs[part.systemID] + "  " + (part.totalSectors / 2 / 1024 >> 0) + " MiB\n");
}
Console.write("\n");

var fs = new Drivers.FAT16(hdd.partitions[0]);
fs.init();
Console.write("Initialized FAT16 partition with label '" + fs.bpb.label + "'\n");
var entries = fs.readRootEntries();
Console.write("Directory listing of '/' (" + entries.length + " items):\n");
for(var i = 0; i < entries.length; i++) {
    Console.write("  " + entries[i].filename + "\n");
}
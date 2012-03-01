Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/keymaps.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

Drivers.loadDriver("ps2kb");
var ps2kb = new Drivers.PS2Keyboard();
var keyboard = new Keyboard(ps2kb, "US");
keyboard.onKeyDown = function(key, scanCode) {
    Kernel.runGC();
    Console.write(key + ", " + scanCode + "\n");
}

Drivers.loadDriver("ide");
Drivers.loadDriver("fat16");
var hdd = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
var fs = new Drivers.FAT16(hdd, 1, 65519);
fs.init();
Console.write("Initialized FAT16 partition with OEM: " + fs.bpb.oem + "\n");
var entries = fs.readRootEntries();
Console.write("Directory listing of '/' (" + entries.length + " items):\n");
for(var i = 0; i < entries.length; i++) {
    Console.write("  " + entries[i].filename + "\n");
}
Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/keymaps.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

Kernel.isrs[32] = function() {
    Console.write(".");
};

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

// vga
Drivers.loadDriver("vga");
var ctx = new Drivers.VGA.Mode13h();
ctx.init();
ctx.drawRgb(0, 0, 320, 200, Kernel.modules["/jsos.rgb"]);

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
Console.write("Initialized FAT16 partition with label '" + fs.bpb.label + "'\n\n");
var entries = fs.readRootEntries();

function outputEnt(ent, level) {
    if(ent.name !== "." && ent.name !== "..") {
        Console.write(level + ent.name + "\n");
        if(ent instanceof Drivers.FAT16.Directory) {
            var children = ent.readEntries();
            for(var i = 0; i < children.length; i++) {
                outputEnt(children[i], level + "  ");
            }
        }
    }
}

for(var i = 0; i < entries.length; i++) {
    outputEnt(entries[i], "");
}
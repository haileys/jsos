console.log("Hello world from JavaScript!");

Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);
Drivers.loadDriver("ps2");
Keyboard.addDriver(new Drivers.PS2());

Drivers.loadDriver("ide");

var ide = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
console.log("before ata read");
var data = ide.readSectorPIO(1);
console.log("after ata read");
console.log("data length: ", data);
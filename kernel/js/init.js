console.log("Hello world from JavaScript!");

Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

Drivers.loadDriver("ps2kb");
Keyboard.addDriver(new Drivers.PS2Keyboard());

Drivers.loadDriver("ps2mouse");
var mouse = new Drivers.PS2Mouse();
mouse.enable();
var x = 0;
var y = 0;
mouse.onUpdate = function(data) {
    x += data.x;
    y += data.y;
    console.log("(" + x + ", " + y + ")");
//    console.log("[ " + (data.leftButton ? "#" : " ") + " " + (data.middleButton ? "#" : " ") + " " + (data.rightButton ? "#" : " ") + " ]");
}

Drivers.loadDriver("ide");
var hdd = new Drivers.IDE(0x1f0, Drivers.IDE.MASTER);
hdd.readSectorPIO(1);
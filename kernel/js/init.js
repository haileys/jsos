console.log("Hello world from JavaScript!");

Kernel.loadImage(Kernel.modules["/kernel/keyboard.jmg"]);
Kernel.loadImage(Kernel.modules["/kernel/keymaps.jmg"]);

Kernel.loadImage(Kernel.modules["/kernel/drivers.jmg"]);

Drivers.loadDriver("ps2kb");
console.log("before create keyboard");
var ps2kb = new Drivers.PS2Keyboard();
var keyboard = new Keyboard(ps2kb, "US");
keyboard.onKeyDown = function(key, scanCode) {
    console.log(key);
}

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

console.log("Finished initializing");
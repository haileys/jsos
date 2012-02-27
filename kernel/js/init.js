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
var tick = 0;
var ABC = " ";
mouse.onUpdate = function(data) {
    x += data.x;
    y += data.y;
    console.log(Kernel.memoryUsage() / 1024, "KiB");
    if(tick++ === 20) {
        tick = 0;
        console.log("--->", typeof Kernel.isrs, typeof mouse.onUpdate);
        Kernel.runGC();
        console.log("===>", typeof Kernel.isrs, typeof mouse.onUpdate);
    }
}
console.log("Finished initializing");
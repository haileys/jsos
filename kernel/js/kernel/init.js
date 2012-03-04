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

var inputBuffer = "";

keyboard.onKeyDown = function(char, scanCode) {
    var rc = Console.cursor();
    log(rc[0] + ", " + rc[1]);
    var row = rc[0];
    if(scanCode === 14) {
        // backspace
        if(inputBuffer.length) {
            inputBuffer = inputBuffer.substr(0, inputBuffer.length - 1);
            Console.cursor(row, 2 + inputBuffer.length);
            Console.write(" ");
            Console.cursor(row, 2 + inputBuffer.length);
        }
    } else if(char) {
        if(char === "\n") {
            Console.write("\n");
            var ent = fs.find(inputBuffer);
            if(ent === null) {
                Console.write("File not found\n");
            } else if(ent instanceof Drivers.FAT16.Directory) {
                Console.write("Directory\n");
            } else if(ent instanceof Drivers.FAT16.File) {
                Console.write("File of size " + ent.size + "\n");
            }
            Console.write("> ");
            inputBuffer = "";
        } else {
            inputBuffer += char;
            Console.write(char);
        }
    } else {
        log(scanCode);
    }
};

Console.write("Interactive FAT shell.\n");
Console.write("> ");

// serial port
Drivers.loadDriver("serial");
var serial = new Drivers.Serial(Drivers.Serial.COM1);
serial.init();
function log(str) {
    if(Drivers.Serial) {
        serial.writeString(str + "\n");
    }
};
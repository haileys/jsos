(function() {
    var KEY_RELEASE = 0x80;
    var KEY_CAPS_LOCK = 58;
    
    function Keyboard(driver) {
        var self = this;
    
        this.driver = driver;
        this.keyStates = [];
        this.capsLock = false;
        for(var i = 0; i < 128; i++) {
            this.keyStates[i] = false;
        }
        
        this.driver.onScanCode = function(scanCode) {
            self.dispatchScanCode(scanCode);
        };
        this.driver.init();
    }
    
    Keyboard.prototype.shiftState = function() {
        return !!(this.keyStates[42] || this.keyStates[54]);
    };
    
    Keyboard.prototype.controlState = function() {
        return !!(this.keyStates[29]);
    };
    
    Keyboard.prototype.altState = function() {
        return !!this.keyStates[56];
    };
    
    Keyboard.prototype.translateScanCode = function(scanCode) {
        var shiftState = this.shiftState();
        if(this.capsLock) {
            shiftState = !shiftState;
        }
        return Keyboard.keyMap[scanCode * 2 + Number(shiftState)];
    };
    
    Keyboard.prototype.dispatchScanCode = function(scanCode) {
        if(scanCode & KEY_RELEASE) {
            scanCode -= KEY_RELEASE;
            this.keyStates[scanCode] = false;
            if(typeof this.onKeyUp === "function") {
                this.onKeyUp(this.translateScanCode(scanCode), scanCode);
            }
        } else {
            if(scanCode === KEY_CAPS_LOCK) {
                this.capsLock = !this.capsLock;
            }
            this.keyStates[scanCode] = true;
            if(typeof this.onKeyDown === "function") {
                this.onKeyDown(this.translateScanCode(scanCode), scanCode);
            }
        }
    };
    
    this.Keyboard = Keyboard;
    
    Keyboard.keyMap = [
        // 0x00
        null, null,
        null, null,
        "1", "!",
        "2", "@",
        "3", "#",
        "4", "$",
        "5", "%",
        "6", "^",
        "7", "&",
        "8", "*",
        "9", "(",
        "0", ")",
        "-", "_",
        "=", "+",
        "\x7f", "\x7f", // backspace
        " ", " ",

        // 0x10
        "q", "Q",
        "w", "W",
        "e", "E",
        "r", "R",
        "t", "T",
        "y", "Y",
        "u", "U",
        "i", "I",
        "o", "O",
        "p", "P",
        "[", "{",
        "]", "}",
        "\n", "\n",
        null, null,  
        "a", "A",
        "s", "S",

        // 0x20
        "d", "D",
        "f", "F",
        "g", "G",
        "h", "H",
        "j", "J",
        "k", "K",
        "l", "L",
        ";", ":",
        "'", "\"",
        "`", "~",
        null, null,
        "\\", "|",
        "z", "Z",
        "x", "X",
        "c", "C",
        "v", "V",

        // 0x30
        "b", "B",
        "n", "N",
        "m", "M",
        ",", "<",
        ".", ">",
        "/", "?",
        null, null,
        "*", "*",
        null, null,
        " ", " ",
        null, null,
        null, null,
        null, null,
        null, null,
        null, null,
        null, null,

        // 0x40
        null,  null,
        null,  null,
        null,  null,
        null,  null,
        null,  null,
        null,  null,
        null,  null,
        "7", "7",
        "8", "8",
        "9", "9",
        "-", "-",
        "4", "4",
        "5", "5",
        "6", "6",
        "+", "+",
        "1", "1",

        // 0x50
        "2", "2",
        "3", "3",
        "0", "0",
        ".", ".",
        null, null
    ];
})();


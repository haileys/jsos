(function() {
    var KEY_RELEASE = 0x80;
    var KEY_CAPS_LOCK = 58;
    
    function Keyboard(driver, keyMap) {
        var self = this;
    
        this.driver = driver;
        this.keyStates = [];
        this.capsLock = false;
        for(var i = 0; i < 128; i++) {
            this.keyStates[i] = false;
        }
        
        this.setKeyMap(keyMap);
        this.driver.onScanCode = function(scanCode) {
            self.dispatchScanCode(scanCode);
        };
        this.driver.enable();
    }
    
    Keyboard.keyMaps = {};
    
    Keyboard.prototype.setKeyMap = function(keyMap) {
        this.keyMapName = keyMap;
        this.keyMap = Keyboard.keyMaps[keyMap];
    };
    
    Keyboard.prototype.shiftState = function() {
        return !!this.keyStates[42];
    };
    
    Keyboard.prototype.controlState = function() {
        return !!this.keyStates[29];
    };
    
    Keyboard.prototype.altState = function() {
        return !!this.keyStates[56];
    };
    
    Keyboard.prototype.translateScanCode = function(scanCode) {
        var shiftState = this.shiftState();
        if(this.capsLock) {
            shiftState = !shiftState;
        }
        return this.keyMap[scanCode * 2 + Number(shiftState)];
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
})();


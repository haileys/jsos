(function() {
    Keyboard = {};
    
    var keyUpListeners = [];
    Keyboard.bindKeyUp = function(fn) {
        keyUpListeners.push(fn);
    };
    
    var keyDownListeners = [];
    Keyboard.bindKeyDown = function(fn) {
        keyDownListeners.push(fn);
    }
    
    var keyPressListeners = [];
    Keyboard.bindKeyPress = function(fn) {
        keyPressListeners.push(fn);
    }
    
    var currentKeyMap = "";
    Keyboard.getKeyMap = function() {
        return currentKeyMap;
    }
    Keyboard.setKeyMap = function(keyMap) {
        currentKeyMap = keyMap;
    };
    
    Keyboard.addDriver = function(driver) {
        driver.onScanCode = Keyboard.dispatchScanCode;
        driver.enable();
    }
    
    Keyboard.dispatchScanCode = function(scanCode) {
        console.log("received scancode", scanCode);
    };
})();


(function() {
    Drivers.VGA = {
        mode13h: function() {
            // mov ax, 13h; int 10h; ret
            var code = "\xb8\x13\x00\xcd\x10\xc3";
            Kernel.realExec(code);
        }
    };
})();
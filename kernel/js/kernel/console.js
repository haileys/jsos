(function() {
    function STDIN() {
        this._buffered = true;
        this._echo = true;
        this._readers = new Queue();
        this._buffer = "";
        this.openCount = 0;
    }
    
    STDIN.prototype.postKey = function(c, scancode) {
        if(c) {
            if(this._echo) {
                if(c === "\x7f" /* backspace */) {
                    if(!this._buffered || this._buffer.length > 0) {
                        var rc = Console.cursor();
                        rc[1]--;
                        if(rc[1] < 0) {
                            rc[1] = Console.size()[0] - 1;
                            rc[0]--;
                            if(rc[0] < 0) {
                                rc[0] = 0;
                            }
                        }
                        Console.cursor(rc[0], rc[1]);
                        Console.write(" ");
                        Console.cursor(rc[0], rc[1]);
                    }
                } else {
                    Console.write(c);
                }
            }
            if(this._buffered) {
                if(c === "\x7f" /* backspace */) {
                    if(this._buffer.length > 0) {
                        this._buffer = this._buffer.substr(0, this._buffer.length - 1);
                    }
                } else {
                    this._buffer += c;
                }
            } else {
                this._buffer += c;
            }
            this._trigger();
        }
    };
    
    STDIN.prototype._trigger = function() {
        if(!this._buffered) {
            while(this._buffer.length > 0 && !this._readers.isEmpty()) {
                var reader = this._readers.pop();
                if(reader.size === 0 || reader.size > this._buffer.length) {
                    var readSize = this._buffer.length;
                } else {
                    var readSize = reader.size;
                }
                var buff = this._buffer.substr(0, readSize);
                this._buffer = this._buffer.substr(readSize);
                reader.callback(false, buff);
            }
        } else {
            while(this._buffer.indexOf("\n") >= 0 && !this._readers.isEmpty()) {
                var nl = this._buffer.indexOf("\n") + 1;
                var reader = this._readers.pop();
                if(reader.size === 0 || reader.size > this._buffer.length) {
                    var readSize = nl;
                } else {
                    var readSize = reader.size;
                }
                var buff = this._buffer.substr(0, readSize);
                this._buffer = this._buffer.substr(readSize);
                reader.callback(false, buff);
            }
        }
    };
    
    STDIN.prototype.ioctl = {
        buffered: function(proc, fd, buffered) {
            fd._buffered = Boolean(buffered);
            fd._trigger();
        },
        echo: function(proc, fd, echo) {
            fd._echo = Boolean(echo);
        }
    };
    
    STDIN.prototype.read = function(size, callback) {
        this._readers.push({ size: size, callback: callback });
        this._trigger();
    };
    
    STDIN.prototype.write = function() {
        return false;
    }
    
    STDIN.prototype.close = function() {};
    
    Console.STDIN = STDIN;
})();
//
// Queue
//

function Queue() {
    this._head = null;
    this._tail = null;
}

Queue.prototype.isEmpty = function() {
    return this._head === null;
};

Queue.prototype.front = function() {
    if(this.isEmpty()) {
        throw new RangeError("Can't pop object from empty queue");
    }
    return this._head.object;
};

Queue.prototype.push = function(object) {
    if(!this.isEmpty()) {
        this._tail.next = { next: null, object: object };
        this._tail = this._tail.next;
    } else {
        this._head = this._tail = { next: null, object: object };
    }
};

Queue.prototype.pop = function() {
    if(this.isEmpty()) {
        throw new RangeError("Can't pop object from empty queue");
    }
    var obj = this._head.object;
    this._head = this._head.next;
    if(this._head === null) {
        this._tail === null;
    }
    return obj;
};

Queue.prototype.pushFront = function(object) {
    if(!this.isEmpty()) {
        this._head = { next: this._head, object: object };
    } else {
        this._head = this._tail = { next: null, object: object };
    }
};

//
// Pipe
//

function Pipe() {
    this._buffer = "";
    this._readers = new Queue();
    this.ioctl = {};
}

Pipe.prototype.read = function(size, callback) {
    if(callback === undefined) {
        callback = size;
        size = undefined;
    }
    if(size < 0) {
        return;
    }
    this._readers.push({ size: size || 0, callback: callback });
    this._trigger();
};

Pipe.prototype.write = function(object) {
    this._buffer += object.toString();
    this._trigger();
};

Pipe.prototype._trigger = function() {
    if(this._readers.isEmpty()) {
        return;
    }
    var reader = this._readers.front();
    if(reader.size === 0 && this._buffer.length > 0) {
        this._readers.pop();
        var buff = this._buffer;
        this._buffer = "";
        reader.callback(false, buff);
        return;
    }
    if(reader.size <= this._buffer.length) {
        this._readers.pop();
        var buff = this._buffer.substr(0, reader.size);
        this._buffer = this._buffer.substr(reader.size);
        reader.callback(false, buff);
        return;
    }
}

//
// Pipe.Sink
//

Pipe.Sink = function(fn) {
    this.fn = fn;
    this.ioctl = {};
};

Pipe.Sink.prototype.read = function(size, callback) {
    callback("not open for reading");
};

Pipe.Sink.prototype.write = function(object) {
    this.fn(object);
};

//
// Pipe.Source
//

Pipe.Source = function(fn) {
    this.fn = fn;
    this.ioctl = {};
};

Pipe.Source.prototype.read = function(size, callback) {
    this.fn(function(err, data) {
        callback(err, data);
    });
};

Pipe.Source.prototype.write = function(object) {
    callback("not open for writing");
};
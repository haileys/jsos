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
    if(size === undefined ? this._buffer.length > 0 : this._buffer.length >= size) {
        if(size === undefined) {
            var buff = this._buffer;
            this._buffer = "";
            callback(false, buff);
        } else {
            var buff = this._buffer.substr(0, size);
        }
        var obj = this._buffer.pop();
        callback(false, obj);
    } else {
        this._readers.push(callback);
    }
};

Pipe.prototype.write = function(object) {
    if(this._readers.isEmpty()) {
        this._buffer.push(object);
    } else {
        var callback = this._readers.pop();
        callback(false, object);
    }
};

//
// Pipe.Sink
//

Pipe.Sink = function(fn) {
    this.fn = fn;
    this.ioctl = {};
};

Pipe.Sink.prototype.read = function(callback) {
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
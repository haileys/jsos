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

//
// Pipe
//

function Pipe() {
    this._buffer = new Queue();
    this._readers = new Queue();
    this.ioctl = {};
}

Pipe.prototype.read = function(callback) {
    if(!this._buffer.isEmpty()) {
        callback(false, this._buffer.pop());
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

Pipe.Source.prototype.read = function(callback) {
    this.fn(function(err, data) {
        callback(err, data);
    });
};

Pipe.Source.prototype.write = function(object) {
    callback("not open for writing");
};
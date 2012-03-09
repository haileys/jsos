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
    this._readers = [];
}

Pipe.prototype.read = function(callback) {
    if(!this._buffer.isEmpty()) {
        callback(this._buffer.pop());
    } else {
        this._readers.push(callback);
    }
};

Pipe.prototype.write = function(object) {
    if(this._readers.count > 0) {
        var readers = this._readers;
        this._readers = [];
        for(var i = 0; i < this._readers.count; i++) {
            readers[i](object);
        }
    } else {
        this._buffer.push(object);
    }
};

Pipe.prototype.clear = function() {
    this._buffer = new Queue();
};
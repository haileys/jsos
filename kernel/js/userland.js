var i = 0;
(function cb() {
//    OS.log(i++);
    OS.yield(cb);
})();
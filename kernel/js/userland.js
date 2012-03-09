var counter = 0;
(function foo() {
    log(counter++);
    foo();
})();
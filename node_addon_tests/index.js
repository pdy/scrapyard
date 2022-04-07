const hello_world = require('bindings')('hello_addon')

console.log(hello_world.sayHi());

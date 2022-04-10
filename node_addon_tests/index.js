const hello_world = require('./lib/bindings.js')

console.log(hello_world.sayHi());

function substrRet(err, val)
{
  if(err)
    console.log(err);
  else
    console.log("val: " + val);
}

console.log("text 2");
hello_world.helloSubstr("text", 2, substrRet);

console.log("text -2");
hello_world.helloSubstr("text", -2, substrRet);

console.log("text 4");
hello_world.helloSubstr("text", 4, substrRet);

var nativeInstance = new hello_world.NativeClass("native_text");
console.log("native_test " + 3);
console.log(nativeInstance.substr(3));


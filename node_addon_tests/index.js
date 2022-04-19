const hello_world = require('./lib/bindings.js')

console.log(hello_world.sayHi());

function substrRet(err, val)
{
  if(err)
    console.log(err);
  else
    console.log("val: " + val);
}

const buffer = Buffer.from([8, 6, 7, 5, 3, 0, 9]);
console.log("reverse call");
hello_world.reverseByteBuffer(buffer, function(err, retBuffer){
  console.log("reverse callback");
  if(err)
    console.log(err);
  else if(retBuffer === undefined)
    console.log("\tret buffer undefined");
  else
  {
    console.log("\torg buffer ");
    console.log(buffer);
    console.log(retBuffer);
    console.log(buffer);
  }
});

console.log("text 2");
hello_world.helloSubstr("text", 2, substrRet);

console.log("text -2");
hello_world.helloSubstr("text", -2, substrRet);

console.log("text 4");
hello_world.helloSubstr("text", 4, substrRet);

var nativeInstance = new hello_world.NativeClass("native_text");
console.log("native_test " + 3);
console.log(nativeInstance.substr(3));




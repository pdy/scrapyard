const hello_world = require('./build/Release/hello_addon.node')

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

//console.log("HelloStr with num 2");
//const nativeStrFail = new hello_world.HelloStr(2);

console.log("HelloStr with str my_text");
const nativeStr = new hello_world.HelloStr("my_text");

console.log("HelloStr::substr(2)");
console.log(nativeStr.substr(2));




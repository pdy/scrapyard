const hello_world = require('./build/Release/hello_addon.node')

console.log(hello_world.sayHi());

function substrRet(err, val)
{
  if(err)
    console.log(err);
  else
    console.log("val: " + val);
}

hello_world.helloSubstr("text", 2, substrRet);
hello_world.helloSubstr("text", -2, substrRet);
hello_world.helloSubstr("text", 4, substrRet);

const addon = require('../build/Release/hello_addon.node')

function sayHi(){
  return addon.sayHi();
}

function helloSubstr(str, posNum, callback) {
  return addon.helloSubstr(str, posNum, callback);
}

function NativeClassStrDemoWrap(str){
  this.substr = function(num){
    return _instance.substr(num);
  }

  var _instance = new addon.HelloStr(str);
}

module.exports = {
  sayHi: sayHi,
  helloSubstr: helloSubstr,

  NativeClass: NativeClassStrDemoWrap

};




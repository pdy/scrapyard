'use strict';

const addon = require('../build/Release/hello_addon.node')

/**
 * @returns {string}
 */
function sayHi(){
  return addon.sayHi();
}

/**
 * @param {string} str
 * @param {number} posNum
 * @param {function} callback
 * @returns {string}
 */
function helloSubstr(str, posNum, callback) {
  return addon.helloSubstr(str, posNum, callback);
}

/**
 * @param {string} str
 */
function NativeClassStrDemoWrap(str){
  
  /**
   * @param {number} num
   * @returns {string}
  */
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




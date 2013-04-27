/* This file is part of AdLeaks.
 * Copyright (C) 2013 Benjamin Güldenring
 * Freie Universität Berlin, Germany
 *
 * AdLeaks is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 *
 * AdLeaks is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AdLeaks.  If not, see <http://www.gnu.org/licenses/>.
 */

// include CryptoJS lib before this file
// include protocol config before this file
// include helpers.js before this file
//

var modulus_N;
var gen_g;
var gen_h;
var param_s;

var n_exp_s;
var n_exp_s_plus_1;
var n_exp_2;

var hashkey_H;

var baseUrl;


function loadConfig(){
  modulus_N = new BigInteger(config.modulus_N, 16);
  gen_g = new BigInteger(config.generator_g, 16);
  gen_h = new BigInteger(config.generator_h, 16);

  param_s = config.param_s;

  n_exp_s = modulus_N.pow(param_s);
  n_exp_s_plus_1 = modulus_N.pow(param_s+1);
  n_exp_2 = modulus_N.pow(2);
  
  hashkey_H = CryptoJS.enc.Hex.parse(config.hashKey_H);

  baseUrl = config.baseUrl;
}

function hashH(toHash){
  return _hash(toHash, hashkey_H);
}

function arrayToNum(array){
  var word_array = CryptoJS.lib.WordArray.create(array);
  var hex_string = CryptoJS.enc.Hex.stringify(word_array);
  var num = new BigInteger(hex_string, 16);
  return num;
}

function messageToNum(message){
  var m = CryptoJS.enc.Latin1.parse(message);
  var hexm = CryptoJS.enc.Hex.stringify(m);

  return new BigInteger(hexm, 16);;
}

function concatNums(n, m){
  var toShift = m.bitLength();
  toShift = toShift + toShift % 8;
  var nshifted = n.shiftLeft(toShift);

  return nshifted.add(m);
}

function encodeChunk(c, t){
  // format in base64
  // and fix the cases where half bytes occur (or the base64 conversion below fails)
  var _resC = c.toString(16);
  if(_resC.length % 2 == 1)
    _resC = "0" + _resC
  var resC = CryptoJS.enc.Base64.stringify(CryptoJS.enc.Hex.parse(_resC));
  var _resT = t.toString(16);
  if(_resT.length % 2 == 1)
    _resT = "0" + _resT;
  var resT = CryptoJS.enc.Base64.stringify(CryptoJS.enc.Hex.parse(_resT));

  return resC + ";" + resT;
}

function encryptChunk(params){
  return encryptData(params.m, params.r0, params.r1, params.r2);
}

function encryptNeutral(params){
  return encryptData("", new Array(), params.r1, params.r2);
}

function encryptData(_m, _r0, _r1, _r2){
// from the tech report:
// EncData(m, r0) = 
//  r1,r2 <- R
//  chk <- 
//    if m,r0 = 0
//      then 0
//    else H(m,r0)
//  c <- psi(m; hchk · g^r1 )
//  t <- psi(r0||r1; g^r2 )
//  return c, t

  var r0 = arrayToNum(_r0);
  var r1 = arrayToNum(_r1);
  var r2 = arrayToNum(_r2);
  var m = messageToNum(_m);
  var chk;

  if(m.equals(BigInteger.ZERO) && r0.equals(BigInteger.ZERO))
    chk = BigInteger.ZERO;
  else
    chk = hashBigInt(hashH, concatNums(m, r0));

  console.log(r0.toString(16));
  console.log(r1.toString(16));

  var hchk = gen_h.modPow(chk, n_exp_s_plus_1);

  // c = (1+N)^m * (g^r1)^(N^s) mod N^(s+1)
  //   =           (g^r1)^(N^s) mod N^(s+1)
  //   when m = 0;
  var c_m = BigInteger.ONE;
  if(! m.equals(BigInteger.ZERO))
  {
    c_m = c_m.add(modulus_N);
    c_m = c_m.modPow(m, n_exp_s_plus_1);
  }
  var g_exp_r1 = gen_g.modPow(r1, n_exp_s_plus_1);
  var c_r = g_exp_r1.multiply(hchk);
  var c = c_m.multiply(c_r);
  c = c.mod(n_exp_s_plus_1);

  r0 = r0.shiftLeft(256+1024);
  var r0r1 = r1.add(r0);
  // t = (1+N)^m * (g^r2)^N mod N^2
  //   = 1 + m*N * (g^r2)^N mod N^2
  var t_m = r0r1.multiply(modulus_N);
  t_m = t_m.add(BigInteger.ONE);
  var g_exp_r2 = gen_g.modPow(r2, n_exp_2);
  var t_r = g_exp_r2.modPow(modulus_N, n_exp_2);

  var t = t_m.multiply(t_r);
  t = t.mod(n_exp_2);

  return encodeChunk(c, t);
}


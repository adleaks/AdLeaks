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

function createRandom(rSize){
  var _r;
  var r;

  if(typeof window != 'undefined')
  {
    try
    {
      _r = new Uint32Array(rSize);
      window.crypto.getRandomValues(_r);
    }
    catch(error)
    {
      console.log("critical error: getRandomValues() failed, aborting");
      return null;
    }
    r = new Array();
    for(i in _r)
      r[i] = _r[i];
  }
  else  // for node.js
  { 
    _r = require('crypto').randomBytes(32);
    r = new Array();
    for(var i = 0; i < _r.length; i+=4)
    {
      r[i%4] = _r[i];
      r[i%4] |= _r[i+1]<<8;
      r[i%4] |= _r[i+2]<<16;
      r[i%4] |= _r[i+3]<<24;
    }  
  }

  return r;
}

function protocolCreateMessageEncryptNeutral(){
  var rSize = 32*8/32;

  r1 = createRandom(rSize);
  r2 = createRandom(rSize);
  
  var message = 
    {
      encryptNeutral : 1,
      params: {r1 : r1, r2 : r2}
    }

  return message;
}

function protocolCreateMessageEncryptTestData(m){
  var rSize = 32*8/32;

  r0 = createRandom(rSize);
  r1 = createRandom(rSize);
  r2 = createRandom(rSize);

  var message = 
    {
      encryptTestData : 1,
      params: {m : m, r0 : r0, r1 : r1, r2 : r2}
    }

  return message;
}


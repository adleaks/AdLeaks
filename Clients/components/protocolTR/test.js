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

var assert = require("assert")


// this really works and makes "navigator" accessible from everywhere
// (we need this hack for the BigInteger library)
if(typeof navigator == 'undefined')
{
  var navigator = { 
    appName: ""
  };  
}

describe('protocolTR', function(){
  it('init',
    function(){
      loadConfig();
    });

  it('encrypt zero', 
    function(){
      var rSize = 32*8/32;
      var r1 = createRandom(rSize);
      var r2 = createRandom(rSize);

      var z = encryptNeutral({r1:r1, r2:r2});
      console.log("encrypt zero: " + z);
    })
  it('encrypt chunk',
    function(){
      var rSize = 32*8/32;
      var r0 = createRandom(rSize);
      var r1 = createRandom(rSize);
      var r2 = createRandom(rSize);

      var z = encryptChunk({m:"hello world", r0:r0, r1:r1, r2:r2});
      console.log("encrypt chunk: " + z);
    })
})



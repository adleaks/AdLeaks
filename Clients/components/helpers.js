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

function _hash(toHash, fkey){
  var x = CryptoJS.lib.WordArray.create([]);
  x.concat(fkey);
  x.concat(toHash);
  var res = CryptoJS.SHA256(x);
  delete x;
  return res;
}

function hashBigInt(hashfunc, toHash){
  var str = toHash.toString(16);
  if(str.length % 2 == 1)
  {
    str = "0" + str;    // we hate half bytes
  }
  var wordarray = CryptoJS.enc.Hex.parse(str);
  var hashed = hashfunc(wordarray);
  var hashednum = new BigInteger(CryptoJS.enc.Hex.stringify(hashed), 16);

  delete hashed;
  delete wordarray;
  delete str;

  return hashednum;
}



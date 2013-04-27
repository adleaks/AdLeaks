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

loadConfig();
self.onmessage = function(e) {
  var msg = JSON.parse(e.data);

  if(msg.encryptNeutral == 1)
  {
    self.postMessage('doing encryption...');
    var start = new Date().getTime();
    var val = encryptNeutral(msg.params);
    var end = new Date().getTime();
    self.postMessage('did it in ' + (end-start)/1000 + 's');
    self.postMessage("Encrypted: " + val);
  }
  else if(msg.encryptTestData == 1)
  {
    self.postMessage('doing test data encryption...');
    var start = new Date().getTime();
    var val = encryptChunk(msg.params);
    var end = new Date().getTime();
    self.postMessage('did it in ' + (end-start)/1000 + 's');
    self.postMessage("Encrypted: " + val);
  }

  self.postMessage('sending...')
  var req = new XMLHttpRequest();
  var geturl = baseUrl + val;
  req.open("Get", geturl, true);
  req.onload = function loaded(e){
    self.postMessage("response: " + req.response);
    self.postMessage("done");
  }

  req.send(null);
};



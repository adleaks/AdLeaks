Build the AD's iframe at build/ad/ad_iframe.html by typing "make". 

Folder Structure:
The folder ad/ contains a template for the AD's iframe and its js worker.
These are used together with the external libraries in libs/, 
the files in components/ and the used PROTOCOL (as defined
in the Makefile) with its files in components/<PROTOCOL> to build
the iframe at build/ad/ad_iframe.html.

Interface:
The components/<PROTOCOL>/iframe.js file implements a
  function protocolCreateMessageEncryptNeutral(),
that prepares a message (together with all randomness needed for 
encryption). This message is sent in ad/ad_iframe.template.html to the
background worker, there processed in ad/worker.js and the parameters
passed to the encryption function 
  function encryptNeutral(params)
defined in components/<PROTOCOL>/worker.js.

To implement a new protocol:
  * when using different libs than the ones already 
    defined in the Makefile, adjust the Makefile.
  * In any other case it should suffice to create the files
    in components/<MYPROTOCOL>/ that implement the interface described
    above and set the PROTOCOL variable in the Makefile to MYPROTOCOL.


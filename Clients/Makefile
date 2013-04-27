.PHONY: clean 

PROTOCOL = protocolTR

ADIFRAME = ad_iframe.html

ADWORKERJS = libs/jsbn.js \
	    libs/jsbn2.js \
	    libs/sha256.js \
	    libs/enc-base64-min.js \
	    components/fountain.js \
	    config_$(PROTOCOL).js \
	    components/helpers.js \
	    components/$(PROTOCOL)/$(PROTOCOL).js \
	    components/$(PROTOCOL)/worker.js \
	    ad/worker.js
SENDHTMLJS = $(ADWORKERJS)
TESTLIBS =  libs/jsbn.js \
	    libs/jsbn2.js \
	    libs/sha256.js \
	    libs/enc-base64-min.js \
	    config_$(PROTOCOL).js \
	    components/helpers.js \
	    components/$(PROTOCOL)/$(PROTOCOL).js \
	    components/$(PROTOCOL)/worker.js \
	    components/$(PROTOCOL)/iframe.js 


ADIFRAMEJS = components/$(PROTOCOL)/iframe.js
TEST = components/$(PROTOCOL)/test.js

all: build/ad/ADIFRAME build/sendChunk.html

test: $(TESTLIBS) build/tests
	@cat $(TEST) $(TESTLIBS) > build/tests/$(PROTOCOL).test.js
	mocha -R spec build/tests/$(PROTOCOL).test.js

build/:
	@mkdir build
build/ad/:
	@mkdir build/ad/
build/tests/: build/
	mkdir build/tests

build/adworker.js: build/ $(ADWORKERJS)
	@echo "- collecting scripts to build/adworker.js"
	@cat $(ADWORKERJS) > build/adworker.js

build/ad/ADIFRAME: ad/ad_iframe.template.html build/adworker.js $(ADIFRAMEJS) build/ad/
	@echo "- building build/ad/$(ADIFRAME)"
	@awk '/__INSERT_WORKER_HERE__/{system("cat build/adworker.js");next}1' ad/ad_iframe.template.html > build/$(ADIFRAME).tmp
	@awk '/__INSERT_IFRAMEJS_HERE__/{system("cat $(ADIFRAMEJS)");next}1' build/$(ADIFRAME).tmp > build/ad/$(ADIFRAME)
	@echo done.

build/sendChunk.html: sendChunk.template.html build/adworker.js $(ADIFRAMEJS) build/
	@echo "- building build/sendChunk.html"
	@awk '/__INSERT_WORKER_HERE__/{system("cat build/adworker.js");next}1' sendChunk.template.html > build/sendChunk.html.tmp
	@awk '/__INSERT_IFRAMEJS_HERE__/{system("cat $(ADIFRAMEJS)");next}1' build/sendChunk.html.tmp > build/sendChunk.html
	@echo done.

clean: 
	rm -rf build


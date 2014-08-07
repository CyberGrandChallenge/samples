CB					= LUNGE_00002 YAN01_00001 YAN01_00002 CADET_00001 EAGLE_00004 KPRCA_00001 KPRCA_00003
TEMPLATES			= service-template
DIRS				= bin include lib poller/for-release poller/for-testing pov src
FILES				= Makefile README.md

EXAMPLE_DIR			= $(DESTDIR)/usr/share/cgc-sample-challenges/examples
TEMPLATE_DIR		= $(DESTDIR)/usr/share/cgc-sample-challenges/templates

CB_BASE_DIRS		= $(foreach cb, $(CB), $(foreach dir, $(DIRS), $(addsuffix /$(dir), $(cb))))
TEMPLATE_BASE_DIRS	= $(foreach cb, $(TEMPLATES), $(foreach dir, $(DIRS), $(addsuffix /$(dir), $(cb))))

CB_FILES			= $(foreach cb, $(CB), $(foreach filename, $(FILES), $(addprefix $(cb)/, $(filename))))
TEMPLATE_FILES		= $(foreach cb, $(TEMPLATES), $(foreach filename, $(FILES), $(addprefix $(cb)/, $(filename))))

all:
	@echo done

install_cb: $(CB_BASE_DIRS) $(CB_FILES)

install_template: $(TEMPLATE_BASE_DIRS) $(TEMPLATE_FILES)

install: install_cb install_template
	chmod +x $(EXAMPLE_DIR)/*/bin/*
	chmod +x $(TEMPLATE_DIR)/*/bin/*

$(CB_BASE_DIRS):
	if [ -d examples/$@ ]; then \
		install -d $(EXAMPLE_DIR)/$@ ; \
		install -m 444 examples/$@/* $(EXAMPLE_DIR)/$@ ; \
	fi

$(CB_FILES):
	if [ -f examples/$@ ]; then \
		if [ -x examples/$@ ]; then \
			install examples/$@ $(dir $(EXAMPLE_DIR)/$@) ; \
		else \
			install -m 444 examples/$@ $(dir $(EXAMPLE_DIR)/$@) ; \
		fi \
	fi

$(TEMPLATE_BASE_DIRS):
	if [ -d templates/$@ ]; then \
		install -d $(TEMPLATE_DIR)/$@ ; \
		install -m 444 templates/$@/* $(TEMPLATE_DIR)/$@ ; \
	fi

$(TEMPLATE_FILES):
	if [ -f templates/$@ ]; then \
		if [ -x templates/$@ ]; then \
			install templates/$@ $(dir $(TEMPLATE_DIR)/$@) ; \
		else \
			install -m 444 templates/$@ $(dir $(TEMPLATE_DIR)/$@) ; \
		fi \
	fi

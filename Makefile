CB					= LUNGE_00002 YAN01_00001 YAN01_00002 CADET_00001 EAGLE_00004 KPRCA_00001 KPRCA_00003 TNETS_00002 LUNGE_00005 KPRCA_00015 NRFIN_00003 NRFIN_00010 NRFIN_00013 YAN01_00003
TEMPLATES			= service-template

FILES				= Makefile README.md

DIRS_REQUIRED		= bin pov
DIRS_OPTIONAL		= poller/for-release poller/for-testing
DIRS_CB_REQUIRED	= src
DIRS_CB_OPTIONAL	= include lib support

INSTALL_DIR 		= $(DESTDIR)/usr/share/cgc-sample-challenges
EXAMPLE_DIR			= $(DESTDIR)/usr/share/cgc-sample-challenges/examples
TEMPLATE_DIR		= $(INSTALL_DIR)/templates

get_sub_cbs			= $(notdir $(wildcard $(1)/$(2)/cb_*))
has_sub_cb 			= $(sort $(notdir $(patsubst %/,%,$(dir $(wildcard $(1)/$(2)/cb_*)))))

# ARG1 = LIST OF CBs
# ARG2 = base directory containing CBs
get_cb_dirs = $(sort $(foreach cb, $(1), \
	$(foreach cb_dir, $(DIRS_REQUIRED), $(cb)/$(cb_dir)) \
	$(foreach cb_dir, $(DIRS_OPTIONAL), $(patsubst $(2)/%, %, $(wildcard $(2)/$(cb)/$(cb_dir)))) \
	$(if $(call has_sub_cb,$2,$(cb)), \
		$(foreach sub_cb, $(call get_sub_cbs,$(2),$(cb)), \
			$(foreach cb_dir, $(DIRS_CB_REQUIRED), $(cb)/$(sub_cb)/$(cb_dir)) \
			$(foreach cb_dir, $(DIRS_CB_OPTIONAL), $(patsubst $(2)/%, %, $(wildcard $(2)/$(cb)/$(sub_cb)/$(cb_dir)))) \
		), \
		$(foreach cb_dir, $(DIRS_CB_REQUIRED), $(cb)/$(cb_dir)) \
		$(foreach cb_dir, $(DIRS_CB_OPTIONAL), $(patsubst $(2)/%, %, $(wildcard $(2)/$(cb)/$(cb_dir)))) \
	) \
))

get_cb_files = $(sort $(foreach cb, $(1), \
	$(foreach filename, $(FILES), $(cb)/$(filename)) \
))

CB_DIRS  = $(call get_cb_dirs,$(CB),examples)
CB_FILES = $(call get_cb_files,$(CB))

TEMPLATE_DIRS  = $(call get_cb_dirs,$(TEMPLATES),templates)
TEMPLATE_FILES = $(call get_cb_files,$(TEMPLATES))

all: 
	@echo done

install: install_cb install_template

install_cb: $(CB_DIRS) $(CB_FILES)
	chmod +x $(EXAMPLE_DIR)/*/bin/*

install_template: $(TEMPLATE_DIRS) $(TEMPLATE_FILES)
	chmod +x $(TEMPLATE_DIR)/*/bin/*

$(CB_DIRS):
	install -d $(EXAMPLE_DIR)/$@
	install -m 444 examples/$@/* $(EXAMPLE_DIR)/$@

$(CB_FILES):
	install -m 444 examples/$@ $(dir $(EXAMPLE_DIR)/$@)

$(TEMPLATE_DIRS):
	install -d $(TEMPLATE_DIR)/$@
	install -m 444 templates/$@/* $(TEMPLATE_DIR)/$@

$(TEMPLATE_FILES):
	install -m 444 templates/$@ $(dir $(TEMPLATE_DIR)/$@)

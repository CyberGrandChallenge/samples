CB					= $(shell ls examples)
CQE					= $(shell ls cqe-challenges)
TEMPLATES			= service-template LUNGE_00003 LUNGE_00004
FILES				= Makefile README.md

DIRS_REQUIRED		= 
DIRS_OPTIONAL		= poller/for-release poller/for-testing pov ids
DIRS_CB_REQUIRED	= src
DIRS_CB_OPTIONAL	= include lib 

INSTALL_DIR 		= $(DESTDIR)/usr/share/cgc-sample-challenges
EXAMPLE_DIR			= $(DESTDIR)/usr/share/cgc-sample-challenges/examples
CQE_DIR				= $(DESTDIR)/usr/share/cgc-sample-challenges/cqe-challenges
TEMPLATE_DIR		= $(INSTALL_DIR)/templates

get_pov_cbs 		= $(notdir $(wildcard $(1)/$(2)/pov_*))
has_pov_cb 			= $(sort $(notdir $(patsubst %/,%,$(dir $(wildcard $(1)/$(2)/pov_*)))))

get_sub_cbs			= $(notdir $(wildcard $(1)/$(2)/cb_*))
has_sub_cb 			= $(sort $(notdir $(patsubst %/,%,$(dir $(wildcard $(1)/$(2)/cb_*)))))

# ARG1 = LIST OF CBs
# ARG2 = base directory containing CBs
get_cb_dirs = $(sort $(foreach cb, $(1), \
	$(foreach cb_dir, $(DIRS_REQUIRED), $(cb)/$(cb_dir)) \
	$(foreach cb_dir, $(DIRS_OPTIONAL), $(patsubst $(2)/%, %, $(wildcard $(2)/$(cb)/$(cb_dir)))) \
	$(if $(call has_pov_cb,$2,$(cb)), \
		$(foreach pov_cb, $(call get_pov_cbs,$(2),$(cb)), \
			$(patsubst $(2)/%, %, $(2)/$(cb)/$(pov_cb)) \
		) \
	) \
	$(if $(call has_sub_cb,$2,$(cb)), \
		$(foreach sub_cb, $(call get_sub_cbs,$(2),$(cb)), \
			$(foreach cb_dir, $(DIRS_CB_REQUIRED), $(cb)/$(sub_cb)/$(cb_dir)) \
			$(foreach cb_dir, $(DIRS_CB_OPTIONAL), $(patsubst $(2)/%, %, $(wildcard $(2)/$(cb)/$(sub_cb)/$(cb_dir)))) \
		), \
		$(foreach cb_dir, $(DIRS_CB_REQUIRED), $(cb)/$(cb_dir)) \
		$(foreach cb_dir, $(DIRS_CB_OPTIONAL), $(patsubst $(2)/%, %, $(wildcard $(2)/$(cb)/$(cb_dir)))) \
	) \
))

get_support_files = $(sort $(foreach cb, $1, $(if $(wildcard $(2)/$(cb)/support/*), $(shell find $(2)/$(cb)/support -type f -not -empty -not -path '*/\.*'))))

get_cb_files = $(sort $(foreach cb, $(1), \
	$(foreach filename, $(FILES), $(cb)/$(filename)) \
))

CB_DIRS  = $(call get_cb_dirs,$(CB),examples)
CB_FILES = $(call get_cb_files,$(CB))
CB_SUPPORT = $(call get_support_files,$(CB),examples)

TEMPLATE_DIRS  = $(call get_cb_dirs,$(TEMPLATES),templates)
TEMPLATE_FILES = $(call get_cb_files,$(TEMPLATES))
TEMPLATE_SUPPORT = $(call get_support_files,$(TEMPLATES),template)

CQE_DIRS  = $(call get_cb_dirs,$(CQE),cqe-challenges)
CQE_FILES = $(call get_cb_files,$(CQE))
CQE_SUPPORT = $(call get_support_files,$(CQE),cqe-challenges)

all: 
	@echo done
#	@echo $(CQE_SUPPORT)

install: install_cb install_template install_cqe

install_cb: $(CB_DIRS) $(CB_FILES) $(CB_SUPPORT)

install_cqe: $(CQE_DIRS) $(CQE_FILES) $(CQE_SUPPORT)

install_template: $(TEMPLATE_DIRS) $(TEMPLATE_FILES) $(TEMPLATE_SUPPORT)

$(CB_DIRS):
	install -d $(EXAMPLE_DIR)/$@
	install -m 444 examples/$@/* $(EXAMPLE_DIR)/$@

$(CB_FILES):
	install -m 444 examples/$@ $(dir $(EXAMPLE_DIR)/$@)

$(CB_SUPPORT):
	install -m 444 -D $@ $(INSTALL_DIR)/$@

$(CQE_DIRS):
	install -d $(CQE_DIR)/$@
	install -m 444 cqe-challenges/$@/* $(CQE_DIR)/$@

$(CQE_FILES):
	install -m 444 cqe-challenges/$@ $(dir $(CQE_DIR)/$@)

$(CQE_SUPPORT):
	install -m 444 -D $@ $(INSTALL_DIR)/$@

$(TEMPLATE_DIRS):
	install -d $(TEMPLATE_DIR)/$@
	install -m 444 templates/$@/* $(TEMPLATE_DIR)/$@

$(TEMPLATE_FILES):
	install -m 444 templates/$@ $(dir $(TEMPLATE_DIR)/$@)

$(TEMPLATE_SUPPORT):
	install -m 444 -D $@ $(INSTALL_DIR)/$@

.PHONY: $(CQE_SUPPORT) $(CB_SUPPORT) $(TEMPLATE_SUPPORT)

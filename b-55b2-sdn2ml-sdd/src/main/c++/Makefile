#+======================================================================
# Description   : Build C++ drivers and programs, including vnc-mp1
#-======================================================================

SUBDIRS=$(dir $(wildcard */Makefile))

BOLD=\e[1m
NC=\e[0m

.PHONY: all
all:
	@$(foreach dir, $(SUBDIRS), echo -e "$(BOLD)Building $(dir:/=)...$(NC)" && $(MAKE) -C $(dir) $@ &&) :

%:
	@$(foreach dir, $(SUBDIRS), echo -e "$(BOLD)Building $(dir:/=)...$(NC)" && $(MAKE) -C $(dir) $@ &&) :

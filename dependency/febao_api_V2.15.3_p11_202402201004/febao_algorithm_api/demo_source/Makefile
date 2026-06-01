MAKE := make
ROOT_PATH := $(shell pwd)

ALGORITHMS_DIR := baseprice_algorithm theoretical_algorithm volatility_algorithm
DST_ALGORITHM_PLUGIN_DIR := algorithm_plugin

.PHONY: debug clean

release:
	$(foreach algorithm_dir,$(ALGORITHMS_DIR),$(MAKE) builder -C $(ROOT_PATH)/$(algorithm_dir);)
	rm -rf $(DST_ALGORITHM_PLUGIN_DIR)
	mkdir -p $(DST_ALGORITHM_PLUGIN_DIR)
	$(foreach algorithm_dir,$(ALGORITHMS_DIR),mv $(algorithm_dir)/$(algorithm_dir) $(DST_ALGORITHM_PLUGIN_DIR);)
	touch fb_plugin

debug: release

clean:
	$(foreach algorithm_dir,$(ALGORITHMS_DIR),$(MAKE) clean -C $(ROOT_PATH)/$(algorithm_dir);)
	rm -rf $(DST_ALGORITHM_PLUGIN_DIR)
	rm fb_plugin

obj-$(CPTCFG_IWLMVM)   += iwlmvm.o
iwlmvm-y += fw.o mac80211.o nvm.o ops.o phy-ctxt.o mac-ctxt.o
iwlmvm-y += utils.o rx.o tx.o binding.o quota.o sta.o sf.o
iwlmvm-y += scan.o time-event.o rs.o
iwlmvm-y += power.o power_legacy.o bt-coex.o
iwlmvm-y += led.o tt.o
iwlmvm-$(CPTCFG_IWLWIFI_DEBUGFS) += debugfs.o debugfs-vif.o
iwlmvm-$(CPTCFG_IWLWIFI_DEVICE_TESTMODE) += testmode.o
iwlmvm-$(CONFIG_PM_SLEEP) += d3.o

ccflags-y += -D__CHECK_ENDIAN__ -I$(src)/../

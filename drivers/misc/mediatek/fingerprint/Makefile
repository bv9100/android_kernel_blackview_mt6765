ccflags-y += -I$(srctree)/drivers/spi/mediatek/mt6765
ccflags-y += -I$(srctree)/drivers/pinctrl/

obj-y	+= fingerprint.o

obj-$(CONFIG_FINGERPRINT_MICROARRAY_A80T)	+=  microarray_v4.0.01/
obj-$(CONFIG_FINGERPRINT_MICROARRAY)	+=  microarray_v4.0.01/
obj-$(CONFIG_FINGERPRINT_MICROARRAY_FOR_MT6739)	+=  microarray_v4.0.01/
obj-$(CONFIG_FINGERPRINT_MICROARRAY_TEE)	+=  microarray_v4.0.01/
obj-$(CONFIG_FINGERPRINT_MICROARRAY_REE)	+=  microarray_v4.0.01/
obj-$(CONFIG_FINGERPRINT_MICROARRAY_A80T_TEE)	+=  microarray_v4.0.01/
obj-$(CONFIG_FINGERPRINT_SUNWAVECORP_NEW_REE)   +=  sunwave_driver_all_in_one_ree/
obj-$(CONFIG_FINGERPRINT_SUNWAVECORP_NEW_TEE)   +=  sunwave_driver_all_in_one_tee/
obj-$(CONFIG_FINGERPRINT_SUNWAVECORP)   +=  sunwavecorp/
obj-$(CONFIG_BETTERLIFE_FINGERPRINT_REE)   +=  betterlife_fp_ree/
obj-$(CONFIG_FINGERPRINT_CDFINGER_FPS988E)   += cdfinger_fp_ree/
obj-$(CONFIG_BETTERLIFE_FINGERPRINT_TEE)   +=  betterlife_fp_tee/
obj-$(CONFIG_FINGERPRINT_CDFINGER_TEE)	+=  cdfinger_fp_tee/


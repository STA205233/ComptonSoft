#!/usr/bin/env ruby

require 'comptonsoft'

class MyAppDataReduction < ANL::ANLApp
  attr_accessor :tpc_tree_file, :hittree_file
  attr_accessor :quicklook_file
  attr_accessor :gain_tp_file, :gain_tp_hash
  attr_accessor :gain_cache_seconds
  attr_accessor :light_waveform_display_file

  def setup
    add_namespace ComptonSoft

    chain :CSHitCollection
    chain :NanoGRAMSLightWaveformStore
    chain :NanoGRAMSHitExtraction
    extraction_parameters = {
      config_file:     "metadata/config_pipeline.yaml",
      tpctree_file:    @tpc_tree_file,
    }
    extraction_parameters[:quicklook_file] = @quicklook_file if @quicklook_file
    with_parameters(**extraction_parameters)
    
  
    chain :NanoGRAMSMakeLightWaveform
    with_parameters(hit_extraction_module_name:       "NanoGRAMSHitExtraction",
                    light_waveform_store_module_name: "NanoGRAMSLightWaveformStore",
                    range_min_us:                      0.0,
                    range_max_us:                      0.0)

    #chain :NanoGRAMSCorrectPedestal
    #with_parameters(config_file:                      "metadata/config_pipeline.yaml",
    #                 light_waveform_store_module_name: "NanoGRAMSLightWaveformStore",
    #                 pedestal_range_min:               -100.0,
    #                 pedestal_range_max:               0.0)

    #chain :NanoGRAMSCorrectDigitizerOffset
    #with_parameters(config_file:                      "metadata/config_pipeline.yaml",
    #                 light_waveform_store_module_name: "NanoGRAMSLightWaveformStore",
    #                 range_start_index:                0,
    #                 range_stop_index:                 100)

    #chain :NanoGRAMSApplyLightFFTFilter
    #with_parameters(config_file:                      "metadata/config_pipeline.yaml",
    #                 light_waveform_store_module_name: "NanoGRAMSLightWaveformStore",
    #                 low_frequency:                    0.0,
    #                 high_frequency:                   0.1)

    if @light_waveform_display_file
      chain :NanoGRAMSWriteLightWaveform
      with_parameters(config_file:                      "metadata/config_pipeline.yaml",
                       light_waveform_store_module_name: "NanoGRAMSLightWaveformStore",
                       period:                           100,
                       max_saved_events:                 10000)
      chain :SaveData
      with_parameters(output: @light_waveform_display_file)
    end

    chain :NanoGRAMSCalibration
    calibration_parameters = {}
    calibration_parameters[:gain_tp_hash] = @gain_tp_hash if @gain_tp_hash
    calibration_parameters[:gain_tp_file] = @gain_tp_file if @gain_tp_file
    calibration_parameters[:gain_cache_seconds] = @gain_cache_seconds if @gain_cache_seconds
    with_parameters(**calibration_parameters)

    chain :WriteHitTree
    chain :SaveData
    with_parameters(output: @hittree_file)
  end
end


### main ###
data_directory = "data/2025_10_01_02_33_05"

a = MyAppDataReduction.new
a.tpc_tree_file = "#{data_directory}/tpctree.root"
a.hittree_file  = "#{data_directory}/hittree.root"
# a.quicklook_file = "#{data_directory}/quicklook.root"
# a.light_waveform_display_file = "#{data_directory}/light_waveform.root"

a.run(:all, 1000)

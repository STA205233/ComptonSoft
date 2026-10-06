#!/usr/bin/env ruby

require 'comptonsoft'

class NanoGRAMSDataReduction < ANL::ANLApp
  attr_accessor :config_file, :dpp_config_file, :tpc_tree_file
  attr_accessor :gain_tp_file, :run_id
  attr_accessor :hittree_file, :quicklook_file

  def setup
    add_namespace ComptonSoft
    puts @config_file
    puts @dpp_config_file
    puts @tpc_tree_file
    puts @gain_tp_file
    puts  @run_id
    puts @hittree_file
    puts @quicklook_file

    chain :CSHitCollection
    chain :ConstructDetector
    with_parameters(detector_configuration: "database/detector_configuration.xml",
                    detector_parameters: "database/detector_parameters.xml")
                    
    # yaml configuration, calibration and detector parameters (must be chained before the modules using them)
    chain :NanoGRAMSLoadConfig
    with_parameters(config_file: @config_file,
                    dpp_config_file: @dpp_config_file,
                    gain_tp_file: @gain_tp_file)


    # read tpctree: raw ADC, TI, drift time, unixtime -> MCD/unit; light waveforms -> LightData
    # (events with TPC error flags are skipped here)
    chain :NanoGRAMSReadTPCEvents
    with_parameters(
                    tpctree_files: @tpc_tree_file,
                    run_id: @run_id)

    # light: waveform corrections (light.pedestal_correction etc. in the yaml),
    # then judgement (LightGamma/Cosmic/Pileup) and the integrated charge (photon count)
    chain :NanoGRAMSCorrectLightWaveform
    chain :NanoGRAMSAnalyzeLight

    # charge: CMN (median; lower mean for selection) -> temperature correction -> EPI (charge x W_ion)
    chain :CorrectPHA
    with_parameters(pedestal_level: "0",
                    CMN_estimation: 2,
                    gain_function: "1")

    # hits, clustering, flags, recombination correction (the thresholds are given by the yaml)
    chain :SelectHits
    with_parameters(analysis_map: {
                     "NanoGRAMS" => [7, 4, 0.0, 0.0, 0.0]
                    })

    # quicklook of all the events (before the selection)
    if @quicklook_file
      chain :NanoGRAMSQuickLookWriter
      with_parameters(quicklook_file: @quicklook_file,
                      event_types: ["gamma", "other", "cosmic", "pileup", "timeup",
                                      "excluded", "nocluster", "lightnotgamma",
                                      "rej_pixelcount", "rej_collinear", "rej_multicluster", "rej_timeup"],
                      num_hits: -1,
                      save_waveforms: false)
    end

    # selection of gamma-ray events (the other events are skipped)
    chain :NanoGRAMSSelectEvents
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

#!/usr/bin/env ruby
#
# NanoGRAMS data reduction on the detector unit (RealDetectorUnitNanoGRAMS).
#
# Usage:
#   ruby run_data_reduction.rb <time_id>
#   (input: data/<time_id>/tpc_data*.root and data/<time_id>/config_dpp.yaml,
#    output: products/eventfile/<time_id>/hittree.root and quicklook.root)
#
# The analysis parameters are given by the pipeline yaml (metadata/config_pipeline.yaml):
#   general, light (light analysis/corrections), charge (hit threshold, clustering, selection),
#   calibration (gain info file, test pulse, electric field, maximum drift time).
#

require 'comptonsoft'
require 'fileutils'

class NanoGRAMSDataReduction < ANL::ANLApp
  attr_accessor :config_file, :dpp_config_file, :tpctree_files
  attr_accessor :gain_tp_file, :run_id
  attr_accessor :hittree_file, :quicklook_file

  def setup
    add_namespace ComptonSoft

    chain :CSHitCollection
    chain :ConstructDetector
    with_parameters(detector_configuration: "database/detector_configuration_nanograms.xml",
                    detector_parameters: "database/detector_parameters_nanograms.xml")

    # read tpctree: raw ADC, TI, drift time, unixtime -> MCD/unit; light waveforms -> LightData
    # (events with TPC error flags are skipped here)
    chain :NanoGRAMSReadTPCEvents
    with_parameters(config_file: @config_file,
                    dpp_config_file: @dpp_config_file,
                    tpctree_files: @tpctree_files,
                    gain_tp_file: @gain_tp_file,
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
                      "NanoGRAMS" => {
                        detector_type: 7,
                        reconstruction_mode: 4,
                        threshold: 0.0,
                        threshold_cathode: 0.0,
                        threshold_anode: 0.0,
                      }
                    })

    # quicklook of all the events (before the selection)
    if @quicklook_file
      chain :NanoGRAMSQuickLookWriter
      with_parameters(quicklook_file: @quicklook_file,
                      event_types: ["gamma", "other", "cosmic", "pileup", "timeup"],
                      num_hits: -1,
                      save_waveforms: false,
                      output_flush_entries: 1000)
    end

    # selection of gamma-ray events (the other events are skipped)
    chain :NanoGRAMSSelectEvents
    chain :WriteHitTree

    chain :SaveData
    with_parameters(output: @hittree_file)
  end
end

### Main

time_id = ARGV[0] || "20260625_2000_20"
data_dir = File.join("data", time_id)
out_dir = File.join("products", "eventfile", time_id)
FileUtils.mkdir_p(out_dir)

run_id_epoch = Time.utc(2025, 1, 1)
m = time_id.match(/\A(\d{4})(\d{2})(\d{2})_(\d{2})(\d{2})_(\d{2})\z/)
raise "Invalid time_id: #{time_id}" unless m
run_id = (Time.utc(*m.captures.map(&:to_i)) - run_id_epoch).to_i

app = NanoGRAMSDataReduction.new
app.config_file = "metadata/config_pipeline.yaml"
app.dpp_config_file = File.join(data_dir, "config_dpp.yaml")
app.tpctree_files = Dir.glob(File.join(data_dir, "tpc_data*.root")).sort
app.gain_tp_file = "products/testpulse_data.csv"
app.run_id = run_id
app.hittree_file = File.join(out_dir, "hittree.root")
app.quicklook_file = File.join(out_dir, "quicklook.root")
app.run(:all, 10000)

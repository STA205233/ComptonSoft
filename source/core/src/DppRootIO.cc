#include "DppRootIO.hh"
#include "TROOT.h"
#include <iostream>

namespace ngUtil {
DppRootIO::DppRootIO(bool mt, int version) {
  if (mt) {
    ROOT::EnableThreadSafety();
  }
  if (version < 1 || version > 2) {
    std::cerr << "Unknown version(" << version << ")" << std::endl;
    return;
  }
  version_ = version;
  InitProperty();
}
DppRootIO::~DppRootIO() = default;
void DppRootIO::InitProperty() {
  triggerid_ = 0;
  eventid_ = 0;
  realTime_ = 0;
  realTimePrecise_ = 0;
  channel_ = 0;
  qdc_ = 0;
  waveNum_ = 0;
  for (int j = 0; j < MAX_WAVENUM; j++) {
    waveData_[j] = 0;
  }
  isWaveList_ = 1;
  if (version_ > 1) {
    trigger_ = 0;
    waveCompress_ = 0;
  }
  else {
    header_ = 0;
  }
}

bool DppRootIO::LoadTree(TDirectory *dir) {
  tree_ = static_cast<TTree *>(dir->Get("dpplist"));
  if (!tree_) {
    std::cerr << "Tree dpplist can't be found." << std::endl;
    return false;
  }
  readOnly_ = true;
  return true;
}
bool DppRootIO::LoadTree(TTree *tree) {
  tree_ = tree;
  readOnly_ = true;
  return true;
}
void DppRootIO::FetchEvent(DppListDataDefinition *data) {
  if (!tree_) {
    std::cerr << "No tree is loaded" << std::endl;
    return;
  }
  if (!data) {
    std::cerr << "Data pointer is null" << std::endl;
    return;
  }
  data->SetChannel(channel_);
  data->SetEventID(eventid_);
  data->SetQDC(qdc_);
  data->SetRealTime(realTime_);
  data->SetRealTimePrecise(realTimePrecise_);
  data->SetWaveNum(waveNum_);
  data->SetWaveList(true ? isWaveList_ == 1 : false);
  if (version_ == 1) {
    data->SetHeader(header_);
  }
  data->SetWaveData(waveData_);
  if (version_ > 1) {
    data->SetTriggerID(triggerid_);
    data->SetTrigger(true ? trigger_ == 1 : false);
    data->SetWaveCompress(waveCompress_);
  }
  data->SetValid(true);
}
bool DppRootIO::RegisterTDirectoryToWrite(TDirectory *dir) {
  if (readOnly_) {
    std::cerr << "Read only mode: can't do RegisterTDirectoryToWrite()" << std::endl;
    return false;
  }
  dir_ = dir;
  dir_->cd();
  tree_ = new TTree("dpplist", "dpplist");
  if (!tree_) {
    std::cerr << "Tree DppListData can't be created." << std::endl;
    return false;
  }
  readOnly_ = false;
  return true;
}
void DppRootIO::FillTree() {
  if (!tree_) {
    std::cerr << "No tree is loaded" << std::endl;
    return;
  }
  tree_->Fill();
}
void DppRootIO::WriteTree() {
  if (!tree_) {
    std::cerr << "No tree is loaded" << std::endl;
    return;
  }
  if (readOnly_) {
    std::cerr << "Read only mode: can't do WriteTree()" << std::endl;
    return;
  }
  dir_->cd();
  tree_->Write();
}
void DppRootIO::SetBranchAddress() {
  if (!tree_) {
    std::cerr << "No tree is loaded" << std::endl;
    return;
  }
  tree_->SetBranchAddress("event_id", &eventid_);
  tree_->SetBranchAddress("real_time", &realTime_);
  tree_->SetBranchAddress("real_time_frac", &realTimePrecise_);
  tree_->SetBranchAddress("ch", &channel_);
  tree_->SetBranchAddress("qdc", &qdc_);
  tree_->SetBranchAddress("wave_num", &waveNum_);
  if (version_ == 1) {
    tree_->SetBranchAddress("header", &header_);
  }
  tree_->SetBranchAddress("waveform", waveData_.data());
  tree_->SetBranchAddress("is_wavelist", &isWaveList_);
  if (version_ > 1) {
    tree_->SetBranchAddress("trigger_id", &triggerid_);
    tree_->SetBranchAddress("trigger", &trigger_);
    tree_->SetBranchAddress("wave_compress", &waveCompress_);
  }
}
void DppRootIO::RegisterBranch() {
  if (!tree_) {
    std::cerr << "No tree is loaded" << std::endl;
    return;
  }
  if (readOnly_) {
    std::cerr << "Read Mode: can't do RegisterBranch()" << std::endl;
    return;
  }
  tree_->Branch("event_id", &eventid_, "event_id/i");
  tree_->Branch("real_time", &realTime_, "real_time/l");
  tree_->Branch("real_time_frac", &realTimePrecise_, "real_time_frac/b");
  tree_->Branch("ch", &channel_, "ch/s");
  tree_->Branch("qdc", &qdc_, "qdc/s");
  tree_->Branch("wave_num", &waveNum_, "wave_num/s");
  if (version_ > 1) {
    tree_->Branch("trigger_id", &triggerid_, "trigger_id/i");
    tree_->Branch("trigger", &trigger_, "trigger/b");
    tree_->Branch("wave_compress", &waveCompress_, "wave_compress/s");
  }
  else {
    tree_->Branch("header", &header_, "header/I");
  }
  tree_->Branch("waveform", waveData_.data(), "waveform[wave_num]/S");
  tree_->Branch("is_wavelist", &isWaveList_, "is_wavelist/b");
}
void DppRootIO::SetEvent(const DppListDataDefinition &event) {
  if (version_ != event.GetVersion()) {
    std::cerr << "Version mismatch: " << version_ << " != " << event.GetVersion() << std::endl;
    return;
  }
  InitProperty();
  eventid_ = event.GetEventID();
  realTime_ = event.GetRealTime();
  realTimePrecise_ = event.GetRealTimePrecise();
  channel_ = event.GetChannel();
  qdc_ = event.GetQDC();
  waveNum_ = event.GetWaveNum();
  if (version_ == 1) {
    header_ = event.GetHeader();
  }
  const auto &waveData = event.GetWaveData();
  for (int i = 0; i < waveNum_; i++) {
    waveData_[i] = waveData[i];
  }
  isWaveList_ = event.IsWaveList() ? 1 : 0;
  if (version_ > 1) {
    triggerid_ = event.GetTriggerID();
    trigger_ = event.GetTrigger() ? 1 : 0;
    waveCompress_ = event.GetWaveCompress();
  }
}
} // namespace ngUtil

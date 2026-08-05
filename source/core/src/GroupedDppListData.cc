#include "GroupedDppListData.hh"
#include <iostream>

namespace ngUtil {

GroupedDppListData::GroupedDppListData() {
  for (auto &wave_data: waveData_) {
    wave_data.reserve(MAX_WAVENUM);
  }
  Clear();
}

void GroupedDppListData::Clear() {
  for (int i = 0; i < NUM_CH; i++) {
    eventid_[i] = 0;
    realTime_[i] = 0;
    realTimePrecise_[i] = 0;
    ch_[i] = 0;
    qdc_[i] = 0;
    waveNum_[i] = 0;
    header_[i] = 0;
    waveData_[i].clear(); // the capacity is kept
    isWaveList_[i] = true;
    trigger_[i] = false;
    waveCompress_[i] = 0;
    registered_[i] = false;
  }
  numRegistered_ = 0;
  triggerid_ = 0;
  isGrouped_ = false;
  isValid_ = true;
  isFatal_ = false;
}

bool GroupedDppListData::AddEvent(const DppListDataDefinition &event) {
  const int ch = event.GetChannel();
  if (ch < 0 || ch >= NUM_CH) {
    std::cerr << "ERROR: Channel " << ch << " is out of range." << std::endl;
    isFatal_ = true;
    isValid_ = false;
    return false;
  }
  else if (numRegistered_ != 0 && triggerid_ != event.GetTriggerID()) {
    isValid_ = true;
    return false;
  }
  else if (registered_[ch]) {
    std::cerr << "ERROR: Channel " << ch << " is already registered." << std::endl;
    isFatal_ = true;
    isValid_ = false;
    return false;
  }
  if (version_ != event.GetVersion()) {
    std::cerr << "ERROR: Version mismatch" << std::endl;
    isFatal_ = true;
    isValid_ = false;
    return false;
  }
  if (event.IsFatal()) {
    std::cerr << "Event has already fatal error..." << std::endl;
    isFatal_ = true;
    isValid_ = false;
    return false;
  }
  if (!event.IsValid()) {
    isValid_ = false;
    return false;
  }
  if (numRegistered_ == 0) {
    triggerid_ = event.GetTriggerID();
  }
  if (verbose_ > 1) {
    std::cout << "INFO: Adding event (Trigger ID: " << triggerid_
              << ") to channel " << ch << std::endl;
  }
  isWaveList_[ch] = event.IsWaveList();
  eventid_[ch] = event.GetEventID();
  realTime_[ch] = event.GetRealTime();
  realTimePrecise_[ch] = event.GetRealTimePrecise();
  ch_[ch] = event.GetChannel();
  qdc_[ch] = event.GetQDC();
  waveNum_[ch] = event.GetWaveNum();
  header_[ch] = event.GetHeader();
  const std::vector<int16_t> &wave_data = event.GetWaveData();
  waveData_[ch].resize(waveNum_[ch]);
  for (int i = 0; i < waveNum_[ch]; i++) {
    waveData_[ch][i] = wave_data[i];
  }
  registered_[ch] = true;
  waveCompress_[ch] = event.GetWaveCompress();
  trigger_[ch] = event.GetTrigger();
  numRegistered_++;
  return true;
}

} // namespace ngUtil

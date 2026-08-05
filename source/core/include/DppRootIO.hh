#ifndef COMPTONSOFT_DppRootIO_hh
#define COMPTONSOFT_DppRootIO_hh 1

#include "DppListDataDefinition.hh"
#include "DppParameters.hh"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1D.h"
#include "TTree.h"
#include <array>
#include <cstdint>
#include <string>

namespace ngUtil {
/**s
 * @brief Class for ROOT I/O of Dpp List Data
 * @author Shota Arai
 * @date 2024/11/XX First version
 * @date 2025/01/12 Change the type of values.
 */
class DppRootIO {
public:
  DppRootIO(bool mt, int version = 2);
  virtual ~DppRootIO();

protected:
  DppRootIO(const DppRootIO &r) = delete;

private:
  TTree *tree_ = nullptr;
  TDirectory *dir_ = nullptr;

  // TreeDefinition
  uint32_t eventid_ = 0;
  uint64_t realTime_ = 0;
  uint8_t realTimePrecise_ = 0;
  uint16_t channel_ = 0;
  uint16_t qdc_ = 0;
  uint16_t waveNum_ = 0;
  int header_ = 0;
  std::array<int16_t, MAX_WAVENUM> waveData_;
  uint8_t isWaveList_ = false;
  bool readOnly_ = false;
  int verbose_ = 0;
  int version_ = 2;
  uint8_t trigger_ = 0;
  uint32_t triggerid_ = 0;
  uint16_t waveCompress_ = 0;
  void InitProperty();

public:
  bool LoadTree(TDirectory *dir);
  bool LoadTree(TTree *tree);
  void FetchEvent(DppListDataDefinition *data);
  bool RegisterTDirectoryToWrite(TDirectory *dir);
  void FillTree();
  void WriteTree();
  void SetBranchAddress();
  void SetVerbose(int verbose) { verbose_ = verbose; }
  void RegisterBranch();
  auto GetEntries() { return tree_->GetEntries(); }
  auto GetEntry(Long64_t entry) { return tree_->GetEntry(entry); }
  void SetEvent(const DppListDataDefinition &event);

  // Getters
  uint64_t GetEventID() const { return eventid_; }
  uint64_t GetRealTime() const { return realTime_; }
  uint8_t GetRealTimePrecise() const { return realTimePrecise_; }
  uint16_t GetChannel() const { return channel_; }
  uint16_t GetQDC() const { return qdc_; }
  uint16_t GetWaveNum() const { return waveNum_; }
  int GetHeader() const { return header_; }
  const std::array<int16_t, MAX_WAVENUM> &GetWaveData() const { return waveData_; }
  bool IsWaveList() const { return isWaveList_; }
  int16_t GetWaveData(int index) const { return waveData_[index]; }
  uint32_t GetTriggerID() const { return triggerid_; }
  uint16_t GetWaveCompress() const { return waveCompress_; }
  uint8_t GetTrigger() const { return trigger_; }
  int GetVerbose() const { return verbose_; }
  int GetVersion() const { return version_; }
};

} // namespace ngUtil

#endif //COMPTONSOFT_DppRootIO_hh

namespace comptonsoft {
class ConstructDetector;
class ConstructDetectorForSimulation;
class VCSModule;
class CSHitCollection;
class ConstructChannelMap;
class SetNoiseLevels;
class SetBadChannels;
class SetChannelProperties;
class SelectHits;
class AnalyzeHits;
class MakeDetectorHits;
class MakeRawHits;
class MakeDetectorHitsWithTimingProcess;
class ApplyEPICompensation;
class EventSelection;
class EventReconstruction;
class HXIEventSelection;
class CreateRootFile;
class SaveData;
class ReadDataFile;
class ReadDataFile_VME3;
class ReadDataFile_SpW2;
class ReadDataFile_NT;
class ReadDataFile_NB0;
class CorrectPHA;
class RecalculateEPI;
class CalculatePedestalLevels;
class InitialConditionFilter;
class WeightByInitialDirection;
class RecalculateSimulationNoise;
class ComptonModeFilter;
class SelectFullDeposit;
class ComptonEventFilter;
class SelectEventsOnFocalPlane;
class SelectTime;
class FilterByGoodTimeIntervals;
class WriteHitTree;
class ReadHitTree;
class ReadHitTreeAsRawHits;
class ReadHitTreeAsDetectorHits;
class WriteEventTree;
class ReadEventTree;
class ReadEventTreeAsRawHits;
class ReadEventTreeAsDetectorHits;
class WriteComptonEventTree;
class ReadComptonEventTree;
class HistogramPHA;
class HistogramEnergySpectrum;
class HistogramEnergy1D;
class HistogramEnergy2D;
class HistogramARM;
class HistogramARMByPositionMeasurement;
class HistogramAzimuthAngle;
class Histogram2DDeltaEnergyWithARM;
class ResponseMatrix;
class BackProjection;
class BackProjectionSky;
class EfficiencyMapSky;
class QuickAnalysisForDSD;
class AssignTime;
class DefineFrame;
class MakeFrameFITS;
class ConstructFrame;
class ConstructSXIFrame;
class FillFrame;
class LoadFrame;
class LoadReducedFrame;
class XrayEventCollection;
class XrayEventSelection;
class AnalyzeFrame;
class WriteXrayEventTree;
class ReadXrayEventTree;
class SortEventTreeWithTime;
class SetPedestals;
class WritePedestals;
class WriteBadPixels;
class AnalyzeDarkFrame;
class LoadRootFrame;
class DetectBadFrames;
class SetBadFrames;
class SetDynamicPedestals;
class SetPedestalsByMedian;
class HistogramFramePedestal;
class HistogramXrayEventSpectrum;
class HistogramXrayEventAzimuthAngle;
class HistogramXrayEventProperties;
class HistogramRawFrameImage;
class ExtractXrayEventImage;
class ProcessCodedAperture;
class PushToQuickLookDB;
class GetInputFilesFromDirectory;
class SelectEventsWithDetectorSpectrum;
class AssignSXIGrade;
class AEAttitudeCorrection;
class SelectEventsWithCelestialSpectrum;
class AEAssignWeightWithResponseMatrix;
class ExtractPhotoelectronTrajectory;
class AHRayTracingPrimaryGen;
class ListPrimaryGen;
class SimXPrimaryGen;
class AHRadiationBackgroundPrimaryGen;
class AEObservationPrimaryGen;
class CelestialSourcePrimaryGen;
class RadioactiveDecayUserActionAssembly;
class ActivationUserActionAssembly;
class AHStandardUserActionAssembly;
class SampleOpticalDepth;
class ScatteringPickUpData;
class ObservationPickUpData;
class PhysicsListManager;
class SimXIF;
class GenerateSimXEvent;
class OutputSimXPrimaries;
class AssignG4CopyNumber;
class InitialParticleTree;
class DumpMass;
class WriteObservationTree;
class SimulateCXBShieldPlate;
class RescaleSimulationNoiseOfSGDSiUntriggered;
class UniformlyRandomizeEPI;
class ReadSGDEventFITS;
class WriteSGDEventFITS;
class ReadHXIEventFITS;
class WriteHXIEventFITS;
class FilterByGoodTimeIntervalsForSGD;
class FilterByGoodTimeIntervalsForHXI;
}

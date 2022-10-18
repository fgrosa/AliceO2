// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "FairDetector.h"      // for FairDetector
#include <fairlogger/Logger.h> // for LOG, LOG_IF
#include "FairRootManager.h"   // for FairRootManager
#include "FairRun.h"           // for FairRun
#include "FairRuntimeDb.h"     // for FairRuntimeDb
#include "FairVolume.h"        // for FairVolume
#include "FairRootManager.h"

#include "TGeoManager.h"     // for TGeoManager, gGeoManager
#include "TGeoTube.h"        // for TGeoTube
#include "TGeoPcon.h"        // for TGeoPcon
#include "TGeoVolume.h"      // for TGeoVolume, TGeoVolumeAssembly
#include "TString.h"         // for TString, operator+
#include "TVirtualMC.h"      // for gMC, TVirtualMC
#include "TVirtualMCStack.h" // for TVirtualMCStack

#include "ITSMFTBase/SegmentationAlpide.h"
#include "ITSSimulation/DescriptorInnerBarrelITS2.h"

using namespace o2::its;

/// \cond CLASSIMP
ClassImp(DescriptorInnerBarrelITS2);
/// \endcond

//________________________________________________________________
DescriptorInnerBarrelITS2::DescriptorInnerBarrelITS2(int nlayers) : DescriptorInnerBarrel(nlayers)
{
  //
  // Standard constructor
  //

  fSensorLayerThickness = o2::itsmft::SegmentationAlpide::SensorLayerThickness;
}

//________________________________________________________________
void DescriptorInnerBarrelITS2::Configure()
{
  // build ITS2 upgrade detector
  fTurboLayer.resize(fNumLayers);
  fLayerPhi0.resize(fNumLayers);
  fLayerRadii.resize(fNumLayers);
  fLayerZLen.resize(fNumLayers);
  fStavePerLayer.resize(fNumLayers);
  fUnitPerStave.resize(fNumLayers);
  fChipThickness.resize(fNumLayers);
  fDetectorThickness.resize(fNumLayers);
  fStaveTilt.resize(fNumLayers);
  fStaveWidth.resize(fNumLayers);
  fChipTypeID.resize(fNumLayers);
  fBuildLevel.resize(fNumLayers);
  fStaveModelInnerBarrel.resize(fNumLayers);

  // Radii are from last TDR (ALICE-TDR-017.pdf Tab. 1.1)
  std::vector<std::array<double, 6>> IBdat;
  IBdat.emplace_back(std::array<double, 6>{2.24, 2.34, 2.67, 9., 16.42, 12});
  IBdat.emplace_back(std::array<double, 6>{3.01, 3.15, 3.46, 9., 12.18, 16});
  IBdat.emplace_back(std::array<double, 6>{3.78, 3.93, 4.21, 9., 9.55, 20});

  for (auto idLayer{0u}; idLayer < fNumLayers; ++idLayer) {
    fStaveModelInnerBarrel[idLayer] = o2::its::V3Layer::kIBModel4;
    fTurboLayer[idLayer] = true;
    fLayerPhi0[idLayer] = IBdat[idLayer][4];
    fLayerRadii[idLayer] = IBdat[idLayer][1];
    fStavePerLayer[idLayer] = IBdat[idLayer][5];
    fUnitPerStave[idLayer] = IBdat[idLayer][3];
    fChipThickness[idLayer] = 50.e-4;
    fStaveWidth[idLayer] = o2::itsmft::SegmentationAlpide::SensorSizeRows;
    fStaveTilt[idLayer] = radii2Turbo(IBdat[idLayer][2], IBdat[idLayer][0], IBdat[idLayer][1], o2::itsmft::SegmentationAlpide::SensorSizeRows);
    fDetectorThickness[idLayer] = fSensorLayerThickness;
    fChipTypeID[idLayer] = 0;
    fBuildLevel[idLayer] = 0;

    LOG(info) << "L# " << idLayer << " Phi:" << fLayerPhi0[idLayer] << " R:" << fLayerRadii[idLayer] << " Nst:" << fStavePerLayer[idLayer] << " Nunit:" << fUnitPerStave[idLayer]
              << " W:" << fStaveWidth[idLayer] << " Tilt:" << fStaveTilt[idLayer] << " Lthick:" << fChipThickness[idLayer] << " Dthick:" << fDetectorThickness[idLayer]
              << " DetID:" << fChipTypeID[idLayer] << " B:" << fBuildLevel[idLayer];
  }

  fWrapperMinRadius = 2.1;
  fWrapperMaxRadius = 15.4;
  fWrapperZSpan = 70.;
}

//________________________________________________________________
void DescriptorInnerBarrelITS2::GetConfigurationLayers(std::vector<bool>& turbo, std::vector<double>& phi0, std::vector<double>& radii, std::vector<double>& chipThickness, std::vector<int>& unitPerStave, std::vector<double>& staveWidth, std::vector<int>& stavePerLayer, std::vector<double>& staveTilt, std::vector<double>& detThickness, std::vector<int>& chipID, std::vector<int>& buildlev)
{
  turbo = fTurboLayer;
  phi0 = fLayerPhi0;
  radii = fLayerRadii;
  chipThickness = fChipThickness;
  unitPerStave = fUnitPerStave;
  detThickness = fDetectorThickness;
  staveWidth = fStaveWidth;
  stavePerLayer = fStavePerLayer;
  staveTilt = fStaveTilt;
  chipID = fChipTypeID;
  buildlev = fBuildLevel;
}

//________________________________________________________________
V3Layer* DescriptorInnerBarrelITS2::DefineLayer(int idLayer, TGeoVolume* dest)
{
  V3Layer* mGeometry = nullptr;
  if (idLayer >= fNumLayers) {
    LOG(fatal) << "Trying to define layer " << idLayer << " of inner barrel, but only " << fNumLayers << " layers expected!";
    return mGeometry;
  }

  if (fTurboLayer[idLayer]) {
    mGeometry = new V3Layer(idLayer, true, false);
    mGeometry->setStaveWidth(fStaveWidth[idLayer]);
    mGeometry->setStaveTilt(fStaveTilt[idLayer]);
  } else {
    mGeometry = new V3Layer(idLayer, false);
  }

  mGeometry->setPhi0(fLayerPhi0[idLayer]);
  mGeometry->setRadius(fLayerRadii[idLayer]);
  mGeometry->setNumberOfStaves(fStavePerLayer[idLayer]);
  mGeometry->setNumberOfUnits(fUnitPerStave[idLayer]);
  mGeometry->setChipType(fChipTypeID[idLayer]);
  mGeometry->setBuildLevel(fBuildLevel[idLayer]);

  mGeometry->setStaveModel(fStaveModelInnerBarrel[idLayer]);

  if (fChipThickness[idLayer] != 0) {
    mGeometry->setChipThick(fChipThickness[idLayer]);
  }
  if (fDetectorThickness[idLayer] != 0) {
    mGeometry->setSensorThick(fDetectorThickness[idLayer]);
  }

  mGeometry->createLayer(dest);

  return mGeometry; // is this needed?
}
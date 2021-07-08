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

/// \file HFSelOptimisation.cxx
/// \brief task to study preselections
///
/// \author Fabrizio Grosa <fabrizio.grosa@cern.ch>, CERN

#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/HistogramRegistry.h"
#include "AnalysisDataModel/HFSecondaryVertex.h"
#include "AnalysisDataModel/HFCandidateSelectionTables.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;
using namespace o2::aod::hf_cand;

namespace
{

static constexpr int nCospCutsToTest = 15;
static constexpr int nDecLenCutsToTest = 11;
static constexpr int nImpParProdCutsToTest = 11;
static constexpr int nMinDCAxyCutsToTest = 9;
static constexpr int nMinTrackPtCutsToTest = 7;

constexpr float cospCuts[nCospCutsToTest] = {0.70, 0.75, 0.80, 0.85, 0.88, 0.90, 0.92, 0.93, 0.94, 0.95, 0.96, 0.97, 0.98, 0.99, 0.995};
constexpr float decLenCuts[nDecLenCutsToTest] = {0., 0.005, 0.01, 0.015, 0.02, 0.025, 0.03, 0.04, 0.05, 0.075, 0.1};
constexpr float impParProdCuts[nImpParProdCutsToTest] = {-0.00005, -0.00004, -0.00003, -0.00002, -0.00001, 0., 0.00001, 0.00002, 0.00003, 0.00004, 0.00005};
constexpr float minDCAxyCuts[nMinDCAxyCutsToTest] = {0., 0.0005, 0.001, 0.0015, 0.0020, 0.0025, 0.0030, 0.0040, 0.0050};
constexpr float minTrackPtCuts[nMinTrackPtCutsToTest] = {0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60};

auto cospCutsVec = std::vector<float>{cospCuts, cospCuts + nCospCutsToTest};
auto decLenCutsVec = std::vector<float>{decLenCuts, decLenCuts + nDecLenCutsToTest};
auto impParProdCutsVec = std::vector<float>{impParProdCuts, impParProdCuts + nImpParProdCutsToTest};
auto minDCAxyCutsVec = std::vector<float>{minDCAxyCuts, minDCAxyCuts + nMinDCAxyCutsToTest};
auto minTrackPtCutsVec = std::vector<float>{minTrackPtCuts, minTrackPtCuts + nMinTrackPtCutsToTest};

static const int n2Prong = o2::aod::hf_cand_prong2::DecayType::N2ProngDecays;
static const int n3Prong = o2::aod::hf_cand_prong3::DecayType::N3ProngDecays;

static constexpr std::array<std::array<std::string_view, n2Prong + 1>, 3> histoNames2Prong = {{{"hPromptVsPtD0ToPiK", "hPromptVsPtJpsiToEE", "hPromptVsPt2Prong"},
                                                                                               {"hNonPromptVsPtD0ToPiK", "hNonPromptVsPtJpsiToEE", "hNonPromptVsPt2Prong"},
                                                                                               {"hBkgVsPtD0ToPiK", "hBkgVsPtJpsiToEE", "hBkgVsPt2Prong"}}};
static constexpr std::array<std::array<std::string_view, n2Prong + 1>, 3> histoCospNames2Prong = {{{"hPromptCospVsPtD0ToPiK", "hPromptCospVsPtJpsiToEE", "hPromptCospVsPt2Prong"},
                                                                                                   {"hNonPromptCospVsPtD0ToPiK", "hNonPromptCospVsPtJpsiToEE", "hNonPromptCospVsPt2Prong"},
                                                                                                   {"hBkgCospVsPtD0ToPiK", "hBkgCospVsPtJpsiToEE", "hBkgCospVsPt2Prong"}}};
static constexpr std::array<std::array<std::string_view, n2Prong + 1>, 3> histoDecLenNames2Prong = {{{"hPromptDecLenVsPtD0ToPiK", "hPromptDecLenVsPtJpsiToEE", "hPromptDecLenVsPt2Prong"},
                                                                                                     {"hNonPromptDecLenVsPtD0ToPiK", "hNonPromptDecLenVsPtJpsiToEE", "hNonPromptDecLenVsPt2Prong"},
                                                                                                     {"hBkgDecLenVsPtD0ToPiK", "hBkgDecLenVsPtJpsiToEE", "hBkgDecLenVsPt2Prong"}}};
static constexpr std::array<std::array<std::string_view, n2Prong + 1>, 3> histoImpParProdNames2Prong = {{{"hPromptImpParProdVsPtD0ToPiK", "hPromptImpParProdVsPtJpsiToEE", "hPromptImpParProdVsPt2Prong"},
                                                                                                         {"hNonPromptImpParProdVsPtD0ToPiK", "hNonPromptImpParProdVsPtJpsiToEE", "hNonPromptImpParProdVsPt2Prong"},
                                                                                                         {"hBkgImpParProdVsPtD0ToPiK", "hBkgImpParProdVsPtJpsiToEE", "hBkgImpParProdVsPt2Prong"}}};
static constexpr std::array<std::array<std::string_view, n2Prong + 1>, 3> histoMinDCAxyNames2Prong = {{{"hPromptMinDCAxyVsPtD0ToPiK", "hPromptMinDCAxyVsPtJpsiToEE", "hPromptMinDCAxyVsPt2Prong"},
                                                                                                       {"hNonPromptMinDCAxyVsPtD0ToPiK", "hNonPromptMinDCAxyVsPtJpsiToEE", "hNonPromptMinDCAxyVsPt2Prong"},
                                                                                                       {"hBkgMinDCAxyVsPtD0ToPiK", "hBkgMinDCAxyVsPtJpsiToEE", "hBkgMinDCAxyVsPt2Prong"}}};
static constexpr std::array<std::array<std::string_view, n2Prong + 1>, 3> histoMinTrackPtNames2Prong = {{{"hPromptMinTrackPtVsPtD0ToPiK", "hPromptMinTrackPtVsPtJpsiToEE", "hPromptMinTrackPtVsPt2Prong"},
                                                                                                         {"hNonPromptMinTrackPtVsPtD0ToPiK", "hNonPromptMinTrackPtVsPtJpsiToEE", "hNonPromptMinTrackPtVsPt2Prong"},
                                                                                                         {"hBkgMinTrackPtVsPtD0ToPiK", "hBkgMinTrackPtVsPtJpsiToEE", "hBkgMinTrackPtVsPt2Prong"}}};

static constexpr std::array<std::array<std::string_view, n3Prong + 1>, 3> histoNames3Prong = {{{"hPromptVsPtDPlusToPiKPi", "hPromptVsPtLcToPKPi", "hPromptVsPtDsToPiKK", "hPromptVsPtXicToPKPi", "hPromptVsPt3Prong"},
                                                                                               {"hNonPromptVsPtDPlusToPiKPi", "hNonPromptVsPtLcToPKPi", "hNonPromptVsPtDsToPiKK", "hNonPromptVsPtXicToPKPi", "hNonPromptVsPt3Prong"},
                                                                                               {"hBkgVsPtDPlusToPiKPi", "hBkgVsPtLcToPKPi", "hBkgVsPtDsToPiKK", "hBkgVsPtXicToPKPi", "hBkgVsPt3Prong"}}};
static constexpr std::array<std::array<std::string_view, n3Prong + 1>, 3> histoCospNames3Prong = {{{"hPromptCospVsPtDPlusToPiKPi", "hPromptCospVsPtLcToPKPi", "hPromptCospVsPtDsToPiKK", "hPromptCospVsPtXicToPKPi", "hPromptCospVsPt3Prong"},
                                                                                                   {"hNonPromptCospVsPtDPlusToPiKPi", "hNonPromptCospVsPtLcToPKPi", "hNonPromptCospVsPtDsToPiKK", "hNonPromptCospVsPtXicToPKPi", "hNonPromptCospVsPt3Prong"},
                                                                                                   {"hBkgCospVsPtDPlusToPiKPi", "hBkgCospVsPtLcToPKPi", "hBkgCospVsPtDsToPiKK", "hBkgCospVsPtXicToPKPi", "hBkgCospVsPt3Prong"}}};
static constexpr std::array<std::array<std::string_view, n3Prong + 1>, 3> histoDecLenNames3Prong = {{{"hPromptDecLenVsPtDPlusToPiKPi", "hPromptDecLenVsPtLcToPKPi", "hPromptDecLenVsPtDsToPiKK", "hPromptDecLenVsPtXicToPKPi", "hPromptDecLenVsPt3Prong"},
                                                                                                     {"hNonPromptDecLenVsPtDPlusToPiKPi", "hNonPromptDecLenVsPtLcToPKPi", "hNonPromptDecLenVsPtDsToPiKK", "hNonPromptDecLenVsPtXicToPKPi", "hNonPromptDecLenVsPt3Prong"},
                                                                                                     {"hBkgDecLenVsPtDPlusToPiKPi", "hBkgDecLenVsPtLcToPKPi", "hBkgDecLenVsPtDsToPiKK", "hBkgDecLenVsPtXicToPKPi", "hBkgDecLenVsPt3Prong"}}};
static constexpr std::array<std::array<std::string_view, n3Prong + 1>, 3> histoMinDCAxyNames3Prong = {{{"hPromptMinDCAxyVsPtDPlusToPiKPi", "hPromptMinDCAxyVsPtLcToPKPi", "hPromptMinDCAxyVsPtDsToPiKK", "hPromptMinDCAxyVsPtXicToPKPi", "hPromptMinDCAxyVsPt3Prong"},
                                                                                                       {"hNonPromptMinDCAxyVsPtDPlusToPiKPi", "hNonPromptMinDCAxyVsPtLcToPKPi", "hNonPromptMinDCAxyVsPtDsToPiKK", "hNonPromptMinDCAxyVsPtXicToPKPi", "hNonPromptMinDCAxyVsPt3Prong"},
                                                                                                       {"hBkgMinDCAxyVsPtDPlusToPiKPi", "hBkgMinDCAxyVsPtLcToPKPi", "hBkgMinDCAxyVsPtDsToPiKK", "hBkgMinDCAxyVsPtXicToPKPi", "hBkgMinDCAxyVsPt3Prong"}}};
static constexpr std::array<std::array<std::string_view, n3Prong + 1>, 3> histoMinTrackPtNames3Prong = {{{"hPromptMinTrackPtVsPtDPlusToPiKPi", "hPromptMinTrackPtVsPtLcToPKPi", "hPromptMinTrackPtVsPtDsToPiKK", "hPromptMinTrackPtVsPtXicToPKPi", "hPromptMinTrackPtVsPt3Prong"},
                                                                                                         {"hNonPromptMinTrackPtVsPtDPlusToPiKPi", "hNonPromptMinTrackPtVsPtLcToPKPi", "hNonPromptMinTrackPtVsPtDsToPiKK", "hNonPromptMinTrackPtVsPtXicToPKPi", "hNonPromptMinTrackPtVsPt3Prong"},
                                                                                                         {"hBkgMinTrackPtVsPtDPlusToPiKPi", "hBkgMinTrackPtVsPtLcToPKPi", "hBkgMinTrackPtVsPtDsToPiKK", "hBkgMinTrackPtVsPtXicToPKPi", "hBkgMinTrackPtVsPt3Prong"}}};
} // namespace

struct HfSelOptimisation {

  Configurable<std::vector<float>> cospCutsToTest{"cospCutsToTest", std::vector<float>{cospCutsVec}, "cos(theta_P) cut values to test"};
  Configurable<std::vector<float>> decLenCutsToTest{"decLenCutsToTest", std::vector<float>{decLenCutsVec}, "decay length cut values to test"};
  Configurable<std::vector<float>> impParProdCutsToTest{"impParProdCutsToTest", std::vector<float>{impParProdCutsVec}, "impact parameter product cut values to test (2-prongs only)"};
  Configurable<std::vector<float>> minDCAxyCutsToTest{"minDCAxyCutsToTest", std::vector<float>{minDCAxyCutsVec}, "min DCA xy cut values to test"};
  Configurable<std::vector<float>> minTrackPtCutsToTest{"minTrackPtCutsToTest", std::vector<float>{minTrackPtCutsVec}, "min track pT cut values to test"};

  ConfigurableAxis ptBinning{"ptBinning", {0, 0., 2., 5., 20.}, "pT bin limits"};

  AxisSpec ptAxis = {ptBinning, "#it{p}_{T} (GeV/#it{c})"};
  // quantized axes
  AxisSpec cospAxis = {cospCutsToTest->size(), 0.5, cospCutsToTest->size() + 0.5, "cos(#theta_{P}) >"};
  AxisSpec decLenAxis = {decLenCutsToTest->size(), 0.5, decLenCutsToTest->size() + 0.5, "decay length (cm) >"};
  AxisSpec impParProdAxis = {impParProdCutsToTest->size(), 0.5, impParProdCutsToTest->size() + 0.5, "#it{d}_{0}#times#it{d}_{0} (cm^{2}) <"};
  AxisSpec minDCAxyAxis = {minDCAxyCutsToTest->size(), 0.5, minDCAxyCutsToTest->size() + 0.5, "min track #it{d}_{0} (cm) >"};
  AxisSpec minTrackPtAxis = {minTrackPtCutsToTest->size(), 0.5, minTrackPtCutsToTest->size() + 0.5, "min track #it{p}_{T} (cm) >"};

  HistogramRegistry registry{"registry", {}};

  void init(InitContext const&)
  {
    for (int iOrig = 0; iOrig < 3; iOrig++) {
      for (int i2Prong = 0; i2Prong < n2Prong; i2Prong++) {
        registry.add(histoNames2Prong[iOrig][i2Prong].data(), "", HistType::kTH1F, {ptAxis});
        registry.add(histoCospNames2Prong[iOrig][i2Prong].data(), "", HistType::kTH2F, {ptAxis, cospAxis});
        registry.add(histoDecLenNames2Prong[iOrig][i2Prong].data(), "", HistType::kTH2F, {ptAxis, decLenAxis});
        registry.add(histoImpParProdNames2Prong[iOrig][i2Prong].data(), "", HistType::kTH2F, {ptAxis, impParProdAxis});
        registry.add(histoMinDCAxyNames2Prong[iOrig][i2Prong].data(), "", HistType::kTH2F, {ptAxis, minDCAxyAxis});
        registry.add(histoMinTrackPtNames2Prong[iOrig][i2Prong].data(), "", HistType::kTH2F, {ptAxis, minTrackPtAxis});
      }
      for (int i3Prong = 0; i3Prong < n3Prong; i3Prong++) {
        registry.add(histoNames3Prong[iOrig][i3Prong].data(), "", HistType::kTH1F, {ptAxis});
        registry.add(histoCospNames3Prong[iOrig][i3Prong].data(), "", HistType::kTH2F, {ptAxis, cospAxis});
        registry.add(histoDecLenNames3Prong[iOrig][i3Prong].data(), "", HistType::kTH2F, {ptAxis, decLenAxis});
        registry.add(histoMinDCAxyNames3Prong[iOrig][i3Prong].data(), "", HistType::kTH2F, {ptAxis, minDCAxyAxis});
        registry.add(histoMinTrackPtNames3Prong[iOrig][i3Prong].data(), "", HistType::kTH2F, {ptAxis, minTrackPtAxis});
      }
    }
  }

  /// Conjugate-dependent topological cuts
  /// \param candidate is a candidate
  /// \param channel is the candidate channel
  /// \param candType is candidate type (Prompt, NonPrompt, Bkg)
  template <std::size_t candType, std::size_t candOrig, typename T1, typename T2>
  void testSelections2Prong(const T1& candidate, const T2& tracks)
  {
    auto pT = candidate.pt();
    std::array<double, 2> absDCA{std::abs(tracks[0].dcaPrim0()), std::abs(tracks[1].dcaPrim0())};
    std::sort(absDCA.begin(), absDCA.end());

    std::array<double, 2> ptTrack{tracks[0].pt(), tracks[1].pt()};
    std::sort(ptTrack.begin(), ptTrack.end());

    registry.get<TH1>(HIST(histoNames2Prong[candOrig][candType].data()))->Fill(pT);

    for (int iCospCut = 0; iCospCut < cospCutsToTest->size(); iCospCut++) {
      if (candidate.cpa() > cospCuts[iCospCut]) {
        registry.get<TH2>(HIST(histoCospNames2Prong[candOrig][candType].data()))->Fill(pT, iCospCut + 1);
      }
    }

    for (int iDecLenCut = 0; iDecLenCut < decLenCutsToTest->size(); iDecLenCut++) {
      if (candidate.decayLength() > decLenCuts[iDecLenCut]) {
        registry.get<TH2>(HIST(histoDecLenNames2Prong[candOrig][candType].data()))->Fill(pT, iDecLenCut + 1);
      }
    }

    for (int iImpParProd = 0; iImpParProd < impParProdCutsToTest->size(); iImpParProd++) {
      if (candidate.impactParameterProduct() < impParProdCuts[iImpParProd]) {
        registry.get<TH2>(HIST(histoImpParProdNames2Prong[candOrig][candType].data()))->Fill(pT, iImpParProd + 1);
      }
    }

    for (int iMinDCAxy = 0; iMinDCAxy < minDCAxyCutsToTest->size(); iMinDCAxy++) {
      if (absDCA[0] > minDCAxyCuts[iMinDCAxy]) {
        registry.get<TH2>(HIST(histoMinDCAxyNames2Prong[candOrig][candType].data()))->Fill(pT, iMinDCAxy + 1);
      }
    }

    for (int iMinTrackPt = 0; iMinTrackPt < minTrackPtCutsToTest->size(); iMinTrackPt++) {
      if (ptTrack[0] > minTrackPtCuts[iMinTrackPt]) {
        registry.get<TH2>(HIST(histoMinTrackPtNames2Prong[candOrig][candType].data()))->Fill(pT, iMinTrackPt + 1);
      }
    }
  }

  /// Conjugate-dependent topological cuts
  /// \param candidate is a candidate
  /// \param channel is the candidate channel
  /// \param candType is candidate type (Prompt, NonPrompt, Bkg)
  template <std::size_t candType, std::size_t candOrig, typename T1, typename T2>
  void testSelections3Prong(const T1& candidate, const T2& tracks)
  {
    auto pT = candidate.pt();
    std::array<double, 3> absDCA{std::abs(tracks[0].dcaPrim0()), std::abs(tracks[1].dcaPrim0()), std::abs(tracks[2].dcaPrim0())};
    std::sort(absDCA.begin(), absDCA.end());

    std::array<double, 3> ptTrack{tracks[0].pt(), tracks[1].pt(), tracks[2].pt()};
    std::sort(ptTrack.begin(), ptTrack.end());

    registry.get<TH1>(HIST(histoNames3Prong[candOrig][candType].data()))->Fill(pT);

    for (int iCospCut = 0; iCospCut < cospCutsToTest->size(); iCospCut++) {
      if (candidate.cpa() > cospCuts[iCospCut]) {
        registry.get<TH2>(HIST(histoCospNames3Prong[candOrig][candType].data()))->Fill(pT, iCospCut + 1);
      }
    }

    for (int iDecLenCut = 0; iDecLenCut < decLenCutsToTest->size(); iDecLenCut++) {
      if (candidate.decayLength() > decLenCuts[iDecLenCut]) {
        registry.get<TH2>(HIST(histoDecLenNames3Prong[candOrig][candType].data()))->Fill(pT, iDecLenCut + 1);
      }
    }

    for (int iMinDCAxy = 0; iMinDCAxy < minDCAxyCutsToTest->size(); iMinDCAxy++) {
      if (absDCA[0] > minDCAxyCuts[iMinDCAxy]) {
        registry.get<TH2>(HIST(histoMinDCAxyNames3Prong[candOrig][candType].data()))->Fill(pT, iMinDCAxy + 1);
      }
    }

    for (int iMinTrackPt = 0; iMinTrackPt < minTrackPtCutsToTest->size(); iMinTrackPt++) {
      if (ptTrack[0] > minTrackPtCuts[iMinTrackPt]) {
        registry.get<TH2>(HIST(histoMinTrackPtNames3Prong[candOrig][candType].data()))->Fill(pT, iMinTrackPt + 1);
      }
    }
  }

  void process(soa::Join<aod::HfCandProng2, aod::HfCandProng2MCRec> const& cand2Prongs,
               soa::Join<aod::HfCandProng3, aod::HfCandProng3MCRec> const& cand3Prongs,
               aod::BigTracks const&)
  {
    // looping over 2-prong candidates
    for (const auto& cand2Prong : cand2Prongs) {

      auto trackPos = cand2Prong.index0_as<aod::BigTracks>(); // positive daughter
      auto trackNeg = cand2Prong.index1_as<aod::BigTracks>(); // negative daughter
      std::array tracks = {trackPos, trackNeg};

      bool isPrompt = false, isNonPrompt = false, isBkg = false;
      for (int iDecay = 0; iDecay < n2Prong; iDecay++) {
        if (TESTBIT(cand2Prong.hfflag(), iDecay)) {
          if (std::abs(cand2Prong.flagMCMatchRec()) == BIT(iDecay)) {
            if (cand2Prong.originMCRec() == OriginType::Prompt) {
              isPrompt = true;
              switch (iDecay) {
                case o2::aod::hf_cand_prong2::DecayType::D0ToPiK:
                  testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::D0ToPiK, 0>(cand2Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong2::DecayType::JpsiToEE:
                  testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::JpsiToEE, 0>(cand2Prong, tracks);
                  break;
              }
            } else if (cand2Prong.originMCRec() == OriginType::NonPrompt) {
              isNonPrompt = true;
              switch (iDecay) {
                case o2::aod::hf_cand_prong2::DecayType::D0ToPiK:
                  testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::D0ToPiK, 1>(cand2Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong2::DecayType::JpsiToEE:
                  testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::JpsiToEE, 1>(cand2Prong, tracks);
                  break;
              }
            }
          } else {
            isBkg = true;
            switch (iDecay) {
              case o2::aod::hf_cand_prong2::DecayType::D0ToPiK:
                testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::D0ToPiK, 2>(cand2Prong, tracks);
                break;
              case o2::aod::hf_cand_prong2::DecayType::JpsiToEE:
                testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::JpsiToEE, 2>(cand2Prong, tracks);
                break;
            }
          }
        }
      }

      if (isPrompt) {
        testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::N2ProngDecays, 0>(cand2Prong, tracks);
      } else if (isNonPrompt) {
        testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::N2ProngDecays, 1>(cand2Prong, tracks);
      } else if (isBkg) {
        testSelections2Prong<o2::aod::hf_cand_prong2::DecayType::N2ProngDecays, 2>(cand2Prong, tracks);
      }
    } // loop over 2-prong candidates

    // looping over 3-prong candidates
    for (const auto& cand3Prong : cand3Prongs) {

      auto trackFirst = cand3Prong.index0_as<aod::BigTracks>();  // first daughter
      auto trackSecond = cand3Prong.index1_as<aod::BigTracks>(); // second daughter
      auto trackThird = cand3Prong.index2_as<aod::BigTracks>();  // third daughter
      std::array tracks = {trackFirst, trackSecond, trackThird};

      bool isPrompt = false, isNonPrompt = false, isBkg = false;
      for (int iDecay = 0; iDecay < n3Prong; iDecay++) {
        if (TESTBIT(cand3Prong.hfflag(), iDecay)) {
          if (std::abs(cand3Prong.flagMCMatchRec()) == BIT(iDecay)) {
            if (cand3Prong.originMCRec() == OriginType::Prompt) {
              isPrompt = true;
              switch (iDecay) {
                case o2::aod::hf_cand_prong3::DecayType::DPlusToPiKPi:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::DPlusToPiKPi, 0>(cand3Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong3::DecayType::LcToPKPi:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::LcToPKPi, 0>(cand3Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong3::DecayType::DsToPiKK:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::DsToPiKK, 0>(cand3Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong3::DecayType::XicToPKPi:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::XicToPKPi, 0>(cand3Prong, tracks);
                  break;
              }
            } else if (cand3Prong.originMCRec() == OriginType::NonPrompt) {
              isNonPrompt = true;
              switch (iDecay) {
                case o2::aod::hf_cand_prong3::DecayType::DPlusToPiKPi:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::DPlusToPiKPi, 1>(cand3Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong3::DecayType::LcToPKPi:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::LcToPKPi, 1>(cand3Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong3::DecayType::DsToPiKK:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::DsToPiKK, 1>(cand3Prong, tracks);
                  break;
                case o2::aod::hf_cand_prong3::DecayType::XicToPKPi:
                  testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::XicToPKPi, 1>(cand3Prong, tracks);
                  break;
              }
            }
          } else {
            isBkg = true;
            switch (iDecay) {
              case o2::aod::hf_cand_prong3::DecayType::DPlusToPiKPi:
                testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::DPlusToPiKPi, 2>(cand3Prong, tracks);
                break;
              case o2::aod::hf_cand_prong3::DecayType::LcToPKPi:
                testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::LcToPKPi, 2>(cand3Prong, tracks);
                break;
              case o2::aod::hf_cand_prong3::DecayType::DsToPiKK:
                testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::DsToPiKK, 2>(cand3Prong, tracks);
                break;
              case o2::aod::hf_cand_prong3::DecayType::XicToPKPi:
                testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::XicToPKPi, 2>(cand3Prong, tracks);
                break;
            }
          }
        }
      }
      if (isPrompt) {
        testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::N3ProngDecays, 0>(cand3Prong, tracks);
      } else if (isNonPrompt) {
        testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::N3ProngDecays, 1>(cand3Prong, tracks);
      } else if (isBkg) {
        testSelections3Prong<o2::aod::hf_cand_prong3::DecayType::N3ProngDecays, 2>(cand3Prong, tracks);
      }
    } // loop over 3-prong candidates
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  return WorkflowSpec{
    adaptAnalysisTask<HfSelOptimisation>(cfgc)};
}

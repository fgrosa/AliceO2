// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#ifndef O2_ANALYSIS_TRIGGER_H_
#define O2_ANALYSIS_TRIGGER_H_

#include <array>
#include "Framework/AnalysisDataModel.h"

namespace o2::aod
{
namespace filtering
{
DECLARE_SOA_COLUMN(H2, hasH2, bool);   //!
DECLARE_SOA_COLUMN(H3, hasH3, bool);   //!
DECLARE_SOA_COLUMN(He3, hasHe3, bool); //!
DECLARE_SOA_COLUMN(He4, hasHe4, bool); //!

// diffraction
DECLARE_SOA_COLUMN(DG, hasDG, bool); //! Double Gap events, DG

// heavy flavours
DECLARE_SOA_COLUMN(HfHighPt, hasHfHighPt, bool); //! high-pT charm hadron
DECLARE_SOA_COLUMN(HfBeauty, hasHfBeauty, bool); //! beauty hadron
DECLARE_SOA_COLUMN(HfFemto, hasHfFemto, bool);   //! charm-hadron - N pair

} // namespace filtering

DECLARE_SOA_TABLE(NucleiFilters, "AOD", "Nuclei Filters", //!
                  filtering::H2, filtering::H3, filtering::He3, filtering::He4);

constexpr std::array<char[32], 3> AvailableFilters{"NucleiFilters", "DiffractionFilters", "HeavyFlavourFilters"};
constexpr std::array<char[16], 3> FilterDescriptions{"Nuclei Filters", "DiffFilters", "HF Filters"};

using NucleiFilter = NucleiFilters::iterator;

// diffraction
DECLARE_SOA_TABLE(DiffractionFilters, "AOD", "DiffFilters", //! Diffraction filters
                  filtering::DG);
using DiffractionFilter = DiffractionFilters::iterator;

// heavy flavours
DECLARE_SOA_TABLE(HfFilters, "AOD", "HF Filters", //!
                  filtering::HfHighPt, filtering::HfBeauty, filtering::HfFemto);

using HfFilter = HfFilters::iterator;

} // namespace o2::aod

#endif // O2_ANALYSIS_TRIGGER_H_

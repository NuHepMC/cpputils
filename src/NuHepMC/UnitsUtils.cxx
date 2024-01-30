#include "NuHepMC/UnitsUtils.hxx"

#include "NuHepMC/ReaderUtils.hxx"
#include "NuHepMC/EventUtils.hxx"

namespace NuHepMC {

namespace CrossSection {

namespace Units {

NEW_NuHepMC_EXCEPT(NonStandardXSUnitsUsed);

double GetRescaleFactor(HepMC3::GenEvent const &evt, Units::Unit from,
                        Units::Unit const &to) {

  if (from == automatic) {
    from = NuHepMC::GC4::ParseCrossSectionUnits(evt.run_info());
  }

  if ((from.scale == Units::Scale::CustomType) ||
      (from.tgtscale == Units::TargetScale::CustomType)) {
    auto units_str = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());
    throw NonStandardXSUnitsUsed()
        << units_str.first << " " << units_str.second;
  }
  if ((to.scale == Units::Scale::CustomType) ||
      (to.tgtscale == Units::TargetScale::CustomType)) {
    auto units_str = NuHepMC::GC4::ReadCrossSectionUnits(evt.run_info());
    throw NonStandardXSUnitsUsed()
        << units_str.first << " " << units_str.second;
  }

  if ((from.tgtscale == Units::TargetScale::PerTargetMolecule) ||
      (to.tgtscale == Units::TargetScale::PerTargetMolecule)) {
    throw NonStandardXSUnitsUsed() << "Cannot automatically convert between "
                                      "units including PerTargetMolecule";
  }

  double sf = 1;

  if (from.scale != to.scale) {
    static std::map<Units::Scale, double> const xsunit_factors = {
        {Units::Scale::pb, Units::pb},
        {Units::Scale::cm2, Units::cm2},
        {Units::Scale::cm2_ten38, Units::cm2_ten38},
    };
    // pb -> cm2 : 1E-36
    // 1      1E36
    //   1 / 1E36 = 1E-36!
    sf *= xsunit_factors.at(from.scale) / xsunit_factors.at(to.scale);
  }

  if (from.tgtscale != to.tgtscale) {
    auto tgt_part_id = NuHepMC::Event::GetTargetParticle(evt)->pid();
    std::map<Units::TargetScale, double> tsunit_factors = {
        {Units::TargetScale::PerTargetAtom, 1.0 / ((tgt_part_id / 10) % 1000)},
        {Units::TargetScale::PerTargetNucleon, 1.0},
        {Units::TargetScale::PerTargetMolecularNucleon, 1.0},
    };

    sf *= tsunit_factors.at(from.tgtscale) / tsunit_factors.at(to.tgtscale);
  }

  return sf;
}

} // namespace Units
} // namespace CrossSection
} // namespace NuHepMC

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::Scale us) {
  switch (us) {
  case NuHepMC::CrossSection::Units::Scale::CustomType: {
    return os << "CustomXSUnit";
  }
  case NuHepMC::CrossSection::Units::Scale::pb: {
    return os << "pb";
  }
  case NuHepMC::CrossSection::Units::Scale::cm2: {
    return os << "cm2";
  }
  case NuHepMC::CrossSection::Units::Scale::cm2_ten38: {
    return os << "1e-38 cm2";
  }
  default: {
    throw NuHepMC::CrossSection::Units::InvalidUnitType();
  }
  }
}

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::TargetScale ts) {
  switch (ts) {
  case NuHepMC::CrossSection::Units::TargetScale::CustomType: {
    return os << "CustomTargetScale";
  }
  case NuHepMC::CrossSection::Units::TargetScale::PerTargetMolecule: {
    return os << "PerTargetMolecule";
  }
  case NuHepMC::CrossSection::Units::TargetScale::PerTargetAtom: {
    return os << "PerTargetAtom";
  }
  case NuHepMC::CrossSection::Units::TargetScale::PerTargetNucleon: {
    return os << "PerTargetNucleon";
  }
  case NuHepMC::CrossSection::Units::TargetScale::PerTargetMolecularNucleon: {
    return os << "PerTargetMolecularNucleon";
  }
  default: {
    throw NuHepMC::CrossSection::Units::InvalidUnitType();
  }
  }
}

std::ostream &operator<<(std::ostream &os,
                         NuHepMC::CrossSection::Units::Unit const &u) {
  return os << "[" << u.scale << ", " << u.tgtscale << "]";
}
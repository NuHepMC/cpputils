#include "NuHepMC/Reader.hxx"
#include "NuHepMC/AttributeUtils.hxx"

namespace NuHepMC {

int get_in_version(std::shared_ptr<HepMC3::GenRunInfo> gri) {
  return CheckedAttributeValue<int>(gri, "NuHepMC.Version.Major") * 10000 +
         CheckedAttributeValue<int>(gri, "NuHepMC.Version.Minor") * 100 +
         CheckedAttributeValue<int>(gri, "NuHepMC.Version.Patch");
}

template <typename AT>
void UpdateRunInfoAttributeName(std::shared_ptr<HepMC3::GenRunInfo> gri,
                                std::string const &old_name,
                                std::string const &new_name) {

  if (HasAttributeOfType<AT>(gri, old_name)) {
    gri->add_attribute(
        new_name,
        gri->attribute<typename NuHepMC::attr_traits<AT>::type>(old_name));
    gri->remove_attribute(old_name);
  }
}

void update_runinfo_090_to_100(std::shared_ptr<HepMC3::GenRunInfo> gri) {
  if (HasAttributeOfType<std::vector<std::string>>(gri,
                                                   "NuHepMC.Conventions")) {

    static std::map<std::string, char const *> const update_map{
        {"G.C.1", nullptr}, {"G.C.2", nullptr}, {"G.C.3", "G.C.1"},
        {"G.C.4", nullptr}, {"G.C.5", "G.C.2"}, {"G.C.6", "G.C.3"},
        {"G.C.7", "G.C.4"}, {"G.C.8", "G.C.5"}};

    std::vector<std::string> ncnvs;
    for (auto &ocv : CheckedAttributeValue<std::vector<std::string>>(
             gri, "NuHepMC.Conventions")) {

      if (update_map.count(ocv)) {
        char const *ncv = update_map.at(ocv);
        if (ncv) {
          ncnvs.push_back(ncv);
        }
      } else {
        ncnvs.push_back(ocv);
      }
    }
    add_attribute(gri, "NuHepMC.Conventions", ncnvs);
  }

  UpdateRunInfoAttributeName<std::string>(gri, "NuHepMC.Beam.RateUnit",
                                          "NuHepMC.Beam.FluxUnit");

  if (HasAttributeOfType<std::string>(
          gri, "NuHepMC.Units.CrossSection.TargetScale")) {
    auto const &av = CheckedAttributeValue<std::string>(
        gri, "NuHepMC.Units.CrossSection.TargetScale");

    if (av == "PerTarget") {
      add_attribute(gri, "NuHepMC.Units.CrossSection.TargetScale", "PerAtom");
    } else if (av == "PerTargetAtom") {
      add_attribute(gri, "NuHepMC.Units.CrossSection.TargetScale", "PerAtom");
    } else if (av == "PerTargetNucleon") {
      add_attribute(gri, "NuHepMC.Units.CrossSection.TargetScale",
                    "PerNucleon");
    } else if (av == "PerTargetMolecule") {
      throw;
    } else if (av == "PerTargetMolecularNucleon") {
      throw;
    }
  }
}

void update_runinfo(std::shared_ptr<HepMC3::GenRunInfo> gri, int in_version) {
  if (in_version < 10000) {
    update_runinfo_090_to_100(gri);
  }
}

template <typename AT>
void UpdateEventAttributeName(HepMC3::GenEvent &evt,
                              std::string const &old_name,
                              std::string const &new_name) {

  if (HasAttributeOfType<AT>(&evt, old_name)) {
    evt.add_attribute(
        new_name,
        evt.attribute<typename NuHepMC::attr_traits<AT>::type>(old_name));
    evt.remove_attribute(old_name);
  }
}

void update_event_090_to_100(HepMC3::GenEvent &evt) {
  UpdateEventAttributeName<std::vector<double>>(evt, "LabPos", "lab_pos");
  UpdateEventAttributeName<double>(evt, "TotXS", "tot_xs");
  UpdateEventAttributeName<double>(evt, "ProcXS", "proc_xs");
}

void update_event(HepMC3::GenEvent &evt, int in_version) {
  if (in_version < 10000) {
    update_event_090_to_100(evt);
  }
}

bool Reader::read_event(HepMC3::GenEvent &evt) {
  bool rdr_rval = rdr->read_event(evt);

  if (!run_info()) {
    update_runinfo(rdr->run_info(), get_in_version(rdr->run_info()));
  }

  update_event(evt, in_version);
  evt.set_run_info(rdr->run_info());

  return rdr_rval;
}

} // namespace NuHepMC
#include "fmu-uuid.h"

#include <cppfmu_cs.hpp>

#include <cstring>
#include <stdexcept>

class Slave : public cppfmu::SlaveInstance {
public:
  Slave() { Slave::Reset(); }

  void Reset() override { input_ = false; }

  void SetBoolean(const cppfmu::FMIValueReference vr[], std::size_t nvr,
                  const cppfmu::FMIBoolean value[]) override {
    for (std::size_t i = 0; i < nvr; ++i) {
      if (vr[i] == 0) {
        input_ = value[i];
      } else {
        throw std::logic_error("Invalid value reference");
      }
    }
  }

  void GetBoolean(const cppfmu::FMIValueReference vr[], std::size_t nvr,
                  cppfmu::FMIBoolean value[]) const override {
    for (std::size_t i = 0; i < nvr; ++i) {
      if (vr[i] == 0) {
        value[i] = input_;
      } else {
        throw std::logic_error("Invalid value reference");
      }
    }
  }

  bool DoStep(cppfmu::FMIReal /*currentCommunicationPoint*/,
              cppfmu::FMIReal /*communicationStepSize*/,
              cppfmu::FMIBoolean /*newStep*/,
              cppfmu::FMIReal & /*endOfStep*/) override {
    if (fmi2True == input_) {
      return false;
    } else {
      return true;
    }
  }

private:
  cppfmu::FMIBoolean input_;
};

cppfmu::UniquePtr<cppfmu::SlaveInstance> CppfmuInstantiateSlave(
    cppfmu::FMIString /*instanceName*/, cppfmu::FMIString fmuGUID,
    cppfmu::FMIString /*fmuResourceLocation*/, cppfmu::FMIString /*mimeType*/,
    cppfmu::FMIReal /*timeout*/, cppfmu::FMIBoolean /*visible*/,
    cppfmu::FMIBoolean /*interactive*/, cppfmu::Memory memory,
    cppfmu::Logger /*logger*/) {
  if (std::strcmp(fmuGUID, FMU_UUID) != 0) {
    throw std::runtime_error("FMU GUID mismatch");
  }
  return cppfmu::AllocateUnique<Slave>(memory);
}

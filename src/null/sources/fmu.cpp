#include "fmu-uuid.h"
#include <cppfmu_cs.hpp>
#include <cstring>
#include <stdexcept>


class Slave : public cppfmu::SlaveInstance
{
public:
    Slave()
    {
        Slave::Reset();
    }

    void Reset() override
    {
        input_ = 0.0;
        parameter_ = 0.0;
    }

    void SetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIReal value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] == 0) {
                input_ = value[i];
            } else if (vr[i] == 2) {
                parameter_ = value[i];
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    void GetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        cppfmu::FMIReal value[]) const override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] == 0) {
                value[i] = input_;
            } else if (vr[i] == 1) {
                value[i] = 0.0;
            } else if (vr[i] == 2) {
                value[i] = parameter_;
            } else if (vr[i] == 3) {
                value[i] = 0.0;
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    bool DoStep(
        cppfmu::FMIReal /*currentCommunicationPoint*/,
        cppfmu::FMIReal /*communicationStepSize*/,
        cppfmu::FMIBoolean /*newStep*/,
        cppfmu::FMIReal& /*endOfStep*/) override
    {
        return true;
    }

private:
    cppfmu::FMIReal input_;
    cppfmu::FMIReal parameter_;
};


cppfmu::UniquePtr<cppfmu::SlaveInstance> CppfmuInstantiateSlave(
    cppfmu::FMIString  /*instanceName*/,
    cppfmu::FMIString  fmuGUID,
    cppfmu::FMIString  /*fmuResourceLocation*/,
    cppfmu::FMIString  /*mimeType*/,
    cppfmu::FMIReal    /*timeout*/,
    cppfmu::FMIBoolean /*visible*/,
    cppfmu::FMIBoolean /*interactive*/,
    cppfmu::Memory memory,
    cppfmu::Logger /*logger*/)
{
    if (std::strcmp(fmuGUID, FMU_UUID) != 0) {
        throw std::runtime_error("FMU GUID mismatch");
    }
    return cppfmu::AllocateUnique<Slave>(memory);
}

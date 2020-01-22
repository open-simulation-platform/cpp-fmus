#include "fmu-uuid.h"
#include <array>
#include <cppfmu_cs.hpp>
#include <cstring>
#include <stdexcept>


constexpr int dimension = 3;


class Slave : public cppfmu::SlaveInstance
{
public:
    Slave()
    {
        Slave::Reset();
    }

    void Reset() override
    {
        for (auto& x : data_) x = 0.0;
    }

    void SetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIReal value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            data_.at(vr[i]) = value[i];
        }
    }

    void GetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        cppfmu::FMIReal value[]) const override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            value[i] = data_.at(vr[i]);
        }
    }

    bool DoStep(
        cppfmu::FMIReal /*currentCommunicationPoint*/,
        cppfmu::FMIReal /*communicationStepSize*/,
        cppfmu::FMIBoolean /*newStep*/,
        cppfmu::FMIReal& /*endOfStep*/) override
    {
        for (int i = 0; i < dimension; ++i) {
            data_[i+dimension] = data_[i];
        }
        return true;
    }

private:
    std::array<cppfmu::FMIReal, 2*dimension> data_;
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

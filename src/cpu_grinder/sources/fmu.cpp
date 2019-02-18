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
        workPerTime_ = 1000;
        input_ = 0;
        output_ = 0;
    }

    void SetInteger(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIInteger value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] == 0) {
                workPerTime_ = value[i] > 0 ? value[i] : 0;
            } else if (vr[i] == 1) {
                input_ = value[i];
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    void GetInteger(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        cppfmu::FMIInteger value[]) const override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] == 0) {
                value[i] = workPerTime_;
            } else if (vr[i] == 1) {
                value[i] = input_;
            } else if (vr[i] == 2) {
                value[i] = output_;
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    bool DoStep(
        cppfmu::FMIReal /*currentCommunicationPoint*/,
        cppfmu::FMIReal dt,
        cppfmu::FMIBoolean /*newStep*/,
        cppfmu::FMIReal& /*endOfStep*/) override
    {
        const auto work = static_cast<int>(dt*workPerTime_);
        cppfmu::FMIInteger temp = input_;
        for (int i = 0; i < work; ++i) {
            temp = DoWork(temp);
        }
        output_ = temp;
        return true;
    }

private:
    int DoWork(int start)
    {
        volatile int temp = start;
        for (int i = 0; i < 230000; ++i) {
            ++temp;
            temp %= 104729;
        }
        return temp;
    }

    cppfmu::FMIInteger workPerTime_;
    cppfmu::FMIInteger input_;
    cppfmu::FMIInteger output_;
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

#include "fmu-uuid.h"

#include <cppfmu_cs.hpp>

#include <cstring>
#include <stdexcept>
#include <vector>


// NOTE: garbageOutputBase must be >= ignoredInputBase+mockVarCount
constexpr int mockVarCount = 100;
constexpr int ignoredInputBase = 100;
constexpr int garbageOutputBase = 200;


class Slave : public cppfmu::SlaveInstance
{
public:
    Slave(const cppfmu::Memory& memory)
        : ignoredStringInputs_(
            mockVarCount,
            cppfmu::String(cppfmu::Allocator<char>(memory)),
            cppfmu::Allocator<cppfmu::String>(memory))
    {
        Slave::Reset();
    }

    void Reset() override
    {
        workPerTime_ = 1000;
        input_ = 0;
        output_ = 0;
        for (int i = 0; i < mockVarCount; ++i) {
            ignoredRealInputs_[i] = 0.0;
            ignoredIntegerInputs_[i] = 0;
            ignoredBooleanInputs_[i] = cppfmu::FMIFalse;
            ignoredStringInputs_[i].clear();
        }
    }

    void SetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIReal value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                ignoredRealInputs_[vr[i]-ignoredInputBase] = value[i];
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
            if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                value[i] = ignoredRealInputs_[vr[i]-ignoredInputBase];
            } else if (vr[i] >= garbageOutputBase && vr[i] < garbageOutputBase+mockVarCount) {
                value[i] = 0.0;
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
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
            } else if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                ignoredIntegerInputs_[vr[i]-ignoredInputBase] = value[i];
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
            } else if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                value[i] = ignoredIntegerInputs_[vr[i]-ignoredInputBase];
            } else if (vr[i] >= garbageOutputBase && vr[i] < garbageOutputBase+mockVarCount) {
                value[i] = 0;
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    void SetBoolean(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIBoolean value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                ignoredBooleanInputs_[vr[i]-ignoredInputBase] = value[i];
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    void GetBoolean(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        cppfmu::FMIBoolean value[]) const override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                value[i] = ignoredBooleanInputs_[vr[i]-ignoredInputBase];
            } else if (vr[i] >= garbageOutputBase && vr[i] < garbageOutputBase+mockVarCount) {
                value[i] = cppfmu::FMIFalse;
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    void SetString(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIString value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                ignoredStringInputs_[vr[i]-ignoredInputBase] = value[i];
            } else {
                throw std::logic_error("Invalid value reference");
            }
        }
    }

    void GetString(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        cppfmu::FMIString value[]) const override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] >= ignoredInputBase && vr[i] < ignoredInputBase+mockVarCount) {
                value[i] = ignoredStringInputs_[vr[i]-ignoredInputBase].c_str();
            } else if (vr[i] >= garbageOutputBase && vr[i] < garbageOutputBase+mockVarCount) {
                value[i] = "";
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

    cppfmu::FMIReal ignoredRealInputs_[mockVarCount];
    cppfmu::FMIInteger ignoredIntegerInputs_[mockVarCount];
    cppfmu::FMIBoolean ignoredBooleanInputs_[mockVarCount];
    std::vector<cppfmu::String, cppfmu::Allocator<cppfmu::String>> ignoredStringInputs_;
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
    return cppfmu::AllocateUnique<Slave>(memory, memory);
}

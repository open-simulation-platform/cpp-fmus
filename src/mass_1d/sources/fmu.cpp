// Copyright 2015-2019, SINTEF Ocean.
// Distributed under the 3-Clause BSD License.
// (See accompanying file LICENCE.txt or copy at
// https://github.com/viproma/demo-fmus/raw/master/LICENCE.txt.)

#include "fmu-uuid.h"
#include <cppfmu_cs.hpp>
#include <cstring>

enum
{
    VR_force1   = 0,
    VR_force2   = 1,
    VR_pos      = 2,
    VR_vel      = 3,
    VR_der_vel  = 4,
    VR_mass     = 5,
    VR_COUNT    = 6
};


class Mass1DInstance : public cppfmu::SlaveInstance
{
public:
    Mass1DInstance()
    {
        Mass1DInstance::Reset();
    }

private:
    void Reset() override
    {
        for (int i = 0; i < VR_COUNT; ++i) m_values[i] = 0.0;
        m_values[VR_mass] = 1.0;
    }

    void SetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        const cppfmu::FMIReal value[]) override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            m_values[vr[i]] = value[i];
        }
    }

    void GetReal(
        const cppfmu::FMIValueReference vr[],
        std::size_t nvr,
        cppfmu::FMIReal value[]) const override
    {
        for (std::size_t i = 0; i < nvr; ++i) {
            if (vr[i] == VR_der_vel) {
                value[i] = Acceleration();
            } else {
                value[i] = m_values[vr[i]];
            }
        }
    }

    bool DoStep(
        cppfmu::FMIReal /*currentCommunicationPoint*/,
        cppfmu::FMIReal dt,
        cppfmu::FMIBoolean /*newStep*/,
        cppfmu::FMIReal& /*endOfStep*/) override
    {
        const auto acc = Acceleration();
        m_values[VR_pos] += dt*m_values[VR_vel] + 0.5*dt*dt*acc;
        m_values[VR_vel] += dt*acc;
        return true;
    }

    cppfmu::FMIReal Acceleration() const
    {
        return (m_values[VR_force1] + m_values[VR_force2]) / m_values[VR_mass];
    }

    cppfmu::FMIReal m_values[VR_COUNT];
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
    return cppfmu::AllocateUnique<Mass1DInstance>(memory);
}

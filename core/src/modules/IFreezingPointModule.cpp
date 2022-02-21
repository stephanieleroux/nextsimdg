/*!
 * @file IFreezingPointModule.cpp
 *
 * @date Feb 21, 2022
 * @author Tim Spain <timothy.spain@nersc.no>
 */

#include "include/IFreezingPointModule.hpp"

#include "include/LinearFreezing.hpp"
#include "include/UnescoFreezing.hpp"

#include <string>

namespace Module {
const std::string LINEARFREEZING = "LinearFreezing";
const std::string UNESCOFREEZING = "UnescoFreezing";

template <>
Module<Nextsim::IFreezingPoint>::map Module<Nextsim::IFreezingPoint>::functionMap = {
    { LINEARFREEZING, newImpl<Nextsim::IFreezingPoint, Nextsim::LinearFreezing> },
    { UNESCOFREEZING, newImpl<Nextsim::IFreezingPoint, Nextsim::UnescoFreezing> },
};

template <>
Module<Nextsim::IFreezingPoint>::fn Module<Nextsim::IFreezingPoint>::spf
    = functionMap.at(LINEARFREEZING);
template <>
std::unique_ptr<Nextsim::IFreezingPoint> Module<Nextsim::IFreezingPoint>::staticInstance
    = std::move(Module<Nextsim::IFreezingPoint>::spf());

template <> std::string Module<Nextsim::IFreezingPoint>::moduleName() { return "IFreezingPoint"; }

template <> Nextsim::IFreezingPoint& getImplementation<Nextsim::IFreezingPoint>()
{
    return getImplTemplate<Nextsim::IFreezingPoint, IFreezingPointModule>();
}
template <> void setImplementation<Nextsim::IFreezingPoint>(const std::string& implName)
{
    setImplTemplate<IFreezingPointModule>(implName);
}
template <> std::unique_ptr<Nextsim::IFreezingPoint> getInstance()
{
    return getInstTemplate<Nextsim::IFreezingPoint, IFreezingPointModule>();
}
} /* namespace Module */

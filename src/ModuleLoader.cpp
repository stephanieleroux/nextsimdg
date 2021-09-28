/*
 * @file ModuleLoader.cpp
 *
 * @date Sep 23, 2021
 * @author Tim Spain
 */

#include "include/ModuleLoader.hpp"
#include <memory>
#include <stdexcept>

#include "moduleLoaderHeaders.ipp"

#include "moduleLoaderFunctions.ipp"

void throwup(const std::string& module, const std::string& impl)
{
    std::string what = "ModuleLoader::init(): Module ";
    what += module + " does not have an implementation named " + impl;
    throw std::invalid_argument(what);
}

void ModuleLoader::init(const VariablesMap& map)
{
#include "moduleLoaderNames.ipp"

    // Set of all defined interfaces
    for (const auto& element: m_availableImplementationNames) {
        m_modules.insert(element.first);
    }

    // Load the named implementations from the provided map
    for (const auto& i : map) {
        std::string module = i.first;
        std::string impl = i.second;

#include "moduleLoaderAssignments.ipp"
    }
}
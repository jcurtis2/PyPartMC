/*##################################################################################################
# This file is a part of PyPartMC licensed under the GNU General Public License v3 (LICENSE file)  #
# Copyright (C) 2022 University of Illinois Urbana-Champaign                                       #
# Authors: https://github.com/open-atmos/PyPartMC/graphs/contributors                              #
##################################################################################################*/

#pragma once

#include "json_resource.hpp"
#include "pmc_resource.hpp"
#include "gas_data_parameters.hpp"
#include "pybind11/stl.h"
#include "pybind11_json/pybind11_json.hpp"

extern "C" void f_gas_data_ctor(void *ptr) noexcept;
extern "C" void f_gas_data_dtor(void *ptr) noexcept;
extern "C" void f_gas_data_len(const void *ptr, int *len) noexcept;
extern "C" void f_gas_data_from_json(const void *ptr) noexcept;
extern "C" void f_gas_data_to_json(const void *ptr) noexcept;
extern "C" void f_gas_data_spec_by_name(const void *ptr, int *value, const char *name_data,
    const int *name_size) noexcept;
extern "C" void f_gas_data_spec_name_by_index(const void *ptr, const int *i_spec,
    char *name_data) noexcept;

struct GasData {
    PMCResource ptr;
    const nlohmann::json json;

    GasData(const pybind11::tuple &tpl) :
        ptr(f_gas_data_ctor, f_gas_data_dtor),
        json(tpl)
    {
        auto json_array = nlohmann::json::array();
        for (const auto item : tpl)
            json_array.push_back(nlohmann::json::object({{
                item.cast<std::string>(),
                nlohmann::json::array()
            }}));

        JSONResourceGuard<InputJSONResource> guard(json_array);
        f_gas_data_from_json(this->ptr.f_arg());
        guard.check_parameters();
    }

    GasData() :
        ptr(f_gas_data_ctor, f_gas_data_dtor)
    {}

    static auto __str__(const GasData &self) {
        return self.json.dump();
    }   

    static std::size_t __len__(const GasData &self) {
        int len;
        f_gas_data_len(
            self.ptr.f_arg(),
            &len
        );
        return len;
    }

    static auto spec_by_name(const GasData &self, const std::string &name) {
        int value;
        const int name_size = name.size();
        f_gas_data_spec_by_name(
            self.ptr.f_arg(),
            &value,
            name.c_str(),
            &name_size
        );
        if (value == 0)
            throw std::runtime_error("Element not found.");
        return value - 1;
    }

    static auto names(const GasData &self) {

        int len;
        f_gas_data_len(
            self.ptr.f_arg(),
            &len
        );

        char name[GAS_NAME_LEN];
        auto names = pybind11::tuple(len);
        for (int idx = 0; idx < len; idx++) {
             f_gas_data_spec_name_by_index(
                 self.ptr.f_arg(),
                 &idx,
                 name
            );
            names[idx] = std::string(name);
        }
        return names;
    }
};


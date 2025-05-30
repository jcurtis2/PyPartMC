/*##################################################################################################
# This file is a part of PyPartMC licensed under the GNU General Public License v3 (LICENSE file)  #
# Copyright (C) 2022 University of Illinois Urbana-Champaign                                       #
# Authors: https://github.com/open-atmos/PyPartMC/graphs/contributors                              #
##################################################################################################*/

#pragma once

#include "json_resource.hpp"
#include "pmc_resource.hpp"
#include "aero_data.hpp"
#include "aero_dist.hpp"
#include "env_state.hpp"
#include "gas_data.hpp"

extern "C" void f_scenario_ctor(void *ptr) noexcept;
extern "C" void f_scenario_dtor(void *ptr) noexcept;
extern "C" void f_scenario_from_json(
    const void *gas_data,
    const void *aero_data,
    const void *scenario
)
#ifndef __linux__
    noexcept
#endif
;
extern "C" void f_scenario_loss_rate(
    const void *scenario,
    const double *vol,
    const double *density,
    const void *aero_data,
    const void *env_state,
    double *rate
) noexcept;
extern "C" void f_scenario_loss_rate_dry_dep(
    const double *vol,
    const double *density,
    const void *aero_data,
    const void *env_state,
    double *rate
) noexcept;
extern "C" void f_scenario_init_env_state(
    const void *scenario,
    void *env_state,
    const double *time
) noexcept;
extern "C" void f_scenario_aero_dist_emission(
    const void *scenario,
    const void *aero_dist,
    const int *idx
) noexcept;
extern "C" void f_scenario_aero_emission_n_times(
    const void *scenario,
    int *n_times
) noexcept;
extern "C" void f_scenario_emission_rates(
    const void *scenario,
    double *rates,
    const int *len
) noexcept;
extern "C" void f_scenario_emission_time(
    const void *scenario,
    double *times,
    const int *len
) noexcept;
extern "C" void f_scenario_aero_dist_background(
    const void *scenario,
    const void *aero_dist,
    const int *idx
) noexcept;
extern "C" void f_scenario_aero_background_n_times(
    const void *scenario,
    int *n_times
) noexcept;
extern "C" void f_scenario_aero_background_rate_scale(
    const void *scenario,
    double *rates,
    const int *len
) noexcept;
extern "C" void f_scenario_aero_background_time(
    const void *scenario,
    double *times,
    const int *len
) noexcept;

struct Scenario {
    PMCResource ptr;
    const nlohmann::json json;

    Scenario(
        const GasData &gas_data,
        const AeroData &aero_data,
        const nlohmann::json &json
    ) :
        ptr(f_scenario_ctor, f_scenario_dtor),
        json(json)
    {
        // TODO #317 - repeat analogous checks for rates
        for (auto &prof: std::set<std::string>({"height", "temp", "pressure"})) {
            auto prof_key = prof + "_profile";
            if (json.find(prof_key) != json.end()) {
                if (!json[prof_key].is_array() || json[prof_key].size() != 2)
                    throw std::runtime_error(prof_key + " expected to be a 2-element list (of single-element dictionaries)");
                for (auto i=0; i<2; ++i)
                    if (!json[prof_key][i].is_object() || json[prof_key][i].size() != 1)
                        throw std::runtime_error(prof_key + " expected to contain only single-element dicts");
                {
                    auto elem = json[prof_key][0];
                    if (elem.find("time") == elem.end())
                        throw std::runtime_error(prof_key + " first element is expeced to be a single-element dict with 'time' key");
                }
                {
                    auto elem = json[prof_key][1];
                    if (elem.find(prof) == elem.end())
                        throw std::runtime_error(prof_key + " second element is expeced to be a single-element dict with '" + prof + "' key");
                }
                if (
                    !json[prof_key][0]["time"].is_array() || 
                    !json[prof_key][1][prof].is_array() || 
                    json[prof_key][0]["time"].size() != json[prof_key][1][prof].size()
                )
                    throw std::runtime_error(prof_key + " 'time' and '" + prof + "' arrays do not have matching size");
            }
        }

        JSONResourceGuard<InputJSONResource> guard(json, "dist", "mode_name");
        f_scenario_from_json(
            gas_data.ptr.f_arg(),
            aero_data.ptr.f_arg(),
            this->ptr.f_arg()
        );
        guard.check_parameters();
    }

    static auto __str__(const Scenario &self) {
        return self.json.dump();
    }   

    static void init_env_state(
        const Scenario &self,
        EnvState &env_state,
        const double time
    ) {
        f_scenario_init_env_state(
            self.ptr.f_arg(),
            env_state.ptr.f_arg_non_const(),
            &time
        );
    }

    static AeroDist* get_dist(const Scenario &self, const AeroData &aero_data, const int &idx) {
//        if (idx < 0 || idx >= AeroDist::get_n_mode(self))
//            throw std::out_of_range("Index out of range");
        AeroDist *ptr = new AeroDist();
        f_scenario_aero_dist_emission(self.ptr.f_arg(), ptr, &idx);

        return ptr;
    }

    static auto get_emissions_n_times(const Scenario &self) {
        int len;
        f_scenario_aero_emission_n_times(self.ptr.f_arg(), &len);

        return len;
    }

    static auto emission_rate_scale(const Scenario &self) {
        int len;

        f_scenario_aero_emission_n_times(self.ptr.f_arg(), &len);
        std::valarray<double> rates(len);
        f_scenario_emission_rates(
            self.ptr.f_arg(),
            begin(rates),
            &len
        );

        return rates;
    }

    static auto emission_time(const Scenario &self) {
        int len;

        f_scenario_aero_emission_n_times(self.ptr.f_arg(), &len);
        std::valarray<double> times(len);
        f_scenario_emission_time(
            self.ptr.f_arg(),
            begin(times),
            &len
        );

        return times;
    }

    static AeroDist* get_aero_background_dist(const Scenario &self, const AeroData &aero_data, const int &idx) {
        AeroDist *ptr = new AeroDist();
        f_scenario_aero_dist_background(self.ptr.f_arg(), ptr, &idx);

        return ptr;
    }

    static auto get_aero_dilution_n_times(const Scenario &self) {
        int len;
        f_scenario_aero_background_n_times(self.ptr.f_arg(), &len);

        return len;
    }

    static auto aero_dilution_rate(const Scenario &self) {
        int len;

        f_scenario_aero_background_n_times(self.ptr.f_arg(), &len);
        std::valarray<double> rates(len);
        f_scenario_aero_background_rate_scale(
            self.ptr.f_arg(),
            begin(rates),
            &len
        );

        return rates;
    }

    static auto aero_dilution_time(const Scenario &self) {
        int len;

        f_scenario_aero_background_n_times(self.ptr.f_arg(), &len);
        std::valarray<double> times(len);
        f_scenario_aero_background_time(
            self.ptr.f_arg(),
            begin(times),
            &len
        );

        return times;
    }

};

double loss_rate(
    const Scenario &scenario,
    const double vol,
    const double density,
    const AeroData &aero_data,
    const EnvState &env_state
);

double loss_rate_dry_dep(
    const double vol,
    const double density,
    const AeroData &aero_data,
    const EnvState &env_state
);

####################################################################################################
# This file is a part of PyPartMC licensed under the GNU General Public License v3 (LICENSE file)  #
# Copyright (C) 2022-2025 University of Illinois Urbana-Champaign                                  #
# Authors: https://github.com/open-atmos/PyPartMC/graphs/contributors                              #
####################################################################################################

import copy
import platform

import numpy as np
import pytest

import PyPartMC as ppmc
from PyPartMC import si

from .test_aero_data import AERO_DATA_CTOR_ARG_MINIMAL

AERO_MODE_CTOR_LOG_NORMAL = {
    "test_mode": {
        "mass_frac": [{"H2O": [1]}],
        "diam_type": "geometric",
        "mode_type": "log_normal",
        "num_conc": 100 / si.m**3,
        "geom_mean_diam": 2 * si.um,
        "log10_geom_std_dev": np.log10(1.6),
    }
}


AERO_MODE_CTOR_LOG_NORMAL_FULL = {
    "test_mode": {
        "mass_frac": [{"SO4": [1]}],
        "diam_type": "geometric",
        "mode_type": "log_normal",
        "num_conc": 100 / si.m**3,
        "geom_mean_diam": 2 * si.um,
        "log10_geom_std_dev": np.log10(1.6),
    }
}

AERO_MODE_CTOR_LOG_NORMAL_COAGULATION = {
    "test_mode": {
        "mass_frac": [{"SO4": [1]}],
        "diam_type": "geometric",
        "mode_type": "log_normal",
        "num_conc": 1e12 / si.m**3,
        "geom_mean_diam": 2 * si.um,
        "log10_geom_std_dev": np.log10(1.6),
    }
}

AERO_MODE_CTOR_SAMPLED = {
    "test_mode": {
        "mass_frac": [{"H2O": [1]}],
        "diam_type": "geometric",
        "mode_type": "sampled",
        "size_dist": [
            {"diam": [1, 2, 3, 4]},
            {"num_conc": [100, 200, 300]},
        ],
    }
}

AERO_MODE_CTOR_EXP = {
    "test_mode": {
        "mass_frac": [{"H2O": [1]}],
        "diam_type": "geometric",
        "mode_type": "exp",
        "num_conc": 1e9 / si.m**3,
        "diam_at_mean_vol": 20 * si.um,
    }
}


class TestAeroMode:
    @staticmethod
    def test_ctor():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        # act
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)

        # assert
        assert sut is not None

    @staticmethod
    def test_set_vol_frac():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)
        vals = [1.0]

        # act
        sut.vol_frac = vals

        # assert
        assert sut.vol_frac == vals

    @staticmethod
    def test_set_num_conc():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)
        val = 1234

        # act
        sut.num_conc = val

        # assert
        assert sut.num_conc == val

    @staticmethod
    def test_set_gsd():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)
        val = 1.234

        # act
        sut.gsd = val

        # assert
        assert sut.gsd == val

    @staticmethod
    def test_set_char_radius():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)
        val = 1e-9

        # act
        sut.char_radius = val

        # assert
        assert sut.char_radius == val

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_set_vol_frac_invalid():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)

        # act
        try:
            sut.vol_frac = [1.0, 0.0]
        except RuntimeError:
            return

        # assert
        assert False

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_set_sample_invalid():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)

        # act
        try:
            sut.set_sample([1, 10], [10, 10])
        except RuntimeError:
            return

        # assert
        assert False

    @staticmethod
    @pytest.mark.parametrize(
        "modes",
        (
            {
                "test_mono": {
                    "mass_frac": [
                        {"A": [0.2]},
                        {"B": [0.3]},
                        {"C": [0.1]},
                        {"D": [0.4]},
                    ],
                    "mode_type": "mono",
                    "num_conc": 100 / si.m**3,
                    "diam": 2 * si.um,
                }
            },
            {
                "test_logn": {
                    "mass_frac": [
                        {"A": [0.3]},
                        {"B": [0.1]},
                        {"C": [0.1]},
                        {"D": [0.5]},
                    ],
                    "mode_type": "log_normal",
                    "num_conc": 100 / si.m**3,
                    "geom_mean_diam": 2 * si.um,
                    "log10_geom_std_dev": 1.6,
                }
            },
            {
                "test_exp": {
                    "mass_frac": [
                        {"A": [0.6]},
                        {"B": [0.1]},
                        {"C": [0.1]},
                        {"D": [0.2]},
                    ],
                    "mode_type": "exp",
                    "num_conc": 100 / si.m**3,
                    "diam_at_mean_vol": 2 * si.um,
                }
            },
        ),
    )
    @pytest.mark.parametrize("diam_type", ("geometric", "mobility"))
    def test_from_json_mono(modes, diam_type):
        # arrange
        densities = [
            {"A": [1234 * si.kg / si.m**3, 1, 18e-3 * si.kg / si.mol, 0]},
            {"B": [4321 * si.kg / si.m**3, 1, 18e-3 * si.kg / si.mol, 0]},
            {"C": [1111 * si.kg / si.m**3, 1, 18e-3 * si.kg / si.mol, 0]},
            {"D": [2222 * si.kg / si.m**3, 1, 18e-3 * si.kg / si.mol, 0]},
        ]
        aero_data = ppmc.AeroData(densities)
        for mode in modes.values():
            mode["diam_type"] = diam_type
            if diam_type == "mobility":
                mode["pressure"] = 1000 * si.hPa
                if "temp" not in mode:
                    mode["temp"] = 299 * si.K

        # act
        sut = ppmc.AeroMode(aero_data, modes)

        # assert
        mode = modes[tuple(modes.keys())[0]]
        assert sut.num_conc == mode["num_conc"]
        assert sut.type == mode["mode_type"]

    @staticmethod
    def test_get_name():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)

        # act
        name = sut.name

        # assert
        assert tuple(AERO_MODE_CTOR_LOG_NORMAL.keys())[0] == name

    @staticmethod
    def test_set_name():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_LOG_NORMAL)
        val = "abc"
        assert sut.name != val

        # act
        sut.name = val

        # assert
        assert sut.name == val

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_ctor_fails_with_multiple_modes():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        fishy_ctor_arg = copy.deepcopy(AERO_MODE_CTOR_LOG_NORMAL)
        fishy_ctor_arg["xxx"] = fishy_ctor_arg["test_mode"]

        # act
        with pytest.raises(Exception) as exc_info:
            ppmc.AeroMode(aero_data, fishy_ctor_arg)

        # assert
        assert (
            str(exc_info.value)
            == "Single-element dict expected with mode name as key and mode params dict as value"
        )

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_ctor_fails_with_nonunique_mass_fracs():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        fishy_ctor_arg = copy.deepcopy(AERO_MODE_CTOR_LOG_NORMAL)
        fishy_ctor_arg["test_mode"]["mass_frac"].append(
            AERO_MODE_CTOR_LOG_NORMAL["test_mode"]["mass_frac"][0]
        )

        # act
        with pytest.raises(Exception) as exc_info:
            ppmc.AeroMode(aero_data, fishy_ctor_arg)

        # assert
        assert str(exc_info.value) == "mass_frac keys must be unique"

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_fixed_segfault_case_on_circular_reference():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        fishy_ctor_arg = copy.deepcopy(AERO_MODE_CTOR_LOG_NORMAL)
        fishy_ctor_arg["test_mode"]["mass_frac"].append(
            fishy_ctor_arg["test_mode"]["mass_frac"]
        )

        # act
        with pytest.raises(TypeError) as exc_info:
            ppmc.AeroMode(aero_data, fishy_ctor_arg)

        # assert
        assert "incompatible constructor arguments" in str(exc_info.value)

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_sampled_without_size_dist():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        fishy_ctor_arg = copy.deepcopy(AERO_MODE_CTOR_LOG_NORMAL)
        fishy_ctor_arg["test_mode"]["mode_type"] = "sampled"

        # act
        with pytest.raises(Exception) as exc_info:
            ppmc.AeroMode(aero_data, fishy_ctor_arg)

        # assert
        assert str(exc_info.value) == "size_dist key must be set for mode_type=sampled"

    @staticmethod
    @pytest.mark.parametrize(
        "fishy",
        (
            None,
            [],
            [{}, {}, {}],
            [{}, []],
            [{"diam": None}, {}],
            [{"num_conc": None}, {}],
            [{"diam": None, "": None}, {}],
            [{"num_conc": None, "": None}, {}],
        ),
    )
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_sampled_with_fishy_size_dist(fishy):
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        fishy_ctor_arg = copy.deepcopy(AERO_MODE_CTOR_LOG_NORMAL)
        fishy_ctor_arg["test_mode"]["mode_type"] = "sampled"
        fishy_ctor_arg["test_mode"]["size_dist"] = fishy

        # act
        with pytest.raises(Exception) as exc_info:
            ppmc.AeroMode(aero_data, fishy_ctor_arg)

        # assert
        assert (
            str(exc_info.value)
            == "size_dist value must be an iterable of two single-element dicts"
            + " (first with 'diam', second with 'num_conc' as keys)"
        )

    @staticmethod
    @pytest.mark.skipif(platform.machine() == "arm64", reason="TODO #348")
    def test_sampled_with_diam_of_different_len_than_num_conc():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        fishy_ctor_arg = copy.deepcopy(AERO_MODE_CTOR_LOG_NORMAL)
        fishy_ctor_arg["test_mode"]["mode_type"] = "sampled"
        fishy_ctor_arg["test_mode"]["size_dist"] = [
            {"diam": [1, 2, 3]},
            {"num_conc": [1, 2, 3]},
        ]

        # act
        with pytest.raises(Exception) as exc_info:
            ppmc.AeroMode(aero_data, fishy_ctor_arg)

        # assert
        assert (
            str(exc_info.value)
            == "size_dist['num_conc'] must have len(size_dist['diam'])-1 elements"
        )

    @staticmethod
    def test_sampled():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        # act
        sut = ppmc.AeroMode(aero_data, AERO_MODE_CTOR_SAMPLED)

        # assert
        assert sut.type == "sampled"
        assert sut.num_conc == np.sum(
            AERO_MODE_CTOR_SAMPLED["test_mode"]["size_dist"][1]["num_conc"]
        )
        assert (
            sut.sample_num_conc
            == AERO_MODE_CTOR_SAMPLED["test_mode"]["size_dist"][1]["num_conc"]
        )
        assert (
            np.array(sut.sample_radius) * 2
            == AERO_MODE_CTOR_SAMPLED["test_mode"]["size_dist"][0]["diam"]
        ).all()

    @staticmethod
    def test_set_sample():
        # arrange
        aero_data = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        diams = [1, 2, 3, 4]
        num_concs = [100, 200, 300]
        sut = ppmc.AeroMode(
            aero_data,
            {
                "test_mode": {
                    "mass_frac": [{"H2O": [1]}],
                    "diam_type": "geometric",
                    "mode_type": "sampled",
                    "size_dist": [
                        {"diam": diams},
                        {"num_conc": num_concs},
                    ],
                }
            },
        )
        num_conc_orig = sut.num_conc
        # act
        diams = [0.5 * x for x in diams]
        num_concs = [2 * x for x in num_concs]
        sut.set_sample(diams, num_concs)
        # assert
        assert sut.num_conc == np.sum(num_concs)
        assert sut.sample_num_conc == num_concs
        assert (np.array(sut.sample_radius) * 2 == diams).all()
        assert sut.num_conc == num_conc_orig * 2

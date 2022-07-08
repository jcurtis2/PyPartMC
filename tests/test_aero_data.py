####################################################################################################
# This file is a part of PyPartMC licensed under the GNU General Public License v3 (LICENSE file)  #
# Copyright (C) 2022 University of Illinois Urbana-Champaign                                       #
# Authors: https://github.com/open-atmos/PyPartMC/graphs/contributors                              #
####################################################################################################

import PyPartMC as ppmc
from PyPartMC import si

AERO_DATA_CTOR_ARG_MINIMAL = (
    {"H2O": [1000 * si.kg / si.m**3, 1, 18e-3 * si.kg / si.mol, 0]},
)


class TestAeroData:
    @staticmethod
    def test_ctor():
        # arrange

        # act
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        # assert
        assert sut is not None

    @staticmethod
    def test_spec_by_name_found():
        #arrange
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        #act
        value = sut.spec_by_name("H2O")

        #assert
        assert value == 0

    @staticmethod
    def test_spec_by_name_not_found():
        #arrange
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        #act and assert
        try:
            _ = sut.spec_by_name("XXX")
            assert False
        except RuntimeError as error:
            assert str(error) == "Element not found."

    @staticmethod
    def test_len():
        #arrange
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)

        #act
        value = len(sut)

        #assert
        assert value == len(AERO_DATA_CTOR_ARG_MINIMAL)

    @staticmethod
    def test_frac_dim():
        #arrange
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        value = 3

        #act
        sut.frac_dim = value

        #assert
        assert value == sut.frac_dim

    @staticmethod
    def test_vol_fill_factor():
        #arrange
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        value = 1

        #act
        sut.vol_fill_factor = value

        #assert
        assert value == sut.vol_fill_factor

    @staticmethod
    def test_prime_radius():
        #arrange
        sut = ppmc.AeroData(AERO_DATA_CTOR_ARG_MINIMAL)
        value = 44

        #act
        sut.prime_radius = value

        #assert
        assert value == sut.prime_radius

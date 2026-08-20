// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

// This function takes a filename and matches it with the filenames of
// the volumes found at 'https://klacansky.com/open-scivis-datasets/'
// and returns the spacing for it.

export function get(filename) {
    filename = filename.split('/').slice(-1)[0];
    let map = new Map([["3d_neurons_15_sept_2016_2048x2048x1718_uint16.raw", Qt.vector3d(0.267345,0.267345,0.5)],
      ["aneurism_256x256x256_uint8.raw", Qt.vector3d(1,1,1)],
      ["backpack_512x512x373_uint16.raw", Qt.vector3d(0.9766,0.9766,1.25)],
      ["beechnut_1024x1024x1546_uint16.raw", Qt.vector3d(2e-05,2e-05,2e-05)],
      ["blunt_fin_256x128x64_uint8.raw", Qt.vector3d(1,0.75,1)],
      ["bonsai_256x256x256_uint8.raw", Qt.vector3d(1,1,1)],
      ["boston_teapot_256x256x178_uint8.raw", Qt.vector3d(1,1,1)],
      ["bunny_512x512x361_uint16.raw", Qt.vector3d(0.337891,0.337891,0.5)],
      ["csafe_heptane_302x302x302_uint8.raw", Qt.vector3d(1,1,1)],
      ["pig_heart_2048x2048x2612_int16.raw", Qt.vector3d(0.03557,0.03557,0.03557)],
      ["carp_256x256x512_uint16.raw", Qt.vector3d(0.78125,0.390625,1)],
      ["chameleon_1024x1024x1080_uint16.raw", Qt.vector3d(0.09228515625,0.09228515625,0.105)],
      ["present_492x492x442_uint16.raw", Qt.vector3d(1,1,1)],
      ["christmas_tree_512x499x512_uint16.raw", Qt.vector3d(1,1,1)],
      ["prone_512x512x463_uint16.raw", Qt.vector3d(0.625,0.625,1.0)],
      ["duct_193x194x1000_float32.raw", Qt.vector3d(1,1,1)],
      ["engine_256x256x128_uint8.raw", Qt.vector3d(1,1,1)],
      ["foot_256x256x256_uint.raw", Qt.vector3d(1,1,1)],
      ["isotropic_pressure_4096x4096x4096_float32.raw", Qt.vector3d(1,1,1)],
      ["frog_256x256x44_uint8.raw", Qt.vector3d(0.5,0.5,1)],
      ["fuel_64x64x64_uint8.raw", Qt.vector3d(1,1,1)],
      ["vis_male_128x256x256_uint8.raw", Qt.vector3d(1.57774,0.995861,1.00797)],
      ["vertebra_512x512x512_uint16.raw", Qt.vector3d(0.1953,0.1953,0.1953)],
      ["mri_ventricles_256x256x124_uint8.raw", Qt.vector3d(0.9,0.9,0.9)],
      ["mrt_angio_416x512x112_uint16.raw", Qt.vector3d(0.412,0.412,0.412)],
      ["hcci_oh_560x560x560_float32.raw", Qt.vector3d(1,1,1)],
      ["hydrogen_atom_128x128x128_uint8.raw", Qt.vector3d(1,1,1)],
      ["tacc_turbulence_256x256x256_float32.raw", Qt.vector3d(1,1,1)],
      ["jicf_q_1408x1080x1100_float32.raw", Qt.vector3d(1,1,1)],
      ["kingsnake_1024x1024x795_uint8.raw", Qt.vector3d(0.03174,0.03174,0.0688)],
      ["statue_leg_341x341x93_uint8.raw", Qt.vector3d(1,1,4)],
      ["lobster_301x324x56_uint8.raw", Qt.vector3d(1,1,1.4)],
      ["mri_woman_256x256x109_uint16.raw", Qt.vector3d(1,1,1.5)],
      ["magnetic_reconnection_512x512x512_float32.raw", Qt.vector3d(1,1,1)],
      ["marschner_lobb_41x41x41_uint8.raw", Qt.vector3d(1,1,1)],
      ["neghip_64x64x64_uint8.raw", Qt.vector3d(1,1,1)],
      ["neocortical_layer_1_axons_1464x1033x76_uint8.raw", Qt.vector3d(1,1,3.4)],
      ["marmoset_neurons_1024x1024x314_uint8.raw", Qt.vector3d(0.497,0.497,1.5)],
      ["nucleon_41x41x41_uint8.raw", Qt.vector3d(1,1,1)],
      ["pancreas_240x512x512_int16.raw", Qt.vector3d(1.16,1.0,1.0)],
      ["pawpawsaurus_958x646x1088_uint16.raw", Qt.vector3d(0.2275,0.2275,0.2275)],
      ["miranda_1024x1024x1024_float32.raw", Qt.vector3d(1,1,1)],
      ["richtmyer_meshkov_2048x2048x1920_uint8.raw", Qt.vector3d(1,1,1)],
      ["rotstrat_temperature_4096x4096x4096_float32.raw", Qt.vector3d(1,1,1)],
      ["shockwave_64x64x512_uint8.raw", Qt.vector3d(1,1,1)],
      ["silicium_98x34x34_uint8.raw", Qt.vector3d(1,1,1)],
      ["skull_256x256x256_uint8.raw", Qt.vector3d(1,1,1)],
      ["spathorhynchus_1024x1024x750_uint16.raw", Qt.vector3d(0.0215,0.0215,0.047)],
      ["stag_beetle_832x832x494_uint16.raw", Qt.vector3d(1,1,1)],
      ["stent_512x512x174_uint16.raw", Qt.vector3d(0.8398,0.8398,3.2)],
      ["synthetic_truss_with_five_defects_1200x1200x1200_float32.raw", Qt.vector3d(1,1,1)],
      ["tooth_103x94x161_uint8.raw", Qt.vector3d(1,1,1)],
      ["dns_10240x7680x1536_float64.raw", Qt.vector3d(1,1,1)],
      ["woodbranch_2048x2048x2048_uint16.raw", Qt.vector3d(1.8e-05,1.8e-05,1.8e-05)],
      ["zeiss_680x680x680_uint8.raw", Qt.vector3d(1,1,1)]]);
    let scale = map.get(filename);
    return scale === undefined ? Qt.vector3d(1, 1, 1) : scale;
}

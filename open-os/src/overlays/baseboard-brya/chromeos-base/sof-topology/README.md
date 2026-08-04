This file should always contain information regarding which branch/commit
the firmware was built from.

CAVS2.5 001 drop stable release, with dynamic pipeline removed.

repo      : https://chrome-internal.googlesource.com/chromeos/third_party/sound-open-firmware-private/
branch    : cavs2.5-001-drop-stable
commit id : 9a87832712956e9e32f9f94da3ad52bb0212b681

Once built, copy and rename topology files as the following map:
[DRC/EQ-integrated candidates]
tools/build_tools/topology/sof-adl-cs35l41-drceq.tplg                   renamed to sof-adl-cs35l41.tplg
tools/build_tools/topology/sof-adl-max98357a-rt5682-drceq.tplg          renamed to sof-adl-max98357a-rt5682.tplg
tools/build_tools/topology/sof-adl-max98360a-rt5682-drceq.tplg          renamed to sof-adl-max98360a-rt5682.tplg
tools/build_tools/topology/sof-adl-max98390-rt5682-drceq.tplg           renamed to sof-adl-max98390-rt5682.tplg
tools/build_tools/topology/sof-adl-max98390-ssp2-rt5682-ssp0-drceq.tplg renamed to sof-adl-max98390-ssp2-rt5682-ssp0.tplg
tools/build_tools/topology/sof-adl-rt1015-nau8825-drceq.tplg            renamed to sof-adl-rt1015-nau8825.tplg
tools/build_tools/topology/sof-adl-rt1019-nau8825-drceq.tplg            renamed to sof-adl-rt1019-nau8825.tplg
tools/build_tools/topology/sof-adl-rt1019-rt5682-drceq.tplg             renamed to sof-adl-rt1019-rt5682.tplg
tools/build_tools/topology/sof-adl-rt5650-drceq.tplg                    renamed to sof-adl-rt5650.tplg
// TODO(b/340951837): get back DRC/EQ after fix
tools/build_tools/topology/sof-adl-max98360a-nau8825.tplg               renamed to sof-adl-max98360a-nau8825.tplg
// TODO(b/382634062): get back DRC/EQ after fix
tools/build_tools/topology/sof-adl-max98360a-da7219.tplg                renamed to sof-adl-max98360a-da7219.tplg
[PDM1]
tools/build_tools/topology/sof-adl-max98360a-rt5682-2way-pdm1.tplg      renamed to pdm1/sof-adl-max98360a-rt5682-2way.tplg

Copy the following topology files:
tools/build_tools/topology/sof-adl-max98357a-rt5682-4ch.tplg
tools/build_tools/topology/sof-adl-max98360a-rt5682-2way.tplg
tools/build_tools/topology/sof-adl-max98360a-rt5682-4ch.tplg
tools/build_tools/topology/sof-adl-max98373-nau8825.tplg
tools/build_tools/topology/sof-adl-nau8318-nau8825.tplg
tools/build_tools/topology/sof-adl-nau8825.tplg
tools/build_tools/topology/sof-adl-rt5682.tplg
tools/build_tools/topology/sof-adl-rt711.tplg
tools/build_tools/topology/sof-adl-sdw-max98373-rt5682.tplg

Md5sum
3ae2fe040eab3f0184c683bc067da6e5  sof-adl-cs35l41.tplg
fa6667f1d2815e074c3b3e42d6f31d25  sof-adl-max98357a-rt5682-4ch.tplg
f5d4ee4f615bb6e377c46e6688c2c894  sof-adl-max98357a-rt5682.tplg
e313e87fc8141bc62d9fb67853546685  sof-adl-max98360a-da7219.tplg
e2104dd408abe169e11407dafb0b90b4  sof-adl-max98360a-nau8825.tplg
ae727de38b8fd9389dc62fd90e0f221d  sof-adl-max98360a-rt5682-2way.tplg
76bf8c5e3061f86e122335e0559300e3  sof-adl-max98360a-rt5682-4ch.tplg
af22fd43229ed6a849768a8163716572  sof-adl-max98360a-rt5682.tplg
60049a9ba3bf189daa82be06986f8910  sof-adl-max98373-nau8825.tplg
171693a401c6acbe3e176b34120ea44e  sof-adl-max98390-rt5682.tplg
77a02c4d98858f21a833fdccbb1890d5  sof-adl-max98390-ssp2-rt5682-ssp0.tplg
e2104dd408abe169e11407dafb0b90b4  sof-adl-nau8318-nau8825.tplg
e822262d5fee4f16d440a9f939b821cf  sof-adl-nau8825.tplg
ec09adf23d8529dd7979c64beecfad72  sof-adl-rt1015-nau8825.tplg
118c702b2e714a406c6e3df978b85fac  sof-adl-rt1019-nau8825.tplg
6da21c3d653cb96f3cdbe8725d9a8758  sof-adl-rt1019-rt5682.tplg
aac137f4c2d2ef39a42026404cf729a3  sof-adl-rt5650.tplg
1680e57948f92e60afdacb3882769bc6  sof-adl-rt5682.tplg
db14e0a21e66b94fb61abf233cb77ca6  sof-adl-rt711.tplg
cc758a80048fec38bdc622859d4493a8  sof-adl-sdw-max98373-rt5682.tplg
8959f1d7c734df128b120e2cc223bc3c  pdm1/sof-adl-max98360a-rt5682-2way.tplg

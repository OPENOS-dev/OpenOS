This file should always contain information regarding which
branch/commit the firmware was built from.

CAVS2.5 release with HDA topology

repo : https://chrome-internal.googlesource.com/chromeos/third_party/sound-open-firmware-private
branch : cavs2.5-001-drop-stable
commit id : 3c70ca806fb6cab07be2d495ae3876020bab43e4

Once built, copy and rename topology files as the following map:
tools/build_tools/topology/sof-adl-max98360a-da7219-drceq.tplg         renamed to sof-adl-max98360a-da7219.tplg
tools/build_tools/topology/sof-adl-max98360a-nau8825-drceq.tplg        renamed to sof-adl-max98360a-nau8825.tplg
tools/build_tools/topology/sof-adl-max98360a-rt5682-drceq.tplg         renamed to sof-adl-max98360a-rt5682.tplg
tools/build_tools/topology/sof-adl-rt1019-rt5682-drceq.tplg            renamed to sof-adl-rt1019-rt5682.tplg
tools/build_tools/topology/sof-adl-rt1019-rt5682-ramptime700.tplg      renamed to riven/sof-adl-rt1019-rt5682.tplg
tools/build_tools/topology/sof-adl-rt5650-ramptime700.tplg             renamed to craaskov/sof-adl-rt5650.tplg

MD5SUMs
9573b8a59e8c93bdad0fb9b64efdd002  sof-adl-max98360a-da7219.tplg
81697d13d9f10d0008fe979c2df620cc  sof-adl-max98360a-nau8825.tplg
af22fd43229ed6a849768a8163716572  sof-adl-max98360a-rt5682.tplg
6da21c3d653cb96f3cdbe8725d9a8758  sof-adl-rt1019-rt5682.tplg
1c3aa0cfa50f8112972cb300f6e15b06  sof-adl-rt5650.tplg
1680e57948f92e60afdacb3882769bc6  sof-adl-rt5682.tplg
9fc2792b84cbe7e85d99d19f29d84c59  sof-hda-generic-2ch.tplg
cccbc85342c3f63632477a93c918a00b  sof-hda-generic-4ch.tplg
51d2efeb51778adad2bcc964ff80aa40  craaskov/sof-adl-rt5650.tplg
cea3d5bfe60716e211b8b2531516f5b9  riven/sof-adl-rt1019-rt5682.tplg

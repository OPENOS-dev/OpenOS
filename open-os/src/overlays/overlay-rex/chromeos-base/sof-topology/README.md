This file should always contain information regarding which
branch/commit the firmware was built from.

MTL-008 release with audio latency fixes

repo : https://github.com/thesofproject/sof
branch : mtl-008-drop-stable
commid id: fe4bfb8853f8a2151e2c89cda171ab54b1f4c5cc

Copy and rename topology build for this bundle:
${SOF}/tools/build_tools/topology/topology2/production/...
    sof-mtl-max98357a-rt5682-ssp2-ssp0.tplg -> sof-mtl-max98360a-rt5682.tplg
    sof-mtl-rt1019-rt5682.tplg -> sof-mtl-rt1019-rt5682.tplg
    sof-mtl-rt5650.tplg -> sof-mtl-rt5650.tplg
    sof-mtl-sdw-cs42l42-l0-max98363-l2.tplg -> sof-mtl-sdw-cs42l42-l0-max98363-l2.tplg
    sof-mtl-max98357a-rt5682-ssp2-ssp0-2ch-pdm1.tplg -> pdm1/sof-mtl-max98360a-rt5682.tplg

MD5sums:
1034f5c1d07b6347d1d69f601c086a80  sof-mtl-max98360a-rt5682.tplg
1034f5c1d07b6347d1d69f601c086a80  sof-mtl-rt1019-rt5682.tplg
c9992c736e2b4ac8b0fd52343dc6235d  sof-mtl-rt5650.tplg
8f79992f6a27cc8321278dbc9a083157  sof-mtl-sdw-cs42l42-l0-max98363-l2.tplg
76b501155adf7af5322e76ce5aa8ecf1  pdm1/sof-mtl-max98360a-rt5682.tplg

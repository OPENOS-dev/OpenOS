This file should contain the repo/branch/commit information about the repository
from which the included topology file has been created.

repo      : https://chrome-internal-review.googlesource.com/q/project:chromeos%252Fthird_party%252Fsound-open-firmware-private
branch    : cros-internal/cavs2.5-001-drop-stable
commit id : 183f44ac4fbeca2f7db8b0cecea191c45cda600d

Copy the following topology builds and rename them into the bundle:
cp ${SOF}/tools/build_tools/topology/sof-tgl-max98357a-rt5682-pdm1.tplg ./pdm1/sof-tgl-max98357a-rt5682.tplg
cp ${SOF}/tools/build_tools/topology/sof-tgl-max98357a-rt5682-pdm1-drceq.tplg ./pdm1-drceq/sof-tgl-max98357a-rt5682.tplg

md5sums
5b1ec8bd48aaafcb282e8e3505e9e551  sof-tgl-max98357a-rt5682.tplg
3b435c4df7488202f0b539813a000671  sof-tgl-max98373-rt5682.tplg
f3bd3405de0d676464317b7d475a8379  sof-tgl-rt1011-rt5682.tplg
a7d647e50eb651ad02d6d778d5d2ac7c  sof-tgl-rt5682-ssp0-max98373-ssp2.tplg
035903a7623b9c98c75390cc4cffae46  sof-tgl-sdw-max98373-rt5682.tplg
684b5382062514c568b54e7fb5ad92f5  pdm1/sof-tgl-max98357a-rt5682.tplg
b624b8fe216b9600bdf3291178edaa12  pdm1-drceq/sof-tgl-max98357a-rt5682.tplg

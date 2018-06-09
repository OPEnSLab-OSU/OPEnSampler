This schematic uses the TPIC6B596 open-drain shift register from the tpic.lbr eagle library, available at http://www.diymodules.org/eagle-show-library?type=usr&id=2322. 

The base unit is the main water sampler module. The idea is to use 4 TPICs in series to control up to 31 valves corresponding to 31 respective sample bags. The last TPIC output is for a waste valve that either allows water to be drained or allows water to move to the next unit.

Each extension unit is barebones to reduce cost and volume, and will include up to 31 bags and another waste valve. Each unit will be connected to the previous via 6 wires and one water line.
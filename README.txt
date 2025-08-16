WeatherConversion (xml)
- Full xml with all parameters

WeatherConversion (txt)
- Only saves txt files for the following parameters/levels:
	[Parameters] 	Temperature, WindU, WindV, Height, RH
	[Levels]	100, 200, 300

==================================================
LIBRARIES TO INSTALL
==================================================

sudo apt update
sudo apt install libtinyxml2-dev
sudo apt install libeccodes0 libeccodes-dev

==================================================
MAIN COMMANDS
==================================================

[xml]
wsl
g++ cscript.cpp -o grib2_to_xml -leccodes -ltinyxml2
./grib2_to_xml

[txt]
wsl
g++ newcscript.cpp -o grib2_to_txt -leccodes -ltinyxml2
./grib2_to_txt

==================================================
METEO OPTIONS
==================================================

130 - Temperature (K)
131 - WindU (m/sec)
132 - WindV (m/sec)
156 - Height (m) <GPH Geo-potential Height>
157 - RH (%)
159 - Sunset Time (minutes since midnight)
160 - Sunrise Time (minutes since midnight)

==================================================
METEO OPTIONS (NOMADS VER)
==================================================

ParamAndUnits="130"
ParameterName="Temperature"

ParamAndUnits="131"
ParameterName="U component of wind"

ParamAndUnits="132"
ParameterName="V component of wind"

ParamAndUnits="156"
ParameterName="Geopotential height"

ParamAndUnits="157"
ParameterName="Relative humidity"
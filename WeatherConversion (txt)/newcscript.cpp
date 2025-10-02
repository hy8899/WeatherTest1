#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <iomanip>
#include <filesystem>
#include <eccodes.h>
#include "tinyxml2.h"

using namespace tinyxml2;

#include <iostream>
#include <cmath>
#include <ctime>

bool isComputeSunriseSunset = false;

double singaporeLat = 1.3521;
double singaporeLon = 103.8198;

int currYear = 2025;
double timezoneOffset = 8.0;

// Convert degrees <-> radians
inline double deg2rad(double d) { return d * M_PI / 180.0; }
inline double rad2deg(double r) { return r * 180.0 / M_PI; }

int dayOfYear(int year, int month, int day) {
    static const int mdays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int doy = 0;
    for (int m = 1; m < month; ++m) {
        doy += mdays[m - 1];
        if (m == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) ++doy;
    }
    doy += day;
    return doy;
}

double equationOfTime(int doy) {
    double B = 2.0 * M_PI * (doy - 81) / 364.0;
    return 229.18 * (0.000075 + 0.001868 * cos(B) - 0.032077 * sin(B)
        - 0.014615 * cos(2 * B) - 0.040849 * sin(2 * B));
}

double solarDeclination(int doy) {
    return 0.409 * sin(2.0 * M_PI * (doy - 81) / 364.0);
}

// Returns sunrise/sunset in LOCAL minutes with decimals
bool computeSunriseSunsetLocal(
    int year, int month, int day,
    double latitude, double longitude,
    double tzOffsetHours,
    double& sunriseMin, double& sunsetMin)  // now doubles
{
    int n = dayOfYear(year, month, day);
    double decl = solarDeclination(n);
    double eqTime = equationOfTime(n);

    double zenith = deg2rad(90.833);
    double latRad = deg2rad(latitude);

    double cosH = (cos(zenith) - sin(latRad) * sin(decl)) / (cos(latRad) * cos(decl));
    if (cosH > 1.0 || cosH < -1.0) return false;

    double H = acos(cosH);
    double deltaMinutes = rad2deg(H) * 4.0;

    double solarNoonUTC = 720 - (4 * longitude) - eqTime;
    double sunriseUTC = solarNoonUTC - deltaMinutes;
    double sunsetUTC = solarNoonUTC + deltaMinutes;

    sunriseUTC += tzOffsetHours * 60.0;
    sunsetUTC += tzOffsetHours * 60.0;

    auto norm = [](double m) { while (m < 0)m += 1440; while (m >= 1440)m -= 1440; return m; };
    sunriseMin = norm(sunriseUTC);
    sunsetMin = norm(sunsetUTC);

    return true;
}

// Helper to convert grib time (yyyymmddhhmm) to ISO8601 string
std::string timeToISO(int year, int month, int day, int hour, int minute) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:00.000Z",
             year, month, day, hour, minute);
    return std::string(buf);
}

int main() {

    const char* input_file = "newsingapore.grib2";
    //const char* output_file = "newoutput.xml";

    FILE* f = fopen(input_file, "rb");
    if (!f) {
        std::cerr << "Failed to open input file\n";
        return 1;
    }

    codes_handle* h = nullptr;
    int err = 0;

    while ((h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err)) != nullptr) {

        XMLDocument doc;
        XMLDeclaration* decl = doc.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"");
        doc.InsertFirstChild(decl);

        // add root element
        //XMLElement* root = doc.NewElement("gribObjects");
        //doc.InsertEndChild(root);
        
        long edition = 0, dataDate, dataTime;
        codes_get_long(h, "edition", &edition);
        codes_get_long(h, "dataDate", &dataDate);
        codes_get_long(h, "dataTime", &dataTime);

        int y = dataDate / 10000;
        int m = (dataDate / 100) % 100;
        int d = dataDate % 100;
        int hh = dataTime / 100;
        int mm = dataTime % 100;
        std::string run_time = timeToISO(y, m, d, hh, mm);

        long forecast_time_units = 0;
        codes_get_long(h, "indicatorOfUnitOfTimeRange", &forecast_time_units);

        char name[128], units[64];
        size_t name_len = sizeof(name), units_len = sizeof(units);
        codes_get_string(h, "shortName", name, &name_len);
        codes_get_string(h, "units", units, &units_len);
        std::string parameter_name = std::string(name) + "[" + std::string(units) + "]";

        std::string filename = input_file;
        size_t pos = filename.find(".grib2");
        if (pos != std::string::npos)
            filename = filename.substr(0, pos);

        size_t values_len = 0;
        codes_get_size(h, "values", &values_len);
        std::vector<double> values(values_len);
        codes_get_double_array(h, "values", values.data(), &values_len);

        char parameterName[100];
        size_t nameLen = sizeof(parameterName);
        int err = codes_get_string(h, "name", parameterName, &nameLen);

        // ==============================
        // GRIB OBJECT Element
        // ==============================
        XMLElement* msgElem = doc.NewElement("gribObject");

        msgElem->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        msgElem->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        msgElem->SetAttribute("xmlns", "http://c4i.mlt/ICDU/schema");

        msgElem->SetAttribute("ForecastTime", run_time.c_str());
        msgElem->SetAttribute("RunTime", run_time.c_str());
        msgElem->SetAttribute("ParameterName", parameterName);
        msgElem->SetAttribute("FileName", filename.c_str());

        doc.InsertEndChild(msgElem);

        long section1Length = 0;
        codes_get_long(h, "section1Length", &section1Length);

        // ==============================
        // INDICATOR Element
        // ==============================
        XMLElement* indicator = doc.NewElement("Indicator");
        indicator->SetAttribute("xmlns", "");
        indicator->SetAttribute("GribHeader", "GRIB");
        indicator->SetAttribute("NumOfOctets", std::to_string(section1Length).c_str());
        indicator->SetAttribute("GribType", std::to_string(edition).c_str());
        msgElem->InsertEndChild(indicator);

        long tablesVersion, centre, genProcID, gridTypeCode;
        long parameterNumber, typeOfLevel, level, timeRangeIndicator, subcenterID;
        long decimalScaleFactor;

        codes_get_long(h, "tablesVersion", &tablesVersion);
        codes_get_long(h, "centre", &centre);
        codes_get_long(h, "generatingProcessIdentifier", &genProcID);
        codes_get_long(h, "gridTypeCode", &gridTypeCode);
        codes_get_long(h, "paramId", &parameterNumber);
        codes_get_long(h, "typeOfLevel", &typeOfLevel);
        codes_get_long(h, "level", &level);
        codes_get_long(h, "indicatorOfUnitOfTimeRange", &timeRangeIndicator);
        codes_get_long(h, "subCentre", &subcenterID);
        codes_get_long(h, "decimalScaleFactor", &decimalScaleFactor);

        size_t section4Length = 0;
        codes_get_size(h, "section4Length", &section4Length);

        long month, day, hour, minute;
        codes_get_long(h, "month", &month);
        codes_get_long(h, "day", &day);
        codes_get_long(h, "hour", &hour);
        codes_get_long(h, "minute", &minute);

        // =======================================
        // PRODUCT DEFINITION Element
        // =======================================
        XMLElement* prodDef = doc.NewElement("ProductDefinition");
        prodDef->SetAttribute("xmlns", "");
        prodDef->SetAttribute("NumOfOctets", std::to_string(section4Length * 4).c_str());
        prodDef->SetAttribute("ParamTableVersion", std::to_string(tablesVersion).c_str());
        prodDef->SetAttribute("OrigCenter", std::to_string(centre).c_str());
        prodDef->SetAttribute("ProcessID", std::to_string(genProcID).c_str());

        int gridCode = static_cast<int>(gridTypeCode & 0xFF);
        prodDef->SetAttribute("GridID", std::to_string(gridCode).c_str());
        //prodDef->SetAttribute("GridID", std::to_string(gridTypeCode).c_str());

        prodDef->SetAttribute("GDS_BDS_Flag", "128");

        int paramNumber = static_cast<int>(parameterNumber & 0xFF);
        prodDef->SetAttribute("ParamAndUnits", std::to_string(paramNumber).c_str());
        //prodDef->SetAttribute("ParamAndUnits", std::to_string(parameterNumber).c_str());
        
        prodDef->SetAttribute("LevelType", std::to_string(typeOfLevel).c_str());
        prodDef->SetAttribute("Level", std::to_string(level).c_str());
        prodDef->SetAttribute("ReferenceTime", std::to_string(d).c_str());
        prodDef->SetAttribute("Month", std::to_string(month).c_str());
        prodDef->SetAttribute("Day", std::to_string(day).c_str());
        prodDef->SetAttribute("Hour", std::to_string(hour).c_str());
        prodDef->SetAttribute("Minute", std::to_string(minute).c_str());
        prodDef->SetAttribute("ForecastTimeUnits", std::to_string(forecast_time_units).c_str());
        prodDef->SetAttribute("PeriodOfTime1", std::to_string(hh).c_str());
        prodDef->SetAttribute("PeriodOfTime2", std::to_string(mm).c_str());
        prodDef->SetAttribute("TimeRangeIndicator", std::to_string(timeRangeIndicator).c_str());

        double numInAvg = 0;
        codes_get_double(h, "average", &numInAvg);
        prodDef->SetAttribute("NumInAverage",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(numInAvg)).str().c_str());

        long numMissing = 0;
        codes_get_long(h, "numberOfMissing", &numMissing);
        prodDef->SetAttribute("NumMissingAverage", std::to_string(numMissing).c_str());

        prodDef->SetAttribute("CenturyOfReference", "21");
        prodDef->SetAttribute("SubcenterID", std::to_string(subcenterID).c_str());
        prodDef->SetAttribute("DecimalScaleFactor", std::to_string(decimalScaleFactor).c_str());
        prodDef->SetAttribute("Reserved1", "0");
        msgElem->InsertEndChild(prodDef);

        // GridDescription (lat/lon degrees)
        long Ni, Nj;
        double lat1, lon1, lat2, lon2, di, dj;
        long scanningMode;

        codes_get_long(h, "Ni", &Ni);
        codes_get_long(h, "Nj", &Nj);
        codes_get_double(h, "latitudeOfFirstGridPointInDegrees", &lat1);
        codes_get_double(h, "longitudeOfFirstGridPointInDegrees", &lon1);
        codes_get_double(h, "latitudeOfLastGridPointInDegrees", &lat2);
        codes_get_double(h, "longitudeOfLastGridPointInDegrees", &lon2);
        codes_get_double(h, "iDirectionIncrementInDegrees", &di);
        codes_get_double(h, "jDirectionIncrementInDegrees", &dj);

        codes_get_long(h, "scanningMode", &scanningMode);

        long section3Length;
        long NV, PV, dataRepType;

        codes_get_long(h, "section3Length", &section3Length);
        codes_get_long(h, "NV", &NV);
        codes_get_long(h, "PV", &PV);
        codes_get_long(h, "dataRepresentationTemplateNumber", &dataRepType);
        //codes_get_long(h, "dataRepresentationType", &dataRepType);

        // =======================================
        // GRID DESCRIPTION Element
        // =======================================
        XMLElement* gridDesc = doc.NewElement("GridDescription");
        gridDesc->SetAttribute("xmlns", "");
        gridDesc->SetAttribute("NumOfOctets", section3Length);
        gridDesc->SetAttribute("NV", NV);
        gridDesc->SetAttribute("PV", "255");
        gridDesc->SetAttribute("DataRepresentationType", dataRepType);

        // =======================================
        // Lat Lon Projection Element
        // =======================================
        XMLElement* proj = doc.NewElement("LatLonProjection");
        proj->SetAttribute("Ni", std::to_string(Ni).c_str());
        proj->SetAttribute("Nj", std::to_string(Nj).c_str());

        prodDef->SetAttribute("Lat1",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(lat1 * 1000.0)).str().c_str());

        prodDef->SetAttribute("Lon1",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(lon1 * 1000.0)).str().c_str());

        proj->SetAttribute("Flags", "128");

        prodDef->SetAttribute("Lat2",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(lat2 * 1000.0)).str().c_str());

        prodDef->SetAttribute("Lon2",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(lon2 * 1000.0)).str().c_str());

        prodDef->SetAttribute("Di",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(di * 1000.0)).str().c_str());

        prodDef->SetAttribute("Dj",
            (std::ostringstream() << std::fixed << std::setprecision(0) << std::trunc(dj * 1000.0)).str().c_str());

        proj->SetAttribute("ScanFlags", std::to_string(scanningMode).c_str());
        proj->SetAttribute("Reserved", "0");

        gridDesc->InsertEndChild(proj);
        msgElem->InsertEndChild(gridDesc);

        long binaryScaleFactor, numberOfValues;
        double referenceValue;
        codes_get_long(h, "binaryScaleFactor", &binaryScaleFactor);
        codes_get_double(h, "referenceValue", &referenceValue);
        codes_get_long(h, "numberOfValues", &numberOfValues);

        // =======================================
        // BINARY DATA element
        // =======================================
        XMLElement* binaryData = doc.NewElement("BinaryData");
        binaryData->SetAttribute("xmlns", "");
        binaryData->SetAttribute("NumOfOctets", std::to_string(values_len * 4).c_str());
        binaryData->SetAttribute("FlagsAndUnused", "12");
        binaryData->SetAttribute("BinaryScaleFactor", std::to_string(binaryScaleFactor).c_str());
        binaryData->SetAttribute("ReferenceValue", std::to_string(referenceValue).c_str());
        binaryData->SetAttribute("DatumPacked", std::to_string(numberOfValues).c_str());
        binaryData->SetAttribute("OptionalFlags", "0");

        // =======================================
        // TempDataArray Element
        // =======================================
        XMLElement* tempArray = doc.NewElement("TempDataArray");
        std::string valStr;
        for (auto v : values) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f ", v);
            valStr += buf;
        }
        tempArray->SetText(valStr.c_str());
        binaryData->InsertEndChild(tempArray);
        msgElem->InsertEndChild(binaryData);

        // =======================================
        // END Element
        // =======================================
        XMLElement* endTag = doc.NewElement("End");
        endTag->SetAttribute("xmlns", "");
        endTag->SetAttribute("EndString", "7777");
        msgElem->InsertEndChild(endTag);

        //root->InsertEndChild(msgElem);


        // =======================================
        // Create TXT File
        // =======================================
        std::set<long> validLevels = {100, 200, 300};
        std::set<long> validParamIds = {130, 131, 132, 156, 157, 158, 159, 160};

        // create txt file if parameter ID & altitude is valid
        bool isCreateTxtFile = (validParamIds.count(parameterNumber) > 0) && (validLevels.count(level) > 0);
        if (isCreateTxtFile) {

            // create folder if not created yet
            std::string pName(parameterName);
            if (!std::filesystem::exists(pName)) {
                std::filesystem::create_directories(pName);
            }

            // set output file name 
            std::string output_file = pName + "/" + 
                pName + "_" +
                std::to_string(d) + 
                std::to_string(month) + 
                std::to_string(day) + "_" + 
                std::to_string(hour) + "_" + 
                std::to_string(level) + "_" + 
                std::to_string(hh) + "_" + 
                std::to_string(parameterNumber) + "_" + 
                std::to_string(di) +
                ".txt";

            // save txt file data
            XMLError eResult = doc.SaveFile(output_file.c_str());

            if (eResult != XML_SUCCESS) {
                std::cerr << "Failed to write TXT file\n";
                return 1;
            } else {
                std::cout << std::left << std::setw(25) << pName << std::right << std::setw(5) << level << " ✅\n";
            }

            // =======================================
            // Sunset/Sunrise Times
            // =======================================
            if (!isComputeSunriseSunset)
            {
                double rise, set;
                if (computeSunriseSunsetLocal(currYear, month, day, singaporeLat, singaporeLon, timezoneOffset, rise, set)) {

                    std::ofstream outFile("sunrise_sunset_times.txt");

                    if (!outFile) {
                        std::cerr << "Error: Could not open file for writing\n";
                        return 1;
                    }

                    outFile << "Sunrise Time: " << rise << " minutes after midnight\n";
                    outFile << "Sunset Time: " << set << " minutes after midnight\n";

                    outFile.close();
                    std::cout << "Saved sunrise_sunset_times.txt ✅\n";
                }

                isComputeSunriseSunset = true;
            }
        }

        codes_handle_delete(h);
    }

    fclose(f);
    return 0;
}
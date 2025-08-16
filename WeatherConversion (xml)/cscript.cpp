#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <eccodes.h>
#include "tinyxml2.h"

using namespace tinyxml2;

// Helper to convert grib time (yyyymmddhhmm) to ISO8601 string
std::string timeToISO(int year, int month, int day, int hour, int minute) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:00Z",
             year, month, day, hour, minute);
    return std::string(buf);
}

int main() {
    const char* input_file = "newsingapore.grib2";
    const char* output_file = "newoutput.xml";

    FILE* f = fopen(input_file, "rb");
    if (!f) {
        std::cerr << "Failed to open input file\n";
        return 1;
    }

    XMLDocument doc;
    XMLElement* root = doc.NewElement("gribObject");
    root->SetAttribute("xmlns", "http://c4i.mlt/ICDU/schema");
    root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    root->SetAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
    doc.InsertFirstChild(root);

    codes_handle* h = nullptr;
    int err = 0;
    int messageCount = 0;

    while ((h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err)) != nullptr) {
        messageCount++;

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
        //long forecast_time = 0;
        codes_get_long(h, "indicatorOfUnitOfTimeRange", &forecast_time_units);
        //codes_get_long(h, "forecastTime", &forecast_time);

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

        XMLElement* msgElem = doc.NewElement("GRIB_Message");
        msgElem->SetAttribute("ForecastTime", run_time.c_str());
        msgElem->SetAttribute("RunTime", run_time.c_str());

        char parameterName[100];
        size_t nameLen = sizeof(parameterName);
        int err = codes_get_string(h, "name", parameterName, &nameLen);
        msgElem->SetAttribute("ParameterName", parameterName);

        msgElem->SetAttribute("FileName", filename.c_str());

        long section1Length = 0;
        codes_get_long(h, "section1Length", &section1Length);
        XMLElement* indicator = doc.NewElement("Indicator");
        indicator->SetAttribute("GribHeader", "GRIB");
        indicator->SetAttribute("NumOfOctets", std::to_string(section1Length).c_str());
        indicator->SetAttribute("GribType", std::to_string(edition).c_str());
        msgElem->InsertEndChild(indicator);

        long tablesVersion, centre, genProcID, gridTypeCode;
        long parameterNumber, typeOfLevel, level, timeRangeIndicator, localDefNumber;
        long decimalScaleFactor;

        codes_get_long(h, "tablesVersion", &tablesVersion);
        codes_get_long(h, "centre", &centre);
        codes_get_long(h, "generatingProcessIdentifier", &genProcID);
        codes_get_long(h, "gridTypeCode", &gridTypeCode);
        codes_get_long(h, "paramId", &parameterNumber);
        codes_get_long(h, "typeOfLevel", &typeOfLevel);
        codes_get_long(h, "level", &level);
        codes_get_long(h, "indicatorOfUnitOfTimeRange", &timeRangeIndicator);
        codes_get_long(h, "localDefinitionNumber", &localDefNumber);
        codes_get_long(h, "decimalScaleFactor", &decimalScaleFactor);

        size_t section4Length = 0;
        codes_get_size(h, "section4Length", &section4Length);

        long month, day, hour, minute;
        codes_get_long(h, "month", &month);
        codes_get_long(h, "day", &day);
        codes_get_long(h, "hour", &hour);
        codes_get_long(h, "minute", &minute);

        XMLElement* prodDef = doc.NewElement("ProductDefinition");
        prodDef->SetAttribute("NumOfOctets", std::to_string(section4Length * 4).c_str());
        prodDef->SetAttribute("ParamTableVersion", std::to_string(tablesVersion).c_str());
        prodDef->SetAttribute("OrigCenter", std::to_string(centre).c_str());
        prodDef->SetAttribute("ProcessID", std::to_string(genProcID).c_str());
        prodDef->SetAttribute("GridID", std::to_string(gridTypeCode).c_str());
        prodDef->SetAttribute("GDS_BDS_Flag", "128");
        prodDef->SetAttribute("ParamAndUnits", std::to_string(parameterNumber).c_str());
        prodDef->SetAttribute("LevelType", std::to_string(typeOfLevel).c_str());
        prodDef->SetAttribute("Level", std::to_string(level).c_str());
        prodDef->SetAttribute("ReferenceTime", std::to_string(d).c_str());
        prodDef->SetAttribute("Month", std::to_string(month).c_str());
        prodDef->SetAttribute("Day", std::to_string(day).c_str());
        prodDef->SetAttribute("Hour", std::to_string(hour).c_str());
        prodDef->SetAttribute("Minute", std::to_string(minute).c_str());
        prodDef->SetAttribute("ForecastTimeUnits", std::to_string(forecast_time_units).c_str());
        //prodDef->SetAttribute("PeriodOfTime1", std::to_string(forecast_time).c_str());
        prodDef->SetAttribute("PeriodOfTime1", std::to_string(hh).c_str());
        prodDef->SetAttribute("PeriodOfTime2", std::to_string(mm).c_str());
        prodDef->SetAttribute("TimeRangeIndicator", std::to_string(timeRangeIndicator).c_str());

        double numInAvg = 0;
        codes_get_double(h, "average", &numInAvg);

        std::ostringstream oss;
        oss << std::setprecision(6) << numInAvg;
        prodDef->SetAttribute("NumInAverage", oss.str().c_str());

        long numMissing = 0;
        codes_get_long(h, "numberOfMissing", &numMissing);
        prodDef->SetAttribute("NumMissingAverage", std::to_string(numMissing).c_str());

        prodDef->SetAttribute("CenturyOfReference", "21");
        prodDef->SetAttribute("SubcenterID", std::to_string(localDefNumber).c_str());
        prodDef->SetAttribute("DecimalScaleFactor", std::to_string(decimalScaleFactor).c_str());
        prodDef->SetAttribute("Reserved1", "0");
        msgElem->InsertEndChild(prodDef);

        // GridDescription (stub)
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
        codes_get_long(h, "dataRepresentationType", &dataRepType);

        XMLElement* gridDesc = doc.NewElement("GridDescription");
        gridDesc->SetAttribute("NumOfOctets", section3Length);
        gridDesc->SetAttribute("NV", NV);
        gridDesc->SetAttribute("PV", PV);
        gridDesc->SetAttribute("DataRepresentationType", dataRepType);


        XMLElement* proj = doc.NewElement("LatLonProjection");
        proj->SetAttribute("Ni", std::to_string(Ni).c_str());
        proj->SetAttribute("Nj", std::to_string(Nj).c_str());
        proj->SetAttribute("Lat1", std::to_string(lat1 * 1000.0f).c_str());
        proj->SetAttribute("Lon1", std::to_string(lon1 * 1000.0f).c_str());
        proj->SetAttribute("Flags", "128");
        proj->SetAttribute("Lat2", std::to_string(lat2 * 1000.0f).c_str());
        proj->SetAttribute("Lon2", std::to_string(lon2 * 1000.0f).c_str());
        proj->SetAttribute("Di", std::to_string(di).c_str());
        proj->SetAttribute("Dj", std::to_string(dj).c_str());
        proj->SetAttribute("ScanFlags", std::to_string(scanningMode).c_str());
        proj->SetAttribute("Reserved", "0");

        gridDesc->InsertEndChild(proj);
        msgElem->InsertEndChild(gridDesc);

        long binaryScaleFactor, numberOfValues;
        double referenceValue;
        codes_get_long(h, "binaryScaleFactor", &binaryScaleFactor);
        codes_get_double(h, "referenceValue", &referenceValue);
        codes_get_long(h, "numberOfValues", &numberOfValues);

        XMLElement* binaryData = doc.NewElement("BinaryData");
        binaryData->SetAttribute("NumOfOctets", std::to_string(values_len * 4).c_str());
        binaryData->SetAttribute("FlagsAndUnused", "12");
        binaryData->SetAttribute("BinaryScaleFactor", std::to_string(binaryScaleFactor).c_str());
        binaryData->SetAttribute("ReferenceValue", std::to_string(referenceValue).c_str());
        binaryData->SetAttribute("DatumPacked", std::to_string(numberOfValues).c_str());
        binaryData->SetAttribute("OptionalFlags", "0");

        XMLElement* tempArray = doc.NewElement("TempDataArray");
        std::string valStr;
        for (auto v : values) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.8f ", v);
            valStr += buf;
        }
        tempArray->SetText(valStr.c_str());
        binaryData->InsertEndChild(tempArray);
        msgElem->InsertEndChild(binaryData);

        XMLElement* endTag = doc.NewElement("End");
        endTag->SetAttribute("EndString", "7777");
        msgElem->InsertEndChild(endTag);

        root->InsertEndChild(msgElem);
        codes_handle_delete(h);
    }

    fclose(f);

    XMLError eResult = doc.SaveFile(output_file);
    if (eResult != XML_SUCCESS) {
        std::cerr << "Failed to write XML file\n";
        return 1;
    } else {
        std::cout << "\u2705 XML written to: " << output_file << " (" << messageCount << " messages)\n";
    }

    return 0;
}
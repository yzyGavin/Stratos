/*
 * Airplaine.h
 *
 *  Created on: 29.10.2017
 *      Author: Karol
 */

#ifndef FLIGHTCONTROL_AIRCRAFTRECORD_H_
#define FLIGHTCONTROL_AIRCRAFTRECORD_H_

#include "string"
#include "ADSBMessage.h"

#define DEFAULT_LIVE_SPAN 120U //120s in ms

class AircraftRecord
{
public:
	AircraftRecord() = default;
	AircraftRecord(const std::string& ICAO_Address);

	const std::string& GetICAO_Address() const {return ICAO_Address;}
	bool IsRecordExpiered() const {return recordExpiered;}

	void SetAltitude(const uint32_t& newAltitude);
	void SetFlightName(const std::string& newFlightName);

	uint32_t altitude;
	bool altitudeKnown;
	std::string altStr;

	void Tick(uint32_t ticks);

private:
	std::string ICAO_Address;

	std::string flightName;
	bool flightNameKnown;


	uint32_t ticksToExpire;
	bool recordExpiered;

};

#endif /* FLIGHTCONTROL_AIRCRAFTRECORD_H_ */
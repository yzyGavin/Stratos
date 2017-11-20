/*
 * Airplaine.cpp
 *
 *  Created on: 29.10.2017
 *      Author: Karol
 */

#include <AircraftRecord.h>

AircraftRecord::AircraftRecord(const std::string& ICAO_Address) : ICAO_Address(ICAO_Address)
{
	ticksToExpire = DEFAULT_LIVE_SPAN;
	recordExpiered = false;
	altitudeKnown = false;
	flightNameKnown = false;
}

void AircraftRecord::Tick(uint32_t ticks)
{

	if(ticksToExpire > 0U)
	{
		if(ticksToExpire > ticks)
		{
			ticksToExpire -= ticks;
		}
		else
		{
			ticksToExpire = 0U;
		}
		if(ticksToExpire == 0U)
		{
			recordExpiered = true;
		}

	}
}

void AircraftRecord::SetAltitude(const uint32_t& newAltitude)
{
	char buff[256];
	sprintf(buff,"%lu ft",newAltitude);
	altStr = std::string(buff);
	altitude = newAltitude;
	altitudeKnown = true;
	ticksToExpire = DEFAULT_LIVE_SPAN;
}
void AircraftRecord::SetFlightName(const std::string& newFlightName)
{
	flightName = newFlightName;
	flightNameKnown = true;
	ticksToExpire = DEFAULT_LIVE_SPAN;
}


/*
Very simple stategy sample implementation.
*/


#include "strategy.h"

const float SimpleStrategy::MAX_FUEL_PER_METER = 0.0008f;	// [kg/m] fuel consumtion.
const int SimpleStrategy::PIT_DAMMAGE = 5000;				// [-]


SimpleStrategy::SimpleStrategy() :
	m_fuelchecked(false),
	m_fuelperlap(0.0f),
	m_lastpitfuel(0.0f),
	m_fuelsum(0.0f)
{
	// Nothing so far.
}


SimpleStrategy::~SimpleStrategy()
{
	// Nothing so far.
}


// Trivial strategy: fill in as much fuel as required for the whole race, or if the tank is
// too small fill the tank completely.
void SimpleStrategy::setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s, int index)
{
	// Load and set parameters.
	float fuel = GfParmGetNum(*carParmHandle, MYBOT_SECT_PRIV, MYBOT_ATT_FUELPERLAP, (char*)NULL, t->length*MAX_FUEL_PER_METER);
	m_expectedfuelperlap = fuel;
	float maxfuel = GfParmGetNum(*carParmHandle, SECT_CAR, PRM_TANK, (char*)NULL, 100.0f);
	fuel *= (s->_totLaps + 1.0f);
	m_lastfuel = MIN(fuel, maxfuel);
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*)NULL, m_lastfuel);
}


void SimpleStrategy::update(tCarElt* car, tSituation *s)
{
	// Fuel statistics update.
	int id = car->_trkPos.seg->id;
	// Range must include enough segments to be executed once guaranteed.
	if (id >= 0 && id < 5 && !m_fuelchecked) {
		if (car->race.laps > 1) {
			m_fuelperlap = MAX(m_fuelperlap, (m_lastfuel + m_lastpitfuel - car->priv.fuel));
			m_fuelsum += (m_lastfuel + m_lastpitfuel - car->priv.fuel);
		}
		m_lastfuel = car->priv.fuel;
		m_lastpitfuel = 0.0;
		m_fuelchecked = true;
	}
	else if (id > 5) {
		m_fuelchecked = false;
	}
}


bool SimpleStrategy::needPitstop(tCarElt* car, tSituation *s)
{
	// Question makes only sense if there is a pit.
	if (car->_pit != NULL) {
		// Do we need to refuel?
		int laps = car->_remainingLaps - car->_lapsBehindLeader;
		if (laps > 0) {
			float cmpfuel = (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
			// TODO: Investigate if buggy for two pit stops in one lap, BUG?
			if (car->_fuel < 1.5*cmpfuel &&
				car->_fuel < laps*cmpfuel)
			{
				return true;
			}
		}

		// Do we need to repair and the pit is free?
		if (car->_dammage > PIT_DAMMAGE && isPitFree(car)) {
			return true;
		}
	}
	return false;
}


bool SimpleStrategy::isPitFree(tCarElt* car)
{
	if (car->_pit != NULL) {
		if (car->_pit->pitCarIndex == TR_PIT_STATE_FREE) {
			return true;
		}
	}
	return false;
}


float SimpleStrategy::pitRefuel(tCarElt* car, tSituation *s)
{
	float fuel;
	float cmpfuel = (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
	fuel = MAX(MIN((car->_remainingLaps + 1.0f)*cmpfuel - car->_fuel,
		car->_tank - car->_fuel),
		0.0f);
	m_lastpitfuel = fuel;
	return fuel;
}


int SimpleStrategy::pitRepair(tCarElt* car, tSituation *s)
{
	return car->_dammage;
}


SimpleStrategy2::~SimpleStrategy2()
{
}

void SimpleStrategy2::setFuelAtRaceStart(tTrack * t, void ** carParmHandle, tSituation * s, int index)
{
}

float SimpleStrategy2::pitRefuel(tCarElt * car, tSituation * s)
{
	return 0.0f;
}

void SimpleStrategy2::update(tCarElt* car, tSituation *s)
{
	// Fuel statistics update.
	int id = car->_trkPos.seg->id;
	// Range must include enough segments to be executed once guaranteed.
	if (id >= 0 && id < 5 && !m_fuelchecked) {
		if (car->race.laps > 1) {
			//m_fuelperlap = MAX(m_fuelperlap, (m_lastfuel + m_lastpitfuel - car->priv.fuel));
			m_fuelsum += (m_lastfuel + m_lastpitfuel - car->priv.fuel);
			m_fuelperlap = (m_fuelsum / (car->race.laps - 1));
			// This is here for adding strategy decisions, otherwise it could be moved to pitRefuel
			// for efficiency.
			updateFuelStrategy(car, s);
		}
		m_lastfuel = car->priv.fuel;
		m_lastpitfuel = 0.0;
		m_fuelchecked = true;
	}
	else if (id > 5) {
		m_fuelchecked = false;
	}
}

void SimpleStrategy2::updateFuelStrategy(tCarElt * car, tSituation * s)
{
}

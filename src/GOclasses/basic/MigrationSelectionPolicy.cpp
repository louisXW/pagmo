/*****************************************************************************
 *   Copyright (C) 2008, 2009 Advanced Concepts Team (European Space Agency) *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

#include "MigrationSelectionPolicy.h"
#include "../../exceptions.h"

// 09/03/2009: Initial version by Marek Rucinski.

int MigrationSelectionPolicy::getNumberOfIndividualsToMigrate(const Population& population)
{
	if(migrationRateAbs < 0) {
		if(migrationRateFrac > 1.0) {
			pagmo_throw(assertion_error, "Fractional migration rate is grater than 1!");
		}
		return (int)(migrationRateFrac * (double)population.size());
	} else {
		if(migrationRateAbs > population.size()) {
			pagmo_throw(assertion_error, "Absolute migration rate exceeds population size!");
		}
		return migrationRateAbs;
	}
}

std::ostream &operator<<(std::ostream &s, const MigrationSelectionPolicy& msp)
{
	s << "Selection policy type: " << typeid(msp).name() << std::endl;
	s << "Migration rate (out):  " << (msp.migrationRateAbs < 0 ? msp.migrationRateFrac : msp.migrationRateAbs) << std::endl;
	return s;
}
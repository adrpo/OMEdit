/*
 * This file is part of OpenModelica.
 *
 * Copyright (c) 1998-2014, Open Source Modelica Consortium (OSMC),
 * c/o Linköpings universitet, Department of Computer and Information Science,
 * SE-58183 Linköping, Sweden.
 *
 * All rights reserved.
 *
 * THIS PROGRAM IS PROVIDED UNDER THE TERMS OF GPL VERSION 3 LICENSE OR
 * THIS OSMC PUBLIC LICENSE (OSMC-PL) VERSION 1.2.
 * ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS PROGRAM CONSTITUTES
 * RECIPIENT'S ACCEPTANCE OF THE OSMC PUBLIC LICENSE OR THE GPL VERSION 3,
 * ACCORDING TO RECIPIENTS CHOICE.
 *
 * The OpenModelica software and the Open Source Modelica
 * Consortium (OSMC) Public License (OSMC-PL) are obtained
 * from OSMC, either from the above address,
 * from the URLs: http://www.ida.liu.se/projects/OpenModelica or
 * http://www.openmodelica.org, and in the OpenModelica distribution.
 * GNU version 3 is obtained from: http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without
 * even the implied warranty of  MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE, EXCEPT AS EXPRESSLY SET FORTH
 * IN THE BY RECIPIENT SELECTED SUBSIDIARY LICENSE CONDITIONS OF OSMC-PL.
 *
 * See the full OSMC Public License conditions for more details.
 *
 */
/*
 * @author Volker Waurich <volker.waurich@tu-dresden.de>
 */

#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H


#include <osg/Timer>
#include <cmath>


class TimeManager
{
 public:

	TimeManager() = delete;
	TimeManager(const double simTime, const double realTime, const double realTimeFactor, const double visTime,
				const double hVisual, const double startTime, const double endTime);

	~TimeManager() = default;
	TimeManager(const TimeManager& omvm) = delete;
	TimeManager& operator=(const TimeManager& omvm) = delete;

	void updateTick();

	int getSliderPosition() const;

	void setSliderRange(const int min, const int max);

	/*! \brief Returns the end time of the simulation. */
	double getEndTime() const;
	/*! \brief Sets the end time of the simulation to the given value. */
	void setEndTime(const double endTime);

	/*! \brief Returns the start time of the simulation. */
	double getStartTime() const;
	/*! \brief Sets the start time of the simulation to the given value. */
	void setStartTime(const double startTime);

	/*! \brief Returns the current simulation time. */
	double getSimTime() const;
	/*! Sets the simulation time to the given value. */
	void setSimTime(const double simTime);

	/*! \brief Returns the current visualization time. */
	double getVisTime() const;
	/*! \brief Sets the visualization time to the given value. */
	void setVisTime(const double visTime);

	/*! \brief Returns the current step size of the simulation. */
	double getHVisual() const;
	/*! \brief Sets the step size to the given value. */
	void setHVisual(const double hVis);

	/*! \brief Returns real time. */
	double getRealTime() const;

	/*! \brief Returns the real time factor. */
	double getRealTimeFactor() const;
	/*! \brief Sets the real time factor to the given value. */
	void setRealTimeFactor(const double rtf);

	/*! \brief Returns true, if the visualization is currently paused and false otherwise. */
	bool isPaused() const;
	/*! \brief Sets pause status to new value. */
	void setPause(const bool status);

 private:
	//! Time of the current simulation step.
	double _simTime;
	//! Current real time.
	double _realTime;
	//! Real time factor.
	double _realTimeFactor;
	//! Time of current scene update.
	double _visTime;
	//! Step size for the scene updates in milliseconds.
	double _hVisual;
	//! Start time of the simulation.
	double _startTime;
	//! End time of the simulation.
	double _endTime;
	//! This variable indicates if the simulation/visualization currently pauses.
	bool _pause;

	osg::Timer _visualTimer;

	//! Range of the slider widget.
	int _sliderRange;
};


#endif

/*
 * main_xmm_tests.cpp
 *
 * Test suite for the XMM library
 *
 * Contact:
 * - Jules Françoise <jules.francoise@ircam.fr>
 *
 * This code has been initially authored by Jules Françoise
 * <http://julesfrancoise.com> during his PhD thesis, supervised by Frédéric
 * Bevilacqua <href="http://frederic-bevilacqua.net>, in the Sound Music
 * Movement Interaction team <http://ismm.ircam.fr> of the
 * STMS Lab - IRCAM, CNRS, UPMC (2011-2015).
 *
 * Copyright (C) 2015 UPMC, Ircam-Centre Pompidou.
 *
 * This File is part of XMM.
 *
 * XMM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XMM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XMM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "xmm/catch_test.h"


//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_COLOUR_NONE
#define CATCH_CONFIG_NOSTDOUT
#include "xmm/catch.hpp"
#define XMM_TESTING
#include "xmm/xmm.h"
//#include "xmm/catch_utilities.h"

#include "logger.h"

std::ostringstream dlog_stream;

namespace Catch {
	std::ostream& cout()
	{
//		return std::cout;
		return dlog_stream;

	}

	std::ostream& cerr()
	{
//		return std::cerr;
		return dlog_stream;
	}
}

int catch_test( int argc, char* const argv[] )
{
	// global setup...

	char* arg[3];
	arg[0] = argv[0];
	arg[1] = "-d";
	arg[2] = "yes";

	int result = Catch::Session().run( 3, arg);
	//int result = Catch::Session().run( argc, argv);

	DBG("%s", dlog_stream.str().c_str());


	// global clean-up...

	return result;
}


/*
 * PSIClusterTester.cpp
 *
 *  Created on: May 6, 2013
 *      Author: Jay Jay Billings
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Regression

#include <boost/test/included/unit_test.hpp>
#include <PSICluster.h>
#include <memory>
#include <typeinfo>
#include <limits>
#include <math.h>

using namespace std;
using namespace xolotlCore;

/**
 * This suite is responsible for testing the PSICluster.
 */
BOOST_AUTO_TEST_SUITE(PSICluster_testSuite)

/** This operation checks the loader. */
BOOST_AUTO_TEST_CASE(checkDiffusionCoefficient) {

	// Local Declarations
	double temperature = 1.0;
	PSICluster cluster(1);

	// Check E_m = 0.0
	cluster.setMigrationEnergy(0.0);
	cluster.setDiffusionFactor(1.0);
	BOOST_REQUIRE_CLOSE(cluster.getDiffusionCoefficient(1.0),exp(0.0),0.00001);

	// Make sure the diffusion coefficient is 0.0 if E_m is infinite
	cluster.setMigrationEnergy(std::numeric_limits<double>::infinity());
	cluster.setDiffusionFactor(1.0);
	BOOST_REQUIRE_CLOSE(cluster.getDiffusionCoefficient(1.0),0.0,0.00000);

	// Make sure the diffusion coefficient is zero if the diffusion factor is zero
	cluster.setMigrationEnergy(5.0);
	cluster.setDiffusionFactor(0.0);
	BOOST_REQUIRE_CLOSE(cluster.getDiffusionCoefficient(1.0),0.0,0.00000);

	// Make sure the diffusion coefficient is equal to the diffusion factor
	// if the temperature is infinite
	cluster.setMigrationEnergy(5.0);
	cluster.setDiffusionFactor(1.0);
	BOOST_REQUIRE_CLOSE(cluster.getDiffusionCoefficient(
					std::numeric_limits<double>::infinity()),1.0,0.00000);

	// Throw something random in there to be certain
	cluster.setMigrationEnergy(0.013);
	cluster.setDiffusionFactor(1.08E10);
	BOOST_REQUIRE_CLOSE(cluster.getDiffusionCoefficient(1500.0),9766651101.800613,0.0000001);
}

/**
 * This operation tests the copy constructor.
 */
BOOST_AUTO_TEST_CASE(checkCopying) {

	// Local Declarations
	PSICluster cluster(1);
	cluster.setDiffusionFactor(1.0);
	
	// Check Reactant copying
	PSICluster copiedCluster(cluster);
	copiedCluster.setDiffusionFactor(0.5); // This should not happen!
	
	// Check the diffusion factor
	BOOST_CHECK_CLOSE(copiedCluster.getDiffusionFactor(), 0.5, 1e-5); // This should compare the two directly!
	// BOOST_REQUIRE_CLOSE(cluster.getDiffusionFactor(),copiedCluster.getDiffusionFactor(),1e-5);
	// ONLY USE BOOST_REQUIRE, not BOOST_CHECK!
	BOOST_CHECK(abs(cluster.getDiffusionFactor() - 
		copiedCluster.getDiffusionFactor()) > 0.01);
	
	// Check the migration energy
	cluster.setMigrationEnergy(1.0);
	copiedCluster.setMigrationEnergy(3.0);
	BOOST_CHECK(cluster.getMigrationEnergy() <
		copiedCluster.getMigrationEnergy());
}

BOOST_AUTO_TEST_SUITE_END()

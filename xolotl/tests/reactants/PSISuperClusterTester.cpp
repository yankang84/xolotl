#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Regression

#include <boost/test/included/unit_test.hpp>
#include <PSICluster.h>
#include <PSISuperCluster.h>
#include <HDF5NetworkLoader.h>
#include <PSIHeCluster.h>
#include <PSIMixedCluster.h>
#include <XolotlConfig.h>
#include <DummyHandlerRegistry.h>
#include <Constants.h>
#include <Options.h>

using namespace std;
using namespace xolotlCore;

/**
 * This suite is responsible for testing the PSISuperCluster.
 */
BOOST_AUTO_TEST_SUITE(PSISuperCluster_testSuite)

/**
 * This operation checks the ability of the PSISuperCluster to describe
 * its connectivity to other clusters.
 */
BOOST_AUTO_TEST_CASE(checkConnectivity) {
	// Initialize MPI for HDF5
	int argc = 0;
	char **argv;
	MPI_Init(&argc, &argv);

	// Create the network loader
	HDF5NetworkLoader loader = HDF5NetworkLoader(
			make_shared<xolotlPerf::DummyHandlerRegistry>());
	// Define the filename to load the network from
	string sourceDir(XolotlSourceDirectory);
	string pathToFile("/tests/testfiles/tungsten_diminutive_2D.h5");
	string filename = sourceDir + pathToFile;
	// Give the filename to the network loader
	loader.setFilename(filename);
	// Set grouping parameters
	loader.setVMin(1);
	loader.setHeWidth(4);
	loader.setVWidth(1);

	// Create the options needed to load the network
	Options opts;
	// Load the network
	auto network = loader.load(opts);

	// Set the temperature in the network
	int networkSize = network->size();
	double temperature = 1000.0;
	network->setTemperature(temperature);
	network->computeRateConstants();
	// Recompute Ids and network size and redefine the connectivities
	network->reinitializeConnectivities();

	// Check the reaction connectivity of the super cluster
	auto& reactant = network->getAll(ReactantType::PSISuper).begin()->second;

	// Check the type name
	BOOST_REQUIRE(ReactantType::PSISuper == reactant->getType());
	auto reactionConnectivity = reactant->getConnectivity();

	// Check the connectivity for He, V, and I
	int connectivityExpected[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			1, 1, 1, 1, 1, 1, 0 };

	for (unsigned int i = 0; i < reactionConnectivity.size(); i++) {
		BOOST_REQUIRE_EQUAL(reactionConnectivity[i], connectivityExpected[i]);
	}

	return;
}

/**
 * This operation checks the ability of the PSISuperCluster to compute the total flux.
 */
BOOST_AUTO_TEST_CASE(checkTotalFlux) {

	// Create the network loader
	HDF5NetworkLoader loader = HDF5NetworkLoader(
			make_shared<xolotlPerf::DummyHandlerRegistry>());
	// Define the filename to load the network from
	string sourceDir(XolotlSourceDirectory);
	string pathToFile("/tests/testfiles/tungsten_diminutive_2D.h5");
	string filename = sourceDir + pathToFile;
	// Give the filename to the network loader
	loader.setFilename(filename);
	// Set grouping parameters
	loader.setVMin(1);
	loader.setHeWidth(4);
	loader.setVWidth(1);

	// Create the options needed to load the network
	Options opts;
	// Load the network
	auto network = loader.load(opts);

	// Set the temperature in the network
	int networkSize = network->size();
	double temperature = 1000.0;
	network->setTemperature(temperature);
	network->computeRateConstants();
	// Recompute Ids and network size and redefine the connectivities
	network->reinitializeConnectivities();

	// Check the reaction connectivity of the super cluster
	auto& cluster = network->getAll(ReactantType::PSISuper).begin()->second;
	// Get one that it combines with (He)
	auto secondCluster = (PSICluster *) network->get(Species::He, 1);
	// Set the concentrations
	cluster->setConcentration(0.5);
	secondCluster->setConcentration(0.5);

	// Get and check the flux
	double flux = cluster->getTotalFlux();
	BOOST_REQUIRE_CLOSE(0.0, flux, 0.1);

	return;
}

/**
 * This operation checks the PSISuperCluster get*PartialDerivatives methods.
 */
BOOST_AUTO_TEST_CASE(checkPartialDerivatives) {

	// Create the network loader
	HDF5NetworkLoader loader = HDF5NetworkLoader(
			make_shared<xolotlPerf::DummyHandlerRegistry>());
	// Define the filename to load the network from
	string sourceDir(XolotlSourceDirectory);
	string pathToFile("/tests/testfiles/tungsten_diminutive_2D.h5");
	string filename = sourceDir + pathToFile;
	// Give the filename to the network loader
	loader.setFilename(filename);
	// Set grouping parameters
	loader.setVMin(1);
	loader.setHeWidth(4);
	loader.setVWidth(1);

	// Create the options needed to load the network
	Options opts;
	// Load the network
	auto network = loader.load(opts);

	// Set the temperature in the network
	int networkSize = network->size();
	double temperature = 1000.0;
	network->setTemperature(temperature);
	network->computeRateConstants();
	// Recompute Ids and network size and redefine the connectivities
	network->reinitializeConnectivities();

	// Check the reaction connectivity of the super cluster
	auto& cluster = network->getAll(ReactantType::PSISuper).begin()->second;

	// Local Declarations
	// The vector of partial derivatives to compare with
	double knownPartials[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			-2.2407564e-19, 0.0, 0.0, 0.0, 0.0 };
	// Set the concentration
	cluster->setConcentration(0.5);

	// Set up the network to be able to compute the partial derivatives
	xolotlCore::IReactionNetwork::SparseFillMap dfill;
	network->getDiagonalFill(dfill);
	// Get the dof
	const int dof = network->getDOF();
	// Initialize the arrays for the reaction partial derivatives
	std::vector<int> reactionSize;
	reactionSize.resize(dof);
	std::vector<size_t> reactionStartingIdx;
	reactionStartingIdx.resize(dof);
	auto nPartials = network->initPartialsSizes(reactionSize,
			reactionStartingIdx);
	std::vector<int> reactionIndices;
	reactionIndices.resize(nPartials);
	network->initPartialsIndices(reactionSize, reactionStartingIdx,
			reactionIndices);
	std::vector<double> reactionVals;
	reactionVals.resize(nPartials);
	// Compute the partial derivatives
	network->computeAllPartials(reactionStartingIdx,
			reactionIndices, reactionVals);

	// Check the size of the partials
	int id = cluster->getId() - 1;
	BOOST_REQUIRE_EQUAL(reactionSize[id], 14U);

	// Check all the values
	for (unsigned int i = 0; i < reactionSize[id]; i++) {
		BOOST_REQUIRE_CLOSE(reactionVals[reactionStartingIdx[id] + i],
				knownPartials[i], 0.1);
	}

	return;
}

/**
 * This operation checks the reaction radius for PSISuperCluster.
 */
BOOST_AUTO_TEST_CASE(checkReactionRadius) {

	// Create the network loader
	HDF5NetworkLoader loader = HDF5NetworkLoader(
			make_shared<xolotlPerf::DummyHandlerRegistry>());
	// Define the filename to load the network from
	string sourceDir(XolotlSourceDirectory);
	string pathToFile("/tests/testfiles/tungsten_diminutive_2D.h5");
	string filename = sourceDir + pathToFile;
	// Give the filename to the network loader
	loader.setFilename(filename);
	// Set grouping parameters
	loader.setVMin(1);
	loader.setHeWidth(4);
	loader.setVWidth(1);

	// Create the options needed to load the network
	Options opts;
	// Load the network
	auto network = loader.load(opts);

	// Check the reaction connectivity of the super cluster
	auto& cluster = network->getAll(ReactantType::PSISuper).begin()->second;

	// Check the radius
	BOOST_REQUIRE_CLOSE(0.137265, cluster->getReactionRadius(), 0.001);

	// Finalize MPI
	MPI_Finalize();

	return;
}

BOOST_AUTO_TEST_SUITE_END()

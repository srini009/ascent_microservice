/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <ams/Client.hpp>
#include <tclap/CmdLine.h>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <ascent.hpp>
#include <conduit.hpp>
#include <conduit_blueprint.hpp>
#include <mpi.h>

namespace tl = thallium;
using namespace conduit;
int use_local = 0;

static std::string g_address_file;
static std::string g_address;
static std::string g_protocol;
static std::string g_node;
static unsigned    g_provider_id;
static std::string g_log_level = "info";

static void parse_command_line(int argc, char** argv);

static std::string read_nth_line(const std::string& filename, int n)
{
   std::ifstream in(filename.c_str());

   std::string s;
   //for performance
   s.reserve(200);

   //skip N lines
   for(int i = 0; i < n; ++i)
       std::getline(in, s);

   std::getline(in,s);
   return s;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    parse_command_line(argc, argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Initialize the thallium server
    tl::engine engine(g_protocol, THALLIUM_CLIENT_MODE);
    ascent::Ascent a;

    try {

        // Initialize a Client
        ams::Client client(engine);

        // Open the Database "mydatabase" from provider 0
        ams::NodeHandle node =
            client.makeNodeHandle(g_address, g_provider_id,
                    ams::UUID::from_string(g_node.c_str()));

        node.sayHello();

	Node n;
	n["mpi_comm"] = MPI_Comm_c2f(MPI_COMM_WORLD);
	n["runtime/type"] = "ascent";
	//n["runtime/vtkm/backend"] = "openmp";

	if(!use_local) {
		node.ams_open(n);
	} else {
		a.open(n);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	Node mesh;
    	conduit::blueprint::mesh::examples::braid("hexs",
        	                                      32,
                	                              32,
                        	                      32,
                                	              mesh);


	/*if(!use_local) {
		node.ams_publish(mesh);
	} else {
		a.publish(mesh);
	}*/

	Node actions;
    	Node &add_act = actions.append();
	add_act["action"] = "add_queries";

	// declare a queries to ask some questions
	Node &queries = add_act["queries"] ;

	// Create a 1D binning projected onto the x-axis
	queries["q1/params/expression"] = "binning('radial','max', [axis('x',num_bins=20)])";
	queries["q1/params/name"] = "1d_binning";

	// Create a 2D binning projected onto the x-y plane
	queries["q2/params/expression"] = "binning('radial','max', [axis('x',num_bins=20), axis('y',num_bins=20)])";
	queries["q2/params/name"] = "2d_binning";

	// Create a binning that emulates a line-out, that is, bin all values
	// between x = [-1,1], y = [-1,1] along the z-axis in 20 bins.
	// The result is a 1x1x20 array
	queries["q3/params/expression"] = "binning('radial','max', [axis('x',[-1,1]), axis('y', [-1,1]), axis('z', num_bins=20)])";
	queries["q3/params/name"] = "3d_binning";

	MPI_Barrier(MPI_COMM_WORLD);

	/*if(!use_local) {
		node.ams_execute(actions);
	} else {
		a.execute(actions);
	}*/

	if(!use_local) {
		node.ams_publish_and_execute(mesh, actions);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if(!use_local) {
		node.ams_close();
	} else {
		a.close();
	}

    } catch(const ams::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    MPI_Finalize();
    return 0;
}

void parse_command_line(int argc, char** argv) {

    try {
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

        TCLAP::CmdLine cmd("Ams client", ' ', "0.1");
        TCLAP::ValueArg<std::string> addressArg("a","address","Address or server", true,"","string");
        TCLAP::ValueArg<unsigned>    providerArg("p", "provider", "Provider id to contact (default 0)", false, 0, "int");
        TCLAP::ValueArg<std::string> nodeArg("r","node","Node id", true, ams::UUID().to_string(),"string");
        TCLAP::ValueArg<std::string> logLevel("v","verbose", "Log level (trace, debug, info, warning, error, critical, off)", false, "info", "string");
        cmd.add(addressArg);
        cmd.add(providerArg);
        cmd.add(nodeArg);
        cmd.add(logLevel);
        cmd.parse(argc, argv);

	/* The logic below grabs the server address corresponding the client's MPI rank (MXM case) */
	size_t pos = 0;
        g_address_file = addressArg.getValue();
	std::string delimiter = " ";
	std::string l = read_nth_line(g_address_file, rank+1);
	pos = l.find(delimiter);
	std::string server_rank_str = l.substr(0, pos);
	std::stringstream s_(server_rank_str);
	int server_rank;
	s_ >> server_rank;
	assert(server_rank == rank);
	l.erase(0, pos + delimiter.length());
	g_address = l;

        g_provider_id = providerArg.getValue();
        g_node = read_nth_line(nodeArg.getValue(), rank);
        g_log_level = logLevel.getValue();
        g_protocol = g_address.substr(0, g_address.find(":"));
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

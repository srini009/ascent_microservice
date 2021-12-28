/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <ams/Client.hpp>
#include <spdlog/spdlog.h>
#include <tclap/CmdLine.h>
#include <iostream>
#include <ascent.hpp>
#include <conduit.hpp>
#include <conduit_blueprint.hpp>

namespace tl = thallium;
using namespace conduit;

static std::string g_address;
static std::string g_protocol;
static std::string g_node;
static unsigned    g_provider_id;
static std::string g_log_level = "info";

static void parse_command_line(int argc, char** argv);

static void
tutorial_tets_example(Node &mesh)
{
    mesh.reset();

    //
    // (create example tet mesh from blueprint example 2)
    //
    // Create a 3D mesh defined on an explicit set of points,
    // composed of two tets, with two element associated fields
    //  (`var1` and `var2`)
    //

    // create an explicit coordinate set
    double X[5] = { -1.0, 0.0, 0.0, 0.0, 1.0 };
    double Y[5] = { 0.0, -1.0, 0.0, 1.0, 0.0 };
    double Z[5] = { 0.0, 0.0, 1.0, 0.0, 0.0 };
    mesh["coordsets/coords/type"] = "explicit";
    mesh["coordsets/coords/values/x"].set(X, 5);
    mesh["coordsets/coords/values/y"].set(Y, 5);
    mesh["coordsets/coords/values/z"].set(Z, 5);


    // add an unstructured topology
    mesh["topologies/mesh/type"] = "unstructured";
    // reference the coordinate set by name
    mesh["topologies/mesh/coordset"] = "coords";
    // set topology shape type
    mesh["topologies/mesh/elements/shape"] = "tet";
    // add a connectivity array for the tets
    int64 connectivity[8] = { 0, 1, 3, 2, 4, 3, 1, 2 };
    mesh["topologies/mesh/elements/connectivity"].set(connectivity, 8);

    const int num_elements = 2;
    float var1_vals[num_elements] = { 0, 1 };
    float var2_vals[num_elements] = { 1, 0 };
    
    // create a field named var1
    mesh["fields/var1/association"] = "element";
    mesh["fields/var1/topology"] = "mesh";
    mesh["fields/var1/values"].set(var1_vals, 2);

    // create a field named var2
    mesh["fields/var2/association"] = "element";
    mesh["fields/var2/topology"] = "mesh";
    mesh["fields/var2/values"].set(var2_vals, 2);

    //  make sure the mesh we created conforms to the blueprint
    Node verify_info;
    if(!blueprint::mesh::verify(mesh, verify_info))
    {
        std::cout << "Mesh Verify failed!" << std::endl;
        std::cout << verify_info.to_yaml() << std::endl;
    }
}

int main(int argc, char** argv) {
    parse_command_line(argc, argv);
    spdlog::set_level(spdlog::level::from_str(g_log_level));

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

        int32_t result;
        node.computeSum(32, 54, &result);
	std::cout << "Result is: " << result << std::endl;
	Node n;
	n["runtime/type"] = "ascent";
  	n["runtime/vtkm/backend"] = "openmp";
	node.ams_open(n);
	Node mesh;
    	conduit::blueprint::mesh::examples::braid("hexs",
        	                                      2,
                	                              2,
                        	                      2,
                                	              mesh);

	node.ams_publish(mesh);

	Node actions;
    	Node &add_act = actions.append();
	add_act["action"] = "add_extracts";

	// add a relay extract that will write mesh data to 
	// blueprint hdf5 files
	Node &extracts = add_act["extracts"];
	extracts["e1/type"] = "relay";
	extracts["e1/params/path"] = "out_export_braid_all_fields";
	extracts["e1/params/protocol"] = "blueprint/mesh/hdf5";

	node.ams_execute(actions);

	node.ams_close();

    } catch(const ams::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        exit(-1);
    }

    return 0;
}

void parse_command_line(int argc, char** argv) {
    try {
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
        g_address = addressArg.getValue();
        g_provider_id = providerArg.getValue();
        g_node = nodeArg.getValue();
        g_log_level = logLevel.getValue();
        g_protocol = g_address.substr(0, g_address.find(":"));
    } catch(TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

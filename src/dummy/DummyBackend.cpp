/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "DummyBackend.hpp"
#include <iostream>
#include <ascent/ascent.hpp>
#include <mpi.h>

AMS_REGISTER_BACKEND(dummy, DummyNode);


/*static ascent::Ascent get_ascent_lib() {
	static ascent::Ascent a;
	return a;
}*/

void DummyNode::sayHello() {
    std::cout << "Hello World" << std::endl;
}

ams::RequestResult<bool> DummyNode::ams_open(std::string opts) {
    conduit::Node n;
    n.parse(opts,"conduit_json");
    /* Regardless of whether or not the client uses MPI, me the server needs to use MPI
     * because I am launched as a separate MPI program */
    n["mpi_comm"] = 0;
    n["messages"] = "verbose";
    std::cout << "Ascent Init with options: " << n.to_yaml() << std::endl;

    ascent_lib.open(n);

    conduit::Node x;
    ascent_lib.info(x);
    std::cout << "Before execute info is: " << x.to_yaml() << std::endl;

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_close() {
    std::cout << "Ascent Close!" << std::endl;

    ascent_lib.close();

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_publish(std::string bp_mesh) {
    conduit::Node n;
    std::cout << "Ascent Publish!" << std::endl;
    n.parse(bp_mesh,"conduit_json");

    conduit::Node x;
    ascent_lib.info(x);
    std::cout << "Before publish info is: " << x.to_yaml() << std::endl;

    ascent_lib.publish(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_execute(std::string actions) {
    conduit::Node n;
    std::cout << "Ascent Execute!" << std::endl;
    n.parse(actions,"conduit_json");

    conduit::Node x;
    ascent_lib.info(x);
    std::cout << "Before execute info is: " << x.to_yaml() << std::endl;

    ascent_lib.execute(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

void DummyNode::ams_open_publish_execute(std::string open_opts, std::string bp_mesh, std::string actions) {
    conduit::Node n, n_mesh, n_opts;
    std::cout << "Ascent Open, Publish, Execute, and Close!" << std::endl;

    double start = MPI_Wtime();

    ascent::Ascent a_lib;
    n.parse(actions,"conduit_json");
    n_mesh.parse(bp_mesh,"conduit_json");
    n_opts.parse(open_opts,"conduit_json");
    n_opts["mpi_comm"] = MPI_Comm_c2f(MPI_COMM_WORLD);

    /* Perform the ascent viz as a single, atomic operation within the context of the RPC */
    a_lib.open(n_opts);
    a_lib.publish(n_mesh);
    a_lib.execute(n);
    a_lib.close();

    double end = MPI_Wtime();

    std::cout << "Total server time for ascent call: " << end-start << std::endl;
}

ams::RequestResult<bool> DummyNode::ams_publish_and_execute(std::string bp_mesh, std::string actions) {
    conduit::Node n, n_mesh;
    std::cout << "Ascent Publish and Execute!" << std::endl;

    n.parse(actions,"conduit_json");
    n_mesh.parse(bp_mesh,"conduit_json");

    /* Publish and execute as a single atomic operation */
    ascent_lib.publish(n_mesh);
    ascent_lib.execute(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<int32_t> DummyNode::computeSum(int32_t x, int32_t y) {
    ams::RequestResult<int32_t> result;
    result.value() = x + y;
    return result;
}

ams::RequestResult<bool> DummyNode::destroy() {
    ams::RequestResult<bool> result;
    result.value() = true;
    // or result.success() = true
    return result;
}

std::unique_ptr<ams::Backend> DummyNode::create(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<ams::Backend>(new DummyNode(config));
}

std::unique_ptr<ams::Backend> DummyNode::open(const thallium::engine& engine, const json& config) {
    (void)engine;
    return std::unique_ptr<ams::Backend>(new DummyNode(config));
}

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
ascent::Ascent ascent_lib;

void DummyNode::sayHello() {
    std::cout << "Hello World" << std::endl;
}

ams::RequestResult<bool> DummyNode::ams_open(std::string opts) {
    conduit::Node n;
    n.parse(opts,"conduit_json");
    n["mpi_comm"] = 0; //Use MPI anyway!
    std::cout << "Ascent Init!" << std::endl;

    ascent_lib.open(n);

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

    ascent_lib.publish(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_execute(std::string actions) {
    conduit::Node n;
    std::cout << "Ascent Execute!" << std::endl;
    n.parse(actions,"conduit_json");

    ascent_lib.execute(n);

    ams::RequestResult<bool> result;
    result.value() = true;
    return result;
}

ams::RequestResult<bool> DummyNode::ams_publish_and_execute(std::string bp_mesh, std::string actions) {
    conduit::Node n, n_mesh;
    std::cout << "Ascent Publish and Execute!" << std::endl;
    n.parse(actions,"conduit_json");
    n_mesh.parse(bp_mesh,"conduit_json");
    std::cout << "Received: " << n.to_yaml() << std::endl;

    ascent_lib.publish(n_mesh);
    ascent_lib.execute(n);

    conduit::Node x;
    ascent_lib.close();
    ascent_lib.info(x);
    std::cout << "Info is: " << x.to_string() << std::endl;

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

/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include "ams/Exception.hpp"
#include "ams/Client.hpp"
#include "ams/NodeHandle.hpp"
#include "ams/RequestResult.hpp"

#include "ClientImpl.hpp"
#include "NodeHandleImpl.hpp"

#include <thallium/serialization/stl/string.hpp>

namespace tl = thallium;

namespace ams {

Client::Client() = default;

Client::Client(const tl::engine& engine)
: self(std::make_shared<ClientImpl>(engine)) {}

Client::Client(margo_instance_id mid)
: self(std::make_shared<ClientImpl>(mid)) {}

Client::Client(const std::shared_ptr<ClientImpl>& impl)
: self(impl) {}

Client::Client(Client&& other) = default;

Client& Client::operator=(Client&& other) = default;

Client::Client(const Client& other) = default;

Client& Client::operator=(const Client& other) = default;


Client::~Client() = default;

const tl::engine& Client::engine() const {
    return self->m_engine;
}

Client::operator bool() const {
    return static_cast<bool>(self);
}

NodeHandle Client::makeNodeHandle(
        const std::string& address,
        uint16_t provider_id,
        const UUID& node_id,
        bool check) const {
    auto endpoint  = self->m_engine.lookup(address);
    auto ph        = tl::provider_handle(endpoint, provider_id);
    RequestResult<bool> result;
    result.success() = true;
    if(check) {
        result = self->m_check_node.on(ph)(node_id);
    }
    if(result.success()) {
        auto node_impl = std::make_shared<NodeHandleImpl>(self, std::move(ph), node_id);
        return NodeHandle(node_impl);
    } else {
        throw Exception(result.error());
        return NodeHandle(nullptr);
    }
}

std::string Client::getConfig() const {
    return "{}";
}

}
